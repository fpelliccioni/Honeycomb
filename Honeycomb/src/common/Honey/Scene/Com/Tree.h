// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Object.h"
#include "Honey/Graph/Tree.h"

namespace honey
{

/// Tree component
class Tree : public SceneComponent, public TreeNode<SceneObject*>
{
public:
    COMPONENT(Tree)
    typedef TreeNode<SceneObject*> Node;

    Tree()                                                      : _cb(*this) { listeners().setCallback(&_cb); }
    Tree& operator=(const Tree& rhs)                            { Node::operator=(rhs); return *this; }
    ListenerList& listeners()                                   { return SceneComponent::listeners(); }

private:
    /// A Tree listener must receive messages from the Component and the TreeNode
    struct Callback : ListenerList::Callback
    {
        Callback(Tree& tree)                                    : _tree(tree) {}
        virtual void onAdd(const Listener& listener)            { _tree.Node::listeners().add(listener); }
        virtual void onRemove(const Listener& listener)         { _tree.Node::listeners().remove(listener); }
        Tree& _tree;
    };

    /// Disallow setting key directly.  Users must call setInstId instead.
    void setKey(const Id& key)                                  { Node::setKey(key); }

    virtual void onComInsert()
    {
        setData(&obj());
        //Ignore null ids for performance
        if (obj().getInstId() != idnull) setKey(obj().getInstId());
        obj().listeners().add(Listener::create<SceneObject::sigSetInstId>(bind(&Tree::setKey,this,_2), this));
    }

    virtual void onComRemove()
    {
        setParent(nullptr);
        clearChildren();
        setData(nullptr);
        obj().listeners().remove(this);
    }

    Callback _cb;
};
COMPONENT_REG(Tree)

}
