// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Future/Future.h"
#include "Honey/Thread/Future/SharedFuture.h"
#include "Honey/Thread/Pool.h"
#include "Honey/Misc/Range.h"

namespace honey { namespace future
{

//====================================================
// waitAll / waitAny
//====================================================

inline void waitAll() {}

/// Wait for all futures to be ready
template<class Future, class... Futures, typename mt::disable_if<mt::isRange<Future>::value, int>::type=0>
void waitAll(Future&& f, Futures&&... fs)                   { f.wait(); waitAll(forward<Futures>(fs)...); }

/// Wait for all futures in a range to be ready
template<class Range, typename std::enable_if<mt::isRange<Range>::value, int>::type=0>
void waitAll(Range&& range)                                 { for (auto& e : range) e.wait(); }

/** \cond */
namespace priv
{
    /// Helper to wait on multiple futures concurrently
    class waitAny : public mt::FuncptrBase
    {
    public:
        waitAny()       : td(*threadData()), readyState(nullptr) {}

        ~waitAny()
        {
            for (auto& e : td.states)
            {
                ConditionLock::Scoped _(e->waiters);
                stdutil::erase(e->onReady, FuncptrCreate(*this));
            }
            td.states.clear();
        }

        void add(const FutureBase& f)
        {
            auto& state = f.__stateBase();
            state.addOnReady(*this);
            td.states.push_back(&state);
        }
        
        int wait()
        {
            ConditionLock::Scoped _(td.cond);
            while(true)
            {
                auto it = find(td.states, [&](mt_elemOf(td.states)& e) { return e == readyState; });
                if (it != td.states.end()) return it - td.states.begin();
                td.cond.wait();
            }
        }

        void operator()(StateBase& src)
        {
            ConditionLock::Scoped _(td.cond);
            if (readyState) return;
            readyState = &src;
            td.cond.signal();
        }
        
    private:
        /// waitAny() needs state that is expensive to create,
        /// so instead of creating the state every call, each thread has its own state cache.
        struct ThreadData
        {
            vector<StateBase*> states;
            ConditionLock cond;
        };
        mt_staticObj((thread::Local<ThreadData>), threadData,);
        
        ThreadData& td;
        StateBase* readyState;
    };
}
/** \endcond */

/// Wait for any futures to be ready, returns index of ready future
template<class Future, class... Futures, typename mt::disable_if<mt::isRange<Future>::value, int>::type=0>
int waitAny(Future&& f, Futures&&... fs)
{
    priv::waitAny waiter;
    array<const FutureBase*, sizeof...(Futures)+1> futures = {&f, &fs...};
    for (auto& e : futures) waiter.add(*e);
    return waiter.wait();
}

/// Wait for any futures in a range to be ready, returns iterator to ready future
template<class Range, typename std::enable_if<mt::isRange<Range>::value, int>::type=0>
auto waitAny(Range&& range) -> mt_iterOf(range)
{
    priv::waitAny waiter;
    for (auto& e : range) waiter.add(e);
    return next(begin(range), waiter.wait());
}

//====================================================
// async
//====================================================

///Uncomment to debug async scheduler
//#define future_async_debug

/** \cond */
namespace priv
{
    struct Task : SmallAllocatorObject, thread::Pool::Task
    {
        virtual void operator()() = 0;
        
        virtual void log(const String& file, int line, const String& msg) const
        {
            int pos = file.find_last_of(String("\\/"));
            String filename = pos != String::npos ? file.substr(pos+1) : file;
            debug::print(sout() << "[async: " << std::hex << reinterpret_cast<intptr_t>(this) << std::dec << ":" << Thread::current().threadId() << ", "
                                << filename << ":" << line << "] " << msg << endl);
        }
        
        #ifdef future_async_debug
            virtual bool logEnabled() const { return true; }
        #else
            virtual bool logEnabled() const { return false; }
        #endif
    };
    
    template<class Func>
    struct Task_ : Task
    {
        Task_(Func&& f)                     : f(forward<Func>(f)) {}
        virtual void operator()()           { f(); delete_(this); }
        Func f;
    };
    
    //TODO: this is a work-around for clang's broken std::function, it can't handle move-only bind args
    template<class R, class Func, class... Args>
    struct Bind
    {
        Bind(Func&& f, Args&&... args)      : f(forward<Func>(f)), args(forward<Args>(args)...) {}
        //clang's std::function tries to copy, just move
        Bind(const Bind& rhs)               : Bind(move(const_cast<Bind&>(rhs))) {}
        Bind(Bind&&) = default;
        
