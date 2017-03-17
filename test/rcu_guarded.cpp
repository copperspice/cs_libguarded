
#include <libguarded/rcu_guarded.hpp>
#include <libguarded/rcu_list.hpp>

#include <thread>
#include <iostream>

#include <boost/test/unit_test.hpp>

using namespace libguarded;

BOOST_AUTO_TEST_CASE(rcu_guarded_1)
{
    rcu_guarded<rcu_list<int>> my_list;

    {
        auto h = my_list.lock_write();
        h->push_back(42);
    }

    {
        auto h = my_list.lock_read();

        int count = 0;
        for (auto & item : *h) {
            ++count;
            BOOST_CHECK_EQUAL(item, 42);
        }
        BOOST_CHECK_EQUAL(count, 1);
    }

    {
        auto rh = my_list.lock_read();
        auto wh = my_list.lock_write();

        auto iter = rh->begin();

        wh->erase(wh->begin());

        int count = 0;
        for (; iter != rh->end(); ++iter) {
            ++count;
            BOOST_CHECK_EQUAL(*iter, 42);
        }
        BOOST_CHECK_EQUAL(count, 1);
    }

    {
        auto h = my_list.lock_read();

        int count = 0;
        volatile int escape;
        for (auto & item : *h) {
            escape = item;
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 0);
    }

    {
	constexpr const int num_writers = 8;
	std::atomic<int> t_writers_done{0};

        std::vector<std::thread> threads;
        for (int i = 0; i < num_writers; ++i) {
            threads.emplace_back([&]() {
                while (!t_writers_done.load()) {
                    auto rh = my_list.lock_read();
                    volatile int escape;
                    for (auto item : *rh) {
                        escape = item;
                    }
                }

            });

            threads.emplace_back([&]() {
                int count = 0;
                while (!t_writers_done.load() && count < 1000) {
                    auto wh = my_list.lock_write();
                    volatile int escape;
                    for (auto item : *wh) {
                        escape = item;
                    }
                    for (int i = 0; i < 2; ++i) {
                        wh->emplace_back(i);
                        wh->emplace_front(i - 1);
                        wh->push_back(i + 4);
                        wh->push_front(i - 7);
                    }
                    ++count;
                }
                ++t_writers_done;
            });
        };

        threads.emplace_back([&]() {
            while (t_writers_done.load() != num_writers) {
                auto wh = my_list.lock_write();
                for (auto iter = wh->begin(); iter != wh->end();) {
                    iter = wh->erase(iter);
                }
            }

            // Do one last time now that writers are finished
            auto wh = my_list.lock_write();
            for (auto iter = wh->begin(); iter != wh->end();) {
                iter = wh->erase(iter);
            }
        });

        while (!t_writers_done.load()) {
        }

        for (auto & thread : threads) {
            thread.join();
        }
    }

    {
        auto h = my_list.lock_read();

        int count = 0;
        volatile int escape;
        for (auto & item : *h) {
            escape = item;
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 0);
    }
}
