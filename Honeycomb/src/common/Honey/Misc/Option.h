// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Debug.h"

namespace honey
{

class StringStream;

/// Null option type. See \ref optnull.
class optnull_t {};
/** \cond */
mt_staticObj(const optnull_t, _optnull,)
/** \endcond */
/// Null option, use to reset an option to an uninitialized state or test for initialization
#define optnull _optnull()

/** \cond */
namespace priv { struct optionTag {}; }
/** \endcond */

/// Enables any class to be optional so it can exist in an uninitialized null state.
/**
    Variables are commonly defined as pointers for the sole reason that pointers can exist in a null state.
    In this case, an option can be used instead of a pointer to make the behavior explicit and encourage stack allocation.
    Option syntax is also clearer for function calls:
 
        func(iterator* optIter = nullptr) {...} func(&iter)     ---->  func(option<iterator> optIter = optnull) {...}   func(iter)
        func(int* retVal = nullptr) {...}       func(&retInt)   ---->  func(option<int&> retVal = optnull) {...}        func(retInt)
 
    An option is large enough to hold an instance of its wrapped type `T`. The instance is not constructed until it is needed,
    so `T` does not have to be default constructible. On the first assignment the instance is copy/move constructed,
    assignments thereafter use copy/move assign.

    Assigning to `optnull` will reset the option to an uninitialized state (destructing any wrapped instance).

    \see optnull

    Example:
 
    \code
 
    func(option<char> o = optnull)
    {
        option<int&> rInt;
        int i, j;
        if (o == optnull) o = 'a';      //Set a default value if caller didn't specify a char
        rInt = i;                       //First assignment, binds the reference.
        rInt = 2;                       //Assignment to bound reference: i = 2
        rInt = &j;                      //Rebind to j -- pointers are convertible to optional references
        rInt = o;                       //j = the wrapped char in 'o'
        rInt = optnull;                 //Reset to null unbound reference
        rInt = nullptr;                 //Same as above
        int a = *o;                     //The * and -> operators can be used to retrieve the wrapped object
        a = o + 1;                      //Options implicitly convert to their wrapped object
    }
 
    \endcode
  */
template<class T>
class option : public priv::optionTag
{
    friend class option;
public:
    typedef T Wrapped;

    /// Uninitialized by default
    option()                                        : _val(nullptr) {}
    option(optnull_t)                               : _val(nullptr) {}
    option(const option& val)                       : _val(nullptr) { operator=(val); }
    template<class U> option(U&& val)               : _val(nullptr) { operator=(forward<U>(val)); }

    ~option()                                       { uninit(); }

    /// Reset the option to an uninitialized state
    option& operator=(optnull_t)                    { uninit(); return *this; }
    option& operator=(const option& rhs)            { return operator=<const option>(rhs); }

    /// Assign option
    template<class U>
    typename std::enable_if<mt::isBaseOf<priv::optionTag,U>::value, option&>::type
        operator=(U&& rhs)
    {
        // Assign rhs' object if available
        if (rhs._val) operator=(forward<typename mt::removeRef<U>::Type::Wrapped>(rhs.get()));
        else uninit();
        return *this;
    }

    /// Assign object
    template<class U>
    typename mt::disable_if<mt::isBaseOf<priv::optionTag,U>::value, option&>::type
        operator=(U&& rhs)
    {
        if (!_val)
        {
            //First assignment, copy construct
            construct<std::is_constructible<T,U>::value>::func(&_storage, forward<U>(rhs));
            _val = reinterpret_cast<T*>(&_storage);
        }
        else
            assign<std::is_assignable<T,U>::value>::func(get(), forward<U>(rhs));
        return *this;
    }

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
    friend StringStream& operator<<(StringStream& os, const option<T>& rhs)     { if (!rhs) return os << "optnull"; return os << *rhs; }

private:
    /// Use generic storage so wrapped object is not constructed until needed
    typedef typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type Storage;

    template<bool isConstructible> struct construct {};
    template<> struct construct<true>               { template<class U> static void func(void* storage, U&& rhs)    { new (storage) T(forward<U>(rhs)); } };
    template<> struct construct<false>              { template<class U> static void func(void*, U&&)                { error("Can't copy construct from type"); } };

    template<bool isAssignable> struct assign {};
    template<> struct assign<true>                  { template<class U> static void func(T& lhs, U&& rhs)   { lhs = forward<U>(rhs); } };
    template<> struct assign<false>                 { template<class U> static void func(T&, U&&)           { error("Can't assign to type"); } };

    /// Destructs wrapped object if initialized
    void uninit()                                   { if (!_val) return; get().~T(); _val = nullptr; }

    T* _val;
    Storage _storage;
};

/// Specialization for references.
/**
  * Option references behave like normal references: the first assignment binds an object, and assignments thereafter operate on the bound object.
  * Assigning to optnull or nullptr will result in a null unbound ref.
  *
  * \see option
  */
template<class T>
class option<T&> : public priv::optionTag
{
    friend class option;
public:
    typedef T& Wrapped;

    option()                                        : _val(nullptr) {}
    option(T* val)                                  { operator=(val); }
    option(optnull_t)                               { operator=(optnull); }
    template<class U> option(U&& val)               : _val(nullptr) { operator=(forward<U>(val)); }

    /// Assign to pointer of same type, binds object
    option& operator=(T* rhs)                       { _val = rhs; return *this; }
    /// Assign to null value, unbinds object
    option& operator=(optnull_t)                    { _val = nullptr; return *this; }
    option& operator=(const option& rhs)            { return operator=<const option>(rhs); }

    /// Assign option
    template<class U>
    typename std::enable_if<mt::isBaseOf<priv::optionTag,U>::value, option&>::type
        operator=(U&& rhs)
    {
        if (rhs._val) operator=(rhs.get());
        else _val = nullptr;
        return *this;
    }

    /// Assign object
    template<class U>
    typename mt::disable_if<mt::isBaseOf<priv::optionTag,U>::value, option&>::type
        operator=(U&& rhs)
    {
        if (!_val)
            //First assignment, bind object
            bind<std::is_convertible<typename mt::addPtr<U>::Type,T*>::value>::func(_val, &rhs);
        else
            assign<std::is_const<T>::value>::func(get(),rhs);
        return *this;
    }

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
    friend StringStream& operator<<(StringStream& os, const option<T&>& rhs)    { if (!rhs) return os << "optnull"; return os << *rhs; }

private:
    template<bool isConvertible> struct bind {};
    template<> struct bind<true>                    { template<class U> static void func(T*& lhs, U* rhs)       { lhs = rhs; } };
    template<> struct bind<false>                   { template<class U> static void func(T*&, U*)               { error("Can't bind reference, rhs type inconvertible"); } };

    template<bool isConst> struct assign {};
    template<> struct assign<true>                  { template<class U> static void func(T&, const U&)          { error("Can't assign const reference"); } };
    template<> struct assign<false>                 { template<class U> static void func(T& lhs, const U& rhs)  { lhs = rhs; } };

    T* _val;
};

}
