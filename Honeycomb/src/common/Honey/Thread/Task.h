// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Pool.h"
#include "Honey/Thread/Future/PackagedTask.h"
#include "Honey/Graph/Dep.h"

namespace honey
{

///Uncomment to debug task scheduler
//#define Task_debug

class Task;
class TaskSched;
/** \cond */
//for weak ptr, is_base_of doesn't work when a class tests itself in the class definition
namespace mt { template<> struct is_base_of<SharedObj, Task> : std::true_type {}; }
/** \endcond */

/// Base class of `Task_`, can be added to scheduler.  Instances must be created through class `Task_`.
class Task : public SharedObj, thread::Pool::Task
{
    friend class TaskSched;
    friend struct mt::Funcptr<void ()>;
    
public:
    typedef SharedPtr<Task> Ptr;
    typedef function<void ()> Func;
    typedef DepNode<Task*> DepNode;
    typedef DepGraph<Task::DepNode> DepGraph;

    /// Check if task is in queue or executing
    bool active() const                             { return _state != State::idle; }
    
    /// Set id used for dependency graph and debug output.
    void setId(const Id& id)                        { assert(!_regCount, "Must unregister prior to modifying"); _depNode.setKey(id); }
    const Id& getId() const                         { return _depNode.getKey(); }
    
    /// Get dependency node.  Upstream and downstream tasks can be specified through the node.
    /**
      * Out links are 'upstream' tasks that will be completed before this one.
      * In links are 'downstream' tasks that will be completed after this one.
      */ 
    DepNode& deps()                                 { assert(!_regCount, "Must unregister prior to modifying"); return _depNode; }

    /// Get id
    operator const Id&() const                      { return getId(); }

    /// Get the current task object. Must be called from a task functor.
    static Task& current();
    
    #ifndef FINAL
        /// Log a message prepending current task info
        #define Task_log(msg)                       { Task::current().log(__FILE__, __LINE__, (msg)); }
    #else
        #define Task_log(...) {}
    #endif
    virtual void log(const String& file, int line, const String& msg) const;
    
    #ifdef Task_debug
        virtual bool logEnabled() const             { return true; }
    #else
        virtual bool logEnabled() const             { return false; }
    #endif
    
protected:
    enum class State
    {
        idle,           ///< Not active
        queued,         ///< Queued for execution
        depUpWait,      ///< Waiting for upstream tasks (dependency subgraph) to complete
        exec,           ///< Executing functor
        depDownWait     ///< Waiting for downsteam tasks (immediate dependees) to complete
    };
    
    Task(const Id& id = idnull);

    virtual void exec() = 0;
    virtual void resetFunctor() = 0;
    
    void bindDirty();
    void operator()();
    /// Clean up task after execution
    void finalize_();
    
    State           _state;
    DepNode         _depNode;
    Mutex           _lock;
    int             _regCount;
    TaskSched*      _sched;
    WeakPtr<Task>   _root;
    int             _bindId;
    bool            _bindDirty;
    int             _depUpWaitInit;
    int             _depUpWait;
    int             _depDownWaitInit;
    int             _depDownWait;
    DepGraph::Vertex* _vertex;
    bool            _onStack;
};

/// Holds a functor and dependency information, enqueue in a scheduler to run the task. \see TaskSched
template<class Result>
class Task_ : public Task
{
public:
    typedef SharedPtr<Task_> Ptr;

    Task_() {}
    /**
      * \param f        functor to execute
      * \param id       used for dependency graph and debug output
      */
    template<class Func>
    Task_(Func&& f, const Id& id = idnull)          : Task(id), _func(forward<Func>(f)) {}

    /// Get future from which delayed result can be retrieved.  The result pertains to a future enqueueing or currently active task.
    /**
      * \throws future::FutureAlreadyRetrieved      if future() has been called more than once per task execution.
      */
    Future<Result> future()                         { Mutex::Scoped _(_lock); return _func.future(); }

    /// Wrapper for Task::current()
    static Task_& current()                         { return static_cast<Task_&>(Task::current()); }

    /// Set functor to execute
    template<class Func>
    void setFunctor(Func&& f)                       { _func = PackagedTask<Result ()>(forward<Func>(f)); }

private:
    virtual void exec()                             { _func.invoke_delayedReady(); }
    //Called by finalizer, already has the lock
    virtual void resetFunctor()                     { _func.setReady(); _func.reset(); }

    PackagedTask<Result ()> _func;
};

/// Task scheduler, serializes and parallelizes task execution, given a dependency graph of tasks and a pool of threads.
/**
  * To run a task, first register it and any dependent tasks with TaskSched::reg(), then call TaskSched::enqueue(rootTask).
  *
  * To provide a custom global scheduler define `TaskSched_createSingleton` and implement TaskSched::createSingleton().
  */
class TaskSched
{
    friend class Task;
    
public:
    /// Get singleton
    static TaskSched& inst()                        { static UniquePtr<TaskSched> inst = &createSingleton(); return *inst; }

    /**
      * \param workerCount      see thread::Pool()
      * \param workerTaskMax    see thread::Pool()
      */
    TaskSched(int workerCount, int workerTaskMax);
    
    /// Register a task.  Task id must be unique.  Once registered, tasks are linked through the dependency graph by id.
    /**
      * Tasks can be registered with multiple schedulers.
      * \return     false if a task with the same id is already registered
      */
    bool reg(Task& task);
    /// Unregister a task.  Returns false if not registered.
    bool unreg(Task& task);

    /// Schedule a task for execution.  Returns false if task is already active.
    /**
      * Enqueuing a task performs a `binding`:
      * - The enqueued task becomes a `root` task, and the entire subgraph of upstream tasks (dependencies) are bound to this root.
      * - The subgraph of tasks are bound to this scheduler
      *
      * A task can be enqueued again once it is complete. Wait for completion by calling Task::future().get() or Task::future().wait().
      * Be wary of enqueueing tasks that are upstream of other currently active tasks.
      *
      * This method will error if:
      * - `task` is not registered
      * - `task` or any upstream tasks are active
      * - a cyclic dependency is detected
      */
    bool enqueue(Task& task);
    
private:
    static TaskSched& createSingleton();
    
    void bind(Task& root);
    String stackTrace();
    bool enqueue_priv(Task& task);
    
    thread::Pool        _pool;
    Mutex               _lock;
    vector<Task*>       _taskStack;
    Task::DepGraph      _depGraph;
    int                 _bindId;
};

#ifndef TaskSched_createSingleton
    /// Default implementation
    inline TaskSched& TaskSched::createSingleton()      { return *new TaskSched(2, 5); }
#endif


/** \cond */
namespace task { namespace priv
{
    /// Test task scheduler
    void test();
} }
/** \endcond */

}
