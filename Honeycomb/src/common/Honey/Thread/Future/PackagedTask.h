// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Future/Future.h"

namespace honey
{

/// A container that wraps a function so that its result is stored in a future when invoked
template<class Sig> class PackagedTask;

template<class R, class... Args>
class PackagedTask<R (Args...)> : mt::NoCopy
{
public:
    PackagedTask()                                  : _func(nullptr), _invoked(false) {}
    template<class F>
    PackagedTask(F&& f)                             : _func(forward<F>(f)), _invoked(false) {}
    template<class F, class Alloc>
    PackagedTask(F&& f, Alloc&& alloc)              : _func(std::allocator_arg_t(), alloc, forward<F>(f)), _promise(alloc), _invoked(false) {}
    PackagedTask(PackagedTask&& rhs)                : _func(move(rhs._func)), _promise(move(rhs._promise)), _invoked(rhs._invoked) {}

    PackagedTask& operator=(PackagedTask&& rhs)     { _func = move(rhs._func); _promise = move(rhs._promise); _invoked = rhs._invoked; return *this; }

    /// Get future from which delayed result can be retrieved
    /**
      * \throws promise::FutureAlreadyRetrieved     if future() has been called more than once.
      */
    Future<R> future()                              { return _promise.future(); }

    /// Invoke stored function to evaluate result for associated future.
    /**
      * \throws promise::AlreadySatisfied   if the function has already been invoked
      * \throws promise::NoState            if invalid
      */
    template<class... Args>
    void operator()(Args&&... args)                 { invoke(true, forward<Args>(args)...); }
    /// Same as operator() except don't make future ready.  User is responsible to call setReady() afterwards.
    template<class... Args>
    void invoke_delayedReady(Args&&... args)        { invoke(false, forward<Args>(args)...); }

    /// Signal to future that result is ready for retrieval.  This is only needed after a call to invoke_delayedReady().
    void setReady()
    {
        if (!isValid()) throw_ promise::NoState();
        assert(_invoked && !_promise._state->ready);
        _promise._state->setReady();
    }

    /// Check if this instance has state and can be used.  State can be transferred out to another instance through move-assignment.
    bool isValid() const                            { return _promise.isValid(); }

    /// Reset the function so it can be invoked again, a new future is created for the next result
    void reset()
    {
        if (!isValid() || !_invoked) return;
        _promise = Promise<R>();
        _invoked = false;
    }

private:
    template<class... Args>
    void invoke(bool setReady, Args&&... args)
    {
        if (!isValid()) throw_ promise::NoState();
        if (_invoked) throw_ promise::AlreadySatisfied();
        _invoked = true;
        promise::priv::invoke<R>()(*_promise._state, setReady, _func, forward<Args>(args)...);
    }

    function<R (Args...)> _func;
    Promise<R> _promise;
    bool _invoked;
};

}
