// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Preprocessor.h"

namespace honey
{

/// Meta-programming and compile-time util
/**
  * \defgroup Meta  Meta-programming
  */
/// @{

/// Meta-programming and compile-time util
namespace mt
{

/// Remove the unused parameter warning
#define mt_unused(Param)                                        (void)Param;

/// Returns the same type passed in. Can be used as a barrier to prevent type deduction.
template<class T> struct identity                               { typedef T type; };
/// Holds a constant integral value
template<class T, T val> struct Value                           { static const T value = val; };
/// Always returns true.  Can be used to force a clause to be type dependent.
template<class...> struct True                                  : Value<bool, true> {};
/// Variant of True for integers
template<int...> struct True_int                                : Value<bool, true> {};
/// Check if type is std::true_type
template<class T> struct IsTrue                                 : Value<bool, std::is_same<T, std::true_type>::value> {};
/// Special void type, use where void is intended but implicit members are required (default ctor, copyable, etc.)
struct Void {};
/// Use to differentiate an overloaded function by type. Accepts parameter default value: `func(tag<0> _ = 0)`
template<int> struct tag                                        { tag() {} tag(int) {} };

/// Add reference to type
template<class T> struct addRef                                 : std::add_lvalue_reference<T> {};
/// Remove reference from type
template<class T> struct removeRef                              : std::remove_reference<T> {};
/// Add pointer to type
template<class T> struct addPtr                                 : std::add_pointer<T> {};
/// Remove pointer from type
template<class T> struct removePtr                              : std::remove_pointer<T> {};
/// Add top-level const qualifier and reference to type
template<class T> struct addConstRef                            : addRef<typename std::add_const<T>::type> {};
/// Remove reference and top-level const qualifier from type
template<class T> struct removeConstRef                         : std::remove_const<typename removeRef<T>::type> {};

/// Check if type is an lvalue reference
template<class T> struct isLref                                 : Value<bool, std::is_lvalue_reference<T>::value> {};
/// Check if type is an rvalue reference
template<class T> struct isRref                                 : Value<bool, std::is_rvalue_reference<T>::value> {};
/// Check if type is a reference
template<class T> struct isRef                                  : Value<bool, std::is_reference<T>::value> {};
/// Check if type is a pointer
template<class T> struct isPtr                                  : Value<bool, std::is_pointer<T>::value> {};
/// Check if type is a tuple
/** \class isTuple */
template<class T> class isTuple;

/// Opposite of std::enable_if
template<bool b, class T = void> struct disable_if              : std::enable_if<!b, T> {};

/// Variant of std::conditional for integers, stores result in `value`
template<bool b, int64 t, int64 f> struct conditional_int       : Value<int64, f> {};

/// Version of std::is_base_of that removes reference qualifiers before testing
template<class Base, class Derived> struct is_base_of           : std::is_base_of<typename removeConstRef<Base>::type, typename removeConstRef<Derived>::type> {};

/// Check if functor is callable with arguments
/** \class isCallable */
template<class Func, class... Args> class isCallable;
    
/// Create a method to check if a class has a member with matching name and type
/**
  * `Result` stores the test result. `type` stores the member type if it exists, mt::Void otherwise.
  *
  *     struct A { int Foo;         };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, int A::*>           ->  { value = true,    type = int A::*         }
  *     struct A { void Foo(int);   };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, void (A::*)(int)>   ->  { value = true,    type = void (A::*)(int) }
  *     struct A { int Bar;         };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, int A::*>           ->  { value = false,   type = mt::Void         }
  *
  * mt_hasMember2() can be used to specify the test function name. \n
  * mt_hasMember2() must be used for operator checks because of the special characters in the operator name.
  */
#define mt_hasMember(MemberName)                                mt_priv_hasMember(MemberName, MemberName)
#define mt_hasMember2(MemberName, TestName)                     mt_priv_hasMember(MemberName, TestName)

/// Create a method to check if a class has a nested type/class
/**
  * `value` stores the test result. `type` stores the nested type if it exists, mt::Void otherwise.
  *
  *     struct A { typedef int Foo; };              =>  mt_hasType(Foo);            ->  mt_hasType_Foo<A>   ->  { value = true,     type = int          }
  *     struct A { template<class> struct Foo{}; }; =>  mt_hasType2(Foo<int>, Foo); ->  mt_hasType_Foo<A>   ->  { value = true,     type = A::Foo<int>  }
  *     struct A { typedef int Bar; };              =>  mt_hasType(Foo);            ->  mt_hasType_Foo<A>   ->  { value = false,    type = mt::Void     }
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
  * \retval param<N>::type  parameter types, from 0 to `arity`-1
  */
template<class T> struct funcTraits;

/// Create an object that can be retrieved safely from a static context
#define mt_staticObj(Class, Func, Ctor)                         static inline UNBRACKET(Class)& Func()  { static UNBRACKET(Class) _obj Ctor; return _obj; }

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

/// Get maximum of all arguments
/** \class max */
template<int64... vals> struct max;
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

template<int64 t, int64 f> struct conditional_int<true, t, f>   : Value<int64, t> {};

#define mt_priv_hasMember(MemberName, TestName)                                                                                 \
    template<class Class, class MemberType>                                                                                     \
    class mt_hasMember_##TestName                                                                                               \
    {                                                                                                                           \
        template<class T, T>                        struct matchType;                                                           \
        template<class T> static auto               memberMatch(void*) -> decltype(declval<matchType<MemberType, &T::MemberName>>(), std::true_type()); \
        template<class T> static std::false_type    memberMatch(...);                                                           \
    public:                                                                                                                     \
        static const bool value = mt::IsTrue<decltype(memberMatch<Class>(nullptr))>::value;                                     \
        typedef typename std::conditional<value, MemberType, mt::Void>::type type;                                              \
    };

#define mt_priv_hasType(TypeName, TestName)                                                                                     \
    template<class Class>                                                                                                       \
    class mt_hasType_##TestName                                                                                                 \
    {                                                                                                                           \
        template<class T> static auto               test(void*) -> decltype(declval<typename T::TypeName>(), std::true_type()); \
        template<class T> static std::false_type    test(...);                                                                  \
                                                                                                                                \
        template<bool Res, class Enable = void>                                                                                 \
        struct testType                                             { typedef mt::Void type; };                                 \
        template<bool Res>                                                                                                      \
        struct testType<Res, typename std::enable_if<Res>::type>    { typedef typename Class::TypeName type; };                 \
    public:                                                                                                                     \
        static const bool value = mt::IsTrue<decltype(test<Class>(nullptr))>::value;                                            \
        typedef typename testType<value>::type type;                                                                            \
    };
    
/** \endcond */

template<class T>
class isTuple
{
    template<class T_> static auto                              test(void*) -> decltype(declval<typename std::tuple_element<0,T_>::type>(), std::true_type());
    template<class T_> static std::false_type                   test(...);
public:
    static const bool value = IsTrue<decltype(test<T>(nullptr))>::value;
};

template<class Func, class... Args>
class isCallable
{
    template<class F> static auto                               test(void*) -> decltype(declval<F>()(declval<Args>()...), std::true_type());
    template<class F> static std::false_type                    test(...);
public:
    static const bool value = IsTrue<decltype(test<Func>(nullptr))>::value;
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
#define PARAM(It)       template<int _> struct param<It-1, _> { typedef T##It type; };

#define STRUCT(It, Ptr)                                                                     \
    template<class R ITERATE_(1,It,T_PARAM)>                                                \
    struct funcTraits<R Ptr ( ITERATE_(1,It,T_SPEC) )>                                      \
    {                                                                                       \
        typedef R Sig( ITERATE_(1,It,T_SPEC) );                                             \
        typedef void Base;                                                                  \
        typedef R Return;                                                                   \
        static const int arity = It;                                                        \
        template<int N, int _=0> struct param;                                              \
        ITERATE_(1,It,PARAM)                                                                \
    };                                                                                      \

#define M_PARAM(It)       template<int _> struct param<It, _> { typedef T##It type; };

#define M_STRUCT(It, Const)                                                                 \
    template<class R, class Base_ ITERATE_(1,It,T_PARAM)>                                   \
    struct funcTraits<R (Base_::*) ( ITERATE_(1,It,T_SPEC) ) Const>                         \
    {                                                                                       \
        typedef R (Base_::*Sig) ( ITERATE_(1,It,T_SPEC) );                                  \
        typedef Base_ Base;                                                                 \
        typedef R Return;                                                                   \
        static const int arity = It+1;                                                      \
        template<int N, int _=0> struct param;                                              \
        template<int _> struct param<0, _> { typedef Const Base* type; };                   \
        ITERATE_(1,It,M_PARAM)                                                              \
    };                                                                                      \
                                                                                            \
    namespace priv                                                                          \
    {                                                                                       \
    template<class R, class Base_ ITERATE_(1,It,T_PARAM)>                                   \
    struct functorTraits<R (Base_::*) ( ITERATE_(1,It,T_SPEC) ) Const>                      \
    {                                                                                       \
        typedef R Sig( ITERATE_(1,It,T_SPEC) );                                             \
        typedef void Base;                                                                  \
        typedef R Return;                                                                   \
        static const int arity = It;                                                        \
        template<int N, int _=0> struct param;                                              \
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
template<int64 val, int64... vals> struct max<val, vals...>     : Value<int64, (val > max<vals...>::value ? val : max<vals...>::value)> {};
template<int64 val> struct max<val>                             : Value<int64, val> {};
template<> struct log2Floor<0>                                  : Value<int, -1> {};
template<int64 a> struct gcd<a, 0>                              : Value<int64, abs<a>::value> {};
template<int64 b> struct gcd<0, b>                              : Value<int64, abs<b>::value> {};
/** \endcond */
/// @}

} }