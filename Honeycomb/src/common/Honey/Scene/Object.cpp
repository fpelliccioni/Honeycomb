// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/Object.h"
#include "Honey/Scene/Com/Tree.h"
#include "Honey/Scene/Com/Tm.h"
#include "Honey/Scene/Com/TreeIdMap.h"

namespace honey
{

SceneObject::SceneObject()
{
    //Create default components
    addCom(new Tree);
    addCom(new TreeIdMap::Elem);
    addCom(new Tm);
}

}