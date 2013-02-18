// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Clock.h"
#include "Honey/Thread/Lock/platform/Mutex.h"

namespace honey
{

template<class Lockable> class UniqueLock;

/// A thread lock where the lock is acquired by suspending thread execution until it becomes available.
/**
  * Use when threads are expected to acquire the lock for a long time before releasing. \n
  * The lock is non-recursive: a thread can only acquire the lock once, a second attempt without unlocking first will deadlock.
  */
class Mutex : private platform::Mutex
{
    typedef platform::Mutex Super;
    friend class Super;
public:
    typedef Super::Handle Handle;
    typedef UniqueLock<Mutex> Scoped;

    Mutex()                                         : Super(true) {}
    /// Can't copy, silently inits to default
    Mutex(const Mutex&)                             : Super(true) {}

    ~Mutex()                                        {}

    /// Can't copy, silently does nothing
    Mutex& operator=(const Mutex&)                  { return *this; }

    /// Acquire the lock.  Thread suspends until lock becomes available.
    void lock()                                     { Super::lock(); }
    /// Release the lock.
    void unlock()                                   { Super::unlock(); }

    /// Attempt to acquire the lock, returns immediately.  Returns true if the lock was acquired, false otherwise.
    bool tryLock()                                  { return Super::tryLock(); }

    /// Attempt to acquire the lock for an amount of time.  Returns true if the lock was acquired, false if timed out.
    template<class Rep, class Period>
    bool tryLock(Duration<Rep,Period> time)         { return Super::tryLock(time); }

    /// Attempt to acquire the lock until a certain time.  Returns true if the lock was acquired, false if timed out.
    template<class Clock, class Dur>
    bool tryLock(TimePoint<Clock,Dur> time)         { return Super::tryLock(time); }

    /// Get platform handle
    Handle& handle()                                { return Super::handle(); }

private:
    /// Create a non-timed basic mutex
    Mutex(mt::tag<0>)                               : Super(false) {}
};

}
