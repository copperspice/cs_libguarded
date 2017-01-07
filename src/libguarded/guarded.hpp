/*
 * Copyright 2016-2017 Ansel Sermersheim
 *
 * All rights reserved
 *
 * This file is part of libguarded. Libguarded is free software
 * released under the BSD 2-clause license. For more information see
 * the LICENCE file provided with this project.
 */

#ifndef INCLUDED_LIBGUARDED_GUARDED_HPP
#define INCLUDED_LIBGUARDED_GUARDED_HPP

#include <memory>
#include <mutex>

namespace libguarded
{

template <typename T, typename M = std::mutex>
class guarded
{
  private:
    class deleter;

  public:
    using handle = std::unique_ptr<T, deleter>;

    template <typename... Us>
    guarded(Us &&... data);

    // exclusive access
    handle lock();
    handle try_lock();

    template <class Duration>
    handle try_lock_for(const Duration & duration);

    template <class TimePoint>
    handle try_lock_until(const TimePoint & timepoint);

  private:
    class deleter
    {
      public:
        using pointer = T *;

        deleter(std::unique_lock<M> lock) : m_lock(std::move(lock))
        {
        }

        void operator()(T * ptr)
        {
            if (m_lock.owns_lock()) {
                m_lock.unlock();
            }
        }

      private:
        std::unique_lock<M> m_lock;
    };

    T m_obj;
    M m_mutex;
};

template <typename T, typename M>
template <typename... Us>
guarded<T, M>::guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
auto guarded<T, M>::lock() -> handle
{
    std::unique_lock<M> lock(m_mutex);
    return handle(&m_obj, deleter(std::move(lock)));
}

template <typename T, typename M>
auto guarded<T, M>::try_lock() -> handle
{
    std::unique_lock<M> lock(m_mutex, std::try_to_lock);

    if (lock.owns_lock()) {
        return handle(&m_obj, deleter(std::move(lock)));
    } else {
        return handle(nullptr, deleter(std::move(lock)));
    }
}

template <typename T, typename M>
template <typename Duration>
auto guarded<T, M>::try_lock_for(const Duration & d) -> handle
{
    std::unique_lock<M> lock(m_mutex, d);

    if (lock.owns_lock()) {
        return handle(&m_obj, deleter(std::move(lock)));
    } else {
        return handle(nullptr, deleter(std::move(lock)));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto guarded<T, M>::try_lock_until(const TimePoint & tp) -> handle
{
    std::unique_lock<M> lock(m_mutex, tp);

    if (lock.owns_lock()) {
        return handle(&m_obj, deleter(std::move(lock)));
    } else {
        return handle(nullptr, deleter(std::move(lock)));
    }
}
}

#endif
