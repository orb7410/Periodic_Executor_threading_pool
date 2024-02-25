#include "mu_test.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include "periodic_executor.hpp"

using namespace std::chrono_literals;
using namespace mt;

void task1()
{
    std::cout << "task1 executed" << std::endl;
}

bool task2()
{
    std::cout << "task2 executed" << std::endl;
    return true;
}

bool task3()
{
    std::cout << "task3 executed" << std::endl;
    return true;
}

void task4()
{
    std::cout << "task4 executed" << std::endl;
}

void task5() {
    std::cout << "Task5 executed" << std::endl;
    throw std::runtime_error("Test5 exception");
}


BEGIN_TEST(test_add_task_pause_resume)
    PeriodicExecutor executor(10);
    executor.submit(std::function<void()>(task4), 100ms, 8);
    executor.submit(std::function<void()>(task1), 100ms, 8);
    executor.submit(std::function<bool()>(task2), 100ms);
    executor.submit(std::function<bool()>(task3), 100ms);

    std::this_thread::sleep_for(4s);  
    TRACER << "sleeping \n";
    executor.pause();
    std::this_thread::sleep_for(4s);  
    executor.resume();

    std::this_thread::sleep_for(4s); 
    TRACER << "sleeping \n" ;
    executor.shutDown();
    ASSERT_PASS();
END_TEST


BEGIN_TEST(test_add_task_with_vector)
    TRACER << "test 2";
    std::vector<std::chrono::steady_clock::time_point> v;
    std::vector<std::chrono::steady_clock::time_point> v2;
    PeriodicExecutor executor(10);

    executor.submit([&v](){ 
        TRACER << "task lambda executed";
        v.push_back(std::chrono::steady_clock::now());
    }, 100ms, 8);

    executor.submit([&v2](){ 
        TRACER << "task lambda2 executed";
        v2.push_back(std::chrono::steady_clock::now());
    }, 100ms, 3);

    std::this_thread::sleep_for(6s);  
    TRACER << "sleeping ";
    executor.shutDown();

    auto diff = v.back() - v.front();
    auto expectedDiff = 800ms;
    auto toCheck = diff - expectedDiff;
    ASSERT_THAT( toCheck  <= 10ms);

    ASSERT_EQUAL(v.size(), 8);
    ASSERT_EQUAL(v2.size(), 3);
END_TEST

BEGIN_TEST(throws_Exception)
    using namespace mt;
    PeriodicExecutor executor(10);

    std::vector<std::chrono::steady_clock::time_point> v;

    executor.submit([&v](){ 
        TRACER << "task lambda executed";
        v.push_back(std::chrono::steady_clock::now());
        throw std::runtime_error("task lambda exception");
    }, 100ms, 8);
    std::this_thread::sleep_for(std::chrono::seconds(3)); // pause for 3 seconds

    executor.shutDown();
    ASSERT_EQUAL(v.size(), 1);
END_TEST

BEGIN_TEST(test_add_task_more_then_capacity)
    PeriodicExecutor executor(2);
    executor.submit(std::function<void()>(task4), 100ms, 8);
    executor.submit(std::function<void()>(task1), 100ms, 8);
    executor.submit(std::function<bool()>(task2), 100ms);
    executor.submit(std::function<bool()>(task3), 100ms);

    std::this_thread::sleep_for(4s);  
    TRACER << "sleeping \n";
    executor.pause();
    std::this_thread::sleep_for(4s);  
    executor.resume();

    std::this_thread::sleep_for(4s); 
    TRACER << "sleeping \n" ;
    executor.shutDown();
    ASSERT_PASS();
END_TEST

TEST_SUITE(因果応報 [inga ōhō: bad causes bring bad results])
    TEST(test_add_task_pause_resume)
    TEST(test_add_task_with_vector)
    TEST(throws_Exception)
    TEST(test_add_task_more_then_capacity)
END_SUITE
