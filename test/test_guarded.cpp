#include <libguarded/guarded.hpp>

#define BOOST_TEST_MODULE guarded_test
#include <boost/test/included/unit_test.hpp>

#include <atomic>
#include <thread>

using namespace libguarded;

BOOST_AUTO_TEST_CASE(guarded_1)
{

    guarded<int, std::timed_mutex> data(0);

    {
        auto data_handle = data.lock();

        ++(*data_handle);
    }

    {
        auto data_handle = data.try_lock();

        std::atomic<bool> th1_ok(true);
        std::atomic<bool> th2_ok(true);
        std::atomic<bool> th3_ok(true);

        BOOST_CHECK(data_handle != nullptr);
        BOOST_CHECK_EQUAL(*data_handle, 1);

        /* These tests must be done from another thread, because on
           glibc std::mutex is actually a recursive mutex. */

        std::thread th1([&data, &th1_ok]() {
            auto data_handle2 = data.try_lock();
            if (data_handle2 != nullptr)
                th1_ok = false;
        });

        std::thread th2([&data, &th2_ok]() {
            auto data_handle2 =
                data.try_lock_for(std::chrono::milliseconds(20));
            if (data_handle2 != nullptr)
                th2_ok = false;
        });

        std::thread th3([&data, &th3_ok]() {
            auto data_handle2 =
                data.try_lock_until(std::chrono::steady_clock::now() +
                                    std::chrono::milliseconds(20));
            if (data_handle2 != nullptr)
                th3_ok = false;
        });

        th1.join();
        th2.join();
        th3.join();
        BOOST_CHECK(th1_ok == true);
        BOOST_CHECK(th2_ok == true);
        BOOST_CHECK(th3_ok == true);
    }
}

BOOST_AUTO_TEST_CASE(guarded_2)
{
    guarded<int> data(0);

    std::thread th1([&data]() {
        for (int i = 0; i < 10000; ++i) {
            auto data_handle = data.lock();
            ++(*data_handle);
        }
    });

    std::thread th2([&data]() {
        for (int i = 0; i < 10000; ++i) {
            auto data_handle = data.lock();
            ++(*data_handle);
        }
    });

    th1.join();
    th2.join();

    auto data_handle = data.lock();

    BOOST_CHECK_EQUAL(*data_handle, 20000);
}
