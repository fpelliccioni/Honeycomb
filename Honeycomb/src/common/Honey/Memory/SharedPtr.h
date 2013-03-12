// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Meta.h"
#include "Honey/Thread/Atomic.h"
#include "Honey/Misc/Debug.h"

namespace honey
{

/** \cond */
namespace priv
{
    /// Control block base for shared pointer.  Holds strong/weak reference counts.
    class SharedControlBase
    {
    public:
        SharedControlBase()                             : _count(0), _weakCount(1) {}

        /// Increase reference count by 1
        void ref()                                      { ++_count; }
        /// Increase reference count by 1 if count is not 0 (ie. if object is alive then lock it).  Returns true on success.
        bool refLock()                                  { int old; do { old = _count; if (old <= 0) return false; } while (!_count.cas(old+1,old)); return true; }
        /// Decrease reference count by 1. Finalizes when count is 0.
        void unref()                                    { if (--_count > 0) return; finalize(); }
        /// Get reference count
        int count() const                               { return _count; }
    
        /// Increase weak reference count by 1
        void refWeak()                                  { ++_weakCount; }
        /// Decrease weak reference count by 1.  Destroys when count is 0.
        void unrefWeak()                                { if (--_weakCount > 0) return; destroy(); }
        /// Get weak reference count
        int weakCount() const                           { return _weakCount; }

    protected:
        /// Called to finalize the object when strong reference count reaches 0.  Call unrefWeak() in the finalizer to destroy this control block.
        virtual void finalize() = 0;
        /// Called to destroy this control block when weak reference count reaches 0
        virtual void destroy() = 0;
    private:
        atomic::Var<int> _count;
        atomic::Var<int> _weakCount;
    };

    /// Control block for shared pointer.  Holds pointer and calls finalizer.  Alloc is used to deallocate the control block.
    template<class T, class Fin, class Alloc_>
    class SharedControl : public SharedControlBase
    {
    public:
        typedef typename Alloc_::template rebind<SharedControl>::other Alloc;

        SharedControl(T* ptr, Fin&& f, const Alloc& a)  : _ptr(ptr), _fin(forward<Fin>(f)), _alloc(a) {}
    protected:
        virtual void finalize()                         { _fin(_ptr); unrefWeak(); }
        virtual void destroy()                          { delete_(this, _alloc); }
    private:
        T* _ptr;
        Fin _fin;
        Alloc _alloc;
    };

    /// Control pointer storage for non-intrusive pointers
    template<bool isIntrusive>
    struct SharedControlStorage
    {
        SharedControlStorage()                          : __control(nullptr) {}
        SharedControlBase* _control() const             { return __control; }
        void _control(SharedControlBase* ptr)           { __control = ptr; }
        SharedControlBase* __control;
    };
    /// Control pointer storage for intrusive pointers
    template<>
    struct SharedControlStorage<true>
    {
        void* _control() const                          { return nullptr; }
        void _control(void*) {}
    };
}
/** \endcond */

//====================================================
// SharedObj
//====================================================

/// Reference-counted object for intrusive shared pointers.
/**
  * When possible, objects pointed to by SharedPtr should inherit from SharedObj, this is an intrusive pointer. \n
  * Intrusive pointers are safer than non-intrusive pointers as the shared ptr can be cast to a raw pointer and back again without issue. \n
  * The same operation with a non-intrusive pointer would result in each shared ptr holding a separate reference count, which will cause a crash.
  *
  * A shared object will be destroyed when the last strong reference (SharedPtr) is released,
  * but it will not be deallocated until the last weak reference (WeakPtr) is released.
  *
  * A shared object does not have to be assigned to a shared ptr to be destroyed properly, it has a normal life cycle.
  */
class SharedObj : mt::NoCopy
{
    template<class> friend class SharedPtr;
    template<class> friend class WeakPtr;

public:
    /// Construct with allocator that is called to deallocate this shared object when all references have been released.
    template<class Alloc = std::allocator<SharedObj>, class Alloc_ = typename mt::removeRef<Alloc>::type::template rebind<SharedObj>::other>
    SharedObj(Alloc&& a = Alloc())                      : _control(new (_controlMem) Control(this, Alloc_(forward<Alloc>(a)))) {}
    
    virtual ~SharedObj() {}

protected:
    /// Destroys object. Called when strong reference count reaches 0.  May be overridden to prevent destruction.
    virtual void finalize()                             { Control* control = _control; this->~SharedObj(); control->unrefWeak(); }

private:
    /**
      * This object must be destroyed separately from its control to implement weak references.
      * Although this object gets destroyed, its memory is held until its control has also been destroyed.
      */
    struct Control : priv::SharedControlBase
    {
        template<class Alloc>
        Control(SharedObj* obj, Alloc&& a)              : _obj(obj), _dealloc([=](SharedObj* p) mutable { a.deallocate(p,1); }) {}

