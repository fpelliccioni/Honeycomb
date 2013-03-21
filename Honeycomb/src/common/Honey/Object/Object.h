// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/Id.h"
#include "Honey/Memory/SharedPtr.h"

namespace honey
{
    
/// Base class for objects
/** \ingroup Component */
class Object : public SharedObj
{
public:
    /// Object type info
    class TypeInfo
    {
    public:
        TypeInfo(const String& name)                    : _name(name), _id(name) {}
        virtual ~TypeInfo() {}
        
        bool operator==(const TypeInfo& rhs) const      { return _id == rhs._id; }
        bool operator!=(const TypeInfo& rhs) const      { return !operator==(rhs); }
        bool operator==(const Id& rhs) const            { return _id == rhs; }
        bool operator!=(const Id& rhs) const            { return !operator==(rhs); }
        
        /// Get type name
        const String& name() const                      { return _name; }
        /// Get type id
        const Id& id() const                            { return _id; }

        operator const Id&() const                      { return _id; }

        /// To string
        friend StringStream& operator<<(StringStream& os, const TypeInfo& val)  { return os << val.name(); }

    protected:
        String _name;
        Id _id;
    };

    Object(const Id& id = idnull)                       : _instId(id) {}
    virtual ~Object()                                   {}

    /// Set per instance id
    virtual void setInstId(const Id& id)                { _instId = id; }
    const Id& getInstId() const                         { return _instId; }

protected:
    Id _instId;
};

}