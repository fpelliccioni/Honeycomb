// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Com/CullVol.h"
#include "Honey/Thread/Lock/Spin.h"

namespace honey
{

/// Base class for space components
class SceneSpace : public SceneComponent
{
public:
    COMPONENT(SceneSpace)

    /// Add object to this space. Object will be added as child of space and inherit this space's transform.
    virtual void add(SceneObject& obj) = 0;
    /// Remove object from this space.
    virtual void remove(SceneObject& obj) = 0;

    /// Visitor functor for enum function.
    class EnumVisitor
    {
    public:
        #define ENUM_LIST(e,_)  \
            e(_, cont)          \
            e(_, stop)          \
            e(_, skipChildren)  \

        /**
          * \retval cont            continue enumeration
          * \retval stop            stop enumeration
          * \retval skipChildren    skip enumeration of children
          */
        ENUM(SceneSpace::EnumVisitor, State);
        #undef ENUM_LIST

        EnumVisitor()                       : _bv(nullptr), _state(State::cont) {}

        /// Visit object
        virtual void operator()(const SceneSpace& space, SceneObject& obj) = 0;

        /// Set bounding volume used in enumeration
        void setBv(const BoundVolBase* bv)  { _bv = bv; }
        const BoundVolBase* getBv() const   { return _bv; }

        /// Set state of enumeration (continue/stop enumerating)
        void setState(State state)          { _state = state; }
        State getState() const              { return _state; }

    private:
        const BoundVolBase* _bv;
        State _state;
    };

    /// Enumerate objects within world space bounding volume.  If `bv` is `optnull` then all objects will be enumerated.
    /**
      * Enumerated object world transforms will be updated.
      * Method will recurse into any spaces found in enumerated objects.
      */ 
    virtual void enume(EnumVisitor& visitor, option<const BoundVolBase&> bv = optnull) const = 0;
};
COMPONENT_REG(SceneSpace)

}