        R operator()()                      { return func(typename mt::IntSeqGen<sizeof...(Args)>::type()); }
        template<int... Seq>
        R func(mt::IntSeq<Seq...>)          { return f(forward<Args>(get<Seq>(args))...); }
        
        Func f;
        tuple<Args...> args;
    };
    
    template<class Func, class... Args>
    struct Bind<void, Func, Args...>
    {
        Bind(Func&& f, Args&&... args)      : f(forward<Func>(f)), args(forward<Args>(args)...) {}
        Bind(const Bind& rhs)               : Bind(move(const_cast<Bind&>(rhs))) {}
        Bind(Bind&&) = default;
        
        void operator()()                   { func(typename mt::IntSeqGen<sizeof...(Args)>::type()); }
        template<int... Seq>
        void func(mt::IntSeq<Seq...>)       { f(forward<Args>(get<Seq>(args))...); }
        
        Func f;
        tuple<Args...> args;
    };
    
    template<class Func, class... Args, class R = typename std::result_of<Func(Args...)>::type>
    Bind<R, Func, Args...> bind(Func&& f, Args&&... args)   { return Bind<R, Func, Args...>(forward<Func>(f), forward<Args>(args)...); }
}
/** \endcond */

struct AsyncSched;
struct AsyncSched_tag {};
AsyncSched& async_createSingleton();

struct AsyncSched : thread::Pool, AsyncSched_tag
{
    AsyncSched(int workerCount, int workerTaskMax)          : thread::Pool(workerCount, workerTaskMax) {}
    
    static AsyncSched& inst()                               { static UniquePtr<AsyncSched> inst = &async_createSingleton(); return *inst; }
    
    template<class Func>
    void operator()(Func&& f)                               { enqueue(*new priv::Task_<Func>(forward<Func>(f))); }
};

#ifndef future_async_createSingleton
    /// Default implementation
    inline AsyncSched& async_createSingleton()              { return *new AsyncSched(2, 5); }
#endif
    
/// Call a function asynchronously, returns a future with the result of the function call.
template<class Sched, class Func, class... Args, typename std::enable_if<mt::is_base_of<AsyncSched_tag, Sched>::value, int>::type=0>
Future<typename std::result_of<Func(Args...)>::type>
    async(Sched&& sched, Func&& f, Args&&... args)
{
    typedef typename std::result_of<Func(Args...)>::type R;
    PackagedTask<R()> task(priv::bind(forward<Func>(f), forward<Args>(args)...), SmallAllocator<int>());
    auto future = task.future();
    sched(move(task));
    return future;
}

/// Async using global scheduler
/**
  * To provide a custom global scheduler define `future_async_createSingleton` and implement future::async_createSingleton().
  */
template<class Func, class... Args>
Future<typename std::result_of<Func(Args...)>::type>
    async(Func&& f, Args&&... args)                         { return async(AsyncSched::inst(), forward<Func>(f), forward<Args>(args)...); }
    
}

//====================================================
// then
//====================================================

template<class Subclass, class R>
template<class Sched, class Func>
auto FutureCommon<Subclass, R>::then(Sched&& sched, Func&& f) -> Future<typename std::result_of<Func(Subclass)>::type>
{
    using namespace future::priv;
    if (!subc()._state) throw_ future::NoState();
    typedef typename std::result_of<Func(Subclass)>::type R2;
    Promise<R2> promise{SmallAllocator<int>()};
    auto future = promise.future();
    
    struct onReady : mt::FuncptrBase, SmallAllocatorObject
    {
        onReady(Subclass&& cont, Promise<R2>&& promise, Sched&& sched, Func&& f) :
            cont(move(cont)), promise(move(promise)), sched(forward<Sched>(sched)), f(forward<Func>(f)) {}
        
        void operator()(StateBase& src)
        {
            if (src.ready)
            {
                PackagedTask<R2()> task(future::priv::bind(forward<Func>(this->f), move(this->cont)),
                                        SmallAllocator<int>(), move(this->promise));
                this->sched(move(task));
            }
            delete_(this);
        }
        
        Subclass cont;
        Promise<R2> promise;
        Sched sched;
        Func f;
    };

    subc()._state->addOnReady(*new onReady(move(subc()), move(promise), forward<Sched>(sched), forward<Func>(f)));
    return future;
}

template<class Subclass, class R>
template<class Func>
auto FutureCommon<Subclass, R>::then(Func&& f) -> Future<typename std::result_of<Func(Subclass)>::type>
                                                            { return then(future::AsyncSched::inst(), forward<Func>(f)); }

namespace future
{

//====================================================
// whenAll / whenAny
//====================================================

/** \cond */
namespace priv
{
    template<class Func, class Futures, int... Seq>
    void when_init(Func& func, Futures& fs, mt::IntSeq<Seq...>)
                                                            { mt_unpackEval(get<Seq>(fs).__state().addOnReady(func)); }
    
