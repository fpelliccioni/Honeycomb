// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Com/Space.h"

namespace honey
{

/// Space that is ordered by a hierarchy of bounding volumes
/**
  * Objects can be added through this interface or through the Tree component of any children or descendants. \n
  * Objects do not generally require a transform component.  A tm is required only in certain situations like if culling is enabled. \n
  * The CullVol component is used to specify culling for an object.
  */
class TreeSpace : public SceneSpace
{
public:
    COMPONENT_SUB(SceneSpace, TreeSpace)

    /// Adds object as child. Objects without culling enabled will always be enumerated.
    virtual void add(SceneObject& obj)                  { this->obj().com<Tree>().addChild(obj.com<Tree>()); }
    virtual void remove(SceneObject& obj)               { this->obj().com<Tree>().removeChild(obj.com<Tree>()); }

    virtual void enume(EnumVisitor& visitor, option<const BoundVolBase&> bv = optnull) const;

protected:
    static ComRegistry::DepNode createTypeDep()         { ComRegistry::DepNode node; node.add(Tree::s_comType()); return node; }

    void enumTree(EnumVisitor& visitor, SceneObject& obj) const;
    void enumObject(EnumVisitor& visitor, SceneObject& obj) const;
    bool cull(SceneObject& obj, const BoundVolBase& bv) const;
};
COMPONENT_REG(TreeSpace)


}