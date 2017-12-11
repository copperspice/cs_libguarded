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

#ifndef LIBGUARDED_ORDERED_GUARDED_HPP
#define LIBGUARDED_ORDERED_GUARDED_HPP

#include <memory>
#include <mutex>
#include <utility>

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

/**
   \headerfile ordered_guarded.hpp <libguarded/ordered_guarded.hpp>

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
    decltype(std::declval<Func>()(std::declval<T>())) modify(Func && func) ;

    template <typename Func>
    void read(Func && func) const;

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
decltype(std::declval<Func>()(std::declval<T>())) ordered_guarded<T, M>::modify(Func && func)
{
    std::lock_guard<M> lock(m_mutex);

    return func(m_obj);
}

template <typename T, typename M>
template <typename Func>
void ordered_guarded<T, M>::read(Func && func) const
{
    std::shared_lock<M> lock(m_mutex);

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
