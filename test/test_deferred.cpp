#include <libguarded/deferred_guarded.hpp>

#include <boost/test/unit_test.hpp>

#include <thread>

#ifndef HAVE_CXX14
#error This file requires the C++14 shared_mutex functionality
#endif

#include <shared_mutex>
using shared_mutex = std::shared_timed_mutex;

using namespace libguarded;

BOOST_AUTO_TEST_CASE(deferred_guarded_1)
{

    deferred_guarded<int, shared_mutex> data(0);

    data.modify_detach([](int & x) { ++x; });

    {
        auto data_handle = data.lock_shared();

        BOOST_CHECK(data_handle != nullptr);
        BOOST_CHECK_EQUAL(*data_handle, 1);

        std::atomic<bool> th1_ok(true);
        std::atomic<bool> th2_ok(true);
        std::atomic<bool> th3_ok(true);

        std::thread th1([&data, &th1_ok]() {
            auto data_handle2 = data.try_lock_shared();
            if (data_handle2 == nullptr)
                th1_ok = false;
            if (*data_handle2 != 1)
                th1_ok = false;
        });

        std::thread th2([&data, &th2_ok]() {
            auto data_handle2 = data.try_lock_shared_for(std::chrono::milliseconds(20));
            if (data_handle2 == nullptr)
                th2_ok = false;
            if (*data_handle2 != 1)
                th2_ok = false;
        });

        std::thread th3([&data, &th3_ok]() {
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

BOOST_AUTO_TEST_CASE(deferred_guarded_2)
{
    deferred_guarded<int, shared_mutex> data(0);

    std::thread th1([&data]() {
        for (int i = 0; i < 100000; ++i) {
            data.modify_detach([](int & x) { ++x; });
        }
    });

    std::thread th2([&data]() {
        for (int i = 0; i < 100000; ++i) {
            auto fut = data.modify_async([](int & x) -> int { return ++x; });
            fut.wait();
        }
    });

    std::thread th3([&data]() {
        for (int i = 0; i < 100000; ++i) {
            auto fut = data.modify_async([](int & x) -> void { ++x; });
            fut.wait();
        }
    });

    std::thread th4([&data]() {
        int last_val = 0;
        while (last_val != 300000) {
            auto data_handle = data.lock_shared();
            BOOST_CHECK(last_val <= *data_handle);
            last_val = *data_handle;
        }
    });

    th1.join();
    th2.join();
    th3.join();
    th4.join();

    auto data_handle = data.lock_shared();

    BOOST_CHECK_EQUAL(*data_handle, 300000);
}
