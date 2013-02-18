// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/Com/CullVol.h"

namespace honey
{

void CullBoxGen::update(bool updateTree)
{
    if (!updateTree)
    {
        //Update just this node
        update_priv();
        return;
    }
    //Cull volume generation requires children to be updated before parents
    for (auto& e : obj().com<Tree>().preOrdR())
    {
        auto& obj = **e;
        if (!obj.hasCom<CullVolBase>()) obj.addCom(new CullBoxGen);
        if (!obj.hasCom<CullBoxGen>()) continue;
        obj.com<CullBoxGen>().update_priv();
    }
}

void CullBoxGen::update_priv()
{
    Box shape;
    _cull = false;

    for (Tree::Node& e : obj().com<Tree>().children())
    {
        auto& obj = **e;
        if (!obj.hasCom<CullVolBase>() || !obj.com<CullVolBase>().getCullEnable()) continue;

        //Transform child bv into our local space
        BoundVolAny bvTm;
        obj.com<CullVolBase>().bv().clone(bvTm);
        bvTm.mul(obj.com<Tm>());

        //Convert to box
        Box box = bvTm.toBox();
        if (!_cull)
        {
            //First box found, use it and enable culling
            shape = box;
            _cull = true;
        }
        else
            //Extent current box
            shape.extend(box);
    }

    if (_cull)
        setShape(shape);
}

}
