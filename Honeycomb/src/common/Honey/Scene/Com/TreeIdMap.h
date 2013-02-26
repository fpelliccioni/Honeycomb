// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Com/Tree.h"

namespace honey
{

/// Provides a fast way to retrieve objects in a tree by instance id.
/**
  * A TreeIdMap does not track the sub-tree of a descendant that is itself a TreeIdMap.
  *
  * When added to a tree (through Tree::addChild() or Tree::setParent()),
  * an object with a TreeIdMap::Elem component will add itself to its nearest ancestor map (TreeIdMap).
  *
  * Descendants of the object are ignored (not added/removed) for performance,
  * so call buildMap afterwards if an accurate map of the tree is desired.
  */
class TreeIdMap : public SceneComponent
{
public:
    typedef std::unordered_multimap<Id, SceneObject*> Map;
    typedef Map::const_iterator MapIter;
    COMPONENT(TreeIdMap)

    /// Element that tracks its Id in the nearest TreeIdMap ancestor.
    class Elem : public SceneComponent
    {
    public:
        COMPONENT(Elem, "TreeIdMap::Elem")

    private:
        static ComRegistry::DepNode createTypeDep()             { ComRegistry::DepNode node; node.add(Tree::s_comType()); return node; }

        virtual void onComInsert()
        {
            obj().listeners().add(Listener::create<SceneObject::sigSetInstId>(bind(
                [&](const Id& id)
                {
                    //Update in our map
                    if (obj().hasCom<TreeIdMap>()) obj().com<TreeIdMap>().update(*this, id);
                    //Update in parent map
                    TreeIdMap* map = idMap();
                    if (map) map->update(*this, id);
                }
                ,_2), this));

            obj().com<Tree>().listeners().add(Listener::create<Tree::sigSetParent>(bind(
                [&](Tree::Node* parent)
                {
                    //Technically we should move the entire tree of ids to the new map
                    //but for performance we will only move this node's id to the new parent

                    //Remove from old parent
                    TreeIdMap* map = idMap();
                    if (map) map->remove(*this);
                    //Add to new parent
                    map = idMap(parent);
                    if (map) map->add(*this);
                }
                ,_2), this));

            //Add to our map
            if (obj().hasCom<TreeIdMap>()) obj().com<TreeIdMap>().add(*this);
            //Add to parent map
            TreeIdMap* map = idMap();
            if (map) map->add(*this);
        }

        virtual void onComRemove()
        {
            obj().listeners().remove(this);
            obj().com<Tree>().listeners().remove(this);

            //Remove from our map
            if (obj().hasCom<TreeIdMap>()) obj().com<TreeIdMap>().remove(*this);
            //Remove from parent map
            TreeIdMap* map = idMap();
            if (map) map->remove(*this);
        }

        /// Get the nearest ancestor TreeIdMap
        TreeIdMap* idMap()                                      { return idMap(obj().com<Tree>().getParent()); }

        /// Get the nearest ancestor TreeIdMap to parent
        TreeIdMap* idMap(Tree::Node* parent)
        {
            for (; parent && !(*parent)->hasCom<TreeIdMap>(); parent = (*parent)->com<Tree>().getParent());
            return parent ? &(*parent)->com<TreeIdMap>() : nullptr;
        }
    };

    /// Build id map.
    void buildMap()
    {
        _map.clear();

        //Add all descendants that are elements.
        auto range = obj().com<Tree>().preOrd();
        for (auto it = begin(range); it != end(range); ++it)
        {
            auto& obj = ***it;
            //Add to map. Ignore null ids.
            if (obj.hasCom<Elem>() && obj.getInstId() != idnull) add(obj.com<Elem>());
            //Skip descendants of objects that have a TreeIdMap
            if (obj.hasCom<TreeIdMap>() && &obj != &this->obj()) it.skipChildren();
        }
    }

    /// Get object in tree with instance id.  Returns null if not found.
    SceneObject* object(const Id& id) const
    {
        auto it = _map.find(id);
        return it != _map.end() ? it->second : nullptr;
    }

    /// Get all objects in tree with matching instance id
    /**
      * The object (SceneObject*) is stored at MapIter->second
      */
    Range_<MapIter, MapIter> objects(const Id& id) const        { return range(_map.equal_range(id)); }

protected:
    static ComRegistry::DepNode createTypeDep()                 { ComRegistry::DepNode node; node.add(Tree::s_comType()); return node; }

    void add(Elem& elem)
    {
        SpinLock::Scoped _(_lock);
        //Ignore null ids
        if (elem.obj().getInstId() == idnull) return;
        _map.insert(make_pair(elem.obj().getInstId(), &elem.obj()));
    }

    void remove(Elem& elem)
    {
        SpinLock::Scoped _(_lock);
        auto it = stdutil::find(_map, elem.obj().getInstId(), &elem.obj());
        if (it == _map.end()) return;
        _map.erase(it);
    }

    void update(Elem& elem, const Id& id)
    {
        SpinLock::Scoped _(_lock);
        remove(elem);
        //Ignore null ids
        if (id == idnull) return;
        _map.insert(make_pair(id, &elem.obj()));
    }

    virtual void onComInsert()                                  { buildMap(); }
    virtual void onComRemove()                                  { _map.clear(); }

private:

    Map         _map;
    SpinLock    _lock;
};
COMPONENT_REG(TreeIdMap)
COMPONENT_REG(TreeIdMap::Elem)

}
