// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/Stream.h"
#include "Honey/String/Hash.h"

namespace honey
{

/// Methods to create and operate on string ids.
/**
  * \defgroup Id    String Identifier
  *
  * String ids provide a fast way to compare strings. \n
  * An Id is composed of a string and its hashed integer value. The strings are hashed using Hash::fast().
  *
  * \see ID to fetch cached ids from string literals on the fly.
  *
  * Ids are switchable.  The cases must be defined in IDSWITCH_LIST.
  * If no match is found then `default` will be chosen as expected.
  *
  * An example that switches on the string "foo":
  *
  *     Id id = "foo";
  *
  *     #define IDSWITCH_LIST(c) c(foo) c(bar) c(eggs, "eggz") c(spam)
  *     IDSWITCH(id)
  *     #undef IDSWITCH_LIST
  *     {
  *     IDCASE(bar):
  *         break;
  *     IDCASE(foo):
  *     IDCASE(eggs):       //This block will be chosen by "foo" or "eggz"
  *         break;
  *     IDCASE(default):
  *         break;
  *     }
  *     IDSWITCH_END
  */

/// Holds a string and its hashed value for fast comparison ops. See \ref Id
/** \ingroup Id */
class Id
{
public:
    Id()                                        : _hash(0) {}
    Id(const String& name)                      : _name(name), _hash(Hash::fast(_name)) {}
    Id(const StringStream& stream)              { operator=(stream); }
    Id(const Char* name)                        { operator=(name); }
    Id(const char* name)                        { operator=(name); }
    Id(const Id& id)                            : _name(id._name), _hash(id._hash) {}
    Id(Id&& val)                                { operator=(move(val)); }
    
    Id& operator=(const Id& rhs)                { _name = rhs._name; _hash = rhs._hash; return *this; }
    Id& operator=(Id&& rhs)                     { _name = move(rhs._name); _hash = rhs._hash; return *this; }
    Id& operator=(const String& rhs)            { return assign(rhs); }
    Id& operator=(const StringStream& rhs)      { return assign(rhs); }
    Id& operator=(const Char* rhs)              { return assign(rhs); }
    Id& operator=(const char* rhs)              { return assign(rhs); }

    bool operator==(const Id& rhs) const        { return _hash == rhs._hash; }
    bool operator!=(const Id& rhs) const        { return _hash != rhs._hash; }
    bool operator<=(const Id& rhs) const        { return _hash <= rhs._hash; }
    bool operator>=(const Id& rhs) const        { return _hash >= rhs._hash; }
    bool operator< (const Id& rhs) const        { return _hash < rhs._hash; }
    bool operator> (const Id& rhs) const        { return _hash > rhs._hash; }

    /// Get string that this id represents
    const String& name() const                  { return _name; }
    /// Get hashed integer value of name
    int hash() const                            { return _hash; }
    
    /// Same as name()
    operator const String&() const              { return _name; }
    /// Same as hash()
    operator int() const                        { return _hash; }

    ///Output Id to string stream
    friend StringStream& operator<<(StringStream& os, const Id& val)    { return os << (val.hash() ? val.name() : "idnull"); }

private:
    Id& assign(const String& name)
    {
        _name = name;
        _hash = Hash::fast(_name);
        return *this;
    }

    String _name;
    int _hash;
};

}

/** \cond */
/// Allow class to be used as key in unordered containers
template<>
struct std::hash<honey::Id>
{
    size_t operator()(const honey::Id& val) const   { return val.hash(); }
};
/** \endcond */

namespace honey
{

/** \addtogroup Id */
/// @{

/// Create an Id that can be retrieved safely from a static context
#define IdStatic(Func, IdString) mt_staticObj(const Id, Func, (IdString))
/** \cond */
IdStatic(_idnull, "")
/** \endcond */
/// Null id
#define idnull _idnull()


//===================================================================
// IDSWITCH Public
//===================================================================

/// See \ref Id
#define IDSWITCH(id)                                                                                                                            \
    {                                                                                                                                           \
        enum                                                                                                                                    \
        {                                                                                                                                       \
            _IdSwitchEnum_default = -1,                                                                                                         \
            IDSWITCH_LIST(IDSWITCH_ENUMELEM)                                                                                                    \
            __IdSwitchEnum_Count                                                                                                                \
        };                                                                                                                                      \
        static const unordered_map<Id, int> _IdSwitchMap = priv::IdSwitchMapFactory() IDSWITCH_LIST(IDSWITCH_MAPELEM);                          \
        unordered_map<Id, int>::const_iterator _IdSwitchMapIt = _IdSwitchMap.find(id);                                                          \
        switch(_IdSwitchMapIt != _IdSwitchMap.end() ? _IdSwitchMapIt->second : -1)                                                              \

/// See \ref Id
#define IDCASE(name)                    case _IdSwitchEnum_##name

/// See \ref Id
#define IDSWITCH_END                    }

/// @}
//===================================================================
// IDSWITCH Private
//===================================================================
/** \cond */

#define IDSWITCH_ENUMELEM(...)          EVAL(TOKCAT(IDSWITCH_ENUMELEM_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
#define IDSWITCH_ENUMELEM_1(name)       _IdSwitchEnum_##name, 
#define IDSWITCH_ENUMELEM_2(name, str)  IDSWITCH_ENUMELEM_1(name)

#define IDSWITCH_MAPELEM(...)           EVAL(TOKCAT(IDSWITCH_MAPELEM_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
#define IDSWITCH_MAPELEM_1(name)        << #name 
#define IDSWITCH_MAPELEM_2(name, str)   << UNBRACKET(str) 

namespace priv
{
    struct IdSwitchMapFactory
    {
        IdSwitchMapFactory()                           : count(0) {}

        IdSwitchMapFactory& operator<<(const Id& id)
        {
            auto res = map.insert(make_pair(id,count++));
            if (!res.second) error(sout() << "IdSwitchMap hash collision found: Id 1: " << id << " ; Id 2: " << res.first->first);
            return *this;
        }

        operator unordered_map<Id, int>() const         { return map; }

        int count;
        unordered_map<Id, int> map;
    };
}

/** \endcond */
//===================================================================

}

