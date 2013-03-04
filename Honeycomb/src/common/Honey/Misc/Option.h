// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Debug.h"

namespace honey
{

class StringStream;

/// Null option type. See \ref optnull.
struct optnull_t {};
/** \cond */
mt_staticObj(const optnull_t, _optnull,{})
/** \endcond */
/// Null option, use to reset an option to an uninitialized state or test for initialization
#define optnull _optnull()

/// Enables any type to be optional so it can exist in an uninitialized null state.
/**
    An option is large enough to hold an instance of its wrapped type.
    On construction or first assignment the instance is copy/move constructed, assignments thereafter use copy/move assign.
    Assigning to `optnull` will reset the option to an uninitialized state, destructing any instance.
 
    Supports wrapped const/ref types. \see option<T&> for wrapped ref types.
    
    Variables are commonly defined as pointers for the sole reason that pointers can exist in a null state.
    In this case, an option can be used instead of a pointer to make the behavior explicit and encourage stack allocation.
    Option syntax is also clearer for function calls:
 
        func(iterator* optIter = nullptr);  func(&iter)     ---->  func(option<iterator> optIter = optnull);    func(iter)
        func(int* retVal = nullptr);        func(&retInt)   ---->  func(option<int&> retVal = optnull);         func(retInt)
 
    \see optnull

    Example:
 
    \code
 
    func(option<char> o = optnull)
    {
        option<int&> rInt;
        int i, j;
        if (o == optnull) o = 'a';      //Set a default value if caller didn't specify a char
        rInt.bind(i);                   //Bind the reference before use
        rInt = 2;                       //Assignment to bound reference: i = 2
        rInt.bind(&j);                  //Rebind to j from pointer
        rInt = o;                       //j = the wrapped char in 'o'
        rInt = optnull;                 //Reset to null unbound reference
        int a = *o;                     //The * and -> operators can be used to retrieve the wrapped object
        a = o + 1;                      //Options implicitly convert to their wrapped object
    }
 
    \endcode
  */
template<class T>
class option
{
    template<class T_> friend class option;
public:
    /// Uninitialized by default
    option()                                        : _val(nullptr) {}
    option(optnull_t)                               : _val(nullptr) {}
    option(const option& rhs)                       : _val(nullptr) { if (rhs._val) construct(rhs.get()); }
    option(option& rhs)                             : _val(nullptr) { if (rhs._val) construct(rhs.get()); }
    option(option&& rhs)                            : _val(nullptr) { if (rhs._val) construct(move(rhs.get())); }
    template<class U> option(const option<U>& rhs)  : _val(nullptr) { if (rhs._val) construct(rhs.get()); }
    template<class U> option(option<U>& rhs)        : _val(nullptr) { if (rhs._val) construct(rhs.get()); }
    template<class U> option(option<U>&& rhs)       : _val(nullptr) { if (rhs._val) construct(move(rhs.get())); }
    template<class U> option(U&& rhs)               : _val(nullptr) { construct(forward<U>(rhs)); }

    ~option()                                       { uninit(); }

    /// Reset the option to an uninitialized state
    option& operator=(optnull_t)                    { uninit(); return *this; }

    /// Assign wrapped object
    option& operator=(const option& rhs)                        { return operator=<T>(rhs); }
    option& operator=(option& rhs)                              { return operator=<T>(rhs); }
    option& operator=(option&& rhs)                             { return operator=<T>(move(rhs)); }
    template<class U> option& operator=(const option<U>& rhs)   { if (rhs._val) operator=(rhs.get()); else uninit(); return *this; }
    template<class U> option& operator=(option<U>& rhs)         { if (rhs._val) operator=(rhs.get()); else uninit(); return *this; }
    template<class U> option& operator=(option<U>&& rhs)        { if (rhs._val) operator=(move(rhs.get())); else uninit(); return *this; }
    
    /// Assign object.  On the first assignment the instance is copy/move constructed, assignments thereafter use copy/move assign.
    /**
      * For an option to be assignable to `rhs` it must also be constructible with `rhs`, this is necessary for allowing the option to be nullable.
      */
    template<class U> option& operator=(U&& rhs)    { if (!_val) construct(forward<U>(rhs)); else get() = forward<U>(rhs); return *this; }

    bool operator==(optnull_t) const                { return !_val; }
    bool operator!=(optnull_t) const                { return !operator==(optnull); }

    const T& operator*() const                      { return get(); }
    T& operator*()                                  { return get(); }
    const T* operator->() const                     { return &get(); }
    T* operator->()                                 { return &get(); }
    operator const T&() const                       { return get(); }
    operator T&()                                   { return get(); }

