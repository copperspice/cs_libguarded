
#include <libguarded/cow_guarded.hpp>

#include <thread>

#include <boost/test/unit_test.hpp>

using namespace libguarded;

BOOST_AUTO_TEST_CASE(cow_guarded_1)
{

    cow_guarded<int, std::timed_mutex> data(0);

    {
        auto data_handle = data.lock();

        ++(*data_handle);
    }

    {
        auto data_handle = data.lock_shared();

        BOOST_CHECK(data_handle != nullptr);
        BOOST_CHECK_EQUAL(*data_handle, 1);

        std::thread th1([&data]() {
            auto data_handle2 = data.try_lock_shared();
            BOOST_CHECK(data_handle2 != nullptr);
            BOOST_CHECK_EQUAL(*data_handle2, 1);
        });

        std::thread th2([&data]() {
            auto data_handle2 = data.try_lock_shared_for(std::chrono::milliseconds(20));
            BOOST_CHECK(data_handle2 != nullptr);
            BOOST_CHECK_EQUAL(*data_handle2, 1);

        });

        std::thread th3([&data]() {
            auto data_handle2 = data.try_lock_shared_until(std::chrono::steady_clock::now() +
                                                           std::chrono::milliseconds(20));
            BOOST_CHECK(data_handle2 != nullptr);
            BOOST_CHECK_EQUAL(*data_handle2, 1);
        });

        th1.join();

        th2.join();
        th3.join();
    }

    {
        auto data_handle = data.lock();

        auto data_handle2 = data.lock_shared();

        ++(*data_handle);
        BOOST_CHECK_EQUAL(*data_handle, 2);

        BOOST_CHECK(data_handle2 != nullptr);
        BOOST_CHECK_EQUAL(*data_handle2, 1);

        data_handle.cancel();
        BOOST_CHECK(data_handle == nullptr);

        BOOST_CHECK(data_handle2 != nullptr);
        BOOST_CHECK_EQUAL(*data_handle2, 1);
    }

    {
        auto data_handle = data.lock_shared();

        BOOST_CHECK(data_handle != nullptr);
        BOOST_CHECK_EQUAL(*data_handle, 1);
    }
}

BOOST_AUTO_TEST_CASE(cow_guarded_2)
{
    cow_guarded<int, std::timed_mutex> data(0);

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
        for (int i = 0; i < 100000; ++i) {
            while (last_val != 200000) {
                auto data_handle = data.lock_shared();
                BOOST_CHECK(last_val <= *data_handle);
                last_val = *data_handle;
            }
        }
    });

    th1.join();
    th2.join();
    th3.join();

    auto data_handle = data.lock_shared();

    BOOST_CHECK_EQUAL(*data_handle, 200000);
}
