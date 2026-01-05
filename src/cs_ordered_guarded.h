/***********************************************************************
*
* Copyright (c) 2016-2026 Ansel Sermersheim
*
* This file is part of CsLibGuarded.
*
* CsLibGuarded is free software which is released under the BSD 2-Clause license.
* For license details refer to the LICENSE provided with this project.
*
* CsLibGuarded is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef CSLIBGUARDED_ORDERED_GUARDED_H
#define CSLIBGUARDED_ORDERED_GUARDED_H

#include <memory>
#include <mutex>
#include <type_traits>
#include <shared_mutex>

namespace libguarded
{

/**
   \headerfile cs_ordered_guarded.h <CsLibGuarded/ordered_guarded.h>

   This templated class wraps an object. The protected object may be
   read by any number of threads simultaneously, but only one thread
   may modify the object at a time.

   This class will use std::shared_timed_mutex for the internal
   locking mechanism by default. In C++17, the class std::shared_mutex
   is available as well.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
template <typename T, typename M = std::shared_timed_mutex>
class ordered_guarded
{
   private:
      class shared_deleter;

   public:
      using shared_handle = std::unique_ptr<const T, shared_deleter>;

      /**
        Construct a guarded object. This constructor will accept any number
        of parameters, all of which are forwarded to the constructor of T.
       */
      template <typename... Us>
      ordered_guarded(Us &&... data);

      template <typename Func>
      decltype(auto) modify(Func &&func);

      template <typename Func>
      [[nodiscard]] decltype(auto) read(Func &&func) const;

      [[nodiscard]] shared_handle lock_shared() const;
      [[nodiscard]] shared_handle try_lock_shared() const;

      template <class Duration>
      [[nodiscard]] shared_handle try_lock_shared_for(const Duration &duration) const;

      template <class TimePoint>
      [[nodiscard]] shared_handle try_lock_shared_until(const TimePoint &timepoint) const;

   private:
      class shared_deleter
      {
         public:
            using pointer = const T *;

            shared_deleter() : m_deleter_mutex(nullptr) {}

            shared_deleter(M &mutex)
               : m_deleter_mutex(&mutex)
            {
            }

            void operator()(const T *ptr) {
               if (ptr && m_deleter_mutex) {
                  m_deleter_mutex->unlock_shared();
               }
            }

         private:
            M *m_deleter_mutex;
      };

      T m_obj;
      mutable M m_mutex;
};

template <typename T, typename M>
template <typename... Us>
ordered_guarded<T, M>::ordered_guarded(Us &&... data)
   : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
template <typename Func>
decltype(auto) ordered_guarded<T, M>::modify(Func &&func)
{
   std::lock_guard<M> lock(m_mutex);

   return func(m_obj);
}

template <typename T, typename M>
template <typename Func>
decltype(auto) ordered_guarded<T, M>::read(Func &&func) const
{
   std::shared_lock<M> lock(m_mutex);

   return func(m_obj);
}

template <typename T, typename M>
auto ordered_guarded<T, M>::lock_shared() const -> shared_handle
{
   m_mutex.lock_shared();
   return std::unique_ptr<const T, shared_deleter>(&m_obj, shared_deleter(m_mutex));
}

template <typename T, typename M>
auto ordered_guarded<T, M>::try_lock_shared() const -> shared_handle
{
   if (m_mutex.try_lock_shared()) {
      return std::unique_ptr<const T, shared_deleter>(&m_obj, shared_deleter(m_mutex));
   } else {
      return std::unique_ptr<const T, shared_deleter>(nullptr, shared_deleter(m_mutex));
   }
}

template <typename T, typename M>
template <typename Duration>
auto ordered_guarded<T, M>::try_lock_shared_for(const Duration &duration) const -> shared_handle
{
   if (m_mutex.try_lock_shared_for(duration)) {
      return std::unique_ptr<const T, shared_deleter>(&m_obj, shared_deleter(m_mutex));
   } else {
      return std::unique_ptr<const T, shared_deleter>(nullptr, shared_deleter(m_mutex));
   }
}

template <typename T, typename M>
template <typename TimePoint>
auto ordered_guarded<T, M>::try_lock_shared_until(const TimePoint &timepoint) const -> shared_handle
{
   if (m_mutex.try_lock_shared_until(timepoint)) {
      return std::unique_ptr<const T, shared_deleter>(&m_obj, shared_deleter(m_mutex));
   } else {
      return std::unique_ptr<const T, shared_deleter>(nullptr, shared_deleter(m_mutex));
   }
}

}  // namespace libguarded

#endif
