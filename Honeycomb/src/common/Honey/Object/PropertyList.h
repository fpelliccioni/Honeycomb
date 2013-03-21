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

    Property(const String& name)                                    : PropertyBase(name), List(1) {}
    Property(const String& name, const List& list)                  : PropertyBase(name), List(list) {}
    Property(const String& name, int size, const T& val = T())      : PropertyBase(name), List(size, val) {}
    template<class Iter>
    Property(const String& name, Iter&& first, Iter&& last)         : PropertyBase(name), List(forward<Iter>(first), forward<Iter>(last)) {}

    static const TypeInfo& s_type();
    virtual const TypeInfo& type() const                            { return s_type(); }
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
template<> inline auto Property<IntList>::s_type() -> const TypeInfo&       { static TypeInfo _("IntList"); return _; }

/// Real list property
typedef vector<Real> RealList;
template<> inline auto Property<RealList>::s_type() -> const TypeInfo&      { static TypeInfo _("RealList"); return _; }

/// String list property
template<> inline auto Property<String::List>::s_type() -> const TypeInfo&  { static TypeInfo _("String::List"); return _; }

}
