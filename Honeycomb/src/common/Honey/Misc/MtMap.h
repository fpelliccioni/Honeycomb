// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/Id.h"
#include "Honey/Misc/Option.h"

namespace honey
{

/// class MtMap, a compile-time associative heterogeneous container.
/**
    \defgroup MtMap     Meta-map

    Each value may have a different type.
    Lookups are resolved at compile-time, so they are O(1) constant-time.
            
    Features:
        - Supports all types including constants and references
        - Keys can be required or optional on construction (fails at compile-time)
        - Keys can be initialized in any order, keys not in the map are ignored
        - The map can be copy/move assigned to any other map, non matching keys are ignored

    For unwieldly functions with many arguments a map is ideal to enable "keyword arguments", see the example. \n
    The map can also be used as an indexable array, see mtkeygen and the example.

    Example:

    \code

    mtkey(key_int);         //Construct keys
    mtkey(key_id);
    mtkey(key_char);
    mtkey(key_string);

    mtkeygen(key_index);    //Construct a templated key generator that can be used to turn the map into an array,
                            //indexable by static ints and compile-time integer arithmetic.

    template<class T> void iterTest(T&);
    void keywordTest();

    void test()
    {
                                                            //Declare a map. Note the "Value, key" order, similar to a variable declaration
        typedef MtMap<int, key_int, Id, key_id>::type FooMap;
                                                                //Construct using mtmap factory to initialize in any order
                                                                //All keys are required on construction, for optional keys see keyword args example
        FooMap foo = mtmap(key_id() = "foo", key_int() = 1);    //Key matching is resolved at compile-time, move semantics ensure fast init

        int x = foo[key_int()];                             //Get at key
        Id myid = foo[key_id()];

        foo[key_int()] = 2;                                 //Set
        foo[key_id()] = "foo2";
                                                                            
        assert(foo.hasKey(key_id()) && FooMap::hasKeyMt<key_id>::value);        //Check if has key at run/compile-time
        assert(( std::is_same< FooMap::getResult<key_id>::type, Id >::value )); //The result type of get(key_id) is Id 
        assert(foo.size() == 2 && FooMap::sizeMt::value == 2);                  //Get number of keys at run/compile-time

        x = foo.get(key_int());                             //Get at key
        foo.set(key_id() = "foo3");                         //Set accepts any type, converts using operator=.  Returns true.
        foo.set(key_char() = 'a');                          //Key doesn't exist, returns false
 
                                                            //Create a map that holds references 
                                                            //char doesn't exist in fooRef, so it will be ignored in the initialization
        MtMap<int&, key_int, Id&, key_id>::type fooRef =
            mtmap(key_id() = myid, key_int() = x, key_char() = 'b');                
                            
        fooRef[key_int()] = 3;                              //Set x to 3
        fooRef[key_id()] = "foo4";                          //Set myid to "foo4"

        foo = fooRef;                                       //Flexible map assignment. Matching keys are copied from fooRef using each value type's operator=
                                                            
                                                            //Insert keys/values into map 
                                                            //Insert asserts that the keys don't already exist
        FooMap::insertResult<String, key_string, int8, key_char>::type fooInsert =
            foo.insert(key_string() = "foo5", key_char() = 'c');

                                                            //Erase keys from the map, it's ok if the keys don't exist
                                                            //Returns decltype(fooInsert)::eraseResult<...>::type
        auto fooErase = fooInsert.erase(key_id(), key_int());

        auto empty = foo.clear();                           //Clear map of keys, returns MtMap<>::type
        assert(empty.empty());                              //Test if empty at run-time

        mt::Void empty_val = empty[key_string()];           //Key doesn't exist, returns mt::Void
        mt_unused(empty_val);
        empty.set(key_string() = "empty");                  //Key doesn't exist, returns false

        enum { IDX0, IDX1 };                                //Use the key generator to make map indexable by static ints and arithmetic             
        MtMap<Id, key_index<IDX0>, String, key_index<IDX1>>::type indexmap =
            mtmap(key_index<IDX0 + 1>() = "idx1", key_index<IDX1 - 1>() = "idx0");

        iterTest(fooInsert);
        keywordTest();
    }

                                                            //Define a functor for iterating over the map
    struct Functor
    {                                                       //Generic catch-all
        template<class Key, class Val>
        void operator()(Key key, const Val& val)
        {
            mt_unused(key);
            debug::print(sout() << "Key: " << key.id() << " ; Value: " << val << endl);
        }
                                                            //Overload for specific key/value pair
        void operator()(key_int key, int& val)
        {
            mt_unused(key);
            debug::print(sout() << "key: " << key.id() << " ; value: " << val << endl);

            val = -1;                                       //Modify value in map
        }
    };

    template<class T>
    void iterTest(T& fooInsert)
    {
                                                            //Use Begin/End and functor to print contents of map
        debug::print(sout() << "--fooInsert--" << endl);
        for_each_mtmap(fooInsert.begin(), fooInsert.end(), Functor());

                                                            //Use iter() to get an iterator by key
        debug::print(sout() << "--fooInsert[key_int, end]--" << endl);
        for_each_mtmap(fooInsert.iter(key_int()), fooInsert.end(), Functor());
    }

                                                            //Define a function that takes keyword arguments.
                                                            //Note that key_id is optional and has a default, all others are required.
                                                            //The default value is wrapped in a lambda so the Id ctor can be omitted.
    void keywordFunc(MtMap<int8, key_char, int, key_int, option<Id>, key_id>::type _)
    {
        _.setDefaults(mtmap(key_id() = []{ return "default"; }));
        debug::print(sout() << "Keyword Args: " << _ << endl);
    }

    void keywordTest()
    {
        keywordFunc(mtmap(key_int() = 1, key_char() = 'c'));
        keywordFunc(mtmap(key_int() = 1, key_char() = 'c', key_id() = "user"));
    }

    \endcode
  */

//====================================================
// Private
//====================================================

// Map type names can get long, must disable decorated name truncated warning
#pragma warning(disable:4503)

#define MTMAPE_PARAMT(It)           COMMA_IFNOT(It,1) class Key##It, class Val##It
#define MTMAPE_PARAM(It)            COMMA_IFNOT(It,1) const MtPair<Key##It, Val##It>& pair##It
#define MTMAPE_ARG(It)              COMMA_IFNOT(It,1) pair##It

/// Init list for constructor, recurse down to tail passing in rest of key/val list
#define MTMAPE_INITLIST_ARG(It)     , pair##It
#define MTMAPE_INITLIST(It)         List(mt::tag<0>() IFEQUAL(It, 1, EMPTY, ITERATE_(2, It, MTMAPE_INITLIST_ARG)) )

#define MTMAPE_CTOR(It)                                                                                     \
    /** Ctor, pairs must be in correct order */                                                             \
    template< ITERATE_(1, It, MTMAPE_PARAMT) >                                                              \
    MtMapElem(mt::tag<0>, ITERATE_(1, It, MTMAPE_PARAM) ) :                                                 \
        MTMAPE_INITLIST(It),                                                                                \
        _val(pair1.val)                                                                                     \
    { static_assert((std::is_same<Key,Key1>::value), "Ctor failed. Key mismatch. Wrong init order."); }     \

#define MTMAPE_INSERT_TYPE_ASSERT(It)                                                                       \
    static_assert(!hasKeyMt<Key##It>::value, "Insert failed. Key " #It " already exists.");

#define MTMAPE_INSERT_TYPE(It)                                                                              \
    /* template to get insert result type. New keys/vals at front, rest of map at back */                   \
    template< ITERATE_(1, It, MTMAPE_PARAMT) >                                                              \
    struct insertResult<ITERATE_(1, It, MTMAP_PARAM) >                                                      \
    {                                                                                                       \
        ITERATE_(1, It, MTMAPE_INSERT_TYPE_ASSERT)                                                          \
        typedef ITERATE_(1, It, MTMAP_TYPE_START) MtMapElem ITERATE_(1, It, MTMAP_TYPE_END) type;           \
    };                                                                                                      \

#define MTMAPE_INSERT(It)                                                                                   \
    /* Insert Function.  Construct new map with key/vals at front, rest of map at back */                   \
    template< ITERATE_(1, It, MTMAPE_PARAMT) >                                                              \
    typename insertResult<ITERATE_(1, It, MTMAP_PARAM)>::type                                               \
        insert(ITERATE_(1, It, MTMAPE_PARAM)) const                                                         \
    {                                                                                                       \
        return typename insertResult<ITERATE_(1, It, MTMAP_PARAM)>::type                                    \
            (mt::tag<1>(), ITERATE_(1, It, MTMAPE_ARG), *this);                                             \
    }                                                                                                       \

#define MTMAPE_INSERT_INITLIST(It)                                                                          \
    IFEQUAL(It, 1, List(map), List(mt::tag<1>() ITERATE_(2, It, MTMAPE_ARG), map))                          \

#define MTMAPE_INSERT_CTOR(It)                                                                              \
    /* Insert ctor, Recurse through key/vals list and then pass construction to map copy ctor */            \
    template< ITERATE_(1, It, MTMAPE_PARAMT) , class Map>                                                   \
    MtMapElem(mt::tag<1>, ITERATE_(1, It, MTMAPE_PARAM), const Map& map) :                                  \
        MTMAPE_INSERT_INITLIST(It),                                                                         \
        _val(pair1.val) {}                                                                                  \

#define MTMAPE_ERASE_PARAMT_DEF(It) COMMA_IFNOT(It,1) class Key##It = mt::Void
#define MTMAPE_ERASE_PARAMT(It)     COMMA_IFNOT(It,1) class Key##It
#define MTMAPE_ERASE_PARAM(It)      COMMA_IFNOT(It,1) Key##It
#define MTMAPE_ERASE_TYPE_MATCH(It) IFEQUAL(It, 1, EMPTY, ||) std::is_same<Key, Key##It>::value

#define MTMAPE_ERASE_TYPE(It)                                                                               \
    /* template to get erase result type.  Reconstruct type without matching keys. */                       \
    template<ITERATE_(1, It, MTMAPE_ERASE_PARAMT)>                                                          \
    struct eraseResult<ITERATE_(1, It, MTMAPE_ERASE_PARAM) >                                                \
    {                                                                                                       \
        typedef typename std::conditional<                                                                  \
            ITERATE_(1, It, MTMAPE_ERASE_TYPE_MATCH),                                                       \
            typename List::template eraseResult<ITERATE_(1, It, MTMAPE_ERASE_PARAM)>::type,                 \
            MtMapElem<Key, Val, typename List::template eraseResult<ITERATE_(1, It, MTMAPE_ERASE_PARAM)>::type> \
        >::type type;                                                                                       \
    };                                                                                                      \

#define MTMAPE_ERASE(It)                                                                                    \
    /* Erase function.  Reconstruct map without matching keys, copy orig map */                             \
    template<ITERATE_(1, It, MTMAPE_ERASE_PARAMT)>                                                          \
    typename eraseResult<ITERATE_(1, It, MTMAPE_ERASE_PARAM)>::type                                         \
        erase(ITERATE_(1, It, MTMAPE_ERASE_PARAM)) const                                                    \
    {                                                                                                       \
        return typename eraseResult<ITERATE_(1, It, MTMAPE_ERASE_PARAM)>::type(*this);                      \
    }                                                                                                       \

#define MTMAP_PARAMT_DEF(It)        COMMA_IFNOT(It,1) class Val##It = mt::Void, class Key##It = mt::Void
#define MTMAP_PARAM(It)             COMMA_IFNOT(It,1) Val##It, Key##It
#define MTMAP_TYPE_START(It)        MtMapElem<Key##It, Val##It, 
#define MTMAP_TYPE_END(It)          > 

#define MTMAP_TYPE(It)                                                                                      \
    template<ITERATE_(1, It, MTMAPE_PARAMT)>                                                                \
    struct MtMap<ITERATE_(1, It, MTMAP_PARAM) > : mt::NoCopy                                                \
    {                                                                                                       \
        typedef ITERATE_(1, It, MTMAP_TYPE_START) MtMapTail ITERATE_(1, It, MTMAP_TYPE_END) type;           \
    };                                                                                                      \

#define MTMAP_CREATE(It)                                                \
    template< ITERATE_(1, It, MTMAPE_PARAMT) >                          \
    typename MtMap< ITERATE_(1, It, MTMAP_PARAM) >::type                \
        mtmap( ITERATE_(1, It, MTMAPE_PARAM) )                          \
    { return typename MtMap< ITERATE_(1, It, MTMAP_PARAM) >::type       \
        (mt::tag<0>(), ITERATE_(1, It, MTMAPE_ARG) ); }                 \


//====================================================
// Public
//====================================================

/// \addtogroup MtMap
/// @{

/// Max number of keys that can be stored in map
#define MTMAP_KEY_MAX 20
/// Max number of keys that can be inserted/erased per call
#define MTMAP_KEY_MODIFY_MAX 5

template<class Key_, class Val_, class List_> class MtMapElem;

/// Declare a map type.  The instantiable map type is `MtMap<Val1,Key1...>::%type`.  The internal map type is complex so this helper is needed. 
template<ITERATE(1, MTMAP_KEY_MAX, MTMAP_PARAMT_DEF), class _ = mt::tag<0>>
struct MtMap {};

/** \cond */
/// Tail of map list
typedef MtMapElem<mt::Void, mt::Void, mt::Void> MtMapTail;
/// Empty map
template<> struct MtMap<>                                           : mt::NoCopy { typedef MtMapTail type; };
ITERATE(1, MTMAP_KEY_MAX, MTMAP_TYPE)
/** \endcond */

/// Construct a key type with name
#define mtkey(Name)                                                             \
    struct Name                                                                 \
    {                                                                           \
        /** Id can be accessed with id() */                                     \
        IdStatic(id, #Name);                                                    \
        /** Generate pairs with any value. Pair may store ref to rvalue. */     \
        template<class Val> MtPair<Name, Val> operator=(Val&& val)              \
        { return MtPair<Name, Val>(forward<Val>(val)); }                        \
    };                                                                          \

/// Construct a templated key generator that creates keys from static ints
#define mtkeygen(Name)                                                          \
    template<int Index> struct Name                                             \
    {                                                                           \
        IdStatic(id, sout() << #Name << "<" << Index << ">");                   \
        template<class Val> MtPair<Name, Val> operator=(Val&& val)              \
        { return MtPair<Name, Val>(forward<Val>(val)); }                        \
    };                                                                          \

/// Key/value pair.  A pair can be constructed with the syntax: `(key() = value)`
template<class Key_, class Val_>
struct MtPair
{
    typedef Key_ Key;
    typedef Val_ Val;
    
    template<class Val>
    MtPair(Val&& val)                                               : key(), val(forward<Val>(val)) {}

    const Key key;
    Val val;
};

/// Bidirectional iterator over map key/value pairs
/**
  * Every iteration step is a new type of iterator and pair, so this iterator can't be used normally.
  * This iterator should be used in a recursive templated function.
  */ 
template<class Head, class Elem = Head>
class MtMapIter
{
    friend class MtMapIter;

    typedef typename Elem::Key Key;
    typedef typename Elem::Val Val;
    typedef typename Head::Super::template findElem<Key>::Next Next;
    typedef typename Head::Super::template findElem<Key>::Prev Prev;

public:
    // Propogate const to pair value
    typedef MtPair< typename Elem::Key,
                    typename std::conditional<std::is_const<Head>::value, const Val&, Val&>::type> Pair;

    MtMapIter(Head& head)                                           : _head(&head), pair(_head->get(Key())) {}

    typedef MtMapIter<Head, Next> NextIter;
    NextIter operator++()                                           { return NextIter(*_head); }
    typedef MtMapIter<Head, Prev> PrevIter;
    PrevIter operator--()                                           { return PrevIter(*_head); }

    template<class Iter> bool operator==(const Iter& rhs) const     { return _head == rhs._head && std::is_same<typename Pair::Key, typename Iter::Pair::Key>::value; }
    template<class Iter> bool operator!=(const Iter& rhs) const     { return !operator==(rhs); }

    const Pair& operator*() const                                   { return pair; }
    Pair& operator*()                                               { return pair; }
    const Pair* operator->() const                                  { return &pair; }
    Pair* operator->()                                              { return &pair; }

private:
    Head* _head;
    Pair pair;
};


/** \cond */
namespace priv
{
    /// If visitor can't accept key/value pair, skip the pair
    template<class Iter, class Func>
    auto for_each_mtmap_call(Iter&& it, Func&& func) -> decltype(func(it->key, it->val), void())
                                                                    { func(it->key, it->val); }
    inline void for_each_mtmap_call(...) {}
}
/// Stop iteration when end is reached
template<class Iter1, class Iter2, class Func>
auto for_each_mtmap(Iter1, Iter2, Func&&) ->
    typename mt::disable_if<!std::is_same<typename Iter1::Pair::Key, typename Iter2::Pair::Key>::value>::type {}
/** \endcond */

/// Iterate over map calling functor (visitor) for each key/value pair
template<class Iter1, class Iter2, class Func>
auto for_each_mtmap(Iter1 itBegin, Iter2 itEnd, Func&& func) ->
    typename std::enable_if<!std::is_same<typename Iter1::Pair::Key, typename Iter2::Pair::Key>::value>::type
{
    priv::for_each_mtmap_call(itBegin, func);
    for_each_mtmap(++itBegin, itEnd, forward<Func>(func));
}


/** \cond */
namespace priv
{
    /// Check if value can be omitted in construction
    template<class Val> struct isOptional                           : mt::Value<bool, false> {};
    template<class Val> struct isOptional<option<Val>>              : mt::Value<bool, true> {};
    template<> struct isOptional<mt::Void>                          : mt::Value<bool, true> {};

    /// Functor to convert map to string
    struct MtMapToString
    {
        MtMapToString(StringStream& os)                             : os(os), count(0) {}
        template<class Key, class Val>
        void operator()(Key, const Val& val)
        {
            if (count++ > 0)
                os << ", ";
            os << Key::id() << " => " << val;
        }
        StringStream& os;
        int count;
    };
}
/** \endcond */

/// Common functions between map elem and the map tail specialization. Use through class MtMapElem.
template<class Subclass, class Key_, class Val_, class List_>
class MtMapCommon
{
    template<class, class, class, class> friend class MtMapCommon;
    template<class, class> friend class MtMapIter;

public:
    typedef Key_ Key;   ///< Key type
    typedef Val_ Val;   ///< Value type
    typedef List_ List; ///< Rest of map

private:
    // Use cond, some of these private types were appearing in Doxygen
    /** \cond */
    static const bool isTail = std::is_same<List, mt::Void>::value;

    /// Private functions for map elem
    template<bool isTail, int _=0>
    struct priv
    {
        /// We don't have this key, recurse towards tail
        template<class Key, class Prev>
        struct findElem                                             : List::Super::template findElem<Key, Subclass> {};

        /// Specialize for our key
        template<class Prev_>
        struct findElem<Key, Prev_>
        {
            typedef Subclass type;
            typedef Prev_ Prev;
            typedef List Next;
        };

        /// Recurse to tail
        template<int Count> struct sizeR                            : List::Super::template sizeR<Count+1> {};
    };

    /// Private functions for tail
    template<int _>
    struct priv<true,_>
    {
        // Fallback for any key
        template<class Key, class Prev_>
        struct findElem
        {
            typedef Subclass type;
            typedef Prev_ Prev;
            typedef Subclass Next;
        };

        /// End recursion
        template<int Count> struct sizeR                            : mt::Value<int, Count> {};
    };

    /// Find map list element with key at compile-time.  Also returns prev/next elements in list at key.
    template<class Key, class Prev = Subclass>
    struct findElem                                                 : priv<isTail>::template findElem<Key, Prev> {};

    /// Recursive size counter, size is at tail
    template<int Count>
    struct sizeR                                                    : priv<isTail>::template sizeR<Count> {};

    /** \endcond */
public:
    /// Check if key exists at compile-time
    template<class Key> struct hasKeyMt                             : mt::Value<bool, !std::is_same<typename findElem<Key>::type, MtMapTail>::value> {};
    /// Check if has key
    template<class Key> bool hasKey(Key) const                      { return hasKeyMt<Key>::value; }

    /// Result of get()
    template<class Key> struct getResult                            { typedef typename findElem<Key>::type::Val type; };

    /// Result of begin()
    struct beginResult
    {
        typedef MtMapIter<const Subclass> ConstType;
        typedef MtMapIter<Subclass> type;
    };
    /// Get beginning of an iterator over keys and values of this map
    typename beginResult::ConstType begin() const                   { return typename beginResult::ConstType(subc()); }
    typename beginResult::type      begin()                         { return typename beginResult::type(subc()); }

    /// Result of end()
    struct endResult
    {
        typedef MtMapIter<const Subclass, MtMapTail> ConstType;
        typedef MtMapIter<Subclass, MtMapTail> type;
    };
    /// Get end of an iterator over keys and values of this map
    typename endResult::ConstType   end() const                     { return typename endResult::ConstType(subc()); }
    typename endResult::type        end()                           { return typename endResult::type(subc()); }

    /// Result of iter()
    template<class Key> struct iterResult
    {
        typedef MtMapIter<const Subclass, typename findElem<Key>::type> ConstType;
        typedef MtMapIter<Subclass, typename findElem<Key>::type> type;
    };
    /// Get iterator to element by key
    template<class Key>
    typename iterResult<Key>::ConstType  iter(Key) const            { return typename iterResult<Key>::ConstType(subc()); }
    template<class Key>
    typename iterResult<Key>::type       iter(Key)                  { return typename iterResult<Key>::type(subc()); }

    /// Result of clear()
    struct clearResult                                              { typedef MtMapTail type; };
    /// Clear map of all keys
    typename clearResult::type clear()                              { return typename clearResult::type(); }

    /// Get size of map at compile-time
    struct sizeMt                                                   : sizeR<0> {};
    /// Get size of map
    int size() const                                                { return sizeMt::value; };

    /// Check if empty at compile-time
    struct emptyMt                                                  : mt::Value<bool, isTail> {};
    /// Check if empty
    bool empty() const                                              { return emptyMt::value; }

    /// Convert to string
    friend StringStream& operator<<(StringStream& os, const Subclass& map)
    { os << "{ "; for_each_mtmap(map.begin(), map.end(), honey::priv::MtMapToString(os)); os << " }"; return os; }

private:
    /// Get the subclass that inherited from this base class
    const Subclass& subc() const                                    { return reinterpret_cast<const Subclass&>(*this); }
    Subclass& subc()                                                { return reinterpret_cast<Subclass&>(*this); }
};

/// Map element in recursive list
/**
  * \see MtMapCommon for the rest of the members.
  * \see \ref MtMap for more info and examples.
  */  
template<class Key_, class Val_, class List_>
class MtMapElem : public MtMapCommon<MtMapElem<Key_,Val_,List_>, Key_, Val_, List_>, public List_
{
    template<class, class, class, class> friend class MtMapCommon;
    template<class, class, class> friend class MtMapElem;
    template<class, class> friend class MtMapIter;
    typedef MtMapCommon<MtMapElem, Key_, Val_, List_> Super;

public:
    /** \cond */
    /// Inherted members
    typedef typename Super::Key Key;
    typedef typename Super::Val Val;
    typedef typename Super::List List;

    template<class Key> struct hasKeyMt                             : Super::template hasKeyMt<Key> {};
    using Super::hasKey;
    template<class Key> struct getResult                            : Super::template getResult<Key> {};
    template<class Key> struct beginResult                          : Super::template beginResult<Key> {};
    using Super::begin;
    template<class Key> struct endResult                            : Super::template endResult<Key> {};
    using Super::end;
    template<class Key> struct iterResult                           : Super::template iterResult<Key> {};
    using Super::iter;
    typedef typename Super::clearResult clearResult;
    using Super::clear;
    typedef typename Super::sizeMt sizeMt;
    using Super::size;
    typedef typename Super::emptyMt emptyMt;
    using Super::empty;
    /** \endcond */

    MtMapElem()                                                     { static_assert(honey::priv::isOptional<Val>::value, "Key not optional. Must provide key to constructor."); }
    
    /// Ctor with key/value argument list
    ITERATE(1, MTMAP_KEY_MAX, MTMAPE_CTOR)
    /// Copy/Move any map type. Init with matching key in other map, recurse to tail.
    template<class Map>
    MtMapElem(Map&& map)                                            : List(forward<Map>(map)), _val(priv<Map>::init(map.get(Key()))) {}

    /// Copy/Move-assign any map type. Assign to matching key in other map, recurse to tail.
    template<class Map>
    MtMapElem& operator=(Map&& rhs)                                 { priv<Map>::assign(_val, rhs.get(Key())); List::operator=(forward<Map>(rhs)); return *this; }

    using List::operator[];
    /// Get value at key
    const Val& operator[](Key) const                                { return _val; }
    Val& operator[](Key)                                            { return _val; }

    using List::get;
    /// Get value at key
    const Val& get(Key) const                                       { return _val; }
    Val& get(Key)                                                   { return _val; }

    using List::set;
    /// Set value at key from the pair `(key() = value)`.  Returns false if the key isn't found.
    template<class Val>
    bool set(const MtPair<Key, Val>& pair)                          { _val = pair.val; return true; }
    
    /// Set any uninitialized optional values to the defaults provided.  A default for a key must be a functor that returns the value, so that the value ctor can be omitted if the key is already set.
    template<class Map>
    void setDefaults(Map&& defaults)                                { priv<Map>::setDefault(_val, defaults.get(Key())); List::setDefaults(forward<Map>(defaults)); }

    /// Result of insert()
    template<ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAP_PARAMT_DEF), class _ = mt::tag<0>>
    struct insertResult {};
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_INSERT_TYPE)

    #define insert(...) __insert()
    #define MtMap void*
    /// Insert pairs of the form `(key() = value)` into the map    
    MtMap insert(MtPair&&...);
    #undef insert
    #undef MtMap
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_INSERT)

    /// Result of erase()
    template<ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_ERASE_PARAMT_DEF), class _ = mt::tag<0>>
    struct eraseResult {};
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_ERASE_TYPE)

    #define erase(...) __erase()
    #define MtMap void*
    /// Erase keys from the map
    MtMap erase(Key...);
    #undef erase
    #undef MtMap
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_ERASE)

