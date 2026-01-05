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

#ifndef CSLIBGUARDED_PLAIN_GUARDED_H
#define CSLIBGUARDED_PLAIN_GUARDED_H

#include <memory>
#include <mutex>

namespace libguarded
{

/**
   \headerfile cs_plain_guarded.h <CsLibGuarded/cs_plain_guarded.h>

   This templated class wraps an object and allows only one thread at a
   time to access the protected object.

   This class will use std::mutex for the internal locking mechanism by
   default. Other classes which are useful for the mutex type are
   std::recursive_mutex, std::timed_mutex, and
   std::recursive_timed_mutex.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
template <typename T, typename M = std::mutex>
class plain_guarded
{
   private:
      class deleter;
      class const_deleter;

   public:
      using handle       = std::unique_ptr<T, deleter>;
      using const_handle = std::unique_ptr<const T, const_deleter>;

      /**
        Construct a guarded object. This constructor will accept any
        number of parameters, all of which are forwarded to the
        constructor of T.
       */
      template <typename... Us>
      plain_guarded(Us &&... data);

      /**
        Acquire a handle to the protected object. As a side effect, the
        protected object will be locked from access by any other
        thread. The lock will be automatically released when the handle
        is destroyed.
       */
      [[nodiscard]] handle lock();
      [[nodiscard]] const_handle lock() const;

      /**
        Attempt to acquire a handle to the protected object. Returns a
        null handle if the object is already locked. As a side effect,
        the protected object will be locked from access by any other
        thread. The lock will be automatically released when the handle
        is destroyed.
       */
      [[nodiscard]] handle try_lock();
      [[nodiscard]] const_handle try_lock() const;

      /**
        Attempt to acquire a handle to the protected object. As a side
        effect, the protected object will be locked from access by any
        other thread. The lock will be automatically released when the
        handle is destroyed.

        Returns a null handle if the object is already locked, and does
        not become available for locking before the time duration has
        elapsed.

        Calling this method requires that the underlying mutex type M
        supports the try_lock_for method.  This is not true if M is the
        default std::mutex.
       */
      template <class Duration>
      [[nodiscard]] handle try_lock_for(const Duration &duration);
      template <class Duration>
      [[nodiscard]] const_handle try_lock_for(const Duration &duration) const;

      /**
        Attempt to acquire a handle to the protected object.  As a side
        effect, the protected object will be locked from access by any other
        thread. The lock will be automatically released when the handle is
        destroyed.

        Returns a null handle if the object is already locked, and does not
        become available for locking before reaching the specified timepoint.

        Calling this method requires that the underlying mutex type M
        supports the try_lock_until method.  This is not true if M is the
        default std::mutex.
       */
      template <class TimePoint>
      [[nodiscard]] handle try_lock_until(const TimePoint &timepoint);
      template <class TimePoint>
      [[nodiscard]] const_handle try_lock_until(const TimePoint &timepoint) const;

   private:
      T m_obj;
      mutable M m_mutex;
};

template <typename T, typename M>
class plain_guarded<T, M>::deleter
{
   public:
      using pointer = T *;

      deleter() = default;
      deleter(std::unique_lock<M> lock);

      void operator()(T *ptr);

   private:
      std::unique_lock<M> m_lock;
};

template <typename T, typename M>
plain_guarded<T, M>::deleter::deleter(std::unique_lock<M> lock)
   : m_lock(std::move(lock))
{
}

template <typename T, typename M>
void plain_guarded<T, M>::deleter::operator()(T *)
{
   if (m_lock.owns_lock()) {
      m_lock.unlock();
   }
}

template <typename T, typename M>
class plain_guarded<T, M>::const_deleter
{
   public:
      using pointer = const T *;

      const_deleter() = default;
      const_deleter(std::unique_lock<M> lock);

      void operator()(const T *ptr);

   private:
      std::unique_lock<M> m_lock;
};

template <typename T, typename M>
plain_guarded<T, M>::const_deleter::const_deleter(std::unique_lock<M> lock)
   : m_lock(std::move(lock))
{
}

template <typename T, typename M>
void plain_guarded<T, M>::const_deleter::operator()(const T *)
{
   if (m_lock.owns_lock()) {
      m_lock.unlock();
   }
}

template <typename T, typename M>
template <typename... Us>
plain_guarded<T, M>::plain_guarded(Us &&... data)
   : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
auto plain_guarded<T, M>::lock() -> handle
{
   std::unique_lock<M> lock(m_mutex);
   return handle(&m_obj, deleter(std::move(lock)));
}

template <typename T, typename M>
auto plain_guarded<T, M>::lock() const -> const_handle
{
   std::unique_lock<M> lock(m_mutex);
   return const_handle(&m_obj, const_deleter(std::move(lock)));
}

template <typename T, typename M>
auto plain_guarded<T, M>::try_lock() -> handle
{
   std::unique_lock<M> lock(m_mutex, std::try_to_lock);

   if (lock.owns_lock()) {
      return handle(&m_obj, deleter(std::move(lock)));
   } else {
      return handle(nullptr, deleter(std::move(lock)));
   }
}

template <typename T, typename M>
auto plain_guarded<T, M>::try_lock() const -> const_handle
{
   std::unique_lock<M> lock(m_mutex, std::try_to_lock);

   if (lock.owns_lock()) {
      return const_handle(&m_obj, const_deleter(std::move(lock)));
   } else {
      return const_handle(nullptr, const_deleter(std::move(lock)));
   }
}

template <typename T, typename M>
template <typename Duration>
auto plain_guarded<T, M>::try_lock_for(const Duration &d) -> handle
{
   std::unique_lock<M> lock(m_mutex, d);

   if (lock.owns_lock()) {
      return handle(&m_obj, deleter(std::move(lock)));
   } else {
      return handle(nullptr, deleter(std::move(lock)));
   }
}

template <typename T, typename M>
template <typename Duration>
auto plain_guarded<T, M>::try_lock_for(const Duration &d) const -> const_handle
{
   std::unique_lock<M> lock(m_mutex, d);

   if (lock.owns_lock()) {
      return const_handle(&m_obj, const_deleter(std::move(lock)));
   } else {
      return const_handle(nullptr, const_deleter(std::move(lock)));
   }
}

template <typename T, typename M>
template <typename TimePoint>
auto plain_guarded<T, M>::try_lock_until(const TimePoint &tp) -> handle
{
   std::unique_lock<M> lock(m_mutex, tp);

   if (lock.owns_lock()) {
      return handle(&m_obj, deleter(std::move(lock)));
   } else {
      return handle(nullptr, deleter(std::move(lock)));
   }
}

template <typename T, typename M>
template <typename TimePoint>
auto plain_guarded<T, M>::try_lock_until(const TimePoint &tp) const -> const_handle
{
    std::unique_lock<M> lock(m_mutex, tp);

    if (lock.owns_lock()) {
        return const_handle(&m_obj, const_deleter(std::move(lock)));
    } else {
        return const_handle(nullptr, const_deleter(std::move(lock)));
    }
}

template <typename T, typename M = std::mutex>
using guarded [[deprecated("renamed to plain_guarded")]] = plain_guarded<T, M>;

}  // namespace libguarded

#endif