        virtual void finalize()                         { _obj->finalize(); }
        
        virtual void destroy()
        {
            SharedObj* p = _obj;
            auto dealloc = move(_dealloc);
            this->~Control();
            dealloc(p);
        }

        SharedObj* _obj;
        function<void (SharedObj*)> _dealloc;
    };

    uint8 _controlMem[sizeof(Control)];
    Control* _control;
};



//====================================================
// SharedPtr
//====================================================

template<class T, class Fin> class UniquePtr;
template<class T> class WeakPtr;

/// Combined intrusive/non-intrusive smart pointer.  Can reference and share any object automatically.
/**
  * Non-intrusive pointers use the allocator and finalizer supplied as arguments. \n
  * Intrusive pointers don't use an allocator and finalize with SharedObj::finalize().
  */
template<class T>
class SharedPtr : private priv::SharedControlStorage<mt::is_base_of<SharedObj, T>::value>
{
    friend class SharedPtr;
    template<class T_> friend class WeakPtr;
    template<class T_, class U> friend SharedPtr<T_> static_pointer_cast(const SharedPtr<U>&);
    template<class T_, class U> friend SharedPtr<T_> dynamic_pointer_cast(const SharedPtr<U>&);
    template<class T_, class U> friend SharedPtr<T_> const_pointer_cast(const SharedPtr<U>&);
    
    static const bool isIntrusive                                   = mt::is_base_of<SharedObj, T>::value;
    
public:
    SharedPtr()                                                     : _ptr(nullptr) {}
    SharedPtr(nullptr_t)                                            : _ptr(nullptr) {}
    /// Reference an object. Non-intrusive pointers use default finalizer for object (deletes object), and default allocator for internal control block.
    template<class U, typename std::enable_if<mt::True<U>::value && isIntrusive, int>::type=0>
    SharedPtr(U* ptr)                                               : _ptr(nullptr) { set(ptr); }
    /// Construct with finalizer/allocator. For non-intrusive pointers only.
    /**
      * Finalizer is run when reference count reaches 0.  Allocator is used for the internal control block. 
      */
    template<class U, class Fin = finalize<U>, class Alloc = std::allocator<U>, typename std::enable_if<mt::True<U>::value && !isIntrusive, int>::type=0>
    SharedPtr(U* ptr, Fin&& f = Fin(), Alloc&& a = Alloc())         : _ptr(nullptr) { set(ptr, forward<Fin>(f), forward<Alloc>(a)); }
    /// Reference the object pointed to by another shared pointer
    SharedPtr(const SharedPtr& ptr)                                 : _ptr(nullptr) { set(ptr); }
    template<class U> SharedPtr(const SharedPtr<U>& ptr)            : _ptr(nullptr) { set(ptr); }
    /// Transfer ownership out of shared pointer, leaving it null
    SharedPtr(SharedPtr&& ptr)                                      { set(move(ptr)); }
    template<class U> SharedPtr(SharedPtr<U>&& ptr)                 { set(move(ptr)); }
    /// Lock a weak pointer to get access to its object.  Shared ptr will be null if the object has already been destroyed.
    template<class U> SharedPtr(const WeakPtr<U>& ptr)              : _ptr(ptr._ptr) { this->_control(ptr._control()); if (_ptr && !getControl<>().refLock()) _ptr = nullptr; }
    /// Transfer ownership out of unique pointer, leaving it null
    template<class U, class Fin> SharedPtr(UniquePtr<U,Fin>&& ptr)  : _ptr(nullptr) { operator=(move(ptr)); }

    ~SharedPtr()                                                    { set(nullptr); }

    /// Set the object referenced by this shared pointer. Non-intrusive pointers use default finalizer/allocator.
    template<class U>
    SharedPtr& operator=(U* rhs)                                    { set(rhs); return *this; }
    SharedPtr& operator=(nullptr_t)                                 { set(nullptr); return *this; }

    SharedPtr& operator=(const SharedPtr& rhs)                      { set(rhs); return *this; }
    template<class U>
    SharedPtr& operator=(const SharedPtr<U>& rhs)                   { set(rhs); return *this; }

    SharedPtr& operator=(SharedPtr&& rhs)                           { set(move(rhs)); return *this; }
    template<class U>
    SharedPtr& operator=(SharedPtr<U>&& rhs)                        { set(move(rhs)); return *this; }

