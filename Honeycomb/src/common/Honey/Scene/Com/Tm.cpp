// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/Com/Tm.h"

namespace honey
{

void Tm::updateWorld(bool ancestorsUpdated, bool updateTree)
{
    //Ensure that this node and its ancestors are updated
    if (_parent && !ancestorsUpdated) _parent->com<Tm>().updateWorld();
    _world.get();
    if (!updateTree) return;
    //Update all nodes starting from first child
    auto range = obj().com<Tree>().preOrd();
    ++range.begin();
    for (auto& e : range) e->com<Tm>().updateWorld(true);
}

bool Tm::worldDirty()
{
    return _parent && _parent->com<Tm>()._worldState != _worldStateParent;
}

void Tm::worldEval(Transform& world)
{
    //A state counter is maintained so children can track changes to their parent's tm world.
    //The state counter increments endlessly, it will eventually pass integer limit and loop.
    if (_parent)
    {
        world = _parent->com<Tm>()._world.raw() * *this;
        _worldStateParent = _parent->com<Tm>()._worldState;
    }
    else
        world = *this;
    ++_worldState;
}

}
