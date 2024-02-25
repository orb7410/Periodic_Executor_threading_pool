#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace advcpp {

template <typename T = int, typename Q = std::queue<T>>   
class WaitableQueue {
public:
    WaitableQueue() = delete;
    explicit WaitableQueue(size_t a_capcity);
    WaitableQueue(WaitableQueue const& a_other) = delete;
    WaitableQueue& operator=(WaitableQueue const& a_other) = delete;
    ~WaitableQueue() = default;

    bool empty() const;
    bool full() const;
    size_t size() const;

    void enqueue(T const& a_item);
    void dequeue(T& a_item);

private:
    bool innerFull() const noexcept;
    bool innerEmpty() const noexcept;

private:   
    Q m_queue;
    mutable std::mutex m_mtx;
    size_t m_capcity;
    std::condition_variable m_notFull;
    std::condition_variable m_notEmpty;
};

} // namespace advcpp

#include "inl/waitable_queue.hxx"