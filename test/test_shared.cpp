/***********************************************************************
*
* Copyright (c) 2015-2020 Ansel Sermersheim
*
* This file is part of CsLibGuarded.
*
* CsLibGuarded is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#include <cs_shared_guarded.h>


#include <atomic>
#include <thread>

#include <shared_mutex>
using shared_mutex = std::shared_timed_mutex;
namespace chrono   = std::chrono;

#include <catch2/catch.hpp>

using namespace libguarded;

TEST_CASE("Shared guarded 1", "[shared_guarded]")
{

    shared_guarded<int, shared_mutex> data(0);

    {
        auto data_handle = data.lock();

	++(*data_handle);

	data_handle.reset();
	data_handle = data.lock();
    }

    {
        auto data_handle = data.try_lock();

        REQUIRE(data_handle != nullptr);
        REQUIRE(*data_handle == 1);

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

        REQUIRE(th1_ok == true);
        REQUIRE(th2_ok == true);
        REQUIRE(th3_ok == true);
    }

    {
        auto data_handle = data.try_lock();

        REQUIRE(data_handle != nullptr);
        REQUIRE(*data_handle == 1);

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

        REQUIRE(th1_ok == true);
        REQUIRE(th2_ok == true);
        REQUIRE(th3_ok == true);
    }

    {
        auto data_handle = data.lock_shared();

        REQUIRE(data_handle != nullptr);
        REQUIRE(*data_handle == 1);

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

        REQUIRE(th1_ok == true);
        REQUIRE(th2_ok == true);
        REQUIRE(th3_ok == true);
    }
}

TEST_CASE("Shared guarded 2", "[shared_guarded]")
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
            REQUIRE(last_val <= *data_handle);
            last_val = *data_handle;
        }
    });

    th1.join();
    th2.join();
    th3.join();

    auto data_handle = data.lock();

    REQUIRE(*data_handle == 200000);
}
