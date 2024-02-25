#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <utility>
#include <atomic>
#include <condition_variable>

namespace mt {

using Flag = std::shared_ptr<std::atomic_bool>;

class ThreadGroup {
public:
    explicit ThreadGroup(size_t a_numOfWorkers, std::function<void()> a_func);
    ThreadGroup(const ThreadGroup& a_other) = delete;
    ThreadGroup& operator=(const ThreadGroup& a_other) = delete;
    ~ThreadGroup();

    bool addWorkers(size_t a_numOfThreads);
    size_t removeWorkers(size_t a_numOfThreads);
    void shutDown();
    size_t workers() const;
    void setRemoveFlag(size_t a_threadNum);
    size_t getIndex();
    size_t removeDoneWorkers(size_t a_numOfThreads, std::condition_variable& a_cvApple, std::atomic<size_t>& a_conter);

private:
    void joinAll();
    // void removeWorker();

private:
    std::function<void()> m_func;
    bool m_shutDown;
    std::vector<std::thread> m_workers;
    std::vector<Flag> m_flags;
    std::vector<Flag> m_removeFlags;
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;
};

} // namespace mt

