// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/CameraPersp.h"
#include "Honey/Scene/Object.h"

namespace honey
{

/// Provides high-level scene control.
/**
  * The scene is set up with default objects (ex. camera, root object). \n
  * The default root object is set up with these components:
  * - TreeIdMap
  * - TreeSpace
  * - LightDir, ambient light
  */
class Scene : public Object
{
public:
    Scene();

    /// Override from object, updates scene manager map
    virtual void setInstId(const Id& id);

    /// Set reference to camera
    void setCamera(option<Camera&> camera)      { _camera = camera.ptr(); }
    Camera& getCamera() const                   { assert(_camera); return *_camera; }
    bool hasCamera() const                      { return _camera; }

    /// Set reference to root scene object
    void setRoot(option<SceneObject&> obj)      { _root = obj.ptr(); }
    SceneObject& getRoot() const                { assert(_root); return *_root; }
    bool hasRoot() const                        { return _root; }

private:
    Camera*        _camera;
    SceneObject*   _root;

    /// Defaults
    CameraPersp    _defCamera;
    SceneObject    _defRoot;
};

}
