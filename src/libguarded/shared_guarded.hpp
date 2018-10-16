/***********************************************************************
 *
 * Copyright (c) 2015-2017 Ansel Sermersheim
 * All rights reserved.
 *
 * This file is part of libguarded
 *
 * libguarded is free software, released under the BSD 2-Clause license.
 * For license details refer to LICENSE provided with this project.
 *
 ***********************************************************************/

#ifndef LIBGUARDED_SHARED_GUARDED_HPP
#define LIBGUARDED_SHARED_GUARDED_HPP

#include <memory>

#include "feature_check.hpp"

namespace libguarded
{

/**
   \headerfile shared_guarded.hpp <libguarded/shared_guarded.hpp>

   This templated class wraps an object and allows only one thread at
   a time to modify the protected object.

   This class will use std::shared_timed_mutex for the internal
   locking mechanism by default. In C++17 the std::shared_mutex class
   is also available.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
template <typename T, typename M = shared_timed_mutex>
class shared_guarded
{
  private:
    class deleter;
    class shared_deleter;

  public:
    using handle        = std::unique_ptr<T, deleter>;
    using shared_handle = std::unique_ptr<const T, shared_deleter>;

    template <typename... Us>
    shared_guarded(Us &&... data);

    // exclusive access
    handle lock();
    handle try_lock();

    template <class Duration>
    handle try_lock_for(const Duration &duration);

    template <class TimePoint>
    handle try_lock_until(const TimePoint &timepoint);

    // shared access, note "shared" in method names
    shared_handle lock_shared() const;
    shared_handle try_lock_shared() const;

    template <class Duration>
    shared_handle try_lock_shared_for(const Duration &duration) const;

    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint &timepoint) const;

  private:
    class deleter
    {
      public:
        using pointer = T *;

        deleter(M &mutex) : m_deleter_mutex(mutex)
        {
        }

        void operator()(T *ptr)
        {
            if (ptr) {
                m_deleter_mutex.unlock();
            }
        }

      private:
        M &m_deleter_mutex;
    };

    class shared_deleter
    {
      public:
        using pointer = const T *;

        shared_deleter(M &mutex) : m_deleter_mutex(mutex)
        {
        }

        void operator()(const T *ptr)
        {
            if (ptr) {
                m_deleter_mutex.unlock_shared();
            }
        }

      private:
        M &m_deleter_mutex;
    };

    T m_obj;
    mutable M m_mutex;
};

template <typename T, typename M>
template <typename... Us>
shared_guarded<T, M>::shared_guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
auto shared_guarded<T, M>::lock() -> handle
{
    m_mutex.lock();
    return handle(&m_obj, deleter(m_mutex));
}

template <typename T, typename M>
auto shared_guarded<T, M>::try_lock() -> handle
{
    if (m_mutex.try_lock()) {
        return handle(&m_obj, deleter(m_mutex));
    } else {
        return handle(nullptr, deleter(m_mutex));
    }
}

template <typename T, typename M>
template <typename Duration>
auto shared_guarded<T, M>::try_lock_for(const Duration &duration) -> handle
{
    if (m_mutex.try_lock_for(duration)) {
        return handle(&m_obj, deleter(m_mutex));
    } else {
        return handle(nullptr, deleter(m_mutex));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto shared_guarded<T, M>::try_lock_until(const TimePoint &timepoint) -> handle
{
    if (m_mutex.try_lock_until(timepoint)) {
        return handle(&m_obj, deleter(m_mutex));
    } else {
        return handle(nullptr, deleter(m_mutex));
    }
}

template <typename T, typename M>
auto shared_guarded<T, M>::lock_shared() const -> shared_handle
{
    m_mutex.lock_shared();
    return shared_handle(&m_obj, shared_deleter(m_mutex));
}

template <typename T, typename M>
auto shared_guarded<T, M>::try_lock_shared() const -> shared_handle
{
    if (m_mutex.try_lock_shared()) {
        return shared_handle(&m_obj, shared_deleter(m_mutex));
    } else {
        return shared_handle(nullptr, shared_deleter(m_mutex));
    }
}

template <typename T, typename M>
template <typename Duration>
auto shared_guarded<T, M>::try_lock_shared_for(const Duration &d) const -> shared_handle
{
    if (m_mutex.try_lock_shared_for(d)) {
        return shared_handle(&m_obj, shared_deleter(m_mutex));
    } else {
        return shared_handle(nullptr, shared_deleter(m_mutex));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto shared_guarded<T, M>::try_lock_shared_until(const TimePoint &tp) const -> shared_handle
{
    if (m_mutex.try_lock_shared_until(tp)) {
        return shared_handle(&m_obj, shared_deleter(m_mutex));
    } else {
        return shared_handle(nullptr, shared_deleter(m_mutex));
    }
}
} // namespace libguarded

#endif
