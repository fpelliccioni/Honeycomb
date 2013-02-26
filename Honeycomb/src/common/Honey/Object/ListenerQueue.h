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

/// Listener that holds queued slot for delayed processing of signals.
/**
  * \ingroup Signal
  *
  * Signal args must be default constructible and assignable.
  */
class ListenerQueue : public Listener
{
public:
    typedef SharedPtr<ListenerQueue> Ptr;
    typedef SharedPtr<const ListenerQueue> ConstPtr;

    /// Construct with slot to receive `Signal` using function `F`.  The object instance and id are used together to identify this listener.
    template<class Signal, class F>
    static ListenerQueue& create(F&& f, const void* obj = nullptr, const Id& id = idnull)
    {
        return *new ListenerQueue(*new priv::SlotQueue<Signal,Signal::arity,F>(id, forward<F>(f)), obj, id);
    }
    
    /// Dispatch all signals stored in queue, clears queue when done
    void process()                                          { _slot.process(); }
    /// Remove all signals stored in queue
    void clear()                                            { _slot.clear(); }
    
private:
    template<class SlotQueue>
    ListenerQueue(SlotQueue& slot, const void* obj, const Id& id) :
        Listener(slot, obj, id), _slot(slot) {};
    
    priv::SlotQueueBase& _slot;
};

}
