// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Scene.h"

namespace honey
{

/// List of scenes
class SceneList
{
    friend class Scene;
public:
    ~SceneList();

    /// Get singleton
    mt_staticObj(SceneList, inst,);

    /// Add scene to list, becomes owner of scene object
    void add(Scene& scene, const Id& id)        { insert(scene, id, _list.size()); }

    /// Insert scene into index
    void insert(Scene& scene, const Id& id, int index)
    {
        assert(!contains(id));
        scene.setInstId(id);
        _list.insert(_list.begin() + index, &scene);
        _map[scene.getInstId()] = &scene;
    }

    /// Remove scene from list by id. Does not delete scene
    void remove(const Id& id)
    {
        Map::iterator it = _map.find(id);
        if (it == _map.end()) return;
        stdutil::erase(_list, it->second);
        _map.erase(it);
    }

    /// Remove scene from list by index. Does not delete scene
    void removeAt(int index)                    { return remove(at(index).getInstId()); }

    /// Check if list has scene
    bool contains(const Id& id) const           { return _map.find(id) != _map.end(); }

    /// Get scene by id
    Scene& get(const Id& id)
    {
        Map::iterator it = _map.find(id);
        assert(it != _map.end(), sout() << "Scene not found: " << id);
        return *it->second;
    }

    /// Get number of scenes in list
    int count() const                           { return _list.size(); }
    /// Get scene by index
    Scene& at(int index)                        { return *_list[index]; }

private:
    SceneList();

    /// Update scene id in map.  Call before changing id. 
    void updateMap(Scene& scene, const Id& id)
    {
        Map::iterator it = _map.find(scene.getInstId());
        if (it != _map.end() && it->second == &scene) _map.erase(it);
        _map[id] = &scene;
    }

    typedef unordered_map<Id, Scene*> Map;
    typedef vector<Scene*> List;

    List _list;
    Map _map;
};

}
