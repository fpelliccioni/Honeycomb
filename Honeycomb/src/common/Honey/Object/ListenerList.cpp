// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Object/ListenerList.h"

namespace honey
{

void ListenerList::add(const Listener& listener)
{
    SpinLock::Scoped _(_lock);

    //Add listener to base map
    _baseMap.insert(make_pair(listener.base(), &listener));

    //Add listener slots to signal map
    auto& slots = listener.slots();
    for (auto& slot : slots)
    {
        auto& index = _signalMap[slot->signalId()];
        index.list.push_back(slot);
        index.map.insert(make_pair(slot, --index.list.end()));
    }

    if (_cb) _cb->onAdd(listener);
}

void ListenerList::remove(const Listener& listener)
{
    SpinLock::Scoped _(_lock);
    Listener::ConstPtr __ = &listener;  //Prevent destruction in scope

    //Remove listener from base map
    auto itMap = stdutil::find(_baseMap, listener.base(), &listener);
    if (itMap != _baseMap.end()) _baseMap.erase(itMap);

    //Remove listener slots from signal map
    auto& slots = listener.slots();
    for (auto& slot : slots)
    {
        auto itMap = _signalMap.find(slot->signalId());
        if (itMap == _signalMap.end()) continue;
        auto& index = itMap->second;
        auto itIndex = index.map.find(slot);
        if (itIndex == index.map.end()) continue;
        index.list.erase(itIndex->second);
        index.map.erase(itIndex);
        if (index.list.empty()) _signalMap.erase(itMap);
    }

    if (_cb) _cb->onRemove(listener);
}

void ListenerList::removeAll(const void* base)
{
    SpinLock::Scoped _(_lock);
    auto itPair = _baseMap.equal_range(base);
    for (auto it = itPair.first; it != itPair.second;) remove(*it++->second);
}

void ListenerList::removeAll(const void* base, const Id& id)
{
    SpinLock::Scoped _(_lock);
    auto itPair = _baseMap.equal_range(base);
    for (auto it = itPair.first; it != itPair.second;)
    {
        auto& listener = it++->second;
        if (listener->id() != id) continue;
        remove(*listener);
    }
}

void ListenerList::clear()
{
    SpinLock::Scoped _(_lock);
    while (!_baseMap.empty()) remove(*_baseMap.begin()->second);
}

const ListenerList::SlotList* ListenerList::slotList(const Id& signalId) const
{
    auto itMap = _signalMap.find(signalId);
    if (itMap == _signalMap.end()) return nullptr;
    return &itMap->second.list;
}

}
