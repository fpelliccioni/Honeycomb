// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/Com/NspTreeSpace.h"

namespace honey
{

template<class NspTree>
void NspTreeSpace<NspTree>::enume(EnumVisitor& visitor, option<const BoundVolBase&> bv) const
{
    //Init visitor
    visitor.setBv(bv.ptr());
    visitor.setState(EnumVisitor::State::cont);

    //Make sure bv's in tree are up-to-date
    const_cast<NspTreeSpace*>(this)->update();

    //Enum root object
    enumObject(visitor, obj());
    if (visitor.getState() != EnumVisitor::State::cont) return;

    Box box;
    if (visitor.getBv())
    {
        //Transform world bv into tree local space
        BoundVolAny bvTm;
        visitor.getBv()->clone(bvTm);
        bvTm.mul(obj().template com<Tm>().world(true).inverse());
        box = bvTm.toBox();
    }
    else
        //No bv, just use tree bounds to get all objects
        box = _tree.bounds();
    //Enum children
    SharedMutex::SharedScoped _(_treeLock);
    NspTreeEnum treeVisitor(*this, visitor);
    _tree.enume(treeVisitor, box);
}

template class NspTreeSpace<BspTree>;
template class NspTreeSpace<QuadTree>;
template class NspTreeSpace<OctTree>;

}