    template<class Result>
    struct whenAll_onReady
    {        
        template<class Futures, int... Seq>
        static void func(Promise<Result>& promise, Futures& fs, mt::IntSeq<Seq...>)
                                                            { promise.setValue(make_tuple(get<Seq>(fs).__state().result()...)); }
        template<class Range>
        static void func(Promise<Result>& promise, Range& range)
                                                            { Result res; for (auto& e : range) res.push_back(e.__state().result()); promise.setValue(res); }
    };
    
    template<>
    struct whenAll_onReady<void>
    {        
        template<class Futures, int... Seq>
        static void func(Promise<void>& promise, Futures&, mt::IntSeq<Seq...>)
                                                            { promise.setValue(); }
        template<class Range>
        static void func(Promise<void>& promise, Range&)
                                                            { promise.setValue(); }
    };
    
    template<class Futures, int... Seq>
    int whenAny_indexOf(StateBase& src, Futures& fs, mt::IntSeq<Seq...>)
                                                            { return mt::indexOf(&src, &get<Seq>(fs).__state()...); }
    template<class Range>
    int whenAny_indexOf(StateBase& src, Range& range)       { int i = -1; return find(range, [&](mt_elemOf(range)& e) { return ++i, &src == &e.__state(); }) != end(range) ? i : -1; }
    
    template<class Result_>
    struct whenAny_onReady
    {
        template<class Futures, int... Seq, class Result = tuple<int, Result_>>
        static void func(Promise<Result>& promise, Futures& fs, mt::IntSeq<Seq...>, int i)
                                                            { promise.setValue(make_tuple(i, mt::valAt(i, get<Seq>(fs)...).__state().result())); }
        template<class Range, class Result = tuple<typename mt::iterOf<Range>::type, Result_>>
        static void func(Promise<Result>& promise, Range& range, int i)
                                                            { auto it = next(begin(range), i); promise.setValue(make_tuple(it, it->__state().result())); }
    };
    
