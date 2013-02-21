// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Future/Promise.h"

namespace honey
{

namespace future
{
    #define ENUM_LIST(e,_)  \
        e(_, ready)         \
        e(_, timeout)       \

    /**
      * \retval ready       the future result is ready
      * \retval timeout     timeout expired
      */
    ENUM(, Status);
    #undef ENUM_LIST

    /** \cond */
    namespace priv { class AnyWaiter; }
    /** \endcond */
}

/// Base class for Future types
class FutureBase : mt::NoCopy
{
    friend class future::priv::AnyWaiter;
public:
    /// Check if this instance has state and can be used.  State can be transferred out to another instance through move-assignment.
    bool isValid() const                                        { return stateBase(); }

    /// Wait until result is ready. \throws promise::NoState
    void wait() const                                           { wait(MonoClock::TimePoint::max); }
    /// Wait for an amount of time before giving up. \throws promise::NoState
    template<class Rep, class Period>
    future::Status wait(Duration<Rep,Period> time) const        { return wait(MonoClock::now() + time); }
    /// Wait until a certain time before giving up. \throws promise::NoState
    template<class Clock, class Dur>
    future::Status wait(TimePoint<Clock,Dur> time) const
    {
        auto state = stateBase();
        if (!state) throw_ promise::NoState();
        ConditionLock::Scoped _(state->waiters);
        return wait(*state, time);
    }

protected:
    typedef promise::priv::StateBase StateBase;

    virtual StateBase* stateBase() const = 0;

    template<class Clock, class Dur>
    future::Status wait(promise::priv::StateBase& state, TimePoint<Clock,Dur> time) const
    {
        while (!state.ready)
        {
            if (!state.waiters.wait(time)) return future::Status::timeout;
        }
        return future::Status::ready;
    }
};

template<class R> class SharedFuture;

/// Unique future, guarantees sole access to a future function result.
template<class R>
class Future : public FutureBase
{
    template<class R_> friend class Promise;
    template<class R_> friend class SharedFuture;

public:
    typedef promise::priv::State<R> State;

    Future()                                                    : _state(nullptr) {}
    Future(Future&& rhs)                                        : _state(nullptr) { operator=(forward<Future>(rhs)); }

    Future& operator=(Future&& rhs)                             { _state = move(rhs._state); return *this; }

    /// Share the future between multiple future objects. This future is rendered invalid.
    SharedFuture<R> share()                                     { return SharedFuture<R>(_state); }

    /// Get the future result, waiting if necessary. Throws any exception stored in the result.
    /**
      * \throws promise::NoState    if the result has been retrieved with get() more than once
      */
    R get();

protected:
    virtual StateBase* stateBase() const                        { return _state; }

private:
    Future(const SharedPtr<State>& state)                       : _state(state) {}

    template<class R_>
    struct getResult        { static R_&& func(const SharedPtr<State>& state) { return move(state->result); } };
    template<class R_>
    struct getResult<R_&>   { static R_& func(const SharedPtr<State>& state) { return *state->result; } };

    SharedPtr<State> _state;
};

template<class R>
inline R Future<R>::get()
{
    wait();
    R res = getResult<R>::func(_state);
    auto e = _state->e;
    _state = nullptr;
    if (e) e->raise();
    return res;
}

template<>
inline void Future<void>::get()
{
    wait();
    auto e = _state->e;
    _state = nullptr;
    if (e) e->raise();
}

}
