#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <utility>
#include <atomic>
#include <functional>
#include <condition_variable>
#include "waitable_queue.hpp"
#include "thread_group.hpp"

namespace mt {

class ThreadPool {
public:
    explicit ThreadPool(size_t a_capacity, size_t a_minThreads = 2, size_t a_maxThreads = std::thread::hardware_concurrency());
    ThreadPool(const ThreadPool& a_other) = delete;
    ThreadPool& operator=(const ThreadPool& a_other) = delete; 
    ~ThreadPool();

    bool submit(std::function<void()>& a_task);
    size_t addWorkers(size_t a_numOfThreads);
    size_t removeWorkers(size_t a_numOfThreads);
    void shutDown();
    int16_t workersSize() const;
    int16_t tasksSize() const;
    int16_t applesNum() const;

private:
    void worker();
    void insertApple(size_t a_apples);

private:
    bool m_shutdown;
    size_t m_minThreads; 
    size_t m_maxThreads;
    size_t m_capacity; 
    std::atomic<size_t> m_threadCounter; 
    advcpp::WaitableQueue<std::function<void()>> m_tasks;
    mutable std::mutex m_mtxGroup;
    std::condition_variable m_cvApple;
    ThreadGroup m_threads;

};

} // namespace mt