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

    PropertyBase(const String& name)                        : _name(name), _id(name) {}

    /// Get property name
    const String& name() const                              { return _name; }
    /// Get property id
    const Id& id() const                                    { return _id; }
    /// Get property type info
    virtual const TypeInfo& type() const = 0;
    /// Create a clone of this property
    virtual PropertyBase& clone() const = 0;

protected:
    const String _name;
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

    Property(const String& name)                            : PropertyBase(name) {}
    Property(const String& name, const T& val)              : PropertyBase(name), _val(val) {}

    /// Static function to get property type info
    static const TypeInfo& s_type();
    virtual const TypeInfo& type() const                    { return s_type(); }

    virtual Property& clone() const                         { return *new Property(*this); }

    Property& operator=(const Property& rhs)                { _val = rhs._val; return *this; }
    Property& operator=(const T& rhs)                       { _val = rhs; return *this; }

    const T& operator*() const                              { return _val; }
    T& operator*()                                          { return _val; }
    const T* operator->() const                             { return &_val; }
    T* operator->()                                         { return &_val; }
    operator const T&() const                               { return _val; }
    operator T&()                                           { return _val; }

private:
    T _val;
};

/// Integer property
template<> inline auto Property<int>::s_type() -> const TypeInfo&       { static TypeInfo _("int"); return _; }
/// Real property
template<> inline auto Property<Real>::s_type() -> const TypeInfo&      { static TypeInfo _("Real"); return _; }
/// String property
template<> inline auto Property<String>::s_type() -> const TypeInfo&    { static TypeInfo _("String"); return _; }

}

