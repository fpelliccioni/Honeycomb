// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Object/Listener.h"
#include "Honey/Thread/Lock/Spin.h"

namespace honey
{
/** \cond */
namespace priv
{
    struct SlotQueueBase
    {
        virtual void process() = 0;
        virtual void clear() = 0;
    };

    template<class Signal, int Arity, class F> struct SlotQueue;

    /// This class determines how args are stored in queue
    template<class T>
    struct SlotQueueArg
    {
        typedef T StorageType;
        template<class T2>
        static void store(T& lhs, T2&& rhs)     { lhs = forward<T2>(rhs); }
        static T& load(T& lhs)                  { return lhs; }
    };

    /// For refs, store arg as pointer
    template<class T>
    struct SlotQueueArg<T&>
    {
        typedef T* StorageType;
        template<class T2>
        static void store(T*& lhs, T2&& rhs)    { lhs = &rhs; }
        static T& load(T* lhs)                  { return *lhs; }
    };

    #define ARG_TYPE(It)                        priv::SlotQueueArg<typename Signal::template param<It-1>::type>
    #define ARG_M(It)                           typename ARG_TYPE(It)::StorageType a##It; 
    #define ARG_STORE(It)                       ARG_TYPE(It)::store(arg.a##It, a##It); 
    #define ARG_LOAD(It)                        COMMA_IFNOT(It,1) ARG_TYPE(It)::load(arg.a##It)

    #define CLASS(It)                                                                                   \
        template<class Signal, class F> class SlotQueue<Signal,It,F>                                    \
            : public priv::SlotSignal<Signal,It>, public SlotQueueBase                                  \
        {                                                                                               \
            typedef priv::SlotSignal<Signal,It> Super;                                                  \
        public:                                                                                         \
            SlotQueue(const Id& id, F&& f)      : Super(id), _f(forward<F>(f)) {}                       \
            virtual void operator()(ITERATE_(1,It,SLOT_SIGNAL_PARAM))                                   \
            {                                                                                           \
                SpinLock::Scoped _(_lock);                                                              \
                _arg.push_back(Arg());                                                                  \
                Arg& arg = _arg.back();                                                                 \
                ITERATE_(1,It,ARG_STORE)                                                                \
            }                                                                                           \
                                                                                                        \
            virtual void process()                                                                      \
            {                                                                                           \
                SpinLock::Scoped _(_lock);                                                              \
                for (auto i : range(size(_arg)))                                                        \
                {                                                                                       \
                     Arg& arg = _arg[i];                                                                \
                    _f(ITERATE_(1,It,ARG_LOAD));                                                        \
                }                                                                                       \
                _arg.clear();                                                                           \
            }                                                                                           \
                                                                                                        \
            virtual void clear()                                                                        \
            {                                                                                           \
                SpinLock::Scoped _(_lock);                                                              \
                _arg.clear();                                                                           \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            struct Arg { ITERATE_(1,It,ARG_M) };                                                        \
                                                                                                        \
            typename mt::removeConstRef<F>::type _f; /* F is forwarded, remove any c/r for storage */   \
            vector<Arg, SmallAllocator<Arg>> _arg;                                                      \
            SpinLock _lock;                                                                             \
        };                                                                                              \

    ITERATE(0, SLOT_ARG_MAX, CLASS);
    #undef ARG_TYPE
    #undef ARG_M
    #undef ARG_STORE
    #undef ARG_LOAD
    #undef CLASS
}
/** \endcond */
//====================================================
// ListenerQueue
//====================================================

/// Listener that holds queued slots for delayed processing of signals.
/**
  * \ingroup Signal
  *
  * Signal args must be default constructible and assignable.
  * For queue storage, const and reference are automatically removed from the signal arg types.
  */
class ListenerQueue : public Listener
{
    typedef priv::SlotQueueBase SlotQueueBase;
public:
    typedef SharedPtr<ListenerQueue> Ptr;
    typedef SharedPtr<const ListenerQueue> ConstPtr;

    /// The base instance and id are used together to identify this listener
    ListenerQueue(const void* base = nullptr, const Id& id = idnull)        : Listener(base, id) {};

    /// Construct with slot and listener id
    template<class Signal, class F>
    static ListenerQueue& create(F&& f, const void* base = nullptr, const Id& id = idnull)
                                                        { auto& inst = *new ListenerQueue(base, id); inst.add<Signal>(forward<F>(f)); return inst; }

    virtual ~ListenerQueue()                            { clear(); }

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
        auto slot = new priv::SlotQueue<Signal,Signal::arity,F>(id, forward<F>(f));
        auto it = _list.insert(pos, slot);
        if (id != idnull) _map[id] = it;
        auto posQueue = _queueList.end();
        if (pos != _list.end())
        {
            auto itMap = _queueMap.find(*pos);
            assert(itMap != _queueMap.end());
            posQueue = itMap->second;
        }
        _queueMap[slot] = _queueList.insert(posQueue, slot);
        return it;
    }

    /// Remove slot at `pos`.  Returns position of the next slot.
    virtual ListIter remove(ListIter pos);
    /// Remove all slots
    virtual void clear();

    /// Dispatch all signals stored in queue, clears queue when done
    void process();
    /// Remove all signals stored in queue
    void clearQueue();

protected:
    typedef list<SlotQueueBase*, SmallAllocator<SlotQueueBase*>> QueueList;
    typedef UnorderedMap<SlotBase*, QueueList::iterator, SmallAllocator>::type QueueMap;

    /// Each slot has a corresponding queued slot, queued slot list must have same order as slot list
    QueueList _queueList;
    QueueMap _queueMap;
};

}
