#include <iostream>
#include <ostream>
#include <thread>
#include <functional>
#include <atomic>
#include "thread_group.hpp"

namespace mt {

ThreadGroup::ThreadGroup(size_t a_numOfWorkers, std::function<void()> a_func)
: m_func(a_func)
, m_shutDown(false)
{
    if(1 > a_numOfWorkers) {
        a_numOfWorkers = 1;
    }

    m_workers.reserve(a_numOfWorkers);
    m_flags.reserve(a_numOfWorkers);
    m_removeFlags.reserve(a_numOfWorkers);

    addWorkers(a_numOfWorkers);
}


ThreadGroup::~ThreadGroup()
{
    joinAll();
}


void ThreadGroup::shutDown()
{
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        if(m_shutDown) {
            return;
        }
    }

	for(auto& f : m_flags) {
        *f = false;
    }

	{
        std::unique_lock<std::mutex> lock(m_mtx);
        m_shutDown = true;
    }
}


bool ThreadGroup::addWorkers(size_t a_numOfThreads)
{
    std::unique_lock<std::mutex> lock(m_mtx);

    if(m_shutDown || a_numOfThreads == 0) {
        return false;
    }
    lock.unlock();

    for(size_t i = 0; i < a_numOfThreads; ++i) {
        auto flag = std::make_shared<std::atomic_bool>(true);
        auto removeflag = std::make_shared<std::atomic_bool>(false);
        m_flags.push_back(flag); 
        m_removeFlags.push_back(removeflag); 

        auto f = [this, flagToCheck = m_flags.back()]() {
            while (*flagToCheck) {
                m_func();
            }
        };

		{
        	std::unique_lock<std::mutex> lock(m_mtx);
        	m_workers.emplace_back(std::thread(f));
		}
    }

    return true;
}

size_t ThreadGroup::removeWorkers(size_t a_numOfThreads)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_workers.size() <= 1 || a_numOfThreads == 0 || m_shutDown) {
        return 0;
    }
    size_t removed = 0;
    a_numOfThreads = std::min(a_numOfThreads, m_workers.size()-1);
    for (size_t i = 0; i < a_numOfThreads; ++i) {
        *m_flags.back() = false;
        auto& w = m_workers.back();
        if(w.joinable()) {
            w.join();
        }
        m_flags.pop_back();
        m_workers.pop_back();
        ++removed;
    }
    return removed;
}

size_t ThreadGroup::removeDoneWorkers(size_t a_numOfThreads, std::condition_variable& a_cvApple, std::atomic<size_t>& a_counter)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if(m_workers.size() <= 1 || a_numOfThreads == 0 || m_shutDown) {
        return 0;
    }

    a_cvApple.wait(lock, [&a_numOfThreads, &a_counter]() {
                return (a_counter == a_numOfThreads);
            });

    a_numOfThreads = std::min(a_numOfThreads, m_workers.size()-1);
    size_t removed = 0;

    for (size_t i = 0; i < m_workers.size(); ++i) {
        if (removed >= a_numOfThreads) {
            return removed;
        }
        if (*(m_removeFlags[i]) == true) {
            m_workers[i].join(); // Wait for the thread to finish before erasing it
           
            m_workers.erase(m_workers.begin() + i);
            m_removeFlags.erase(m_removeFlags.begin() + i);
            m_flags.erase(m_flags.begin() + i);
            ++removed;
            if (m_workers.empty()) {
                return removed;
            }
            --i; // Need to decrement the index to account for the erased element
        }
    }

    return removed;
}

size_t ThreadGroup::workers() const
{
    std::unique_lock<std::mutex> lock(m_mtx);
    return m_workers.size();
}

void ThreadGroup::joinAll()
{
    for (auto& t : m_workers) {
        t.join();
    }
}

void ThreadGroup::setRemoveFlag(size_t a_threadNum)
{
    *(m_removeFlags[a_threadNum]) = true;
}


size_t ThreadGroup::getIndex() {
    std::thread::id this_id = std::this_thread::get_id();
    for(size_t i = 0; i < m_workers.size(); ++i) {
        if(m_workers[i].get_id() == this_id) {
            return i;
        }
    }
    return m_workers.size(); // If thread not found
}
} //mt
