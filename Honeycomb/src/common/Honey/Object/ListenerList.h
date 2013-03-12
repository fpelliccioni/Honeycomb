// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/Listener.h"
#include "Honey/Thread/Lock/Spin.h"

namespace honey
{

/// Collection of listeners
/**
  * \ingroup Signal
  *
  * Listeners receive signals in the same order as the listeners are added.
  */
class ListenerList
{
public:
    /// Callback to handle events from this class
    struct Callback
    {
        virtual void onAdd(const Listener& listener)        { mt_unused(listener); }
        virtual void onRemove(const Listener& listener)     { mt_unused(listener); }
    };

    typedef list<SlotBase*, SmallAllocator<SlotBase*>> SlotList;
    typedef UnorderedMultiMap<const void*, Listener::ConstPtr, SmallAllocator>::type ObjMap;

    ListenerList()                                      : _cb(nullptr) {}
    virtual ~ListenerList()                             { clear(); }

    /// Add a listener shared reference
    void add(const Listener& listener);
    /// Remove a listener
    void remove(const Listener& listener);
    /// Remove all listeners with object instance
    void remove(const void* obj);
    /// Remove all listeners with object instance and id
    void remove(const void* obj, const Id& id);
    /// Remove all listeners
    void clear();

    /// Get all listeners, ordered by object instance
    const ObjMap& list() const                          { return _objMap; }

    /// Get slots that receive signal.  Returns null if none found.
    template<class Signal>
    const SlotList* slotList() const                    { return slotList(Signal::id()); }

    /// Send a signal to all listeners
    template<class Signal, class... Args>
    void dispatch(Args&&... args) const
    {
        SpinLock::Scoped _(const_cast<ListenerList*>(this)->_lock);
        auto slots = slotList(Signal::id());
        if (!slots) return;
        for (auto& e : *slots)
            (static_cast<priv::SlotSignal<Signal,Signal::arity>&>(*e))(args...);
    }

    /// Set callback to handle events from this class
    void setCallback(Callback* cb)                      { _cb = cb; }

private:
    typedef UnorderedMultiMap<SlotBase*, SlotList::iterator, SmallAllocator>::type SlotMap;
    struct SlotIndex
    {
        SlotList list;
        SlotMap map;
    };
    typedef UnorderedMap<Id, SlotIndex, SmallAllocator>::type SignalMap;

    const SlotList* slotList(const Id& signalId) const;

    ObjMap      _objMap;
    SignalMap   _signalMap;
    SpinLock    _lock;
    Callback*   _cb;
};

}
