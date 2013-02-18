// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/MtMap.h"
#include "Honey/Misc/Range.h"
#include "Honey/Thread/Lock/Unique.h"

namespace honey
{
/// Mutex lock util
namespace lock
{

/// Max args for lock related variable argument functions
#define LOCK_ARG_MAX 5

//====================================================
#define tryLock(...) __tryLock()
/// Try to lock a series of lockables. Locks either all or none.
/**
  * Returns the zero-based index of the first failed lock, or -1 if all locks were successful.
  */
int tryLock(Locks&...);
#undef tryLock

#define PARAMT(It)  COMMA_IFNOT(It,1) class L##It
#define PARAM(It)   COMMA_IFNOT(It,1) L##It& l##It
#define ARG(It)     COMMA_IFNOT(It,2) l##It

#define FUNC(It)                                                                                                        \
    template<ITERATE_(1, It, PARAMT)>                                                                                   \
    typename mt::disable_if<mt::isRange<L1>::value, int>::type                                                          \
        tryLock(ITERATE_(1, It, PARAM))                                                                                 \
    {                                                                                                                   \
        UniqueLock<L1> lock(l1, lock::Op::tryLock);                                                                     \
        if (!lock.owns()) return 0;                                                                                     \
        IFEQUAL(It, 1, , int failed = tryLock(ITERATE_(2, It, ARG)); if (failed >= 0) return failed+1; )                \
        lock.release();                                                                                                 \
        return -1;                                                                                                      \
    }                                                                                                                   \
     
ITERATE(1, LOCK_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef ARG
#undef FUNC
//====================================================

/// Try to lock a range of lockables. Locks either all or none.
/**
  * Returns an iterator to the first failed lock, or end if all locks were successful.
  */
template<class Range>
auto tryLock(Range&& range) ->
    typename std::enable_if<mt::isRange<Range>::value, typename itertype(range)>::type
{
    auto& begin = honey::begin(range);
    auto& end = honey::end(range);
    if (begin == end) return end;
    UniqueLock<typename itertype(range)::value_type> lock(*begin, lock::Op::tryLock);
    if (!lock.owns()) return begin;
    auto failed = tryLock(honey::range(next(begin),end));
    if (failed == end) lock.release();
    return failed;
}

//====================================================

/** \cond */
namespace priv
{
    #define PARAMT(It)  COMMA_IFNOT(It,1) class L##It
    #define PARAM(It)   COMMA_IFNOT(It,1) L##It& l##It
    #define ARG(It)     COMMA_IFNOT(It,2) l##It

    /// Part of lock implementation. Locks first mutex then tries to lock rest, returns failed index or -1 on success
    #define FUNC(It)                                                                                                    \
        template<ITERATE_(1, It, PARAMT)>                                                                               \
        typename mt::disable_if<mt::isRange<L1>::value, int>::type                                                      \
            lockTest(ITERATE_(1, It, PARAM))                                                                            \
        {                                                                                                               \
            UniqueLock<L1> lock(l1);                                                                                    \
            IFEQUAL(It, 1, , int failed = tryLock(ITERATE_(2, It, ARG)); if (failed >= 0) return failed+1; )            \
            lock.release();                                                                                             \
            return -1;                                                                                                  \
        }                                                                                                               \

    ITERATE(1, LOCK_ARG_MAX, FUNC)
    #undef FUNC
    mtkeygen(LockIdx);

    /// Part of lock implementation. Locks first mutex then tries to lock rest, returns failed iterator or end on success.
    template<class Range>
    auto lockTest(Range&& range) ->
        typename std::enable_if<mt::isRange<Range>::value, typename itertype(range)>::type
    {
        auto& begin = honey::begin(range);
        auto& end = honey::end(range);
        if (begin == end) return end;
        UniqueLock<typename itertype(range)::value_type> lock(*begin);
        auto failed = tryLock(honey::range(next(begin),end));
        if (failed == end) lock.release();
        return failed;
    }
}
/** \endcond */

#define lock(...) __lock()
/// Lock a series of lockables safely without deadlocking.
/** 
  * Deadlock can be avoided by waiting only for the first lock, then trying to lock the others without waiting.
  * If any of the others fail, restart and wait for a failed lock instead.
  * For example:
  *
  *     Lock L1 and then call:                          tryLock(L2,L3,L4,L5)
  *     If L2 failed then restart, lock L2 and call:    tryLock(L3,L4,L5,L1)
  */
void lock(Locks&...);
#undef lock

/// A compile-time map is used as an array so we can access the locks by index and use modular arithmetic to rotate the locks.
#define MAP_TYPE(It) COMMA_IFNOT(It,1) L##It&, priv::LockIdx<It>
#define MAP_INIT(It) COMMA_IFNOT(It,1) priv::LockIdx<It>() = l##It

/// It and Offset are both 1-based indices.  Must add and mod in 0-base, then convert back to 1-base.
#define CASE_ARG(It, Offset, Count) COMMA_IFNOT(It,1) map[priv::LockIdx<(((Offset-1)+(It-1)) % Count) + 1>()]
/// It is a 1-based index, LockTest returns a 0-based.  Must add and mod in 0-base, then convert back to 1-base.
#define CASE(It, Count)                                                                                                 \
    case It:                                                                                                            \
        if ((lockFirst = priv::lockTest( ITERATE2__(1, Count, CASE_ARG, It, Count) )) == -1) return;                    \
        lockFirst = ((lockFirst+(It-1)) % Count) + 1;                                                                   \
        break;                                                                                                          \

#define FUNC(It)                                                                                                        \
    template<ITERATE_(1, It, PARAMT)>                                                                                   \
    typename mt::disable_if<mt::isRange<L1>::value, void>::type                                                         \
        lock(ITERATE_(1, It, PARAM))                                                                                    \
    {                                                                                                                   \
        typedef typename MtMap<ITERATE_(1, It, MAP_TYPE)>::Type Map;                                                    \
        Map map = mtmap( ITERATE_(1, It, MAP_INIT) );                                                                   \
        int lockFirst = 1;                                                                                              \
        while(true) switch(lockFirst) { ITERATE1_(1, It, CASE, It) }                                                    \
    }                                                                                                                   \

ITERATE(1, LOCK_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef ARG
#undef MAP_TYPE
#undef MAP_INIT
#undef CASE_ARG
#undef CASE
#undef FUNC
//====================================================

/// Lock a range of lockables safely without deadlocking.
template<class Range>
typename std::enable_if<mt::isRange<Range>::value, void>::type
    lock(Range&& range)
{
    auto lockFirst = begin(range);
    while(true)
    {
        auto it = ringRange(range,lockFirst);
        if ((lockFirst = priv::lockTest(it)) == end(it)) break;
    }
}


//====================================================
/// Holds shared state for callOnce()
struct OnceFlag
{
    OnceFlag()          : invoked(false), done(false) {}

    bool invoked;
    bool done;
    ConditionLock cond;
};

/** \cond */
namespace priv
{
    /// Call once without function args
    inline void callOnce(OnceFlag& flag, const function<void()>& f)
    {
        ConditionLock::Scoped lock(flag.cond);
        while (!flag.done)
        {
            if (!flag.invoked)
            {
                flag.invoked = true;
                try
                {
                    auto lockGuard = ScopeGuard(lock::lockGuard(lock));
                    lock.unlock();
                    f();
                    lockGuard.release();
                    lock.lock();
                    flag.done = true;
                    flag.cond.broadcast();
                }
                catch(...) { flag.invoked = false; flag.cond.broadcast(); throw; }
            }
            else
            {
                while (!flag.done && flag.invoked) { flag.cond.wait(); }
            }
        }
    }
}
/** \endcond */

#define callOnce(...) __callOnce()
/// Thread-safe way to make sure a function is called only once, useful for singletons and global initializers.
/**
  * One thread is active while all other threads wait for the result.
  * All callers are synchronized on exit.
  * If an exception is thrown, one of the waiting threads will take over as the active thread and try again.
  *
  * \param flag  OnceFlag shared state. All callOnce() calls with the same state are part of the same group that is limited to one function invocation.
  */
void callOnce(OnceFlag& flag, Func&&, Args&&...);
#undef callOnce

#define PARAMT(It)  , class Arg##It
#define PARAM(It)   , Arg##It&& arg##It
#define ARG(It)     , forward<Arg##It>(arg##It)

#define FUNC(It)                                                                                                        \
    template<class F ITERATE_(1,It,PARAMT)>                                                                             \
    void callOnce(OnceFlag& flag, F&& f ITERATE_(1,It,PARAM))                                                           \
    {                                                                                                                   \
        if (flag.done) return;                                                                                          \
        priv::callOnce(flag, bind(forward<F>(f) ITERATE_(1,It,ARG)));                                                   \
    }                                                                                                                   \

ITERATE(0, LOCK_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef ARG
#undef FUNC
//====================================================

} }
