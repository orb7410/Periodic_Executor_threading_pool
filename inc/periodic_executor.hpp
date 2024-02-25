#pragma once
#include <functional>
#include <thread>
#include "waitable_queue.hpp"
#include "task.hpp"

namespace mt {

class PeriodicExecutor {
public:
    explicit PeriodicExecutor(size_t a_capacity); // create one thread to do the executions 
    PeriodicExecutor(PeriodicExecutor const& a_other) = delete;
    PeriodicExecutor operator=(PeriodicExecutor const& a_other) = delete;
    ~PeriodicExecutor();

    void submit(std::function<bool()> a_func, std::chrono::milliseconds a_period);
    void submit(std::function<void()> a_func, std::chrono::milliseconds a_period);
    void submit(std::function<void()> a_func, std::chrono::milliseconds a_period, uint64_t a_repeatCount);

    void pause(); 
    void resume();
    void shutDown(); 

private:
    void run();

private:
    using Task = implementation_details::Task;
    advcpp::WaitableQueue<Task, std::priority_queue<Task>> m_tasks;
    bool m_isRunning;
    bool m_shutDown;
    const size_t m_capacity;
    std::mutex m_mtx;
    std::condition_variable m_cvStop;
    std::thread m_thread;
};

} //mt
