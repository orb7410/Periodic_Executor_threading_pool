#include <mutex>
#include <iostream>
#include <ostream>
#include <condition_variable>
#include "waitable_queue.hpp"
#include "task.hpp"
#include "periodic_executor.hpp"

namespace mt {

using namespace implementation_details;

PeriodicExecutor::PeriodicExecutor(size_t a_capacity) 
: m_tasks(a_capacity+1)
, m_isRunning(true)
, m_shutDown(false)
, m_capacity(a_capacity)
, m_thread([this](){run();})
{
    
}

PeriodicExecutor::~PeriodicExecutor()
{
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_shutDown = true;
        m_cvStop.notify_all();
    }
    m_thread.join();
}

void PeriodicExecutor::submit(std::function<bool()> a_func, std::chrono::milliseconds a_period) 
{
   if(m_tasks.size() >= m_capacity) {   
        std::cerr << "Executor is overloaded, task <bool> submission failed." << std::endl;
        return;
   }

    Task t(a_func, a_period, std::nullopt); 
    m_tasks.enqueue(t);
    m_cvStop.notify_all();
}

void PeriodicExecutor::submit(std::function<void()> a_func, std::chrono::milliseconds a_period) 
{
    if(m_tasks.size() >= m_capacity) {   
        std::cerr << "Executor is overloaded, task <void> submission failed." << std::endl;
        return;
    }
    Task t(a_func, a_period, std::nullopt); 
    m_tasks.enqueue(t);
    m_cvStop.notify_all();
}

void PeriodicExecutor::submit(std::function<void()> a_func, std::chrono::milliseconds a_period, uint64_t a_repeatCount) 
{
    if(m_tasks.size() >= m_capacity) {   
        std::cerr << "Executor is overloaded, task <void> submission failed." << std::endl;
        return;
    }
    Task t(a_func, a_period, a_repeatCount);
    m_tasks.enqueue(t);
    m_cvStop.notify_all();
}

void PeriodicExecutor::run()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    while(!m_shutDown) {
        lock.unlock();

        Task t; //the empty is handle in the q
        m_tasks.dequeue(t);
        
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cvStop.wait(lock, [this](){ 
                        return m_isRunning; 
                    });
        }    
        try{
            t.run(m_mtx, m_cvStop); //return val, what if exception(object policy) the task will count exception
        } catch(std::exception const& x) {
            t.setStopExecutToStop(m_mtx);
            std::cerr << "Exception in task: " << x.what() << std::endl;

        } catch(...) {
            t.setStopExecutToStop(m_mtx);
            std::cerr << "Unknow Exception in task.\n";
        }

        {
            std::unique_lock<std::mutex> lock(m_mtx);
            if (!t.stopExecut()) {
                m_tasks.enqueue(t);
            }
        }

        lock.lock();
    }
}

void PeriodicExecutor::pause()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_isRunning = false;
    m_cvStop.notify_all();
}

void PeriodicExecutor::resume() 
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_isRunning = true;
    m_cvStop.notify_all();
}

void PeriodicExecutor::shutDown() 
{
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_shutDown = false;
        m_cvStop.notify_all();
    }

    Task t([](){}, std::chrono::milliseconds(0), std::nullopt); 
    m_tasks.enqueue(t);
}

} //mt
