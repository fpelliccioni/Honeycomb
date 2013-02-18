// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Object/ListenerQueue.h"

namespace honey
{

ListenerQueue::ListIter ListenerQueue::remove(ListIter pos)
{
    auto it = _queueMap.find(*pos);
    assert(it != _queueMap.end());
    _queueList.erase(it->second);
    _queueMap.erase(it);
    return Listener::remove(pos);
}

void ListenerQueue::clear()
{
    Listener::clear();
    _queueList.clear();
    _queueMap.clear();
}

void ListenerQueue::process()
{
    for (auto& e : _queueList) { e->process(); }
}

void ListenerQueue::clearQueue()
{
    for (auto& e : _queueList) { e->clear(); }
}

}