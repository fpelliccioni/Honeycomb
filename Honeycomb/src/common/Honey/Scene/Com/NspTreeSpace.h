// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Com/TreeSpace.h"
#include "Honey/Graph/NspTree.h"
#include "Honey/Thread/Lock/Shared.h"

namespace honey
{


/// Space divided by N-dimensional spatial partitioning tree.
/**
  * Only the objects added through add() are directly tracked in the space tree (their descendants are ignored).
  * When an object is enumerated, its descendants are also enumerated using a TreeSpace.
  * Any spaces found in the descendants are recursively enumerated.
  *
  * Objects added to the space automatically track changes to their transform and cull volume.
  * If there are any changes then they will update their location in the space tree on the next Enum call.
  *
  * All operations are thread-safe.
  *
  * \see NspTree for more info about spatial tree.
  * \see BspTreeSpace, QuadTreeSpace, OctTreeSpace
  */
template<class NspTree>
class NspTreeSpace : public TreeSpace
{
    friend class ComRegistry;
public:
    class Data : public SceneComponent, public NspTree::template Data<SceneObject*>
    {
        friend class NspTreeSpace;
    public:
        COMPONENT(Data, sout() << "NspTreeSpace<" << NspTree::dim << ">::Data")

    private:
        typedef vector<Data*> List;

        Data(NspTreeSpace& space)                           : NspTree::Data<SceneObject*>(space._tree.concurMax(), nullptr), space(space), dirty(false) {}

        void update()
        {
            //Remove from tree
            space._tree.remove(*this);

            //Transform bv into tree space
            BoundVolAny bvTm;
            obj().com<CullVolBase>().bv().clone(bvTm);
            bvTm.mul(obj().com<Tm>());
            box = bvTm.toBox();

            //Re-insert into tree
            space._tree.add(*this);
            dirty = false;
        }

        static ComRegistry::DepNode createTypeDep()         { ComRegistry::DepNode node; node.add(CullVolBase::s_comType()); return node; }

        virtual void onComInsert()
        {
            data = &obj();
            obj().com<Tm>().listeners().add(Listener::create<Tm::sigTmChange>(bind(&Data::onChange,this), this));
            obj().com<CullVolBase>().listeners().add(Listener::create<CullVolBase::sigShapeChange>(bind(&Data::onChange,this), this));
            onChange();
        }

        virtual void onComRemove()
        {
            SharedMutex::Scoped _(space._treeLock);
            SpinLock::Scoped __(space._updateLock);
            space._tree.remove(*this);
            if (dirty) stdutil::erase(space._updateList, this);
            dirty = false;
            obj().com<Tm>().listeners().removeAll(this);
            obj().com<CullVolBase>().listeners().removeAll(this);
        }

        void onChange()
        {
            if (dirty) return;
            dirty = true;
            SpinLock::Scoped _(space._updateLock);
            space._updateList.push_back(this);
        }

        NspTreeSpace& space;
        bool dirty;
    };

    /// \see NspTree::NspTree() for param info 
    NspTreeSpace(   const Box& treeBox = Box(Vec3(-1000), Vec3(1000)),
                    int depthMax = 5, int concurMax = 6) :
        _tree(treeBox, depthMax, concurMax) {}

    /// Object must have a culling volume
    virtual void add(SceneObject& obj)
    {
        TreeSpace::add(obj);
        assert(!obj.hasCom<Data>());
        obj.addCom(new Data(*this));
    }

    virtual void remove(SceneObject& obj)
    {
        TreeSpace::remove(obj);
        obj.removeCom<Data>();
    }

    virtual void enume(EnumVisitor& visitor, option<const BoundVolBase&> bv = optnull) const;

private:
    struct NspTreeEnum : public NspTree::EnumVisitor
    {
        NspTreeEnum(const NspTreeSpace& space, typename NspTreeSpace::EnumVisitor& spaceVisitor) :
            space(space),
            spaceVisitor(spaceVisitor)
                                                            {}
        
        virtual void operator()(const typename NspTree::DataBase* data)
        {
            //Found an object, enumerate its descendants
            space.enumTree(spaceVisitor, *static_cast<const Data*>(data)->data);
            if (spaceVisitor.getState() == NspTreeSpace::EnumVisitor::State::stop)
                setState(State::stop);
        }

        const NspTreeSpace& space;
        typename NspTreeSpace::EnumVisitor& spaceVisitor;
    };

    static ComRegistry::DepNode createTypeDep()         { ComRegistry::DepNode node = TreeSpace::createTypeDep(); node.add(CullVol<Box>::s_comType()); return node; }

    virtual void onComInsert()                          { obj().com<CullVol<Box>>().setShape(_tree.bounds()); }
    virtual void onComRemove() {}

    void update()
    {
        //Update any changed bounding volumes in the tree
        if (_updateList.empty()) return;
        SharedMutex::Scoped _(_treeLock);
        SpinLock::Scoped __(_updateLock);
        for (auto& e : _updateList) { e->update(); }
        _updateList.clear();
    }

    NspTree             _tree;
    mutable SharedMutex _treeLock;
    typename Data::List _updateList;
    SpinLock            _updateLock;
};

/// Space from a BspTree
class BspTreeSpace : public NspTreeSpace<BspTree>
{
public:
    COMPONENT_SUB(TreeSpace, BspTreeSpace)

    BspTreeSpace() {}
    BspTreeSpace(const Box& treeBox, int depthMax = 5, int concurMax = 6)
        : NspTreeSpace(treeBox,depthMax,concurMax) {}
};
COMPONENT_REG(BspTreeSpace)
COMPONENT_REG(BspTreeSpace::Data)

/// Space from a QuadTree
class QuadTreeSpace : public NspTreeSpace<QuadTree>
{
public:
    COMPONENT_SUB(TreeSpace, QuadTreeSpace)

    QuadTreeSpace() {}
    QuadTreeSpace(const Box& treeBox, int depthMax = 4, int concurMax = 6)
        : NspTreeSpace(treeBox,depthMax,concurMax) {}
};
COMPONENT_REG(QuadTreeSpace)
COMPONENT_REG(QuadTreeSpace::Data)

/// Space from a OctTree
class OctTreeSpace : public NspTreeSpace<OctTree>
{
public:
    COMPONENT_SUB(TreeSpace, OctTreeSpace)

    OctTreeSpace() {}
    OctTreeSpace(const Box& treeBox, int depthMax = 3, int concurMax = 6)
        : NspTreeSpace(treeBox,depthMax,concurMax) {}
};
COMPONENT_REG(OctTreeSpace)
COMPONENT_REG(OctTreeSpace::Data)

}
