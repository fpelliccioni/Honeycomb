// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Numeral.h"
#include "Honey/Misc/Enum.h"
#include "Honey/Misc/Range.h"

namespace honey
{

/// Methods that extend the functionality of the standard library.
/**
  * \defgroup StdUtil   Standard Util
  */
/// @{

/// Safely get the size of a std container as a signed integer. The size() member method returns unsigned which results in a conversion warning.
template<class StdContainer>
int size(const StdContainer& cont)                              { return utos(cont.size()); }

/** \cond */
namespace priv
{
    template<class T, int I>
    struct tupleToString
    {
        static void func(StringStream& os, const T& val)
        {
            os << ", " << get<std::tuple_size<T>::value - I + 1>(val);
            tupleToString<T, I-1>::func(os, val);
        }
    };

    template<class T>
    struct tupleToString<T, 1>
    {
        static void func(StringStream&, const T&) {}
    };
};
/** \endcond */

/// Tuple to string
template<class Tuple>
typename std::enable_if<mt::isTuple<Tuple>::value, StringStream&>::type
    operator<<(StringStream& os, const Tuple& val)
{
    os << "(" << get<0>(val);
    priv::tupleToString<Tuple, std::tuple_size<Tuple>::value>::func(os, val);
    return os << ")";
}

/// See \ref StdUtil
namespace stdutil
{
    /// Create a range over the keys of a map or map iterator range. \see values()
    template<class Range>
    auto keys(Range&& range) ->
        // Doxygen can't parse this
        /** \cond */
        Range_<TupleIter<itertype(range),0>, TupleIter<itertype(range),0>>
        /** \endcond */
    {
        return honey::range(TupleIter<itertype(range),0>(begin(range)), TupleIter<itertype(range),0>(end(range)));
    }

    /// Create a range over the values of a map or map iterator range. \see keys()
    template<class Range>
    auto values(Range&& range) ->
        // Doxygen can't parse this
        /** \cond */
        Range_<TupleIter<itertype(range),1>, TupleIter<itertype(range),1>>
        /** \endcond */
    {
        return honey::range(TupleIter<itertype(range),1>(begin(range)), TupleIter<itertype(range),1>(end(range)));
    }

    /// Convert reverse iterator to forward iterator
    template<class Iter>
    auto reverseIterToForward(Iter&& it) -> typename mt::removeRef<decltype(--it.base())>::type
                                                                { return --it.base(); }

    /// Erase element at index
    template<class T, class A>
    void erase_at(std::vector<T,A>& list, int index)            { list.erase(list.begin()+index); }

    /// Erase first occurrence of value.  Returns iterator to next element after the erased element, or end if not found.
    template<class List>
    typename List::iterator erase(List& list, const typename List::value_type& val)
    {
        auto it = std::find(list.begin(), list.end(), val);
        if (it == list.end()) return list.end();
        return list.erase(it);
    }

    /// Erase using reverse iterator.  Returns reverse iterator to element after erased element.
    template<class List>
    typename List::reverse_iterator erase(List& list, const typename List::reverse_iterator& iter)
    {
        return typename List::reverse_iterator(list.erase(reverseIterToForward(iter)));
    }

    /// Erase all occurrences of value.
    template<class List, class T>
    void erase_all(List& list, const T& val)
    {
        auto it = list.begin();
        while((it = std::find(it, list.end(), val)) != list.end())
            it = list.erase(it);
    }

    /// Get iterator to key with value.  Returns end if not found.
    template<class MultiMap, class Key, class Val>
    auto find(MultiMap& map, const Key& key, const Val& val) -> itertype(map)
    {
        return honey::find(range(map.equal_range(key)), [&](elemtype(map)& e) { return e.second == val; });
    }
}

/// Convenient method to get an unordered map type with custom allocator
template<class Key, class Value, template<class> class Alloc>
struct UnorderedMap : mt::NoCopy
{ typedef unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, Alloc<pair<const Key, Value>>> type; };

/// Convenient method to get an unordered multi-map type with custom allocator
template<class Key, class Value, template<class> class Alloc>
struct UnorderedMultiMap : mt::NoCopy
{ typedef unordered_multimap<Key, Value, std::hash<Key>, std::equal_to<Key>, Alloc<pair<const Key, Value>>> type; };

/// Convenient method to get an unordered set type with custom allocator
template<class Key, template<class> class Alloc>
struct UnorderedSet : mt::NoCopy
{ typedef unordered_set<Key, std::hash<Key>, std::equal_to<Key>, Alloc<Key>> type; };


//====================================================
/** \cond */
#define bind_fill(...) __bind_fill()
#define bind(...) void*
/** \endcond */
/// Version of bind that automatically fills in placeholders for unspecified arguments.
/**
  * ### Example
  * To bind the `this` pointer for a member function: `void Class::func(int, int)`
  * \hiddentable
  * \row The standard bind method is:               \col `bind(&Class::func, this, _1, _2);` \endrow
  * \row `bind_fill` has a more convenient syntax:  \col `bind_fill(&Class::func, this);`    \endrow
  * \endtable
  */
auto bind_fill(Func&&, Args&&...) -> bind(...);
#undef bind_fill
#undef bind
/** \cond */
#define BIND_FILL_ARG_MAX 9

#define PARAMT(It)              , class T##It
#define PARAM(It)               , T##It&& a##It
#define ARG(It)                 , forward<T##It>(a##It)

/** \cond */
namespace priv
{
    template<int Arity> struct bind_fill;

    #define PLACE(It)           , _##It

    #define OP(It, ItMax)                                                                               \
        template<class F ITERATE__(1,It,PARAMT)>                                                        \
        auto operator()(const F& f ITERATE__(1,It,PARAM)) ->                                            \
            decltype(   bind(f                                                                          \
                            ITERATE__(1,It,ARG)                                                         \
                            IFEQUAL(It,ItMax,,ITERATE__(1,PP_SUB(ItMax,It),PLACE))                      \
                        ) )                                                                             \
        {                                                                                               \
            return      bind(f                                                                          \
                            ITERATE__(1,It,ARG)                                                         \
                            IFEQUAL(It,ItMax,,ITERATE__(1,PP_SUB(ItMax,It),PLACE))                      \
                        );                                                                              \
        }                                                                                               \
    
    #define STRUCT(It)                                                                                  \
        template<> struct bind_fill<It>                                                                 \
        {                                                                                               \
            ITERATE1_(0, It, OP, It)                                                                    \
        };                                                                                              \

    ITERATE(0, BIND_FILL_ARG_MAX, STRUCT)
    #undef PLACE
    #undef OP
    #undef STRUCT
}
/** \endcond */

#define FUNC(It)                                                                                        \
    template<class F ITERATE_(1,It,PARAMT)>                                                             \
    auto bind_fill(const F& f ITERATE_(1,It,PARAM)) ->                                                  \
        decltype(   priv::bind_fill<mt::funcTraits<F>::arity>()(f ITERATE_(1,It,ARG)))                  \
    {                                                                                                   \
        return      priv::bind_fill<mt::funcTraits<F>::arity>()(f ITERATE_(1,It,ARG));                  \
    }                                                                                                   \

ITERATE(0, BIND_FILL_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef ARG
#undef FUNC
/** \endcond */
//====================================================

/// @}

}
