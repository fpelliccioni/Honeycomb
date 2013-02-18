// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/Component.h"
#include "Honey/Misc/Enum.h"

namespace honey
{

class SceneObject;

/// Scene component. Component that can be added to a scene object.
class SceneComponent : public Component
{
public:
    inline SceneObject& obj() const;
};

}