    template<class U, class Fin> 
    SharedPtr& operator=(UniquePtr<U,Fin>&& rhs)                    { set(rhs.release(), rhs.getFinalizer()); return *this; }

    template<class U>
    bool operator==(const SharedPtr<U>& rhs) const                  { return get() == rhs.get(); }
    template<class U>
    bool operator!=(const SharedPtr<U>& rhs) const                  { return get() != rhs.get(); }
    template<class U>
    bool operator<=(const SharedPtr<U>& rhs) const                  { return get() <= rhs.get(); }
    template<class U>
    bool operator>=(const SharedPtr<U>& rhs) const                  { return get() >= rhs.get(); }
    template<class U>
    bool operator< (const SharedPtr<U>& rhs) const                  { return get() < rhs.get(); }
    template<class U>
    bool operator> (const SharedPtr<U>& rhs) const                  { return get() > rhs.get(); }

    bool operator==(nullptr_t) const                                { return !get(); }
    bool operator!=(nullptr_t) const                                { return get(); }
    friend bool operator==(nullptr_t, const SharedPtr& rhs)         { return !rhs.get(); }
    friend bool operator!=(nullptr_t, const SharedPtr& rhs)         { return rhs.get(); }

    T* operator->() const                                           { assert(get()); return get(); }
    T& operator*() const                                            { assert(get()); return *get(); }
    operator T*() const                                             { return get(); }

    /// Get the raw pointer to the object
    T* get() const                                                  { return _ptr; }

    /// Dereference the current object and reference a new object. Non-intrusive pointers use default finalizer/allocator.
    template<class U, typename std::enable_if<mt::True<U>::value && isIntrusive, int>::type=0>
    void set(U* ptr)                                                { setControl(ptr, nullptr); }
    /// Set with finalizer/allocator, for non-intrusive pointers only
    template<class U, class Fin = finalize<U>, class Alloc = std::allocator<U>, typename std::enable_if<mt::True<U>::value && !isIntrusive, int>::type=0>
    void set(U* ptr, Fin&& f = Fin(), Alloc&& a = Alloc())
    {
        typedef priv::SharedControl<U,Fin,Alloc> Control;
        typedef typename Alloc::template rebind<Control>::other Alloc_;
        Alloc_ a_ = forward<Alloc>(a);
        setControl(ptr, ptr ? new (a_.allocate(1)) Control(ptr,forward<Fin>(f),a_) : nullptr);
    }
    void set(nullptr_t)                                             { set((T*)nullptr); }

    /// Get number of shared references to the object
    int refCount() const                                            { if (_ptr) return getControl<>().count(); return 0; }
    /// Check whether this is the only shared reference to the object
    bool unique() const                                             { return refCount() == 1; }

private:
    typedef priv::SharedControlBase                                 Control;
    typedef priv::SharedControlStorage<isIntrusive>                 Storage;

    template<class U> void set(const SharedPtr<U>& rhs)             { setControl(rhs._ptr, rhs._control()); }
    template<class U>
    void set(SharedPtr<U>&& rhs)
    {
        _ptr = rhs._ptr; this->_control(rhs._control());
        rhs._ptr = nullptr; rhs._control(nullptr);
    }
    
    template<class U>
    typename std::enable_if<mt::True<U>::value && isIntrusive>::type
        setControl(U* ptr, void*)
    {
        if (ptr) ptr->SharedObj::_control->ref();
        T* oldPtr = _ptr;
        _ptr = ptr;
        if (oldPtr) oldPtr->SharedObj::_control->unref();
    }
    template<class U>
    typename mt::disable_if<mt::True<U>::value && isIntrusive>::type
        setControl(U* ptr, Control* control)
    {
        if (ptr) control->ref();
        T* oldPtr = _ptr; Control* oldControl = Storage::_control();
        _ptr = ptr; Storage::_control(control);
        if (oldPtr) oldControl->unref();
    }

    template<class _ = void>
    typename std::enable_if<mt::True<_>::value && isIntrusive, Control&>::type
        getControl() const                                          { assert(_ptr && _ptr->SharedObj::_control); return *_ptr->SharedObj::_control; }
    template<class _ = void>
    typename mt::disable_if<mt::True<_>::value && isIntrusive, Control&>::type
        getControl() const                                          { assert(Storage::_control()); return *Storage::_control(); }

