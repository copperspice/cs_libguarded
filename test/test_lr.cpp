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

#include <cs_lr_guarded.h>

#include <thread>

#include <catch2/catch.hpp>

using namespace libguarded;

TEST_CASE("LR guarded 1", "[lr_guarded]")
{
   lr_guarded<int, std::timed_mutex> data(0);

   {
      data.modify([](int & x) { ++x; });
   }

   {
      auto data_handle = data.lock_shared();

      REQUIRE(data_handle != nullptr);
      REQUIRE(*data_handle == 1);

      std::thread th1([&data]() {
         auto data_handle2 = data.try_lock_shared();
         REQUIRE(data_handle2 != nullptr);
         REQUIRE(*data_handle2 == 1);
      });

      std::thread th2([&data]() {
         auto data_handle2 = data.try_lock_shared_for(std::chrono::milliseconds(20));
         REQUIRE(data_handle2 != nullptr);
         REQUIRE(*data_handle2 == 1);

      });

      std::thread th3([&data]() {
         auto data_handle2 = data.try_lock_shared_until(std::chrono::steady_clock::now() +
                                                        std::chrono::milliseconds(20));
         REQUIRE(data_handle2 != nullptr);
         REQUIRE(*data_handle2 == 1);
      });

      th1.join();

      th2.join();
      th3.join();
   }

   {
      auto data_handle = data.lock_shared();

      REQUIRE(data_handle != nullptr);
      REQUIRE(*data_handle == 1);
   }
}

TEST_CASE("LR guarded 2", "[lr_guarded]")
{
   lr_guarded<int> data(0);

   std::thread th1([&data]() {
      for (int i = 0; i < 100000; ++i) {
         data.modify([](int & x) { ++x; });
      }
   });

   std::thread th2([&data]() {
      for (int i = 0; i < 100000; ++i) {
         data.modify([](int & x) { ++x; });
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

   auto data_handle = data.lock_shared();

   REQUIRE(*data_handle == 200000);
}
