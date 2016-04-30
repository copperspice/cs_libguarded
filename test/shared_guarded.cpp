#include <libguarded/shared_guarded.hpp>

#include <boost/test/unit_test.hpp>

#include <atomic>
#include <thread>

#ifndef HAVE_CXX14
#error This file requires the C++14 shared_mutex functionality
#endif

#include <shared_mutex>
using shared_mutex = std::shared_timed_mutex;
namespace chrono   = std::chrono;

using namespace libguarded;

BOOST_AUTO_TEST_CASE(shared_guarded_1)
{

    shared_guarded<int, shared_mutex> data(0);

    {
        auto data_handle = data.lock();

        ++(*data_handle);
    }

    {
        auto data_handle = data.try_lock();

        BOOST_CHECK(data_handle != nullptr);
        BOOST_CHECK_EQUAL(*data_handle, 1);

        /* These tests must be done from another thread, because on
           glibc std::mutex is actually a recursive mutex. */

        std::atomic<bool> th1_ok(true);
        std::atomic<bool> th2_ok(true);
        std::atomic<bool> th3_ok(true);

        std::thread th1([&]() {
            auto data_handle2 = data.try_lock();
            if (data_handle2 != nullptr)
                th1_ok = false;
        });

        std::thread th2([&]() {
            auto data_handle2 = data.try_lock_for(std::chrono::milliseconds(20));
            if (data_handle2 != nullptr)
                th2_ok = false;
        });

        std::thread th3([&]() {
            auto data_handle2 = data.try_lock_until(std::chrono::steady_clock::now() +
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

    {
        auto data_handle = data.try_lock();

        BOOST_CHECK(data_handle != nullptr);
        BOOST_CHECK_EQUAL(*data_handle, 1);

        std::atomic<bool> th1_ok(true);
        std::atomic<bool> th2_ok(true);
        std::atomic<bool> th3_ok(true);

        std::thread th1([&]() {
            auto data_handle2 = data.try_lock_shared();
            if (data_handle2 != nullptr)
                th1_ok = false;
        });

        std::thread th2([&]() {
            auto data_handle2 = data.try_lock_shared_for(std::chrono::milliseconds(20));
            if (data_handle2 != nullptr)
                th2_ok = false;
        });

        std::thread th3([&]() {
            auto data_handle2 = data.try_lock_shared_until(std::chrono::steady_clock::now() +
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

    {
        auto data_handle = data.lock_shared();

        BOOST_CHECK(data_handle != nullptr);
        BOOST_CHECK_EQUAL(*data_handle, 1);

        std::atomic<bool> th1_ok(true);
        std::atomic<bool> th2_ok(true);
        std::atomic<bool> th3_ok(true);

        std::thread th1([&]() {
            auto data_handle2 = data.try_lock_shared();
            if (data_handle2 == nullptr)
                th1_ok = false;
            if (*data_handle2 != 1)
                th1_ok = false;
        });

        std::thread th2([&]() {
            auto data_handle2 = data.try_lock_shared_for(std::chrono::milliseconds(20));
            if (data_handle2 == nullptr)
                th2_ok = false;
            if (*data_handle2 != 1)
                th2_ok = false;
        });

        std::thread th3([&]() {
            auto data_handle2 = data.try_lock_shared_until(std::chrono::steady_clock::now() +
                                                           std::chrono::milliseconds(20));
            if (data_handle2 == nullptr)
                th3_ok = false;
            if (*data_handle2 != 1)
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

BOOST_AUTO_TEST_CASE(shared_guarded_2)
{
    shared_guarded<int, shared_mutex> data(0);

    std::thread th1([&data]() {
        for (int i = 0; i < 100000; ++i) {
            auto data_handle = data.lock();
            ++(*data_handle);
        }
    });

    std::thread th2([&data]() {
        for (int i = 0; i < 100000; ++i) {
            auto data_handle = data.lock();
            ++(*data_handle);
        }
    });

    std::thread th3([&data]() {
        int last_val = 0;
        while (last_val != 200000) {
            auto data_handle = data.lock_shared();
            BOOST_CHECK(last_val <= *data_handle);
            last_val = *data_handle;
        }
    });

    th1.join();
    th2.join();
    th3.join();

    auto data_handle = data.lock();

    BOOST_CHECK_EQUAL(*data_handle, 200000);
}
