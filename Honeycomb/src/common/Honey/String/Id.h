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
  * An Id is composed of a string and its hashed integer value. The strings are hashed using hash::fast(). \n
  * In final mode an Id only holds the hash; name() and operator<<() are not available.
  *
  * \see string literal operator `_id` to create ids at compile-time.
  */
/// @{

class IdConstexpr;

/// Holds a string and its hashed value for fast comparison ops. See \ref Id
class Id
{
    friend class IdConstexpr;
public:
    Id()                                        : _hash(0) {}
    Id(const String& name)                      : debug_if(_name(name),) _hash(hash::fast(name)) {}
    Id(const StringStream& stream)              : Id(stream.str()) {}
    Id(const Char* name)                        : Id(String(name)) {}
    Id(const char* name)                        : Id(String(name)) {}
    Id(const IdConstexpr& rhs);
    Id(const Id& rhs)                           : debug_if(_name(rhs._name),) _hash(rhs._hash) {}
    Id(Id&& rhs)                                : debug_if(_name(move(rhs._name)),) _hash(rhs._hash) {}
    
    Id& operator=(const String& name)           { debug_if(_name = name;) _hash = hash::fast(name); return *this; }
    Id& operator=(const StringStream& stream)   { return operator=(stream.str()); }
    Id& operator=(const Char* name)             { return operator=(String(name)); }
    Id& operator=(const char* name)             { return operator=(String(name)); }
    Id& operator=(const Id& rhs)                { debug_if(_name = rhs._name;) _hash = rhs._hash; return *this; }
    Id& operator=(Id&& rhs)                     { debug_if(_name = move(rhs._name);) _hash = rhs._hash; return *this; }
    Id& operator=(const IdConstexpr& rhs);
    
    bool operator==(const Id& rhs) const        { return _hash == rhs._hash; }
    bool operator!=(const Id& rhs) const        { return _hash != rhs._hash; }
    bool operator<=(const Id& rhs) const        { return _hash <= rhs._hash; }
    bool operator>=(const Id& rhs) const        { return _hash >= rhs._hash; }
    bool operator< (const Id& rhs) const        { return _hash < rhs._hash; }
    bool operator> (const Id& rhs) const        { return _hash > rhs._hash; }

    bool operator==(const IdConstexpr& rhs) const;
    bool operator!=(const IdConstexpr& rhs) const;
    bool operator<=(const IdConstexpr& rhs) const;
    bool operator>=(const IdConstexpr& rhs) const;
    bool operator< (const IdConstexpr& rhs) const;
    bool operator> (const IdConstexpr& rhs) const;
    
    #ifndef FINAL
        /// Get string that this id represents
        const String& name() const              { return _name; }
    #endif
    /// Get hashed integer value of name
    int hash() const                            { return _hash; }
    
    /// Same as hash()
    operator int() const                        { return _hash; }

    #ifndef FINAL
        ///Output Id to string stream
        friend StringStream& operator<<(StringStream& os, const Id& val)    { return os << (val.hash() ? val.name() : "idnull"); }
    #endif
    
private:
    Id(const String& name, int hash)            : debug_if(_name(name),) _hash(hash) {}

    #ifndef FINAL
        String _name;
    #endif
    int _hash;
};

/// Create an Id that can be retrieved safely from a static context
#define IdStatic(Func, IdString) mt_staticObj(const Id, Func, (IdString))
/// Null id
#define idnull IdConstexpr()

/// Id created from a string literal at compile-time. \see string literal operator `_id`
class IdConstexpr
{
    friend class Id;
public:
    constexpr IdConstexpr()                     : debug_if(_name(""),) _hash(0) {}
    constexpr IdConstexpr(const char* str, size_t len)
                                                : debug_if(_name(str),) _hash(hash::fast_constexpr(str, len)) {}
    
    constexpr bool operator==(const IdConstexpr& rhs)   { return _hash == rhs._hash; }
    constexpr bool operator!=(const IdConstexpr& rhs)   { return _hash != rhs._hash; }
    constexpr bool operator<=(const IdConstexpr& rhs)   { return _hash <= rhs._hash; }
    constexpr bool operator>=(const IdConstexpr& rhs)   { return _hash >= rhs._hash; }
    constexpr bool operator< (const IdConstexpr& rhs)   { return _hash < rhs._hash; }
    constexpr bool operator> (const IdConstexpr& rhs)   { return _hash > rhs._hash; }
    
    bool operator==(const Id& rhs) const        { return _hash == rhs._hash; }
    bool operator!=(const Id& rhs) const        { return _hash != rhs._hash; }
    bool operator<=(const Id& rhs) const        { return _hash <= rhs._hash; }
    bool operator>=(const Id& rhs) const        { return _hash >= rhs._hash; }
    bool operator< (const Id& rhs) const        { return _hash < rhs._hash; }
    bool operator> (const Id& rhs) const        { return _hash > rhs._hash; }
    
    #ifndef FINAL
        constexpr const char* name()            { return _name; }
    #endif
    constexpr int hash()                        { return _hash; }
    
    constexpr operator int()                    { return _hash; }
    
    #ifndef FINAL
        friend StringStream& operator<<(StringStream& os, const IdConstexpr& val)   { return os << (val.hash() ? val.name() : "idnull"); }
    #endif
    
private:
    #ifndef FINAL
        const char* _name;
    #endif
    int _hash;
};

/// Create an id from a string literal at compile-time.  Can be used as a case expression in a switch block (ex. case "foo"_id: ).
constexpr IdConstexpr operator"" _id(const char* str, size_t len)       { return IdConstexpr(str, len); }
    
/// @}

inline Id::Id(const IdConstexpr& rhs)                       : debug_if(_name(rhs._name),) _hash(rhs._hash) {}
inline Id& Id::operator=(const IdConstexpr& rhs)            { debug_if(_name = rhs._name;) _hash = rhs._hash; return *this; }
inline bool Id::operator==(const IdConstexpr& rhs) const    { return _hash == rhs._hash; }
inline bool Id::operator!=(const IdConstexpr& rhs) const    { return _hash != rhs._hash; }
inline bool Id::operator<=(const IdConstexpr& rhs) const    { return _hash <= rhs._hash; }
inline bool Id::operator>=(const IdConstexpr& rhs) const    { return _hash >= rhs._hash; }
inline bool Id::operator< (const IdConstexpr& rhs) const    { return _hash < rhs._hash; }
inline bool Id::operator> (const IdConstexpr& rhs) const    { return _hash > rhs._hash; }
    
}

/** \cond */
/// Allow class to be used as key in unordered containers
template<>
struct std::hash<honey::Id>
{
    size_t operator()(const honey::Id& val) const           { return val.hash(); }
};

template<>
struct std::hash<honey::IdConstexpr>
{
    size_t operator()(const honey::IdConstexpr& val) const  { return val.hash(); }
};
/** \endcond */