    /// Test whether option is initialized
    explicit operator bool() const                  { return _val; }
    explicit operator bool()                        { return _val; }

    /// Get wrapped object. Option must be initialized.
    const T& get() const                            { assert(_val, "Option not initialized"); return *_val; }
    T& get()                                        { assert(_val, "Option not initialized"); return *_val; }
    
    /// Get pointer to wrapped object. Returns null if not initialized.
    const T* ptr() const                            { return _val; }
    T* ptr()                                        { return _val; }

    /// To string
    friend StringStream& operator<<(StringStream& os, const option& rhs)    { if (!rhs) return os << "optnull"; return os << *rhs; }

private:
    /// Use generic storage so wrapped object is not constructed until needed
    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type Storage;

    template<class U>
    void construct(U&& rhs)                         { _val = reinterpret_cast<T*>(&_storage); new (&_storage) T(forward<U>(rhs)); }
    /// Destructs wrapped object if initialized
    void uninit()                                   { if (!_val) return; get().~T(); _val = nullptr; }

    T* _val;
    Storage _storage;
};

/// Specialization for references.
/**
  * The wrapped reference must be bound before it can be assigned, either construct with an object ref/pointer or call bind().
  * All assignments operate on the bound object.
  * Assigning to optnull or calling bind(nullptr) to will unbind the wrapped reference.
  *
  * \see option
  */
template<class T>
class option<T&>
{
    template<class T_> friend class option;
public:
    option()                                        : _val(nullptr) {}
    option(T* rhs)                                  : _val(nullptr) { bind(rhs); }
    option(optnull_t)                               : _val(nullptr) {}
    option(const option& rhs)                       : _val(nullptr) { bind(rhs._val); }
    option(option& rhs)                             : _val(nullptr) { bind(rhs._val); }
    option(option&& rhs)                            : _val(nullptr) { bind(rhs._val); }
    template<class U> option(const option<U>& rhs)  : _val(nullptr) { bind(rhs._val); }
    template<class U> option(option<U>& rhs)        : _val(nullptr) { bind(rhs._val); }
    template<class U> option(option<U>&& rhs)       : _val(nullptr) { bind(rhs._val); }
    template<class U> option(U&& rhs)               : _val(nullptr) { bind(forward<U>(rhs)); }

    /// Bind wrapped reference to object
    template<class U> void bind(U&& rhs)            { bind(&rhs); }
    /// Bind wrapped reference from object pointer.  Unbinds if pointer is null.
    template<class U> void bind(U* rhs)             { _val = rhs; }
    /// Unbinds object
    void bind(nullptr_t)                            { _val = nullptr; }
    
    /// Unbinds object
    option& operator=(optnull_t)                    { bind(nullptr); return *this; }

    /// Assign wrapped object
    option& operator=(const option& rhs)                        { return operator=<T&>(rhs); }
    option& operator=(option& rhs)                              { return operator=<T&>(rhs); }
    option& operator=(option&& rhs)                             { return operator=<T&>(move(rhs)); }
    template<class U> option& operator=(const option<U>& rhs)   { if (rhs._val) operator=(rhs.get()); return *this; }
    template<class U> option& operator=(option<U>& rhs)         { if (rhs._val) operator=(rhs.get()); return *this; }
    template<class U> option& operator=(option<U>&& rhs)        { if (rhs._val) operator=(move(rhs.get())); return *this; }
    
    /// Assign object.  Reference must be bound to an object before being assigned, this is asserted at runtime.
    template<class U> option& operator=(U&& rhs)    { get() = forward<U>(rhs); return *this; }
    
    bool operator==(optnull_t) const                { return !_val; }
    bool operator!=(optnull_t) const                { return !operator==(optnull); }

    const T& operator*() const                      { return get(); }
    T& operator*()                                  { return get(); }
    const T* operator->() const                     { return &get(); }
    T* operator->()                                 { return &get(); }
    operator const T&() const                       { return get(); }
    operator T&()                                   { return get(); }

    explicit operator bool() const                  { return _val; }
    explicit operator bool()                        { return _val; }

    const T& get() const                            { assert(_val, "Option not initialized"); return *_val; }
    T& get()                                        { assert(_val, "Option not initialized"); return *_val; }

    const T* ptr() const                            { return _val; }
    T* ptr()                                        { return _val; }

    /// To string
    friend StringStream& operator<<(StringStream& os, const option& rhs)    { if (!rhs) return os << "optnull"; return os << *rhs; }

private:
    T* _val;
};
    
}
