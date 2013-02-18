// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/ComObject.h"
#include "Honey/Scene/Component.h"

namespace honey
{

/// Consists of SceneComponent objects.
/**
  * A scene object is set up with default components:
  * - Tree
  * - TreeIdMap::Elem
  * - Tm
  */
class SceneObject : public ComObject
{
public:
    typedef SharedPtr<SceneObject> Ptr;

    SceneObject();
    virtual ~SceneObject() {}
};

inline SceneObject& SceneComponent::obj() const     { return static_cast<SceneObject&>(Component::obj()); }

}