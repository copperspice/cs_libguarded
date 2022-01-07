/***********************************************************************
*
* Copyright (c) 2016-2022 Ansel Sermersheim
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

#include <cs_cow_guarded.h>
#include <cs_plain_guarded.h>
#include <cs_shared_guarded.h>
#include <mutex>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <atomic>
#include <thread>

using namespace libguarded;

TEMPLATE_TEST_CASE("exclusive lock basic", "[exclusive_lock]", plain_guarded<int>,
		shared_guarded<int>, cow_guarded<int>)
{
   SECTION("initialize")
   {
      TestType data(1);
      REQUIRE(*(data.lock()) == 1);
   }

   SECTION("test lock and increment")
   {
      TestType data(0);

      {
         auto data_handle = data.lock();

         ++(*data_handle);
      }
   }

   SECTION("test multiple writers")
   {
      TestType data(0);

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

      REQUIRE(*(data.lock()) == 20000);
   }
}

TEMPLATE_TEST_CASE("exclusive try_lock", "[exclusive_lock]", (plain_guarded<int, std::timed_mutex>),
                  (shared_guarded<int, std::timed_mutex>), (cow_guarded<int, std::shared_timed_mutex>))
{
   TestType data = 1;

   auto data_handle = data.try_lock();

   std::atomic<bool> th1_ok(true);
   std::atomic<bool> th2_ok(true);
   std::atomic<bool> th3_ok(true);

   REQUIRE(data_handle != nullptr);
   REQUIRE(*data_handle == 1);

   // These tests must be done from another thread, because on
   //  glibc std::mutex is actually a recursive mutex.

   std::thread th1([&data, &th1_ok]() {
      auto data_handle2 = data.try_lock();
      if (data_handle2 != nullptr) {
         th1_ok = false;
      }
   });

   std::thread th2([&data, &th2_ok]() {
      auto data_handle2 = data.try_lock_for(std::chrono::milliseconds(20));
      if (data_handle2 != nullptr) {
         th2_ok = false;
      }
   });

   std::thread th3([&data, &th3_ok]() {
      auto data_handle2 = data.try_lock_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(20));
      if (data_handle2 != nullptr) {
         th3_ok = false;
      }
   });

   th1.join();
   th2.join();
   th3.join();
   REQUIRE(th1_ok == true);
   REQUIRE(th2_ok == true);
   REQUIRE(th3_ok == true);
}
