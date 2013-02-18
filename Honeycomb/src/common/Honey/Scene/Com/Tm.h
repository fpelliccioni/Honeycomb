// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Com/Tree.h"
#include "Honey/Math/Alge/Transform.h"
#include "Honey/Misc/Lazy.h"

namespace honey
{

/// Transform component
/**
  * Not all objects in the scene graph require a transform component.
  * Objects without a tm component are skipped in the world tm calculations.
  */
class Tm : public SceneComponent, public Transform
{
public:
    COMPONENT(Tm)

    SIGNAL_DECL(Tm)
    /// Called on shape change
    SIGNAL(sigTmChange, (Tm& src));

    Tm() :
        _parent(nullptr),
        _world(bind_fill(&Tm::worldEval, this), bind(&Tm::worldDirty, this)),
        _worldState(1),
        _worldStateParent(0) {}

    Tm& operator=(const Transform& tm)                          { Transform::operator=(tm); return *this; }

    /// Get world transform.  Also updates and caches this node's world tm, same as updateWorld().
    /**
      * There are two ways to call world():
      * \hiddentable
      * \row Ancestors not updated (slower):    \col Node will recursively check that parent world tms are up-to-date all the way to the root. \endrow
      * \row Ancestors updated (faster):        \col Ancestors are assumed to be up-to-date, no recursive check is necessary. \n
      *                                              This method can be used during a preorder traversal if world(true) is called on every node. \endrow
      * \endtable
      */
    const Transform& world(bool ancestorsUpdated = false) const     { const_cast<Tm*>(this)->updateWorld(ancestorsUpdated); return _world.raw(); }

    /// Get state counter that is incremented every time tm world changes
    int worldState() const                                      { return _worldState; }

    /// Update world tm of this node.  If updateTree is true then entire tree of descendants will also be updated.
    void updateWorld(bool ancestorsUpdated = false, bool updateTree = false);

private:
    static ComRegistry::DepNode createTypeDep()                 { ComRegistry::DepNode node; node.add(Tree::s_comType()); return node; }

    virtual void onComInsert()
    {
        obj().com<Tree>().listeners().add(Listener::create<Tree::sigSetParent>(bind(
            [&](Tree::Node* parent)
            {
                /// Get the nearest ancestor that has a tm component (usually the immediate parent)
                for (; parent && !(*parent)->hasCom<Tm>(); parent = (*parent)->com<Tree>().getParent());
                _parent = parent ? **parent : nullptr;
                _world.setDirty(true);
            }
            , _2), this));
    }

    virtual void onComRemove()
    {
        obj().com<Tree>().listeners().removeAll(this);
    }

    virtual void onTmChange()
    {
        _world.setDirty(true);
        listeners().dispatch<sigTmChange>(*this);
    }

    bool worldDirty();
    void worldEval(Transform& world);

    SceneObject*        _parent;
    lazy<Transform>     _world;
    atomic::Var<int>    _worldState;
    atomic::Var<int>    _worldStateParent;
};
COMPONENT_REG(Tm)

}
