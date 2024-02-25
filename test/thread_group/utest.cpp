#include "mu_test.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <functional>
#include <atomic>
#include "thread_group.hpp"

using namespace mt;


std::mutex mtx;
std::condition_variable cv;

void display(std::string const& sv) {
	std::unique_lock<std::mutex> lock(mtx);
    std::cerr << sv;
}

void print()
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        std::cerr << " run demo \n ";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

BEGIN_TEST(test_one_worker)

    using namespace mt;
        ThreadGroup tg(1, std::function<void()>(print));

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        TRACER <<" before shutdown\n";

        tg.shutDown();

        TRACER <<" after shutdown\n";
        ASSERT_PASS();

END_TEST

BEGIN_TEST(test_ziro_worker)

    using namespace mt;
        ThreadGroup tg(0, std::function<void()>(print));

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        TRACER <<" before shutdown\n";

        tg.shutDown();

        TRACER <<" after shutdown\n";
        ASSERT_PASS();

END_TEST

BEGIN_TEST(test_ten_workers)

    using namespace mt;
    ThreadGroup tg{10, [](){

    }};

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    TRACER <<" before shutdown\n";

    tg.shutDown();

    TRACER <<" after shutdown\n";
    ASSERT_PASS();

END_TEST

BEGIN_TEST(test_add_two_workers)
    using namespace mt;
        ThreadGroup tg(1, std::function<void()>(print));

        ASSERT_EQUAL(tg.workers(), 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        bool isAdded = tg.addWorkers(2);
        TRACER << " 2 workers added \n";
        ASSERT_EQUAL(isAdded, true);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        TRACER <<" before shutdown\n";

        tg.shutDown();

        TRACER <<" after shutdown\n";
        ASSERT_EQUAL(tg.workers(), 3);

END_TEST

BEGIN_TEST(test_add_twice_workers)
    using namespace mt;
        ThreadGroup tg(1, std::function<void()>(print));

        ASSERT_EQUAL(tg.workers(), 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        bool isAdded = tg.addWorkers(2);
        TRACER << " 2 workers added \n";
        ASSERT_EQUAL(tg.workers(), 3);
        ASSERT_EQUAL(isAdded, true);

        isAdded = tg.addWorkers(2);
        TRACER << " 2 workers added \n";

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        TRACER <<" before shutdown\n";

        tg.shutDown();

        TRACER <<" after shutdown\n";
        ASSERT_EQUAL(tg.workers(), 5);
        ASSERT_EQUAL(isAdded, true);

END_TEST




BEGIN_TEST(test_remove_two_workers)
    using namespace mt;

	std::atomic<int> count{0};

	ThreadGroup tg(4, [&count](){
		for (size_t i = 0; i < 100'000; ++i) {
			++count;
		}
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	size_t removed = tg.removeWorkers(2);
	TRACER << " 2 workers removed \n";

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	TRACER <<" before shutdown\n";

	tg.shutDown();

	TRACER <<" after shutdown\n";
	ASSERT_EQUAL(tg.workers(), 2);
	ASSERT_EQUAL(removed, 2);

END_TEST


BEGIN_TEST(test_remove_two_workers_printing)
    using namespace mt;

	std::mutex mtx;
	std::condition_variable cv;

	// ThreadGroup tg(4, std::function<void(void)>{print});
	ThreadGroup tg(4, [&mtx](){
		display("run demo");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	});

	TRACER << " RUNNING \n";
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	size_t removed = tg.removeWorkers(2);
	TRACER << " 2 workers removed \n";

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	TRACER <<" before shutdown\n";

	tg.shutDown();

	TRACER <<" after shutdown\n";
	ASSERT_EQUAL(tg.workers(), 2);
	ASSERT_EQUAL(removed, 2);

END_TEST


BEGIN_TEST(test_remove_two_workers_printing_global)
    using namespace mt;

	std::mutex mtx;
	std::condition_variable cv;

	ThreadGroup tg(4, std::function<void(void)>{print});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	size_t removed = tg.removeWorkers(2);
	TRACER << " 2 workers removed \n";

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	TRACER <<" before shutdown\n";

	tg.shutDown();

	TRACER <<" after shutdown\n";
	ASSERT_EQUAL(tg.workers(), 2);
	ASSERT_EQUAL(removed, 2);

END_TEST


BEGIN_TEST(test_remove_too_many_workers)
    using namespace mt;
        ThreadGroup tg(4, std::function<void()>(print));

        ASSERT_EQUAL(tg.workers(), 4);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        size_t removed = tg.removeWorkers(4);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        TRACER <<" before shutdown\n";

        tg.shutDown();

        TRACER <<" after shutdown\n";
        ASSERT_EQUAL(tg.workers(),1);
        ASSERT_EQUAL(removed, 3);

END_TEST


BEGIN_TEST(test_remove_two_workers_add_2)
    using namespace mt;
        ThreadGroup tg(2, std::function<void()>(print));

        ASSERT_EQUAL(tg.workers(), 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        bool isAdded = tg.addWorkers(2);
        TRACER << " 2 workers added \n";
        ASSERT_EQUAL(tg.workers(), 4);
        ASSERT_EQUAL(isAdded, true);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        size_t removed = tg.removeWorkers(2);
        TRACER << " 2 workers removed \n";

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        TRACER <<" before shutdown\n";

        tg.shutDown();

        TRACER <<" after shutdown\n";
        ASSERT_EQUAL(tg.workers(), 2);
        ASSERT_EQUAL(removed, 2);

END_TEST
TEST_SUITE(因果応報 [inga ōhō: bad causes bring bad results])
    TEST(test_one_worker)
    TEST(test_ziro_worker)
    TEST(test_ten_workers)
    TEST(test_add_two_workers)
    TEST(test_add_twice_workers)
    TEST(test_remove_two_workers)
	TEST(test_remove_two_workers_printing)
	TEST(test_remove_two_workers_printing_global)
    TEST(test_remove_too_many_workers)
    TEST(test_remove_two_workers_add_2)
END_SUITE