private:
    template<class Map>
    struct priv
    {
        typedef typename mt::removeRef<Map>::type MapT;

        static const bool movable = !mt::isLref<Map>::value && !mt::isRef<Val>::value && !mt::isRef<typename MapT::template getResult<Key>::type>::value;
        static const bool optional = honey::priv::isOptional<Val>::value;

        template<class T>
        static auto init(T&& val) -> typename std::enable_if<mt::True<T>::value && movable, decltype(move(val))>::type  { return move(val); }
        template<class T>
        static auto init(T&& val) -> typename mt::disable_if<mt::True<T>::value && movable, T&>::type                   { return val; }
        static Val init(mt::Void)
        {
            static_assert(optional, "Key not optional. Must provide key to constructor.");
            return Val();
        }

        template<class T>
        static auto assign(Val& lhs, T&& rhs) -> typename std::enable_if<mt::True<T>::value && movable>::type           { lhs = move(rhs); }
        template<class T>
        static auto assign(Val& lhs, T&& rhs) -> typename mt::disable_if<mt::True<T>::value && movable>::type           { lhs = rhs; }
        static void assign(Val&, mt::Void){}

        template<class T>
        static auto setDefault(Val& lhs, T&& rhs) -> typename std::enable_if<mt::True<T>::value && optional>::type      { if (!lhs) lhs = rhs(); }
        template<class T>
        static auto setDefault(Val&, T&&) -> typename mt::disable_if<mt::True<T>::value && optional>::type {}
        static void setDefault(Val&, mt::Void) {}
    };

    /// Insert ctors, after args are processed the source map is copied with copy ctor
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_INSERT_CTOR);

    /// Stored value at this key
    Val _val;
};

