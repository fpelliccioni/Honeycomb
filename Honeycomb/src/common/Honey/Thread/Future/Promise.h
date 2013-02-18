// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Exception.h"
#include "Honey/Thread/Condition/Lock.h"

namespace honey
{

/// Promise util
namespace promise
{
    /// Exceptions
    struct Broken : Exception                                   { EXCEPTION(Broken) };
    struct FutureAlreadyRetrieved : Exception                   { EXCEPTION(FutureAlreadyRetrieved) };
    struct AlreadySatisfied : Exception                         { EXCEPTION(AlreadySatisfied) };
    struct NoState : Exception                                  { EXCEPTION(NoState) };

    /** \cond */
    namespace priv
    {
        template<class R> struct State;   

        /// Invoke function with args and set result into state
        template<class R>
        struct invoke
        {
            typedef State<R> State;

            template<class F, class... Args>
            void operator()(State& state, bool setReady, F&& f, Args&&... args)
            {
                try { state.setValue(f(forward<Args>(args)...), setReady); }
                //TODO: msvc bug, replace with ellipsis
                catch (std::exception&) { state.setException(Exception::current(), setReady); }
            }
        };

        /// Void result
        template<>
        struct invoke<void>
        {
            typedef State<void> State;

            template<class F, class... Args>
            void operator()(State& state, bool setReady, F&& f, Args&&... args)
            {
                try { f(forward<Args>(args)...); state.setValue(setReady); }
                //TODO: msvc bug, replace with ellipsis
                catch (std::exception&) { state.setException(Exception::current(), setReady); }
            }
        };

        struct StateBase : SharedObj
        {
            StateBase()                                             : e(nullptr), ready(false), futureRetrieved(false) {}

            void setException(const Exception& e_, bool setReady)
            {
                ConditionLock::Scoped _(waiters);
                if (ready) throw_ AlreadySatisfied();
                e = &e_;
                if (setReady) setReady_();
            }

            void setReady()                                         { ConditionLock::Scoped _(waiters); setReady_(); }

            Exception::ConstPtr e;
            bool ready;
            bool futureRetrieved;
            ConditionLock waiters;
            vector<ConditionLock*> externalWaiters;

        protected:
            void setReady_()
            {
                ready = true;
                waiters.broadcast();
                for (auto& e : externalWaiters) { ConditionLock::Scoped _(*e); e->broadcast(); }
            }
        };

        /// State with generic result
        template<class R>
        struct State : StateBase
        {
            State()                                                 : StateBase() {}

            template<class T>
            void setValue(T&& val, bool setReady)
            {
                ConditionLock::Scoped _(waiters);
                if (ready) throw_ AlreadySatisfied();
                result = forward<T>(val);
                if (setReady) setReady_();
            }

            R result;
        };

        /// State with reference result
        template<class R>
        struct State<R&> : StateBase
        {
            State()                                                 : StateBase(), result(nullptr) {}

            template<class T>
            void setValue(T& val, bool setReady)
            {
                ConditionLock::Scoped _(waiters);
                if (ready) throw_ AlreadySatisfied();
                result = &val;
                if (setReady) setReady_();
            }

            R* result;
        };

        /// State with void result
        template<>
        struct State<void> : StateBase
        {
            State()                                                 : StateBase() {}

            void setValue(bool setReady)
            {
                ConditionLock::Scoped _(waiters);
                if (ready) throw_ AlreadySatisfied();
                if (setReady) setReady_();
            }
        };
    }
    /** \endcond */
}

template<class R> class Future;

/// Container to hold a delayed function result
/**
  * A promise must be fulfilled before being destroyed, otherwise its future will return the exception promise::Broken.
  */
template<class R>
class Promise : mt::NoCopy
{
    template<class R> friend class Future;
    template<class Sig> friend class PackagedTask;
public:
    typedef promise::priv::State<R> State;

    Promise()                                                   : _state(new State) {}
    
    /// Construct with allocator
    template<class Alloc_>
    Promise(Alloc_&& a_)
    {
        typedef mt::removeRef<Alloc_>::Type::rebind<State>::other Alloc;
        Alloc a(a_);
        _state.set(new (a.allocate(1)) State, honey::finalize<State, Alloc>(a), a);
    }
    
    Promise(Promise&& rhs)                                      : _state(nullptr) { operator=(move(rhs)); }
    ~Promise()                                                  { finalize(); }

    Promise& operator=(Promise&& rhs)                           { finalize(); _state = move(rhs._state); return *this; }

    /// Get future from which delayed result can be retrieved.
    /**
      * \throws promise::FutureAlreadyRetrieved     if future() has been called more than once.
      * \throws promise::NoState                    if invalid
      */ 
    Future<R> future()
    {
        if (!isValid()) throw_ promise::NoState();
        ConditionLock::Scoped _(_state->waiters);
        if (_state->futureRetrieved) throw_ promise::FutureAlreadyRetrieved();
        _state->futureRetrieved = true;
        return Future<R>(_state);
    }

    /// Set stored result, value is copied
    /**
      * \throws promise::AlreadySatisfied   if a result has already been set
      * \throws promise::NoState            if invalid
      */ 
    template<class T>
    typename mt::disable_if<mt::True<T>::value && (mt::isRef<R>::value || std::is_void<R>::value)>::type
        setValue(T&& result)                                    { if (!isValid()) throw_ promise::NoState(); _state->setValue(forward<T>(result), true); }
    /// Set stored result for ref result type
    template<class T>
    typename std::enable_if<mt::True<T>::value && mt::isRef<R>::value>::type
        setValue(T& result)                                     { if (!isValid()) throw_ promise::NoState(); _state->setValue(result, true); }
    /// Set stored result for void result type
    void setValue()                                             { static_assert(false, "Only for use with void type"); }

    /// Set stored exception. Exception must be heap allocated.
    /**
      * \throws promise::AlreadySatisfied   if a result has already been set
      * \throws promise::NoState            if invalid
      */ 
    void setException(const Exception& e)                       { if (!isValid()) throw_ promise::NoState(); _state->setException(e, true); }

    /// Check if this instance has state and can be used.  State can be transferred out to another instance through move-assignment.
    bool isValid() const                                        { return _state; }

private:
    void finalize()                                             { if (isValid() && !_state->ready) setException(*new promise::Broken()); }

    SharedPtr<State> _state;
};

template<> void Promise<void>::setValue()                       { if (!isValid()) throw_ promise::NoState(); _state->setValue(true); }

}
