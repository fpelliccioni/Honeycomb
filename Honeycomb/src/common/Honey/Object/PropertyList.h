// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/Property.h"

namespace honey
{

/// Generic vector property
/** \ingroup Component */
template<class T>
class Property<vector<T>> : public vector<T>, public PropertyBase
{
public:
    typedef vector<T> List;
    typedef SharedPtr<Property> Ptr;
    typedef SharedPtr<const Property> ConstPtr;

    Property(const Id& id)                                          : PropertyBase(id), List(1) {}
    Property(const Id& id, const List& list)                        : PropertyBase(id), List(list) {}
    Property(const Id& id, int size, const T& val = T())            : PropertyBase(id), List(size, val) {}
    template<class Iter>
    Property(const Id& id, Iter&& first, Iter&& last)               : PropertyBase(id), List(forward<Iter>(first), forward<Iter>(last)) {}

    static const Id& s_type();
    virtual const Id& type() const                                  { return s_type(); }
    virtual Property& clone() const                                 { return *new Property(*this); }

    int size() const                                                { return honey::size(static_cast<const List&>(*this)); }

    Property& operator=(const Property& rhs)                        { List::operator=(rhs); return *this; }
    Property& operator=(const List& rhs)                            { List::operator=(rhs); return *this; }
    /// Assign to first element in list
    Property& operator=(const T& rhs)                               { assert(size()); (*this)[0] = rhs; return *this; }

    /// Retrieve first element
    operator const T&() const                                       { return (*this)[0]; }
    operator T&()                                                   { return (*this)[0]; }
};

/// Integer list property
typedef vector<int> IntList;
inline const Id& Property<IntList>::s_type()                        { static const Id& id = "IntList"; return id; }

/// Real list property
typedef vector<Real> RealList;
inline const Id& Property<RealList>::s_type()                       { static const Id& id = "RealList"; return id; }

/// String list property
inline const Id& Property<String::List>::s_type()                   { static const Id& id = "String::List"; return id; }

}
