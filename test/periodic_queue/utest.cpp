#include "mu_test.h"

#include <thread>
#include <vector>
#include <array>
#include <iostream>
#include <cassert>
#include <iterator>
#include <functional>
#include <optional>     
#include <algorithm>

#include "waitable_queue.hpp"

class Cat {
public:
    enum Color {
        BLUE,
        RED,
        GREEN, 
        YELLOW
    };
    explicit Cat(size_t a_id, Color a_color);
    ~Cat() = default;
    bool operator<(const Cat& a_other) const {
        return m_id > a_other.m_id;
    }
    size_t m_id;
    Color m_color;
};

inline Cat::Cat(size_t a_id, Color a_color)
: m_id(a_id)
, m_color(a_color)
{
}

BEGIN_TEST(fifo_and_empty_1000_cats)
    size_t size = 1000;
    advcpp::WaitableQueue<Cat> q(1000);
    for(size_t i = 0; i < size; ++i) {
        q.enqueue(Cat(i, Cat::Color::RED));
    }

    for(size_t i = 0; i < size; ++i) {
        Cat c{i, Cat::Color::RED};
        q.dequeue(c);
        ASSERT_EQUAL(c.m_id, i);
    }

    ASSERT_EQUAL(q.size(), 0);
END_TEST

BEGIN_TEST(producer_consumerMnage_milion)
    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;
    advcpp::WaitableQueue<Cat> q(SIZE_1);
    bool isSort;
    std::vector<Cat> v;

    // Create the producer thread
    std::thread producerThread([&q]() {
        for(size_t i = 0; i < SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::RED));
        }
    });

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort]() {
        for(size_t i = 0; i < SIZE_2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);
            if(t.m_id != i) {
                isSort = false;
                break;
            }
        } 
    });

    // Wait for both threads to finish
    producerThread.join();
    consumerThread.join();

    ASSERT_EQUAL(q.size(), 0);
END_TEST


BEGIN_TEST(test_FIFO_2_producer_consumer_milion)
    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;
    advcpp::WaitableQueue<Cat> q(SIZE_1);
    bool isSort = true;

    // Create the producer thread
    std::thread producerThread([&q]() {
        for(size_t i = 0; i < 0.5 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::RED));
        }
    });

    // Create the producer thread
    std::thread producerThread2([&q]() {
        for(size_t i = 0; i < 0.5 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::BLUE));
        }
    });

    std::optional<Cat> lastRedCat;
    std::optional<Cat> lastBlueCat;

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort, &lastRedCat, &lastBlueCat]() {
        for(size_t i = 0; i < SIZE_2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);
            if(t.m_color == Cat::Color::RED) {
                if(!lastRedCat.has_value() || t.m_id == lastRedCat.value().m_id + 1) {
                    lastRedCat = t;
                }
                else {
                    isSort = false;
                    break;
                }
            }
            if(t.m_color == Cat::Color::BLUE) {
                if(!lastBlueCat.has_value() || t.m_id == lastBlueCat.value().m_id + 1) {
                    lastBlueCat = t;
                }
                else {
                    isSort = false;
                    break;
                }
            }
        }
    });

    // Wait for both threads to finish
    producerThread.join();
    producerThread2.join();
    consumerThread.join();

    ASSERT_EQUAL(q.size(), 0);
    ASSERT_THAT(isSort == true);
END_TEST 

BEGIN_TEST(test_FIFO_producer_2_consumer_milion)

    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;
    advcpp::WaitableQueue<Cat> q(SIZE_1);
    bool isSort = true;

    // Create the producer thread
    std::thread producerThread([&q]() {
    for (size_t i = 0; i < SIZE_2; ++i)
        q.enqueue(Cat(i, Cat::Color::RED));
        });

    size_t Counter1 = 0;
    size_t Counter2 = 0;

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort, &Counter1]() {
        for(size_t i = 0; i < SIZE_2 / 2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < Counter1) {
                isSort = false;
                break;
            }
            Counter1 = t.m_id;
        }
    });

    // Create the consumer thread
    std::thread consumerThread2([&q, &isSort, &Counter2]() {
        for(size_t i = 0; i < SIZE_2 / 2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < Counter2) {
                isSort = false;
                break;
            }
            Counter2 = t.m_id;
        }
    });

    // Wait for both threads to finish
    producerThread.join();
    consumerThread.join();
    consumerThread2.join();

    ASSERT_EQUAL(q.size(), 0);
    ASSERT_EQUAL(isSort, true);

