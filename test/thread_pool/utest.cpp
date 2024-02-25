#include "mu_test.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <functional>
#include <atomic>
#include "task.hpp"
#include "waitable_queue.hpp"
#include "thread_group.hpp"
#include "thread_pool.hpp"

using namespace mt;
std::mutex mtx;

void print() 
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        std::cout << " run demo \n ";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

BEGIN_TEST(test_add_2_worker)

    using namespace mt;
    ThreadPool p( 20, 3, 8);
        
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    size_t added = p.addWorkers(2);
    TRACER << " 2 workers added \n";
    
    TRACER <<" before shutdown\n";
        
    p.shutDown();

    TRACER <<" after shutdown\n";
    ASSERT_EQUAL(added, 2);
    ASSERT_EQUAL(p.workersSize() , 5);


END_TEST

BEGIN_TEST(test_submit_1_task)

    using namespace mt;
    ThreadPool p( 20, 2, 8);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::function<void()> task = print;
    bool isSubmit = p.submit(task);
    TRACER << " 2 workers added \n";
    
    TRACER <<" before shutdown\n";
        
    p.shutDown();

    TRACER <<" after shutdown\n";
    ASSERT_EQUAL(isSubmit, 1);
    ASSERT_EQUAL(p.workersSize(), 2);

END_TEST

BEGIN_TEST(test_remove_1_worker)

    using namespace mt;
    ThreadPool p( 8, 3, 8);
        
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    size_t removed = p.removeWorkers(1);
    TRACER << " 2 workers added \n";
    
    TRACER <<" before shutdown\n";
        
    p.shutDown();

    TRACER <<" after shutdown\n";
    ASSERT_EQUAL(removed, 1);
    ASSERT_EQUAL(p.workersSize() , 2);


END_TEST

BEGIN_TEST(test_remove_2_worker)

    using namespace mt;
    ThreadPool p( 8, 4, 8);
        
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    p.addWorkers(2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::function<void()> task = print;
    p.submit(task);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    TRACER << " 2 workers added \n";
    size_t removed = p.removeWorkers(2);
    TRACER << " 2 workers added \n";
    
    TRACER <<" before shutdown\n";
        
    p.shutDown();

    TRACER <<" after shutdown\n";
    ASSERT_EQUAL(removed, 2);
    ASSERT_EQUAL(p.workersSize() , 2);


END_TEST

BEGIN_TEST(test_remove_too_many_worker)

    using namespace mt;
    ThreadPool p( 8, 3, 8);
        
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    size_t removed = p.removeWorkers(3);
    TRACER << " 2 workers added \n";
    
    TRACER <<" before shutdown\n";
        
    p.shutDown();

    TRACER <<" after shutdown\n";
    ASSERT_EQUAL(removed, 2);
    ASSERT_EQUAL(p.workersSize(), 1);


END_TEST

TEST_SUITE(因果応報 [inga ōhō: bad causes bring bad results])
    // TEST(test_add_2_worker)
    // TEST(test_submit_1_task)
    // TEST(test_remove_1_worker)
    TEST(test_remove_2_worker)
    // TEST(test_remove_too_many_worker)
END_SUITE
