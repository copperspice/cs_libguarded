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

#include <cs_rcu_guarded.h>
#include <cs_rcu_list.h>

#include <thread>
#include <iostream>

#include <catch2/catch.hpp>

using namespace libguarded;

TEST_CASE("RCU guarded 1", "[rcu_guarded]")
{
   rcu_guarded<rcu_list<int>> my_list;

   {
      auto h = my_list.lock_write();
      h->push_back(42);
   }

   {
      auto h = my_list.lock_read();

      int count = 0;
      for (auto &item : *h) {
         ++count;
         REQUIRE(item == 42);
      }
      REQUIRE(count == 1);
   }

   {
      auto rh = my_list.lock_read();
      auto wh = my_list.lock_write();

      auto iter = rh->begin();

      wh->erase(wh->begin());

      int count = 0;
      for (; iter != rh->end(); ++iter) {
         ++count;
         REQUIRE(*iter == 42);
      }
      REQUIRE(count == 1);
   }

   {
      auto h = my_list.lock_read();

      int count = 0;
      volatile int escape;

      for (auto &item : *h) {
         escape = item;
         ++count;
      }
      REQUIRE(count == 0);
   }

   {
      constexpr const int num_writers = 8;
      std::atomic<int> t_writers_done{0};

      std::vector<std::thread> threads;

      for (int i = 0; i < num_writers; ++i) {
         threads.emplace_back([&]() {
            while (! t_writers_done.load()) {
               auto rh = my_list.lock_write();
               volatile int escape;

               for (auto item : *rh) {
                  escape = item;
               }
            }
         });

         threads.emplace_back([&]() {
            int count = 0;

            while (! t_writers_done.load() && count < 1000) {
               auto wh = my_list.lock_write();
               volatile int escape;

               for (auto item : *wh) {
                  escape = item;
               }

               for (int i = 0; i < 2; ++i) {
                  wh->emplace_back(i);
                  wh->emplace_front(i - 1);
                  wh->emplace(wh->begin(), i);
                  auto iter = wh->begin();

                  for(int count = 0; count < 500; ++count) {
                     auto last = iter;
                     ++iter;
                     if(iter == wh->end()) {
                        wh->emplace(last, i - 50);
                        break;
                     }
                  }

                  wh->emplace(++(wh->begin()), i);
                  wh->push_back(i + 4);
                  wh->push_front(i - 7);
               }
               ++count;
            }

            ++t_writers_done;
         });
      }

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

      for (auto &thread : threads) {
         thread.join();
      }
   }

   {
      auto h = my_list.lock_read();

      int count = 0;
      volatile int escape;

      for (auto &item : *h) {
         escape = item;
         ++count;
     }
     REQUIRE(count == 0);
   }
}

// allocation events recorded by mock_allocator
struct event {
   size_t size;
   bool allocated;    // true for allocate(), false for deallocate()
};
using event_log = std::vector<event>;

template <typename T>
class mock_allocator
{
   public:
      using value_type = T;

      explicit mock_allocator(event_log *log)
         : log(log)
      {
      }

      mock_allocator(const mock_allocator& other)
         : log(other.log)
      {
      }

      // converting copy constructor (requires friend)
      template <typename> friend class mock_allocator;
      template <typename U>
      mock_allocator(const mock_allocator<U>& other)
         : log(other.log)
      {
      }

      T *allocate(size_t n, const void *hint = nullptr) {
         auto p = std::allocator<T>{}.allocate(n, hint);
         log->emplace_back(event{n * sizeof(T), true});

         return p;
      }

      void deallocate(T *p, size_t n) {
         std::allocator<T>{}.deallocate(p, n);

         if (p) {
            log->emplace_back(event{n * sizeof(T), false});
         }
      }

   private:
      event_log *const log;
};

TEST_CASE("RCU guarded 2", "[rcu_guarded]")
{
   // large value type makes it easy to distinguish nodes from zombies
   // (this avoids any dependency on the private rcu_list node types)
   constexpr size_t value_size = 256;
   auto is_zombie = [=] (const event& e) { return e.size < value_size; };
   auto is_alloc = [] (const event& e) { return e.allocated; };

   using T = std::aligned_storage<value_size>::type;

   event_log log;
   {
      mock_allocator<T> alloc{&log};
      rcu_guarded<rcu_list<T, std::mutex, mock_allocator<T>>> my_list(alloc);

      auto h = my_list.lock_write();  // allocates zombie
      h->emplace_back();              // allocates node
      h->erase(h->begin());           // allocates zombie

      // expect 3 allocations, two of which are zombies. just count events,
      // do not make assumptions about ordering
      REQUIRE(3 == log.size());
      REQUIRE(3 == std::count_if(log.begin(), log.end(), is_alloc));
      REQUIRE(2 == std::count_if(log.begin(), log.end(), is_zombie));
   }

   // expects 3 new deallocations, two of which are zombies
   REQUIRE(6 == log.size());
   REQUIRE(3 == std::count_if(log.begin(), log.end(), is_alloc));
   REQUIRE(4 == std::count_if(log.begin(), log.end(), is_zombie));
}