END_TEST 

bool catComparator(const Cat& a_cat1, const Cat& a_cat2) {
    if (a_cat1.m_color != a_cat2.m_color) {
        return a_cat1.m_color < a_cat2.m_color;
    }
    return a_cat1.m_id < a_cat2.m_id;
}

BEGIN_TEST(test_FIFO_4_producer_4_consumer_milion)
    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;

    advcpp::WaitableQueue<Cat> q(SIZE_1);
    bool isSort = true;

    // Create the producer thread
    std::thread producerThread([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::RED));
        }
    });

    // Create the producer thread
    std::thread producerThread2([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::BLUE));
        }
    });

    // Create the producer thread
    std::thread producerThread3([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::YELLOW));
        }
    });

    // Create the producer thread
    std::thread producerThread4([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::GREEN));
        }
    });

    std::array<size_t, 4> counter1{0};
    std::array<size_t, 4> counter2{0};
    std::array<size_t, 4> counter3{0};
    std::array<size_t, 4> counter4{0};
    std::vector<Cat> Cats1;
    Cats1.reserve(1'000'000/4);
    std::vector<Cat> Cats2;
    Cats2.reserve(1'000'000/4);
    std::vector<Cat> Cats3;
    Cats3.reserve(1'000'000/4);
    std::vector<Cat> Cats4;
    Cats4.reserve(1'000'000/4);

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort, &counter1, &Cats1]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter1[t.m_color]) {
                isSort = false;
                break;
            }
            counter1[t.m_color] = t.m_id;
            Cats1.push_back(t);
        }
    });

    // Create the consumer thread
    std::thread consumerThread2([&q, &isSort, &counter2, &Cats2]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter2[t.m_color])  {
                isSort = false;
                break;
            }
            counter2[t.m_color]  = t.m_id;
            Cats2.push_back(t);
        }
    });

    // Create the consumer thread
    std::thread consumerThread3([&q, &isSort, &counter3, &Cats3]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter3[t.m_color]) {
                isSort = false;
                break;
            }
            counter3[t.m_color] = t.m_id;
            Cats3.push_back(t);
        }
    });

    // Create the consumer thread
    std::thread consumerThread4([&q, &isSort, &counter4, &Cats4]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter4[t.m_color]) {
                isSort = false;
                break;
            }
           counter4[t.m_color] = t.m_id;
           Cats4.push_back(t);
        }
    });

    // Wait for all the threads to finish
    producerThread.join();
    producerThread2.join();
    producerThread3.join();
    producerThread4.join();
    consumerThread.join();
    consumerThread2.join();
    consumerThread3.join();
    consumerThread4.join();

    std::vector<Cat> allCats;
    allCats.reserve(1'000'000);
    allCats.insert(allCats.end(), Cats1.begin(), Cats1.end());
	allCats.insert(allCats.end(), Cats2.begin(), Cats2.end());
	allCats.insert(allCats.end(), Cats3.begin(), Cats3.end());
	allCats.insert(allCats.end(), Cats4.begin(), Cats4.end());
	std::sort(allCats.begin(), allCats.end(), catComparator);

    bool isComplete = true;
	for(size_t i = 0; i < 4; ++i) {
		for(size_t j = 0; j < SIZE_2 / 4; ++j) {
			if(allCats[i * SIZE_2 / 4 + j].m_color != i && allCats[i * SIZE_2 / 4 + j].m_id == j){
				isComplete = false;
				break;
			}
		}
	}

    ASSERT_EQUAL(q.size(), 0);
    ASSERT_EQUAL(isSort, true);
    ASSERT_EQUAL(isComplete, true);

END_TEST


