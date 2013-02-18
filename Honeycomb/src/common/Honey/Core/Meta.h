// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Preprocessor.h"

namespace honey
{

/// Meta-programming and compile-time util
/**
  * \defgroup Meta Meta-programming
  */
/// @{

/// Meta-programming and compile-time util
namespace mt
{

/** \cond */
namespace priv
{
    template<class T, int I> struct extentOfCount {};
    template<class T, size_t Cnt> struct rankOfCount;
}
/** \endcond */

/// Remove the unused parameter warning
#define mt_unused(Param)                                        (void)Param;

/// Returns the same type passed in. Can be used as a barrier to prevent type deduction.
template<class T> struct identity                               { typedef T Type; };
/// Holds a constant integral value
template<class T, T val> struct Value                           { static const T value = val; };
/// Always return true
template<class T> struct True                                   : Value<bool, true> {};
/// Check if type is std::true_type
template<class T> struct IsTrue                                 : Value<bool, std::is_same<T, std::true_type>::value> {};
/// Empty type
struct Void {};
/// Use to differentiate an overloaded function by type. Accepts parameter default value: `func(tag<0> _ = 0)`
template<int> struct tag                                        { tag() {} tag(int) {} };

/// Add reference to type
template<class T> struct addRef                                 { typedef typename std::add_lvalue_reference<T>::type Type; };
/// Remove reference from type
template<class T> struct removeRef                              { typedef typename std::remove_reference<T>::type Type; };
/// Add pointer to type
template<class T> struct addPtr                                 { typedef typename std::add_pointer<T>::type Type; };
/// Remove pointer from type
template<class T> struct removePtr                              { typedef typename std::remove_pointer<T>::type Type; };
/// Add top-level const qualifier and reference to type
template<class T> struct addConstRef                            { typedef typename addRef<typename std::add_const<T>::type>::Type Type; };
/// Remove reference and top-level const qualifier from type
template<class T> struct removeConstRef                         { typedef typename std::remove_const<typename removeRef<T>::Type>::type Type; };

/// Check if type is an lvalue reference
template<class T> struct isLref                                 : Value<bool, std::is_lvalue_reference<T>::value> {};
/// Check if type is an rvalue reference
template<class T> struct isRref                                 : Value<bool, std::is_rvalue_reference<T>::value> {};
/// Check if type is a reference
template<class T> struct isRef                                  : Value<bool, std::is_reference<T>::value> {};
/// Check if type is a pointer
template<class T> struct isPtr                                  : Value<bool, std::is_pointer<T>::value> {};
/// Check if type is an iterator (has iterator category or is pointer) or a reference to one
/** \class isIterator */
template<class T> struct isIterator;
/// Check if type is a range (has std::begin) or a reference to one
/** \class isRange */
template<class T> class isRange;
/// Check if type is a tuple
/** \class isTuple */
template<class T> class isTuple;

/// Call sizeof(extentOf<I>(a)) to get number of elements in I'th dimension of an array variable. Default I is 0.
template<class T> typename priv::extentOfCount<T,0>::Type           extentOf(T&);
template<int I, class T> typename priv::extentOfCount<T,I>::Type    extentOf(T&);

/// Call sizeof(rankOf(a)) to get number of dimensions of an array variable
template<class T> typename priv::rankOfCount<T,0>::Type         rankOf(T&);

/// Opposite of std::enable_if
template<bool b, class T = void> struct disable_if              : std::enable_if<!b, T> {};

/// Variant of std::conditional for integers, stores result in `value`
template<bool b, int64 t, int64 f> struct conditional_int       : Value<int64, f> {};

/// Check if one class is derived from another.  Const qualifiers do not affect result.
/** \class isBaseOf */
template<class Base, class Derived> class isBaseOf;

/// Create a method to check if a class has a member with matching name and type
/**
  * Result stores the test result. Type stores the member type if it exists, mt::Void otherwise.
  *
  *     struct A { int Foo;         };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, int A::*>           ->  { value = true,    Type = int A::*         }
  *     struct A { void Foo(int);   };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, void (A::*)(int)>   ->  { value = true,    Type = void (A::*)(int) }
  *     struct A { int Bar;         };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, int A::*>           ->  { value = false,   Type = mt::Void         }
  *
  * mt_hasMember2() can be used to specify the test function name. \n
  * mt_hasMember2() must be used for operator checks because of the special characters in the operator name.
  */
#define mt_hasMember(MemberName)                                mt_priv_hasMember(MemberName, MemberName)
#define mt_hasMember2(MemberName, TestName)                     mt_priv_hasMember(MemberName, TestName)

/// Create a method to check if a class has a nested type/class
/**
  * `value` stores the test result. `Type` stores the nested type if it exists, mt::Void otherwise.
  *
  *     struct A { typedef int Foo; };              =>  mt_hasType(Foo);            ->  mt_hasType_Foo<A>   ->  { value = true,     Type = int          }
  *     struct A { template<class> struct Foo{}; }; =>  mt_hasType2(Foo<int>, Foo); ->  mt_hasType_Foo<A>   ->  { value = true,     Type = A::Foo<int>  }
  *     struct A { typedef int Bar; };              =>  mt_hasType(Foo);            ->  mt_hasType_Foo<A>   ->  { value = false,    Type = mt::Void     }
  *
  * mt_hasType2() can be used to specify the test function name. \n
  * mt_hasType2() must be used if type has special characters (ie. <>, ::)
  */
#define mt_hasType(TypeName)                                    mt_priv_hasType(TypeName, TypeName)
#define mt_hasType2(TypeName, TestName)                         mt_priv_hasType(TypeName, TestName)

/// Get function type traits
/**
  * \class funcTraits
  *
  * Valid types for `T`:
  * - a function signature
  * - a function pointer / reference
  * - a member function
  * - a functor (function object) with an operator() that has a unique signature (non-templated and non-overloaded)
  * - a lambda function
  *
  * \retval Sig             function signature
  * \retval Base            base class if this is a non-static member function, `void` otherwise
  * \retval Return          return type
  * \retval arity           number of parameters, includes the hidden base pointer as the first param if there's a base class
  * \retval param<N>::Type  parameter types, from 0 to `arity`-1
  */
template<class T> struct funcTraits;

/// Create an object that can be retrieved safely from a static context
#define mt_staticObj(Class, Func, Ctor)                         static inline UNBRACKET(Class)& Func()  { static UNBRACKET(Class) _obj##Ctor; return _obj; }

/// Solves static init order, call in header with unique id
#define mt_staticInit(id)                                       mt::Void __init_##id(); static mt::Void __initVar_##id(__init_##id());
/// Solves static init order, call in source with matching header id
#define mt_staticInit_impl(id)                                  mt::Void __init_##id() { return mt::Void(); }

/// Inherit to declare that class is not copyable
struct NoCopy
{
protected:
    NoCopy() {}
    ~NoCopy() {}
private:
    NoCopy(const NoCopy&);
    NoCopy& operator=(const NoCopy&);
};

/// Get the absolute value of a number
template<int64 val> struct abs                                  : Value<int64, (val < 0) ? -val : val> {};
/// Get the sign of a number
template<int64 val> struct sign                                 : Value<int64, (val < 0) ? -1 : 1> {};
/// Calc the floor of the base 2 log of x
template<int64 x> struct log2Floor                              : Value<int, log2Floor<x/2>::value+1> {};
/// Calc the greatest common divisor of a and b
template<int64 a, int64 b> struct gcd                           : gcd<b, a % b> {};

//====================================================
// Private
//====================================================
/** \cond */

namespace priv
{
    template<class T, size_t N, int I> struct extentOfCount<T[N], I>        { typedef typename extentOfCount<T, I-1>::Type Type; };
    template<class T, size_t N> struct extentOfCount<T[N], 0>               { typedef char (&Type)[N]; };

