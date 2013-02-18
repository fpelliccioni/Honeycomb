// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Thread.h"
#include "Honey/Thread/Future/PackagedTask.h"
#include "Honey/Graph/Dep.h"

namespace honey
{

/// Base class of `Task_`, can be added to scheduler.  Instances must be created through class `Task_`.
class Task : public SharedObj, mt::NoCopy
{
    friend class TaskSched;
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
        void log(const String& file, int line, const String& msg);
    #else
        #define Task_log(...) {}
    #endif

protected:
    #define ENUM_LIST(e,_)      \
        e(_, idle)              \
        e(_, queued)            \
        e(_, depUpWait)         \
        e(_, exec)              \
        e(_, depDownWait)       \

    /**
      * \retval idle            Not active
      * \retval queued          Queued for execution
      * \retval depUpWait       Waiting for upstream tasks (dependency subgraph) to complete
      * \retval exec            Executing functor
      * \retval depDownWait     Waiting for downsteam tasks (immediate dependees) to complete
      */
    ENUM(Task, State);
    #undef ENUM_LIST
    
    Task(const Id& id = idnull);

    virtual void operator()() = 0;
    virtual void resetFunctor() = 0;
    
    void bindDirty();

    State _state;
    DepNode _depNode;
    Mutex _lock;
    int _regCount;
    TaskSched* _sched;
    WeakPtr<Task> _root;
    int _bindId;
    bool _bindDirty;
    int _depUpWaitInit;
    int _depUpWait;
    int _depDownWaitInit;
    int _depDownWait;
    DepGraph::Vertex* _vertex;
    bool _onStack;
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
      * \throws promise::FutureAlreadyRetrieved     if future() has been called more than once per task execution.
      */
    Future<Result> future()                         { Mutex::Scoped _(_lock); return _func.future(); }

    /// Wrapper for Task::current()
    static Task_& current()                         { return static_cast<Task_&>(Task::current()); }

    /// Set functor to execute
    template<class Func>
    void setFunctor(Func&& f)                       { _func = PackagedTask(forward<Func>(f)); }

private:
    virtual void operator()()                       { _func.invoke_delayedReady(); }
    //Called by finalizer, already has the lock
    virtual void resetFunctor()                     { _func.setReady(); _func.reset(); }

    PackagedTask<Result ()> _func;
};

/// Task scheduler, serializes and parallelizes task execution, given a dependency graph of tasks and a pool of threads.
/**
  * To run a task, first register it and any dependent tasks with TaskSched::reg(), then call TaskSched::enqueue(rootTask).
  */
class TaskSched
{
    friend class Task;
public:
    /**
      * \param workerCount      Number of workers
      * \param workerTaskMax    Max size of per-worker task queue, overflow will be pushed onto scheduler queue
      */
    TaskSched(int workerCount = 3, int workerTaskMax = 5);
    ~TaskSched();
    
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
    class Worker
    {
        friend class TaskSched;
    public:
        Worker(TaskSched& sched);
        
        /// Get the current worker, call from inside a task
        static Worker& current()                    { assert(*_current); return **_current; }
        /// Get the current task object. Returns null if worker has not been assigned a task.
        Task* task() const                          { return _task; }
        
    private:
        void start();
        void join();
        void run();
        
        /// Get next task
        Task::Ptr next();
        /// Clean up task after execution
        void finalize(Task& task);
        
        TaskSched& _sched;
        Thread _thread;
        bool _active;
        ConditionLock _cond;
        bool _condWait;
        deque<Task::Ptr> _tasks;
        Task::Ptr _task;
        static thread::Local<Worker*> _current;
    };
    
    void bind(Task& root);
    String stackTrace();
    bool enqueue_priv(Task& task);
    
    int _workerTaskMax;
    Mutex _lock;
    vector<UniquePtr<Worker>> _workers;
    deque<Task::Ptr> _tasks;
    vector<Task*> _taskStack;
    Task::DepGraph _depGraph;
    int _bindId;
};

/** \cond */
namespace task { namespace priv
{
    /// Test task scheduler
    void test();
} }
/** \endcond */

}