/** \cond */
/// Map tail, handles unknown keys and end of recursion
template<>
class MtMapElem<mt::Void, mt::Void, mt::Void> : public MtMapCommon<MtMapTail, mt::Void, mt::Void, mt::Void>
{
    template<class, class, class, class> friend class MtMapCommon;
    template<class, class, class> friend class MtMapElem;
    template<class, class> friend class MtMapIter;
    typedef MtMapCommon<MtMapElem, Key, Val, List> Super;

public:
    MtMapElem() {}
    /// Copy/Move-any ctor, end recursion
    template<class Map> MtMapElem(Map&&) {}
    /// Copy/Move-assign-any, end recursion
    template<class Map> MtMapElem& operator=(Map&&)                 { return *this; }
    /// Fallback
    template<class Key> const Val& operator[](Key) const            { return _val; }
    template<class Key> Val& operator[](Key)                        { return _val; }
    /// Fallback
    template<class Key> const Val& get(Key) const                   { return _val; }
    template<class Key> Val& get(Key)                               { return _val; }
    /// Fallback
    template<class Key, class Val>
    bool set(const MtPair<Key, Val>&)                               { return false; }
    /// End recursion
    template<class Map> void setDefaults(Map&&) {}

    /// Result of insert()
    template<ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAP_PARAMT_DEF), class _ = mt::tag<0>>
    struct insertResult {};
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_INSERT_TYPE)

    #define insert(...) __insert()
    #define MtMap void*
    /// Insert pairs of the form `(key() = value)` into the map    
    MtMap insert(MtPair&&...);
    #undef insert
    #undef MtMap
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_INSERT)

    /// Result of erase()
    template<ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_ERASE_PARAMT_DEF), class _ = mt::tag<0>>
    struct eraseResult                                              { typedef MtMapElem type; };
    
    #define erase(...) __erase()
    #define MtMap void*
    /// Erase keys from the map
    MtMap erase(Key...);
    #undef erase
    #undef MtMap
    ITERATE(1, MTMAP_KEY_MODIFY_MAX, MTMAPE_ERASE)

private:
    /// Ctor, end recursion
    MtMapElem(mt::tag<0>) {}

    Val _val;
};
/** \endcond */

/// `mtmap(MtPair&&...)` Construct a map from `(key() = value)` pairs
static inline MtMap<>::type mtmap()                                 { return MtMap<>::type(); }
ITERATE(1, MTMAP_KEY_MAX, MTMAP_CREATE)

/// @}

}