BEGIN_TEST(fifo_and_empty_1000_cats_priority_queue)
    size_t size = 1000;
    advcpp::WaitableQueue<Cat, std::priority_queue<Cat>> q(1000);
    for(size_t i = 0; i < size; ++i) {
        q.enqueue(Cat(i, Cat::Color::RED));
    }

    for(size_t i = 0; i < size; ++i) {
        Cat c{i, Cat::Color::RED};
        q.dequeue(c);
        ASSERT_EQUAL(c.m_id, i);
    }

    ASSERT_EQUAL(q.size(), 0);
END_TEST


BEGIN_TEST(producer_consumerMnage_milion_priority_Q)
    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;
    advcpp::WaitableQueue<Cat, std::priority_queue<Cat>> q(SIZE_1);
    bool isSort;
    std::vector<Cat> v;

    // Create the producer thread
    std::thread producerThread([&q]() {
        for(size_t i = 0; i < SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::RED));
        }
    });

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort]() {
        for(size_t i = 0; i < SIZE_2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);
            if(t.m_id != i) {
                isSort = false;
                break;
            }
        } 
    });

    // Wait for both threads to finish
    producerThread.join();
    consumerThread.join();

    ASSERT_EQUAL(q.size(), 0);
END_TEST


BEGIN_TEST(test_FIFO_2_producer_consumer__priority_queue)
    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;
    advcpp::WaitableQueue<Cat, std::priority_queue<Cat>> q(SIZE_1);
    bool isSort = true;

    // Create the producer thread
    std::thread producerThread([&q]() {
        for(size_t i = 0; i < 0.5 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::RED));
        }
    });

    // Create the producer thread
    std::thread producerThread2([&q]() {
        for(size_t i = 0; i < 0.5 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::BLUE));
        }
    });

    std::optional<Cat> lastRedCat;
    std::optional<Cat> lastBlueCat;

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort, &lastRedCat, &lastBlueCat]() {
        for(size_t i = 0; i < SIZE_2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);
            if(t.m_color == Cat::Color::RED) {
                if(!lastRedCat.has_value() || t.m_id == lastRedCat.value().m_id + 1) {
                    lastRedCat = t;
                }
                else {
                    isSort = false;
                    break;
                }
            }
            if(t.m_color == Cat::Color::BLUE) {
                if(!lastBlueCat.has_value() || t.m_id == lastBlueCat.value().m_id + 1) {
                    lastBlueCat = t;
                }
                else {
                    isSort = false;
                    break;
                }
            }
        }
    });

    // Wait for both threads to finish
    producerThread.join();
    producerThread2.join();
    consumerThread.join();

    ASSERT_EQUAL(q.size(), 0);
    ASSERT_THAT(isSort == true);
END_TEST 

BEGIN_TEST(test_FIFO_producer_2_consumer_milion_priority_queue)

    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;
    advcpp::WaitableQueue<Cat, std::priority_queue<Cat>> q(SIZE_1);
    bool isSort = true;

    // Create the producer thread
    std::thread producerThread([&q]() {
    for (size_t i = 0; i < SIZE_2; ++i)
        q.enqueue(Cat(i, Cat::Color::RED));
        });

    size_t Counter1 = 0;
    size_t Counter2 = 0;

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort, &Counter1]() {
        for(size_t i = 0; i < SIZE_2 / 2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < Counter1) {
                isSort = false;
                break;
            }
            Counter1 = t.m_id;
        }
    });

    // Create the consumer thread
    std::thread consumerThread2([&q, &isSort, &Counter2]() {
        for(size_t i = 0; i < SIZE_2 / 2; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < Counter2) {
                isSort = false;
                break;
            }
            Counter2 = t.m_id;
        }
    });

    // Wait for both threads to finish
    producerThread.join();
    consumerThread.join();
    consumerThread2.join();

    ASSERT_EQUAL(q.size(), 0);
    ASSERT_EQUAL(isSort, true);

END_TEST 