    template<class T, size_t Cnt> struct rankOfCount                        { typedef char (&Type)[Cnt]; };
    template <class T, size_t N, size_t Cnt> struct rankOfCount<T[N], Cnt>  { typedef typename rankOfCount<T,Cnt+1>::Type Type; };
}

template<int64 t, int64 f> struct conditional_int<true, t, f>   : Value<int64, t> {};

#define mt_priv_hasMember(MemberName, TestName)                                                                                 \
    template<class Class, class MemberType>                                                                                     \
    class mt_hasMember_##TestName                                                                                               \
    {                                                                                                                           \
        template<class T, T>                                    struct matchType;                                               \
        template<class T> static std::true_type                 memberMatch(matchType<MemberType, &T::MemberName>*);            \
        template<class T> static std::false_type                memberMatch(...);                                               \
    public:                                                                                                                     \
        static const bool value = mt::IsTrue<decltype(memberMatch<Class>(nullptr))>::value;                                     \
        typedef typename std::conditional<value, MemberType, mt::Void>::Type Type;                                              \
    };

#define mt_priv_hasType(TypeName, TestName)                                                                     \
    template<class Class>                                                                                       \
    class mt_hasType_##TestName                                                                                 \
    {                                                                                                           \
        template<class T> static std::true_type                 testFunc(typename T::TypeName*);                \
        template<class T> static std::false_type                testFunc(...);                                  \
                                                                                                                \
        template<bool Res, class Enable = void>                                                                 \
        struct type                                             { typedef mt::Void Type; };                     \
        template<bool Res>                                                                                      \
        struct type<Res, typename std::enable_if<Res>::type>    { typedef typename Class::TypeName Type; };     \
    public:                                                                                                     \
        static const bool value = mt::IsTrue<decltype(testFunc<Class>(nullptr))>::value;                        \
        typedef typename type<value>::Type Type;                                                                \
    };

namespace priv
{
    mt_hasType(iterator_category)
    template<class T> struct isIterator                         : Value<bool, isPtr<T>::value || mt_hasType_iterator_category<T>::value> {};
}

/** \endcond */

template<class T> struct isIterator                             : priv::isIterator<typename removeRef<T>::Type> {};

template<class T>
class isRange
{
    template<class _> static std::true_type                     testFunc(decltype(begin(T()))*);
    template<class _> static std::false_type                    testFunc(...);
public:
    static const bool value = IsTrue<decltype(testFunc<T>(nullptr))>::value;
};

template<class T>
class isTuple
{
    template<class T> static std::true_type                     testFunc(typename std::tuple_element<0,T>::type*);
    template<class T> static std::false_type                    testFunc(...);
public:
    static const bool value = IsTrue<decltype(testFunc<T>(nullptr))>::value;
};

template<class Base, class Derived>
class isBaseOf
{
    static std::true_type           testFunc(typename addPtr<typename std::remove_const<Base>::type>::Type);
    static std::false_type          testFunc(...);
public:
    static const bool value = IsTrue<decltype(testFunc(static_cast<typename addPtr<typename std::remove_const<Derived>::type>::Type>(nullptr)))>::value;
};

//====================================================
// funcTraits
//====================================================
/** \cond */
namespace priv
{
    template<class T> struct functorTraits {};
}
/** \endcond */
template<class T> struct funcTraits                             : priv::functorTraits<decltype(&T::operator())> {};
/** \cond */
#define FUNCTRAITS_ARG_MAX 5

#define T_PARAM(It)     , class T##It
#define T_SPEC(It)      COMMA_IFNOT(It,1) T##It
#define PARAM(It)       template<> struct param<It-1> { typedef T##It Type; };

#define STRUCT(It, Ptr)                                                                     \
    template<class R ITERATE_(1,It,T_PARAM)>                                                \
    struct funcTraits<R Ptr ( ITERATE_(1,It,T_SPEC) )>                                      \
    {                                                                                       \
        typedef R Sig( ITERATE_(1,It,T_SPEC) );                                             \
        typedef void Base;                                                                  \
        typedef R Return;                                                                   \
        static const int arity = It;                                                        \
        template<int N> struct param;                                                       \
        ITERATE_(1,It,PARAM)                                                                \
    };                                                                                      \

#define M_PARAM(It)       template<> struct param<It> { typedef T##It Type; };

#define M_STRUCT(It, Const)                                                                 \
    template<class R, class Base ITERATE_(1,It,T_PARAM)>                                    \
    struct funcTraits<R (Base::*) ( ITERATE_(1,It,T_SPEC) ) Const>                          \
    {                                                                                       \
        typedef R (Base::*Sig) ( ITERATE_(1,It,T_SPEC) );                                   \
        typedef Base Base;                                                                  \
        typedef R Return;                                                                   \
        static const int arity = It+1;                                                      \
        template<int N> struct param;                                                       \
        template<> struct param<0> { typedef Const Base* Type; };                           \
        ITERATE_(1,It,M_PARAM)                                                              \
    };                                                                                      \
                                                                                            \
    namespace priv                                                                          \
    {                                                                                       \
    template<class R, class Base ITERATE_(1,It,T_PARAM)>                                    \
    struct functorTraits<R (Base::*) ( ITERATE_(1,It,T_SPEC) ) Const>                       \
    {                                                                                       \
        typedef R Sig( ITERATE_(1,It,T_SPEC) );                                             \
        typedef void Base;                                                                  \
        typedef R Return;                                                                   \
        static const int arity = It;                                                        \
        template<int N> struct param;                                                       \
        ITERATE_(1,It,PARAM)                                                                \
    };                                                                                      \
    }                                                                                       \

ITERATE1(0, FUNCTRAITS_ARG_MAX, STRUCT,     )
ITERATE1(0, FUNCTRAITS_ARG_MAX, STRUCT, (&) )
ITERATE1(0, FUNCTRAITS_ARG_MAX, STRUCT, (*) )
ITERATE1(0, FUNCTRAITS_ARG_MAX, M_STRUCT,       )
ITERATE1(0, FUNCTRAITS_ARG_MAX, M_STRUCT, const )
#undef T_PARAM
#undef T_SPEC
#undef PARAM
#undef STRUCT
#undef M_PARAM
#undef M_STRUCT
/** \endcond */
//====================================================
/** \cond */
template<> struct log2Floor<0>                                  : Value<int, -1> {};
template<int64 a> struct gcd<a, 0>                              : Value<int64, abs<a>::value> {};
template<int64 b> struct gcd<0, b>                              : Value<int64, abs<b>::value> {};
/** \endcond */
/// @}

} }