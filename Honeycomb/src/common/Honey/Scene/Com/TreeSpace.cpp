// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/Com/TreeSpace.h"

namespace honey
{

void TreeSpace::enume(EnumVisitor& visitor, option<const BoundVolBase&> bv) const
{
    //Init visitor
    visitor.setBv(bv.ptr());
    visitor.setState(EnumVisitor::State::cont);

    //Enum root object
    enumObject(visitor, obj());
    if (visitor.getState() != EnumVisitor::State::cont) return;

    //Enum children
    for (Tree::Node& e : obj().com<Tree>().children())
        enumTree(visitor, **e);
}

void TreeSpace::enumTree(EnumVisitor& visitor, SceneObject& obj) const
{
    auto range = obj.com<Tree>().preOrd();
    for (auto it = begin(range); it != end(range); ++it)
    {
        auto& obj = ***it;
 
        if (obj.hasCom<SceneSpace>())
        {
            //Recurse into any subspace
            obj.com<SceneSpace>().enume(visitor, visitor.getBv());

            if (visitor.getState() == EnumVisitor::State::stop) return;
            visitor.setState(EnumVisitor::State::cont);
            it.skipChildren();
            continue;
        }

        //Enum this object
        enumObject(visitor, obj);

        if (visitor.getState() == EnumVisitor::State::stop) return;
        if (visitor.getState() == EnumVisitor::State::skipChildren)
        {
            visitor.setState(EnumVisitor::State::cont);
            it.skipChildren();
        }
    }
}

void TreeSpace::enumObject(EnumVisitor& visitor, SceneObject& obj) const
{
    if (obj.hasCom<Tm>()) obj.com<Tm>().updateWorld(true);

    //Perform culling
    if (visitor.getBv())
    {
        if (cull(obj, *visitor.getBv()))
        {
            visitor.setState(EnumVisitor::State::skipChildren);
            return;
        }
    }

    //Visit object
    visitor(*this, obj);
}

bool TreeSpace::cull(SceneObject& obj, const BoundVolBase& bv) const
{
    if (!obj.hasCom<CullVolBase>() || !obj.com<CullVolBase>().getCullEnable()) return false;
    return !bv.test(obj.com<CullVolBase>().bvWorld(true));
}

}