    T* _ptr;
};

/// \name Shared pointer casts
/// @{

/// \relates SharedPtr
template<class T, class U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U>& rhs)           { SharedPtr<T> ret; ret.setControl(static_cast<T*>(rhs._ptr), rhs._control()); return ret; }
/// \relates SharedPtr
template<class T, class U>
SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U>& rhs)          { SharedPtr<T> ret; ret.setControl(dynamic_cast<T*>(rhs._ptr), rhs._control()); return ret; }
/// \relates SharedPtr
template<class T, class U>
SharedPtr<T> const_pointer_cast(const SharedPtr<U>& rhs)            { SharedPtr<T> ret; ret.setControl(const_cast<T*>(rhs._ptr), rhs._control()); return ret; }
/// @}

//====================================================
// WeakPtr
//====================================================

/// Point to a shared object without holding a reference.  The object is accessible through a lock, which prevents unexpected destruction.
/**
  * Weak pointers can be used to break cyclic references. \n
  * If an outside user releases a reference to one object that internally is also referenced by its members,
  * then the object will not be destroyed as the user expects.
  * This problem can be solved by replacing internal shared pointers with weak pointers until the cycle is broken.
  */
template<class T>
class WeakPtr : private priv::SharedControlStorage<mt::is_base_of<SharedObj, T>::value>
{
    friend class WeakPtr;
    template<class> friend class SharedPtr;
public:
    WeakPtr()                                                       : _ptr(nullptr) {}
    WeakPtr(nullptr_t)                                              : _ptr(nullptr) {}
    /// Must construct from a shared pointer
    template<class U>
    WeakPtr(const SharedPtr<U>& rhs)                                : _ptr(nullptr) { operator=(rhs); }
    WeakPtr(const WeakPtr& rhs)                                     : _ptr(nullptr) { operator=(rhs); }
    template<class U>
    WeakPtr(const WeakPtr<U>& rhs)                                  : _ptr(nullptr) { operator=(rhs); }

    ~WeakPtr()                                                      { if (_ptr) getControl<>().unrefWeak(); }

    WeakPtr& operator=(const WeakPtr& rhs)                          { setControl(rhs._ptr, rhs._control()); return *this; }
    template<class U>
    WeakPtr& operator=(const WeakPtr<U>& rhs)                       { setControl(rhs._ptr, rhs._control()); return *this; }
    template<class U>
    WeakPtr& operator=(const SharedPtr<U>& rhs)                     { setControl(rhs._ptr, rhs._control()); return *this; }
    WeakPtr& operator=(nullptr_t)                                   { set(nullptr); return *this; }

    /// Set to null, release reference
    void set(nullptr_t)                                             { setControl(static_cast<T*>(nullptr), nullptr); }

    /// Acquire access to object.  Shared ptr prevents object from being destroyed while in use.  Returns null if object has been destroyed.
    SharedPtr<T> lock() const                                       { return *this; }

    /// Get strong reference (SharedPtr) count 
    int refCount() const                                            { if (_ptr) return getControl<>().count(); return 0; }
    /// Check whether the object has already been destroyed
    bool expired() const                                            { return refCount() == 0; }

private:
    static const bool isIntrusive                                   = SharedPtr<T>::isIntrusive;
    typedef typename SharedPtr<T>::Control                          Control;
    typedef typename SharedPtr<T>::Storage                          Storage;

    template<class U>
    typename std::enable_if<mt::True<U>::value && isIntrusive>::type
        setControl(U* ptr, void*)
    {
        if (ptr) ptr->SharedObj::_control->refWeak();
        T* oldPtr = _ptr;
        _ptr = ptr;
        if (oldPtr) oldPtr->SharedObj::_control->unrefWeak();
    }
    template<class U>
    typename mt::disable_if<mt::True<U>::value && isIntrusive>::type
        setControl(U* ptr, Control* control)
    {
        if (ptr) control->refWeak();
        T* oldPtr = _ptr; Control* oldControl = Storage::_control();
        _ptr = ptr; Storage::_control(control);
        if (oldPtr) oldControl->unrefWeak();     
    }

    template<class _ = void>
    typename std::enable_if<mt::True<_>::value && isIntrusive, Control&>::type
        getControl() const                                          { assert(_ptr && _ptr->SharedObj::_control); return *_ptr->SharedObj::_control; }
    template<class _ = void>
    typename mt::disable_if<mt::True<_>::value && isIntrusive, Control&>::type
        getControl() const                                          { assert(Storage::_control()); return *Storage::_control(); }

    T* _ptr;
};

}

/** \cond */
namespace std
{
    /// Allow class to be used as key in unordered containers
    template<class T>
    struct hash<honey::SharedPtr<T>>
    {
        size_t operator()(const honey::SharedPtr<T>& val) const     { return reinterpret_cast<size_t>(val.get()); };
    };
}
/** \endcond */
