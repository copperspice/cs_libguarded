/*
 * Copyright 2016-2017 Ansel Sermersheim
 *
 * All rights reserved
 *
 * This file is part of libguarded. Libguarded is free software
 * released under the BSD 2-clause license. For more information see
 * the LICENCE file provided with this project.
 */

#ifndef INCLUDED_LIBGUARDED_ORDERED_GUARDED_HPP
#define INCLUDED_LIBGUARDED_ORDERED_GUARDED_HPP

#include <memory>
#include <mutex>

#if HAVE_CXX14
#include <shared_mutex>
#else
namespace std
{
class shared_timed_mutex;
}
#endif

namespace libguarded
{

template <typename T, typename M = std::shared_timed_mutex>
class ordered_guarded
{
  private:
    class shared_deleter;

  public:
    using shared_handle = std::unique_ptr<const T, shared_deleter>;

    template <typename... Us>
    ordered_guarded(Us &&... data);

    template <typename Func>
    void modify(Func && func);

    shared_handle lock_shared() const;
    shared_handle try_lock_shared() const;

    template <class Duration>
    shared_handle try_lock_shared_for(const Duration & duration) const;

    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint & timepoint) const;

  private:
    class shared_deleter
    {
      public:
        using pointer = const T *;

        shared_deleter(M & mutex) : m_deleter_mutex(mutex)
        {
        }

        void operator()(const T * ptr)
        {
            if (ptr) {
                m_deleter_mutex.unlock_shared();
            }
        }

      private:
        M & m_deleter_mutex;
    };

    T         m_obj;
    mutable M m_mutex;
};

template <typename T, typename M>
template <typename... Us>
ordered_guarded<T, M>::ordered_guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
template <typename Func>
void ordered_guarded<T, M>::modify(Func && func)
{
    std::lock_guard<M> lock(m_mutex);

    func(m_obj);
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
auto ordered_guarded<T, M>::try_lock_shared_for(const Duration & duration) const -> shared_handle
{
    if (m_mutex.try_lock_shared_for(duration)) {
        return std::unique_ptr<const T, shared_deleter>(&m_obj, shared_deleter(m_mutex));
    } else {
        return std::unique_ptr<const T, shared_deleter>(nullptr, shared_deleter(m_mutex));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto ordered_guarded<T, M>::try_lock_shared_until(const TimePoint & timepoint) const
    -> shared_handle
{
    if (m_mutex.try_lock_shared_until(timepoint)) {
        return std::unique_ptr<const T, shared_deleter>(&m_obj, shared_deleter(m_mutex));
    } else {
        return std::unique_ptr<const T, shared_deleter>(nullptr, shared_deleter(m_mutex));
    }
}
}
#endif