    template<>
    struct whenAny_onReady<void>
    {        
        template<class Futures, int... Seq>
        static void func(Promise<int>& promise, Futures&, mt::IntSeq<Seq...>, int i)
                                                            { promise.setValue(i); }
        template<class Range, class Result = typename mt::iterOf<Range>::type>
        static void func(Promise<Result>& promise, Range& range, int i)
                                                            { promise.setValue(next(begin(range), i)); }
    };
}
/** \endcond */

inline Future<tuple<>> whenAll()            { return FutureCreate(tuple<>()); }

/// Returns a future to either a tuple of the results of all futures, or the first exception thrown by the futures.
template<   class... Futures,
            class Result_ = typename std::decay<typename mt::removeRef<typename mt::typeAt<0, Futures...>::type>::type::Result>::type,
            class Result = typename std::conditional<   std::is_same<Result_, void>::value,
                                                        void,
                                                        tuple<typename std::decay<typename mt::removeRef<Futures>::type::Result>::type...>>::type>
Future<Result> whenAll(Futures&&... fs)
{
    using namespace future::priv;
    Promise<Result> promise{SmallAllocator<int>()};
    auto future = promise.future();
    
    struct onReady : mt::FuncptrBase, SmallAllocatorObject
    {
        onReady(Promise<Result>&& promise, Futures&&... fs) :
            promise(move(promise)), fs(forward<Futures>(fs)...), count(0), ready(0), max(sizeof...(fs)) {}
        
        void operator()(StateBase& src)
        {
            SpinLock::Scoped _(this->lock);
            if (src.ready && !this->promise.__state().ready)
            {
                if (src.e)
                    this->promise.setException(*src.e);
                else if (++this->ready == this->max)
                    priv::whenAll_onReady<Result>::func(this->promise, this->fs, typename mt::IntSeqGen<sizeof...(Futures)>::type());
            }
            if (++this->count == this->max) { _.unlock(); delete_(this); }
        }
        
        Promise<Result> promise;
        tuple<Futures...> fs;
        int count;
        int ready;
        int max;
        SpinLock lock;
    };

    auto& func = *new onReady(move(promise), forward<Futures>(fs)...);
    priv::when_init(func, func.fs, typename mt::IntSeqGen<sizeof...(Futures)>::type());
    return future;
}

/// whenAll() for a range of futures
template<   class Range, typename std::enable_if<mt::isRange<Range>::value, int>::type=0,
            class Result_ = typename std::decay<typename mt::elemOf<Range>::type::Result>::type,
            class Result = typename std::conditional<   std::is_same<Result_, void>::value,
                                                        void,
                                                        vector<Result_>>::type>
Future<Result> whenAll(Range&& range)
{
    using namespace future::priv;
    Promise<Result> promise{SmallAllocator<int>()};
    auto future = promise.future();
    
    struct onReady : mt::FuncptrBase, SmallAllocatorObject
    {
        onReady(Promise<Result>&& promise, Range&& range) :
            promise(move(promise)), range(forward<Range>(range)), count(0), ready(0), max(countOf(range)) {}
        
        void operator()(StateBase& src)
        {
            SpinLock::Scoped _(this->lock);
            if (src.ready && !this->promise.__state().ready)
            {
                if (src.e)
                    this->promise.setException(*src.e);
                else if (++this->ready == this->max)
                    priv::whenAll_onReady<Result>::func(this->promise, this->range);
            }
            if (++this->count == this->max) { _.unlock(); delete_(this); }
        }
        
        Promise<Result> promise;
        Range range;
        int count;
        int ready;
        int max;
        SpinLock lock;
    };

    auto& func = *new onReady(move(promise), forward<Range>(range));
    for (auto& e : func.range) e.__state().addOnReady(func);
    return future;
}

/// Returns a future to a tuple of the index of the ready future and its result
template<   class... Futures,
            class Result_ = typename std::decay<typename mt::removeRef<typename mt::typeAt<0, Futures...>::type>::type::Result>::type,
            class Result = typename std::conditional<   std::is_same<Result_, void>::value,
                                                        int,
                                                        tuple<int, Result_>>::type>
Future<Result> whenAny(Futures&&... fs)
{
    using namespace future::priv;
    Promise<Result> promise{SmallAllocator<int>()};
    auto future = promise.future();
    
    struct onReady : mt::FuncptrBase, SmallAllocatorObject
    {
        onReady(Promise<Result>&& promise, Futures&&... fs) :
            promise(move(promise)), fs(forward<Futures>(fs)...), count(0), max(sizeof...(fs)) {}
        
        void operator()(StateBase& src)
        {
            SpinLock::Scoped _(this->lock);
            if (src.ready && !this->promise.__state().ready)
            {
                if (src.e)
                    this->promise.setException(*src.e);
                else
                {
                    auto seq = typename mt::IntSeqGen<sizeof...(Futures)>::type();
                    priv::whenAny_onReady<Result_>::func(this->promise, this->fs, seq, whenAny_indexOf(src, this->fs, seq));
                }
            }
            if (++this->count == this->max) { _.unlock(); delete_(this); }
        }
        
        Promise<Result> promise;
        tuple<Futures...> fs;
        int count;
        int max;
        SpinLock lock;
    };

    auto& func = *new onReady(move(promise), forward<Futures>(fs)...);
    priv::when_init(func, func.fs, typename mt::IntSeqGen<sizeof...(Futures)>::type());
    return future;
}

/// whenAny() for a range of futures
template<   class Range, typename std::enable_if<mt::isRange<Range>::value, int>::type=0,
            class Result_ = typename std::decay<typename mt::elemOf<Range>::type::Result>::type,
            class Result = typename std::conditional<   std::is_same<Result_, void>::value,
                                                        typename mt::iterOf<Range>::type,
                                                        tuple<typename mt::iterOf<Range>::type, Result_>>::type>
Future<Result> whenAny(Range&& range)
{
    using namespace future::priv;
    Promise<Result> promise{SmallAllocator<int>()};
    auto future = promise.future();
    
    struct onReady : mt::FuncptrBase, SmallAllocatorObject
    {
        onReady(Promise<Result>&& promise, Range&& range) :
            promise(move(promise)), range(forward<Range>(range)), count(0), max(countOf(range)) {}
        
        void operator()(StateBase& src)
        {
            SpinLock::Scoped _(this->lock);
            if (src.ready && !this->promise.__state().ready)
            {
                if (src.e)
                    this->promise.setException(*src.e);
                else
                    priv::whenAny_onReady<Result_>::func(this->promise, this->range, whenAny_indexOf(src, this->range));
            }
            if (++this->count == this->max) { _.unlock(); delete_(this); }
        }
        
        Promise<Result> promise;
        Range range;
        int count;
        int max;
        SpinLock lock;
    };

    auto& func = *new onReady(move(promise), forward<Range>(range));
    for (auto& e : func.range) e.__state().addOnReady(func);
    return future;
}

} }
