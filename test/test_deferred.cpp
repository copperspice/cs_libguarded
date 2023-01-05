/***********************************************************************
*
* Copyright (c) 2016-2023 Ansel Sermersheim
*
* This file is part of CsLibGuarded.
*
* CsLibGuarded is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CsLibGuarded is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#include <cs_deferred_guarded.h>

#include <thread>
#include <shared_mutex>
using shared_mutex = std::shared_timed_mutex;

#include <catch2/catch.hpp>

using namespace libguarded;

TEST_CASE("Deferred guarded 1", "[deferred_guarded]")
{
   deferred_guarded<int, shared_mutex> data(0);

   data.modify_detach([](int & x) { ++x; });

   {
      auto data_handle = data.lock_shared();

      REQUIRE(data_handle != nullptr);
      REQUIRE(*data_handle == 1);

      std::atomic<bool> th1_ok(true);
      std::atomic<bool> th2_ok(true);
      std::atomic<bool> th3_ok(true);

      std::thread th1([&data, &th1_ok]() {
         auto data_handle2 = data.try_lock_shared();
         if (data_handle2 == nullptr) {
            th1_ok = false;
         }

         if (*data_handle2 != 1) {
            th1_ok = false;
         }
      });

      std::thread th2([&data, &th2_ok]() {
         auto data_handle2 = data.try_lock_shared_for(std::chrono::milliseconds(20));

         if (data_handle2 == nullptr) {
             th2_ok = false;
         }

         if (*data_handle2 != 1) {
             th2_ok = false;
         }
      });

      std::thread th3([&data, &th3_ok]() {
         auto data_handle2 = data.try_lock_shared_until(std::chrono::steady_clock::now() +
                                                        std::chrono::milliseconds(20));
         if (data_handle2 == nullptr) {
             th3_ok = false;
         }

         if (*data_handle2 != 1) {
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
}

TEST_CASE("Deferred guarded 2", "[deferred_guarded]")
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
         REQUIRE(last_val <= *data_handle);
         last_val = *data_handle;
      }
   });

   th1.join();
   th2.join();
   th3.join();
   th4.join();

   auto data_handle = data.lock_shared();

   REQUIRE(*data_handle == 300000);
}
