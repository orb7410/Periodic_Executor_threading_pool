#pragma once
#include <functional>
#include <optional>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace mt::implementation_details {

class Task { 
public:
    Task() = default;
    Task(std::function<bool()> a_func, std::chrono::milliseconds a_period, std::optional<uint64_t> a_repeatCount = std::nullopt);
    Task(std::function<void()> a_func, std::chrono::milliseconds a_period, std::optional<uint64_t> a_repeatCount = std::nullopt);
    
    bool operator<(Task const& a_other) const noexcept;
    void run(std::mutex& a_mtx, std::condition_variable& a_cv);
    bool stopExecut() const noexcept;
    void updateTime();
    void setStopExecutToStop(std::mutex& a_mtx) noexcept;

private:
    std::function<bool()> m_boolFunc;
    std::function<void()> m_voidFunc;
    std::chrono::steady_clock::time_point m_executionTime;
    std::chrono::milliseconds m_period;
    std::optional<uint64_t> m_repeatCount;
    bool m_stopExecut;
};


} //mt

