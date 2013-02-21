// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/Object.h"
#include "Honey/Object/Property.h"
#include "Honey/Memory/SmallAllocator.h"

namespace honey
{
/** \cond */
namespace property { namespace priv
{
    /// Called by PropertyObject to create a property for type T. May be specialized for a type.
    template<class T> static Property<T>& create(const Id& id)          { return *new Property<T>(id); }
} }
/** \endcond */

/// Object that contains properties
/** \ingroup Component */
class PropertyObject : public Object
{
public:
    typedef UnorderedMap<Id, PropertyBase::Ptr, SmallAllocator>::type PropertyMap;

    PropertyObject() {}

    /// Releases all contained properties
    virtual ~PropertyObject() {}

    /// Add a property reference. Any existing property with the same id will be released and replaced.
    /**
      * Properties contained by this object will be released upon object destruction.
      */ 
    void addProp(PropertyBase& prop)
    {
        assert(prop.id() != idnull, "Property must have valid id");
        PropertyBase::Ptr ptr = &prop;
        auto res = _propMap.insert(make_pair(prop.id(), ptr));
        if (res.second) return;
        res.first->second = ptr;
    }

    /// Wrapper for pointer arg
    void addProp(PropertyBase* prop)                                    { assert(prop); addProp(*prop); }

    /// Check if object contains property with id
    bool hasProp(const Id& id) const                                    { return _propMap.find(id) != _propMap.end(); }

    /// Check if object contains property with id and type T
    template<class T>
    bool hasProp(const Id& id) const
    {
        auto it = _propMap.find(id);
        return it != _propMap.end() && it->second->type() == Property<T>::s_type();
    }

    /// Get property with id of type T.  Adds property if it doesn't exist.
    template<class T>
    Property<T>& getProp(const Id& id)
    {
        PropertyBase* prop;
        auto it = _propMap.find(id);
        if (it != _propMap.end())
            prop = it->second;
        else
        {
            //Property doesn't exist, add it
            prop = &property::priv::create<T>(id);
            addProp(*prop);
        }
        assert(prop->type() == Property<T>::s_type(),
                    sout()  << "Component type mismatch: "
                            << "Request: " << Property<T>::s_type() << " ; Id: " << id
                            << " ; Found: " << prop->type());
        return static_cast<Property<T>&>(*prop);
    }

    /// Get property with id
    PropertyBase& getProp(const Id& id) const
    {
        auto it = _propMap.find(id);
        assert(it != _propMap.end(), sout() << "Property not found. Id: " << id);
        return *it->second;
    }

    /// Get property with id of type T
    template<class T>
    Property<T>& getProp(const Id& id) const
    {
        auto it = _propMap.find(id);
        assert(it != _propMap.end(), sout() << "Property not found. Id: " << id);
        PropertyBase* prop = it->second;
        assert(prop->type() == Property<T>::s_type(),
                    sout()  << "Component type mismatch: "
                            << "Request: " << Property<T>::s_type() << " ; Id: " << id
                            << " ; Found: " << prop->type());
        return static_cast<Property<T>&>(*prop);
    }

    /// Get all properties
    const PropertyMap& getProps() const                                 { return _propMap; }

    /// Assign property with prop's id to prop's value.  Adds property if it doesn't exist.  Returns assigned property.
    template<class T>
    Property<T>& setProp(const Property<T>& prop)                       { return getProp<T>(prop.id()) = prop; }
    /// Assign property with id to prop's value.  Adds property if it doesn't exist.  Returns assigned property.
    template<class T>
    Property<T>& setProp(const Id& id, const Property<T>& prop)         { return getProp<T>(id) = prop; }
    /// Assign property with id to value.  Adds property if it doesn't exist.  Returns assigned property.
    template<class T>
    Property<T>& setProp(const Id& id, const T& val)                    { return getProp<T>(id) = val; }

    /// Shorthand for getProp
    template<class T>
    Property<T>& prop(const Id& id)                                     { return getProp<T>(id); }
    /// Shorthand for getProp
    template<class T>
    Property<T>& prop(const Id& id) const                               { return getProp<T>(id); }
    /// Shorthand for setProp
    template<class T>
    Property<T>& prop(const Property<T>& prop)                          { return setProp<T>(prop); }
    /// Shorthand for setProp
    template<class T>
    Property<T>& prop(const Id& id, const Property<T>& prop)            { return setProp<T>(id, prop); }
    /// Shorthand for setProp
    template<class T>
    Property<T>& prop(const Id& id, const T& val)                       { return setProp<T>(id, val); }

    /// Remove a single property. Returns property if found and removed.
    PropertyBase::Ptr removeProp(PropertyBase& prop)                    { return removeProp(prop.id()); }

    /// Remove a single property with id. Returns property if found and removed.
    PropertyBase::Ptr removeProp(const Id& id)
    {
        auto it = _propMap.find(id);
        return it != _propMap.end() ? removeProp(it) : nullptr;
    }

    /// Remove all properties. Calls functor for each removed property.
    void removeProps(const function<void (PropertyBase&)>& f = [](PropertyBase&){})
    {
        while (!_propMap.empty()) { f(*removeProp(_propMap.begin())); }
    }

private:

    /// Remove a property
    PropertyBase::Ptr removeProp(PropertyMap::iterator it)
    {
        PropertyBase::Ptr prop = it->second;
        _propMap.erase(it);
        return prop;
    }

    PropertyMap _propMap;
};

}

