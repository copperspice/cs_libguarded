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

#include <catch2/catch.hpp>

#include <atomic>
#include <thread>

using namespace libguarded;

TEMPLATE_TEST_CASE("read lock traits", "[read_lock]",
		shared_guarded<int>, cow_guarded<int>)
{
   REQUIRE(std::is_default_constructible_v<typename TestType::shared_handle> == true);
   REQUIRE(std::is_move_constructible_v<typename TestType::shared_handle> == true);
   REQUIRE(std::is_move_assignable_v<typename TestType::shared_handle> == true);
}

TEMPLATE_TEST_CASE("read lock basic", "[read_lock]", shared_guarded<int>, cow_guarded<int>)
{
   SECTION("test multiple read lock")
   {
      TestType data(0);

      {
         auto data_handle = data.lock();

         ++(*data_handle);
      }

      {
         std::thread th1([&data]() { auto data_handle = data.lock_shared(); });
         std::thread th2([&data]() { auto data_handle = data.lock_shared(); });

         th1.join();
         th2.join();
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
