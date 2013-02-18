// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/Scene.h"
#include "Honey/Scene/List.h"
#include "Honey/Scene/Com/TreeSpace.h"
#include "Honey/Scene/Com/TreeIdMap.h"
#include "Honey/Scene/Com/Light.h"

namespace honey
{

Scene::Scene()
{
    setCamera(_defCamera);

    _defRoot.addCom(new TreeIdMap);
    _defRoot.addCom(new TreeSpace);
    _defRoot.addCom(new LightDir);
    _defRoot.com<LightDir>().setDir(Vec3(1,-1,-1));
    _defRoot.com<Light>().setAmbient(Vec3(0.2));
    setRoot(_defRoot);
}

void Scene::setInstId(const Id& id)
{
    SceneList::inst().updateMap(*this, id);
    Object::setInstId(id);
}

}