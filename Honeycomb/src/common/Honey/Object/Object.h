// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Memory/SharedPtr.h"
#include "Honey/String/Id.h"

namespace honey
{

/// Base class for objects
/** \ingroup Component */
class Object : public SharedObj
{
public:
    Object(const Id& id = idnull)                       : _instId(id) {}
    virtual ~Object()                                   {}

    /// Set per instance id
    virtual void setInstId(const Id& id)                { _instId = id; }
    const Id& getInstId() const                         { return _instId; }

protected:
    Id _instId;
};

}