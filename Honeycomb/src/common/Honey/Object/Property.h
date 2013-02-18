// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Enum.h"
#include "Honey/Math/Real.h"

namespace honey
{

/// Base class for all properties
/** \ingroup Component */
class PropertyBase : public Object
{
public:
    typedef SharedPtr<PropertyBase> Ptr;
    typedef SharedPtr<const PropertyBase> ConstPtr;

    PropertyBase(const Id& id)                                      : _id(id) {}

    /// Get property id
    const Id& id() const                                            { return _id; }
    /// Get property type
    virtual const Id& type() const = 0;
    /// Create a clone of this property
    virtual PropertyBase& clone() const = 0;

protected:
    const Id _id;
};

/// Generic property
/** \ingroup Component */
template<class T>
class Property : public PropertyBase
{
public:
    typedef SharedPtr<Property> Ptr;
    typedef SharedPtr<const Property> ConstPtr;

    Property(const Id& id)                                          : PropertyBase(id) {}
    Property(const Id& id, const T& val)                            : PropertyBase(id), _val(val) {}

    /// Static function to get property type
    static const Id& s_type();
    virtual const Id& type() const                               { return s_type(); }

    virtual Property& clone() const                                 { return *new Property(*this); }

    Property& operator=(const Property& rhs)                        { _val = rhs._val; return *this; }
    Property& operator=(const T& rhs)                               { _val = rhs; return *this; }

    const T& operator*() const                                      { return _val; }
    T& operator*()                                                  { return _val; }
    const T* operator->() const                                     { return &_val; }
    T* operator->()                                                 { return &_val; }
    operator const T&() const                                       { return _val; }
    operator T&()                                                   { return _val; }

private:
    T _val;
};

/// Integer property
inline const Id& Property<int>::s_type()                            { static const Id& id = "int"; return id; }
/// Real property
inline const Id& Property<Real>::s_type()                           { static const Id& id = "Real"; return id; }
/// String property
inline const Id& Property<String>::s_type()                         { static const Id& id = "String"; return id; }

}