BEGIN_TEST(test_FIFO_4_producer_4_consumer_milion_priority_queue)
    constexpr size_t SIZE_1 = 1'000;
    constexpr size_t SIZE_2 = 1'000'000;

    advcpp::WaitableQueue<Cat, std::priority_queue<Cat>> q(SIZE_1);
    bool isSort = true;

    // Create the producer thread
    std::thread producerThread([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::RED));
        }
    });

    // Create the producer thread
    std::thread producerThread2([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::BLUE));
        }
    });

    // Create the producer thread
    std::thread producerThread3([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::YELLOW));
        }
    });

    // Create the producer thread
    std::thread producerThread4([&q]() {
        for(size_t i = 0; i < 0.25 * SIZE_2; ++i) {
            q.enqueue(Cat(i, Cat::Color::GREEN));
        }
    });

    std::array<size_t, 4> counter1{0};
    std::array<size_t, 4> counter2{0};
    std::array<size_t, 4> counter3{0};
    std::array<size_t, 4> counter4{0};
    std::vector<Cat> Cats1;
    Cats1.reserve(1'000'000/4);
    std::vector<Cat> Cats2;
    Cats2.reserve(1'000'000/4);
    std::vector<Cat> Cats3;
    Cats3.reserve(1'000'000/4);
    std::vector<Cat> Cats4;
    Cats4.reserve(1'000'000/4);

    // Create the consumer thread
    std::thread consumerThread([&q, &isSort, &counter1, &Cats1]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter1[t.m_color]) {
                isSort = false;
                break;
            }
            counter1[t.m_color] = t.m_id;
            Cats1.push_back(t);
        }
    });

    // Create the consumer thread
    std::thread consumerThread2([&q, &isSort, &counter2, &Cats2]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter2[t.m_color])  {
                isSort = false;
                break;
            }
            counter2[t.m_color]  = t.m_id;
            Cats2.push_back(t);
        }
    });

    // Create the consumer thread
    std::thread consumerThread3([&q, &isSort, &counter3, &Cats3]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter3[t.m_color]) {
                isSort = false;
                break;
            }
            counter3[t.m_color] = t.m_id;
            Cats3.push_back(t);
        }
    });

    // Create the consumer thread
    std::thread consumerThread4([&q, &isSort, &counter4, &Cats4]() {
        for(size_t i = 0; i < SIZE_2 / 4; ++i) {
            Cat t{i, Cat::Color::RED};
            q.dequeue(t);

            if(t.m_id < counter4[t.m_color]) {
                isSort = false;
                break;
            }
           counter4[t.m_color] = t.m_id;
           Cats4.push_back(t);
        }
    });

    // Wait for all the threads to finish
    producerThread.join();
    producerThread2.join();
    producerThread3.join();
    producerThread4.join();
    consumerThread.join();
    consumerThread2.join();
    consumerThread3.join();
    consumerThread4.join();

    std::vector<Cat> allCats;
    allCats.reserve(1'000'000);
    allCats.insert(allCats.end(), Cats1.begin(), Cats1.end());
	allCats.insert(allCats.end(), Cats2.begin(), Cats2.end());
	allCats.insert(allCats.end(), Cats3.begin(), Cats3.end());
	allCats.insert(allCats.end(), Cats4.begin(), Cats4.end());
	std::sort(allCats.begin(), allCats.end(), catComparator);

    bool isComplete = true;
	for(size_t i = 0; i < 4; ++i) {
		for(size_t j = 0; j < SIZE_2 / 4; ++j) {
			if(allCats[i * SIZE_2 / 4 + j].m_color != i && allCats[i * SIZE_2 / 4 + j].m_id == j){
				isComplete = false;
				break;
			}
		}
	}

    ASSERT_EQUAL(q.size(), 0);
    ASSERT_EQUAL(isSort, true);
    ASSERT_EQUAL(isComplete, true);

END_TEST


TEST_SUITE(因果応報 [inga ōhō: bad causes bring bad results])
	TEST(fifo_and_empty_1000_cats)
    TEST(producer_consumerMnage_milion)
    TEST(test_FIFO_2_producer_consumer_milion)
    TEST(test_FIFO_producer_2_consumer_milion)
    TEST(test_FIFO_4_producer_4_consumer_milion)
    TEST(fifo_and_empty_1000_cats_priority_queue)
    TEST(producer_consumerMnage_milion_priority_Q)
    TEST(test_FIFO_2_producer_consumer__priority_queue)
    TEST(test_FIFO_producer_2_consumer_milion_priority_queue)
    TEST(test_FIFO_4_producer_4_consumer_milion_priority_queue)
END_SUITE
