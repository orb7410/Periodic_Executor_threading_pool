#include <condition_variable>
#include <mutex>
#include <iostream>
#include "thread_pool.hpp"

namespace mt {
ThreadPool::ThreadPool(size_t a_capacity, size_t a_minThreads, size_t a_maxThreads)
: m_shutdown(false)
, m_minThreads(std::max(a_minThreads, static_cast<size_t> (2)))
, m_maxThreads(std::max(a_maxThreads, m_minThreads))
, m_capacity (a_capacity)
, m_threadCounter(0)
, m_tasks(a_capacity + 2)
, m_mtxGroup()
, m_cvApple()
, m_threads(m_minThreads, [this]() {worker();})
{
}

ThreadPool::~ThreadPool()
{
    shutDown();
}

void ThreadPool::shutDown()
{
    std::unique_lock<std::mutex> lock(m_mtxGroup);
    if(m_shutdown) {
        return;
    }
    m_shutdown = true;
    insertApple(m_threads.workers());
    m_threads.shutDown();
    m_cvApple.notify_all();
}

size_t ThreadPool::addWorkers(size_t a_numOfThreads)
{
    std::unique_lock<std::mutex> lock(m_mtxGroup);
    bool isAdded = false;
    if (m_threads.workers() >= m_maxThreads) {
        return 0;
    } 
    else if (a_numOfThreads + m_threads.workers() > m_maxThreads) {
        isAdded = m_threads.addWorkers(m_maxThreads - m_threads.workers());
    } 
    else {
        isAdded = m_threads.addWorkers(a_numOfThreads);
    }
    return isAdded ? a_numOfThreads : 0;
}

bool ThreadPool::submit(std::function<void()>& a_task)
{
    std::unique_lock<std::mutex> lock(m_mtxGroup);
    if(m_shutdown) {
        return false;
    }
    if (m_tasks.size() < m_capacity) {
        m_tasks.enqueue(a_task);
        // m_cv.notify_all();
        return true;
    }
    return false;
}

size_t ThreadPool::removeWorkers(size_t a_numOfThreads)
{
    std::unique_lock<std::mutex> lock(m_mtxGroup);
    if (0 == a_numOfThreads) {
        return 0;
    }
    if (m_minThreads >= m_threads.workers() - a_numOfThreads) {
        a_numOfThreads = m_threads.workers() - m_minThreads;
    }
    insertApple(a_numOfThreads);
    m_cvApple.notify_all();
    return m_threads.removeDoneWorkers(a_numOfThreads, m_cvApple, m_threadCounter);
}


void ThreadPool::worker()
{
    std::unique_lock<std::mutex> lock(m_mtxGroup);
    while (!m_shutdown) {
        lock.unlock();
        
        
        std::function<void()> task;
        std::cout <<"befor dequeue \n";
       
        // m_cv.wait(lock, [this]() {
        //     return (!m_tasks.empty() || m_shutdown);
        // });
        
        // if(m_shutdown) {
        //     break;
        // }

        m_tasks.dequeue(task);
        std::cout <<"after dequeue \n";

        try{
			task();
		}
        catch(const char* msg) { //no char, send class
            std::cerr << msg << std::endl;
            ++m_threadCounter;
            m_cvApple.notify_all();
			break;
		}
        catch(std::exception const& x){
			std::cerr << "Exception in task: " << x.what() << std::endl;

		}
        catch(...){
			std::cerr << "Unknow Exception in task. Shame on you!!!\n";
		}
        lock.lock();
    }
}

void ThreadPool::insertApple(size_t a_numOfApple)
{
    for(size_t i = 0; i < a_numOfApple; ++i) {
        if(m_tasks.full()) {
            break;
        }
        m_tasks.enqueue([this](){ 
                            m_threads.setRemoveFlag(m_threads.getIndex());
                            throw "poisoned apple"; 
                        });
    }
}   

int16_t ThreadPool::workersSize() const
{
    std::unique_lock<std::mutex> lock(m_mtxGroup);
    return m_threads.workers();
}

int16_t ThreadPool::tasksSize() const
{
    std::unique_lock<std::mutex> lock(m_mtxGroup);
        if(m_tasks.size() > 0) {
        return m_tasks.size() - m_threadCounter;
    }

    return 0;
}

} //mt
