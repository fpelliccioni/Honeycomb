// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Future/Future.h"
#include "Honey/Misc/Range.h"

namespace honey
{
/// Future util
namespace future
{

#define FUTURE_ARG_MAX 5

/// Wait for all futures in a range to be ready
template<class Range>
typename std::enable_if<mt::isRange<Range>::value, void>::type
    waitAll(Range&& range)
{
    for (auto& e : range) { e.wait(); }
}

//====================================================
#define waitAll(...) __waitAll()
/// Wait for all futures to be ready
void waitAll(Futures&...);
#undef waitAll

#define PARAMT(It)  COMMA_IFNOT(It,1) class F##It
#define PARAM(It)   COMMA_IFNOT(It,1) const F##It& f##It
#define WAIT(It)    f##It.wait(); 

#define FUNC(It)                                                                                                        \
    template<ITERATE_(1, It, PARAMT)>                                                                                   \
    typename mt::disable_if<mt::isRange<F1>::value, void>::type                                                         \
        waitAll(ITERATE_(1, It, PARAM))                                                                                 \
    {                                                                                                                   \
        ITERATE_(1, It, WAIT)                                                                                           \
    }                                                                                                                   \

ITERATE(1, FUTURE_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef WAIT
#undef FUNC
//====================================================

/** \cond */
namespace priv
{
    /// Helper to wait on multiple futures concurrently
    class AnyWaiter
    {
    public:
        AnyWaiter() {}

        ~AnyWaiter()
        {
            auto& td = *threadData();
            for (auto& e : td.states)
            {
                ConditionLock::Scoped _(e->waiters);
                stdutil::erase(e->externalWaiters, &td.cond);
            }
            td.states.clear();
        }

        void add(const FutureBase& future)
        {
            auto& td = *threadData();
            auto state = future.stateBase();
            assert(state);
            ConditionLock::Scoped _(state->waiters);
            state->externalWaiters.push_back(&td.cond);
            td.states.push_back(state);
        }

        int wait()
        {
            auto& td = *threadData();
            ConditionLock::Scoped _(td.cond);
            while(true)
            {
                auto it = find(td.states, [&](elemtype(td.states)& e) { return e->ready; });
                if (it != td.states.end()) return it - td.states.begin();
                td.cond.wait();
            }
        }

    private:
        /// The WaitAny function needs state that is expensive to create,
        /// so instead of creating the state every call, each thread has its own state cache.
        struct ThreadData
        {
            vector<promise::priv::StateBase*> states;
            ConditionLock cond;
        };

        mt_staticObj((thread::Local<ThreadData>), threadData,);
    };
}
/** \endcond */

/// Wait for any futures in a range to be ready, returns iterator to ready future
template<class Range>
auto waitAny(Range&& range) ->
    typename std::enable_if<mt::isRange<Range>::value, itertype(range)>::type
{
    priv::AnyWaiter waiter;
    for (auto& e : range) { waiter.add(e); }
    return next(begin(range), waiter.wait());
}

//====================================================
#define waitAny(...) __waitAny()
/// Wait for any futures to be ready, returns index of ready future
int waitAny(Futures&...);
#undef waitAny

#define PARAMT(It)  COMMA_IFNOT(It,1) class F##It
#define PARAM(It)   COMMA_IFNOT(It,1) const F##It& f##It
#define WAIT(It)    waiter.add(f##It); 

#define FUNC(It)                                                                                                        \
    template<ITERATE_(1, It, PARAMT)>                                                                                   \
    typename mt::disable_if<mt::isRange<F1>::value, int>::type                                                          \
        waitAny(ITERATE_(1, It, PARAM))                                                                                 \
    {                                                                                                                   \
        priv::AnyWaiter waiter;                                                                                         \
        ITERATE_(1, It, WAIT)                                                                                           \
        return waiter.wait();                                                                                           \
    }                                                                                                                   \

ITERATE(1, FUTURE_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef WAIT
#undef FUNC
//====================================================

} }
