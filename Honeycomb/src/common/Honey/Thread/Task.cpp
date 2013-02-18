// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Thread/Task.h"

namespace honey
{

///Uncomment to debug task scheduler
//#define Task_debug

#ifdef Task_debug
    #define Task_log_(task, msg)    { (task).log(__FILE__, __LINE__, (msg)); }
#else
    #define Task_log_(...) {}
#endif

Task::Task(const Id& id) :
    _state(State::idle),
    _depNode(this, id),
    _regCount(0),
    _sched(nullptr),
    _root(nullptr),
    _bindId(0),
    _bindDirty(true),
    _depUpWaitInit(0),
    _depUpWait(0),
    _depDownWaitInit(0),
    _depDownWait(0),
    _vertex(nullptr),
    _onStack(false) {}

Task& Task::current()
{
    Task* task = TaskSched::Worker::current().task();
    assert(task, "No active task in current thread, this method can only be called from a task functor");
    return *task;
}

void Task::bindDirty()
{
    //If we are part of the root's binding, inform root that its subgraph is now dirty 
    auto root = _root.lock();
    if (root && _sched == root->_sched && _bindId == root->_bindId)
        root->_bindDirty = true;
}

#ifndef FINAL
    void Task::log(const String& file, int line, const String& msg)
    {
        int pos = file.find_last_of(String("\\/"));
        String filename = pos != String::npos ? file.substr(pos+1) : file;
        Debug::print(StringStream() << "[Task: " << getId() << ":" << Thread::current().threadId() << ", "
                                    << filename << ":" << line << "] " << msg << endl);
    }
#endif

TaskSched::TaskSched(int workerCount, int workerTaskMax) :
    _workerTaskMax(workerTaskMax),
    _bindId(0)
{       
    for (auto i: range(workerCount)) { _workers.push_back(new Worker(*this)); mt_unused(i); }
    for (auto& e: _workers) e->start();
}

TaskSched::~TaskSched()
{
    for (auto& e: _workers) e->join();
}

bool TaskSched::reg(Task& task)
{
    if (_depGraph.vertex(task) || !_depGraph.add(task._depNode)) return false;
    ++task._regCount;
    //Structural change, must dirty newly linked tasks
    auto vertex = _depGraph.vertex(task);
    assert(vertex);
    for (auto i: range(Task::DepNode::DepType::valMax))
    {
        for (auto& v: vertex->links(Task::DepNode::DepType(i)))
        {
            if (!v->nodes().size()) continue;
            Task& e = ****v->nodes().begin();
            if (e._sched == this) e.bindDirty();
        }
    }
    return true;
}

bool TaskSched::unreg(Task& task)
{
    if (!_depGraph.remove(task._depNode)) return false;
    --task._regCount;
    //Structural change, must dirty task root
    if (task._sched == this)
    {
        task.bindDirty();
        task._sched = nullptr;
        task._root = nullptr;
    }
    return true;
}

void TaskSched::bind(Task& root)
{
    //Binding is a pre-calculation step to optimize worker runtime, we want to re-use these results across multiple enqueues.
    //The root must be dirtied if the structure of its subgraph changes
    Mutex::Scoped _(_lock);
    Task_log_(root, "Binding root and its upstream");
    //Cache the vertex for each task
    root._vertex = _depGraph.vertex(root);
    assert(root._vertex, "Bind failed: task must be registered before binding");
    //The bind id gives us a way to uniquely identify all tasks upstream of root, this is critical when workers are returning downstream.
    ++_bindId;

    _taskStack.clear();
    _taskStack.push_back(&root);
    while (!_taskStack.empty())
    {
        Task& task = *_taskStack.back();
        
        //If already visited
        if (task._sched == this && task._bindId == _bindId)
        {
            //We are referenced by another downstream neighbour
            ++task._depDownWaitInit;
            task._depDownWait = task._depDownWaitInit;
            task._onStack = false;
            _taskStack.pop_back();
            continue;
        }
        
        //Not visited, bind task
        task.bindDirty();
        task._sched = this;
        task._root = Task::Ptr(&root);
        task._bindId = _bindId;
        task._bindDirty = false;
        task._depDownWaitInit = 0;
        task._depDownWait = task._depDownWaitInit;
        task._onStack = true;

        #ifdef DEBUG
        //Validate upstream tasks
        for (auto& vertex: task._vertex->links())
        {
            if (!vertex->nodes().size()) continue;
            Task& e = ****vertex->nodes().begin();
            if (e.active())
            {
                error(StringStream()
                    << "Bind failed: Upstream task already active. "
                    << "Task: " << e.getId() << "; Task's root: " << (e._root.lock() ? e._root.lock()->getId() : idnull) << endl
                    << "Task stack:" << endl << stackTrace());
            }
            
            if (e._onStack)
            {
                error(StringStream()
                    << "Bind failed: Upstream cyclic dependency detected. "
                    << "From task: " << task.getId() << "; To task: " << e.getId() << endl
                    << "Task stack:" << endl << stackTrace());
            }
        }
        #endif
        
        //Bind upstream tasks. Reverse order so that first link is on top of stack.
        task._depUpWaitInit = 0; //count upstream tasks
        for (auto& vertex: reversed(task._vertex->links()))
        {
            if (!vertex->nodes().size()) continue;
            Task& e = ****vertex->nodes().begin();
            e._vertex = vertex;
            _taskStack.push_back(&e);
            ++task._depUpWaitInit;
        }
        task._depUpWait = task._depUpWaitInit;
    }
}

String TaskSched::stackTrace()
{
    unordered_set<Task*> unique;
    int count = 0;
    StringStream os;
    for (auto& e: reversed(_taskStack))
    {
        if (!e->_onStack || !unique.insert(e).second) continue;
        os << count++ << ". " << e->getId() << endl;
    }
    return os;
}

bool TaskSched::enqueue(Task& task)
{
    if (task.active()) return false;
    if (task._sched != this || task._root.lock() != &task || task._bindDirty)
        bind(task);
    return enqueue_priv(task);
}

bool TaskSched::enqueue_priv(Task& task)
{
    {
        Mutex::Scoped _(task._lock);
        switch (task._state)
        {
            case Task::State::idle:
                task._state = Task::State::queued;
                break;
            case Task::State::depUpWait:
                if (task._depUpWait > 0) return false;
                task._state = Task::State::queued;
                break;
            default:
                return false;
        }
    }
    
    //Find smallest worker queue
    int minSize = _workerTaskMax;
    int minIndex = -1;
    for (auto i: range(size(_workers)))
    {
        auto& worker = *_workers[i];
        if (size(worker._tasks) >= minSize) continue;
        minSize = size(worker._tasks);
        minIndex = i;
    }
    
    bool added = false;
    if (minIndex >= 0)
    {
        //Push to worker queue
        do
        {
            Worker& worker = *_workers[minIndex];
            ConditionLock::Scoped _(worker._cond);
            if (size(worker._tasks) >= _workerTaskMax) break;
            
            added = true;
            worker._tasks.push_back(&task);
            Task_log_(task, StringStream()  << "Pushed to worker queue: " << worker._thread.threadId()
                                            << "; Queue size: " << worker._tasks.size());
        } while(false);
    }
    
    if (!added)
    {
        //All worker queues full, push to scheduler queue
        Mutex::Scoped _(_lock);
        _tasks.push_back(&task);
        Task_log_(task, StringStream() << "Pushed to scheduler queue. Queue size: " << _tasks.size());
    }
    
    //Find a waiting worker and signal it, start search at min index
    int first = minIndex >= 0 ? minIndex : 0;
    for (auto i: range(size(_workers)))
    {
        Worker& worker = *_workers[(first + i) % size(_workers)];
        ConditionLock::Scoped _(worker._cond);
        if (!worker._condWait) continue;
        worker._condWait = false;
        worker._cond.signal();
        break;
    }
    
    return true;
}


thread::Local<TaskSched::Worker*> TaskSched::Worker::_current;

TaskSched::Worker::Worker(TaskSched& sched) :
    _sched(sched),
    _thread(honey::bind(&Worker::run, this)),
    _active(false),
    _condWait(false),
    _task(nullptr)
{
}

void TaskSched::Worker::start()
{
    _thread.start();
    //Synchronize with thread
    while (!_active) { ConditionLock::Scoped _(_cond); }
}

void TaskSched::Worker::join()
{
    {
        ConditionLock::Scoped _(_cond);
        _active = false;
        _condWait = false;
        _cond.signal();
    }
    _thread.join();
}

void TaskSched::Worker::run()
{
    {
        ConditionLock::Scoped _(_cond);
        _current = this;
        _active = true;
        _condWait = true;
    }
    
    while (_active)
    {
        while ((_task = next()) != nullptr)
        {            
            //Enqueue upstream tasks
            for (auto& vertex: _task->_vertex->links())
            {
                if (!vertex->nodes().size()) continue;
                _sched.enqueue_priv(****vertex->nodes().begin());
            }
            
            {
                Mutex::Scoped _(_task->_lock);
                //If there is an upstream task then we must wait to start
                if (_task->_depUpWait > 0)
                {
                    _task->_state = Task::State::depUpWait;
                    Task_log_(*_task, StringStream() << "Waiting for upstream. Wait task count: " << _task->_depUpWait);
                    continue;
                }
                assert(!_task->_depUpWait, "Task state corrupt");
                _task->_state = Task::State::exec;
                Task_log_(*_task, "Executing");
            }
            
            (*_task)();
            
            //Finalize any upstream tasks that are waiting
            for (auto& vertex: _task->_vertex->links())
            {
                if (!vertex->nodes().size()) continue;
                Task& e = ****vertex->nodes().begin();
                Mutex::Scoped _(e._lock);
                if (--e._depDownWait > 0) continue;
                finalize(e);
            }
            
            //Re-enqueue any downstream tasks that are waiting
            for (auto& vertex: _task->_vertex->links(Task::DepNode::DepType::in))
            {
                if (!vertex->nodes().size()) continue;
                Task& e = ****vertex->nodes().begin();
                if (e._sched != _task->_sched || e._bindId != _task->_bindId) continue; //This task is not upstream of root
                {
                    Mutex::Scoped _(e._lock);
                    if (--e._depUpWait > 0) continue;
                    if (e._state != Task::State::depUpWait) continue;
                }
                _sched.enqueue_priv(e);
            }
            
            {
                Mutex::Scoped _(_task->_lock);
                //Root task must finalize itself
                if (_task == _task->_root.lock())
                {
                    --_task->_depDownWait;
                    finalize(*_task);
                    continue;
                }
                //If we haven't been finalized yet then we must wait for downstream to finalize us
                if (_task->_state != Task::State::idle)
                {
                    _task->_state = Task::State::depDownWait;
                    Task_log_(*_task, StringStream() << "Waiting for downstream. Wait task count: " << _task->_depDownWait);
                    continue;
                }
            }
        }
        
        //Wait for signal from sched that a task has been queued
        ConditionLock::Scoped _(_cond);
        while (_condWait) _cond.wait();
        _condWait = true;
    }
}

Task::Ptr TaskSched::Worker::next()
{
    //Try to pop from our queue
    {
        ConditionLock::Scoped _(_cond);
        if (_tasks.size())
        {
            Task::Ptr task = move(_tasks.front());
            _tasks.pop_front();
            Task_log_(*task, StringStream() << "Popped from worker queue. Queue size: " << _tasks.size());
            return task;
        }
    }
    
    //Find largest other worker queue
    int maxSize = 0;
    int maxIndex = -1;
    for (auto i: range(size(_sched._workers)))
    {
        Worker& worker = *_sched._workers[i];
        if (size(worker._tasks) <= maxSize) continue;
        maxSize = size(worker._tasks);
        maxIndex = i;
    }
    
    if (maxIndex >= 0)
    {
        //Steal from other worker queue
        do
        {
            Worker& worker = *_sched._workers[maxIndex];
            ConditionLock::Scoped _(worker._cond);
            if (!worker._tasks.size()) break;
            
            Task::Ptr task = move(worker._tasks.front());
            worker._tasks.pop_front();
            Task_log_(*task, StringStream() << "Stolen from worker queue: " << worker._thread.threadId()
                                            << "; Queue size: " << worker._tasks.size());
            return task;
        } while(false);
    }
        
    //Pop task from scheduler queue
    if (_sched._tasks.size())
    {
        do
        {
            Mutex::Scoped _(_sched._lock);
            if (!_sched._tasks.size()) break;
        
            Task::Ptr task = move(_sched._tasks.front());
            _sched._tasks.pop_front();
            Task_log_(*task, StringStream() << "Popped from scheduler queue. Queue size: " << _sched._tasks.size());
            return task;
        } while (false);
    }
    
    return nullptr;
}

void TaskSched::Worker::finalize(Task& task)
{
    //Reset task to initial state
    assert(!task._depDownWait, "Task state corrupt");
    task._depUpWait = task._depUpWaitInit;
    task._depDownWait = task._depDownWaitInit;
    task._state = Task::State::idle;
    task.resetFunctor();
    Task_log_(task, "Finalized");
}

/** \cond */
namespace task { namespace priv
{
    void test()
    {
        //Prints a b c d e f g h i j
        std::map<Char, Task_<void>::Ptr> tasks;
        for (auto i: range(10))
        {
            Id id = StringStream() << Char('a'+i);
            tasks[id.name()[0]] = new Task_<void>([=]{ Task_log(id); }, id);
        }
        
        tasks['j']->deps().add(*tasks['i']);
        tasks['i']->deps().add(*tasks['h']);
        tasks['h']->deps().add(*tasks['g']);
        tasks['g']->deps().add(*tasks['f']);
        tasks['f']->deps().add(*tasks['e']);
        tasks['e']->deps().add(*tasks['d']);
        tasks['d']->deps().add(*tasks['c']);
        tasks['c']->deps().add(*tasks['b']);
        tasks['b']->deps().add(*tasks['a']);

        TaskSched sched;
        for (auto& e: stdutil::values(tasks)) sched.reg(*e);

        auto future = tasks['j']->future();
        sched.enqueue(*tasks['j']);
        future.wait();
    }
} }
/** \endcond */

}




