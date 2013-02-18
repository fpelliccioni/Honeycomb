// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Object/Listener.h"

namespace honey
{

Listener::ListIter Listener::remove(ListIter pos)
{
    auto it = _map.find((*pos)->id());
    if (it != _map.end()) _map.erase(it);
    delete_(*pos);
    return _list.erase(pos);
}

void Listener::clear()
{
    deleteRange(_list);
    _list.clear();
    _map.clear();
}

}