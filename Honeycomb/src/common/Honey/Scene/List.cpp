// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/List.h"

namespace honey
{

SceneList::SceneList()
{
    add(*new Scene(), "Main");
}

SceneList::~SceneList()
{
    deleteRange(_list);
}

}
