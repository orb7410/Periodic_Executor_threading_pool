#include <chrono>
#include <optional>
#include "task.hpp"
#include <mutex>
#include <condition_variable>

namespace mt::implementation_details {

Task::Task(std::function<bool()> a_func, std::chrono::milliseconds a_period, std::optional<uint64_t> a_repeatCount)
: m_boolFunc(a_func)
, m_voidFunc(nullptr)
, m_executionTime(std::chrono::steady_clock::now() + a_period)
, m_period(a_period)
, m_repeatCount(a_repeatCount)
, m_stopExecut(false)
{
}

Task::Task(std::function<void()> a_func, std::chrono::milliseconds a_period, std::optional<uint64_t> a_repeatCount )
: m_boolFunc(nullptr)
, m_voidFunc(a_func)
, m_executionTime(std::chrono::steady_clock::now() + a_period)
, m_period(a_period)
, m_repeatCount(a_repeatCount)
, m_stopExecut(false)
{
}

bool Task::stopExecut() const noexcept
{
    return m_stopExecut;
}

bool Task::operator<(Task const& a_other) const noexcept
{
    return m_executionTime > a_other.m_executionTime;
}

void Task::updateTime()
{
    m_executionTime = std::chrono::steady_clock::now() + m_period;
}

void Task::setStopExecutToStop(std::mutex& a_mtx) noexcept
{
    std::unique_lock<std::mutex> lock(a_mtx);
    m_stopExecut = true;
}

void Task::run(std::mutex& a_mtx, std::condition_variable& a_cv)
{
    {    
        std::unique_lock<std::mutex> lock(a_mtx);
        if (m_stopExecut) {
            return;
        }
    }

    {
        std::unique_lock<std::mutex> lock(a_mtx);
        if (std::cv_status::no_timeout == a_cv.wait_until(lock, m_executionTime)) {
            return; //didnt get to the excecut time
        }
    }
                   
    if (m_voidFunc) {
        m_voidFunc();
    } else if (m_boolFunc) {
        m_boolFunc();
    }

    std::unique_lock<std::mutex> lock(a_mtx);
    if (m_repeatCount && m_repeatCount.value() > 0) {
        --m_repeatCount.value();
    }
    if (m_repeatCount && m_repeatCount.value() == 0) {
        m_stopExecut = true;
        return;
    }

    updateTime();
}



} //mt