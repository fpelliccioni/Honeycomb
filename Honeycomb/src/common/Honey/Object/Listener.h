// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/Signal.h"

namespace honey
{

/// Holds a list of slots that can receive signals
/** \ingroup Signal */
class Listener : public SharedObj, public SmallAllocatorObject
{
public:
    typedef SharedPtr<Listener> Ptr;
    typedef SharedPtr<const Listener> ConstPtr;
    typedef list<SlotBase*, SmallAllocator<SlotBase*>> List;
    typedef List::const_iterator ListIter;

    /// The base instance and id are used together to identify this listener
    Listener(const void* base = nullptr, const Id& id = idnull) :
        SharedObj([&](void* p) { operator delete(p); }),
        _base(base), _id(id) {};

    /// Construct with slot and listener id
    template<class Signal, class F>
    static Listener& create(F&& f, const void* base = nullptr, const Id& id = idnull)
                                                        { auto& inst = *new Listener(base, id); inst.add<Signal>(forward<F>(f)); return inst; }

    virtual ~Listener()                                 { clear(); }

    /// Add slot to receive `Signal` using function F. An id may be specified for slot lookup.  Returns position of new slot.
    template<class Signal, class F>
    ListIter add(F&& f, const Id& id = idnull)          { return insert<Signal>(_list.end(), forward<F>(f), id); }

    /// Set slot at `pos` to receive `Signal` using function F. If id is not specified then existing id will be used.  Returns position of new slot.
    template<class Signal, class F>
    ListIter set(ListIter pos, F&& f, option<const Id&> id = optnull)
    {
        Id slotId = id ? *id : (*pos)->id();
        return insert<Signal>(remove(pos), forward<F>(f), slotId);
    }

    /// Insert slot before `pos` to receive `Signal` using function F. An id may be specified for slot lookup.  Returns position of new slot.
    template<class Signal, class F>
    ListIter insert(ListIter pos, F&& f, const Id& id = idnull)
    {
        auto it = _list.insert(pos, new priv::Slot<Signal,Signal::arity,F>(id, forward<F>(f)));
        if (id != idnull) _map[id] = it;
        return it;
    }

    /// Remove slot at `pos`.  Returns position of the next slot.
    virtual ListIter remove(ListIter pos);
    /// Remove all slots
    virtual void clear();

    /// Get all slots
    const List& slots() const                           { return _list; }
    /// Get slot position by id.  Returns end of list if not found.
    ListIter slot(const Id& id) const                   { auto it = _map.find(id); return it != _map.end() ? it->second : _list.end(); }

    /// Get base instance for listener identification
    const void* base() const                            { return _base; }
    /// Get listener id
    const Id& id() const                                { return _id; }

protected:
    typedef UnorderedMap<Id, List::iterator, SmallAllocator>::type Map;

    const void* _base;
    Id _id;
    List _list;
    Map _map;
};

}
