// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Meta.h"

namespace honey
{

/// Pointer to a unique, non-shared, object.  Finalizer (deletes object by default) is run upon destruction if pointer is not null.
template<class T_, class Fin = finalize<T_>>
class UniquePtr : mt::NoCopy
{
    template<class, class> friend class UniquePtr;
    template<class T> struct PtrType                                { typedef T type; };
    template<class T> struct PtrType<T[]>                           { typedef T type; };
    typedef typename PtrType<T_>::type T;
    
public:
    UniquePtr()                                                     : _ptr(nullptr) {}
    template<class Fin_ = Fin>
    UniquePtr(T* ptr, Fin_&& f = Fin_())                            : _ptr(ptr), _fin(forward<Fin_>(f)) {}
    /// Moves pointer and finalizer out of rhs.  To set a new finalizer into rhs use move assign: rhs = UniquePtr(p,f);
    UniquePtr(UniquePtr&& rhs)                                      : _ptr(rhs.release()), _fin(forward<Fin>(rhs._fin)) {}
    template<class U, class F> UniquePtr(UniquePtr<U,F>&& rhs)      : _ptr(rhs.release()), _fin(forward<F>(rhs._fin)) {}

    ~UniquePtr()                                                    { if (_ptr) _fin(_ptr); }

    /// Set ptr
    UniquePtr& operator=(T* rhs)                                    { set(rhs); return *this; }

    /// Moves pointer and finalizer out of rhs
    UniquePtr& operator=(UniquePtr&& rhs)                           { return operator=<T,Fin>(move(rhs)); }
    template<class U, class F>
    UniquePtr& operator=(UniquePtr<U,F>&& rhs)                      { set(rhs.release()); _fin = forward<F>(rhs._fin); return *this; }

    T* operator->() const                                           { return _ptr; }
    T& operator*() const                                            { return *_ptr; }
    operator T*() const                                             { return _ptr; }

    /// Get the raw pointer to the object
    T* get() const                                                  { return _ptr; }

    /// Get the finalizer
    Fin& finalizer()                                                { return _fin; }
    const Fin& finalizer() const                                    { return _fin; }

    /// Give up ownership of pointer without finalizing and set to null
    T* release()                                                    { T* tmp = _ptr; _ptr = nullptr; return tmp; }

    /// Finalize old pointer and assign new.  Does not finalize if old pointer is the same or null.
    void set(T* p)
    {
        if (_ptr != p)
        {
            T* old = _ptr;
            _ptr = p;
            if (old) _fin(old);
        }
    }

private:
    T* _ptr;
    Fin _fin;
};

/// Helper to create a unique a ptr using type deduction
/** \relates UniquePtr */
template<class T, class Fin = finalize<T>>
UniquePtr<T,Fin> UniquePtrCreate(T* ptr, Fin&& f = Fin())           { return UniquePtr<T,Fin>(ptr, forward<Fin>(f)); }

}

/** \cond */
namespace std
{
    /// Allow class to be used as key in unordered containers
    template<class T, class Fin>
    struct hash<honey::UniquePtr<T,Fin>>
    {
        size_t operator()(const honey::UniquePtr<T,Fin>& val) const     { return reinterpret_cast<size_t>(val.get()); };
    };
}
/** \endcond */