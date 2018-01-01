/***********************************************************************
*
* Copyright (c) 2015-2018 Ansel Sermersheim
* All rights reserved.
*
* This file is part of libguarded
*
* libguarded is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

#ifndef LIBGUARDED_LR_GUARDED_HPP
#define LIBGUARDED_LR_GUARDED_HPP

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

namespace libguarded
{

/**
   \headerfile lr_guarded.hpp <libguarded/lr_guarded.hpp>

   This templated class wraps an object and allows only one thread at
   a time to modify the protected object.

   This class will use std::mutex for the internal locking mechanism
   by default. Other classes which are useful for the mutex type are
   std::recursive_mutex, std::timed_mutex, and
   std::recursive_timed_mutex.

   The handle returned by the various lock methods is moveable and
   copyable.

   Internally this class operates by maintaining two copies of the
   data and applying the same modifications to each copy in
   turn. Therefore the lr_guarded object will consume twice as much
   memory as one T plus a small amount of overhead.

 The T class must be copy constructible and copy assignable.
*/
template <typename T, typename Mutex = std::mutex>
class lr_guarded
{
  private:
    class shared_deleter;

  public:
    using shared_handle = std::unique_ptr<const T, shared_deleter>;

    /**
     Construct an lr_guarded object. This constructor will accept any
     number of parameters, all of which are forwarded to the
     constructor of T.
    */
    template <typename... Us>
    lr_guarded(Us &&... data);

    /**
     Modify the data by passing a functor. The functor must take
     exactly one argument of type T&. The functor will be called
     twice, once for each copy of the data. It must make the same
     modification for both invocations.

     If the first invocation of the functor throws an exception, the
     modification will be rolled back by copying from the unchanged
     copy. If the second invocation throws, the modification will be
     completed by copying from the changed copy. In either case if the
     copy constructor throws, the data is left in an indeterminate
     state.
    */
    template <typename Func>
    void modify(Func && f);

    /**
     Acquire a shared_handle to the protected object. Always succeeds without
     blocking.
    */
    shared_handle lock_shared() const;

    /**
     Acquire a shared_handle to the protected object. Always succeeds without
     blocking.
    */
    shared_handle try_lock_shared() const;

    /**
     Acquire a shared_handle to the protected object. Always succeeds without
     blocking.
    */
    template <class Duration>
    shared_handle try_lock_shared_for(const Duration & duration) const;

    /**
     Acquire a shared_handle to the protected object. Always succeeds without
     blocking.
    */
    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint & timepoint) const;

  private:
    class shared_deleter
    {
      public:
        using pointer = const T *;

        shared_deleter(const shared_deleter &) = delete;
        shared_deleter(shared_deleter &&)      = default;

        shared_deleter(std::atomic<int> & readingCount) : m_readingCount(readingCount)
        {
        }

        void operator()(const T * ptr)
        {
            if (ptr) {
                m_readingCount--;
            }
        }

      private:
        std::atomic<int> & m_readingCount;
    };

    T                        m_left;
    T                        m_right;
    std::atomic<bool>        m_readingLeft;
    std::atomic<bool>        m_countingLeft;
    mutable std::atomic<int> m_leftReadCount;
    mutable std::atomic<int> m_rightReadCount;
    mutable Mutex            m_writeMutex;
};

template <typename T, typename M>
template <typename... Us>
lr_guarded<T, M>::lr_guarded(Us &&... data)
    : m_left(std::forward<Us>(data)...), m_right(m_left), m_readingLeft(true), m_countingLeft(true),
      m_leftReadCount(0), m_rightReadCount(0)
{
}

template <typename T, typename M>
template <typename Func>
void lr_guarded<T, M>::modify(Func && func)
{
    // consider looser memory ordering

    std::lock_guard<M> lock(m_writeMutex);

    T * firstWriteLocation;
    T * secondWriteLocation;

    bool local_readingLeft = m_readingLeft.load();

    if (local_readingLeft) {
        firstWriteLocation  = &m_right;
        secondWriteLocation = &m_left;
    } else {
        firstWriteLocation  = &m_left;
        secondWriteLocation = &m_right;
    }

    try {
        func(*firstWriteLocation);
    } catch (...) {
        *firstWriteLocation = *secondWriteLocation;
        throw;
    }

    m_readingLeft.store(!local_readingLeft);

    bool local_countingLeft = m_countingLeft.load();

    if (local_countingLeft) {
        while (m_rightReadCount.load() != 0) {
            std::this_thread::yield();
        }
    } else {
        while (m_leftReadCount.load() != 0) {
            std::this_thread::yield();
        }
    }

    m_countingLeft.store(!local_countingLeft);

    if (local_countingLeft) {
        while (m_leftReadCount.load() != 0) {
            std::this_thread::yield();
        }
    } else {
        while (m_rightReadCount.load() != 0) {
            std::this_thread::yield();
        }
    }

    try {
        func(*secondWriteLocation);
    } catch (...) {
        *secondWriteLocation = *firstWriteLocation;
        throw;
    }
}

template <typename T, typename M>
auto lr_guarded<T, M>::lock_shared() const -> shared_handle
{
    if (m_countingLeft) {
        m_leftReadCount++;
        if (m_readingLeft) {
            return shared_handle(&m_left, shared_deleter(m_leftReadCount));
        } else {
            return shared_handle(&m_right, shared_deleter(m_leftReadCount));
        }
    } else {
        m_rightReadCount++;
        if (m_readingLeft) {
            return shared_handle(&m_left, shared_deleter(m_rightReadCount));
        } else {
            return shared_handle(&m_right, shared_deleter(m_rightReadCount));
        }
    }
}

template <typename T, typename M>
auto lr_guarded<T, M>::try_lock_shared() const -> shared_handle
{
    return lock_shared();
}

template <typename T, typename M>
template <typename Duration>
auto lr_guarded<T, M>::try_lock_shared_for(const Duration & duration) const -> shared_handle
{
    return lock_shared();
}

template <typename T, typename M>
template <typename TimePoint>
auto lr_guarded<T, M>::try_lock_shared_until(const TimePoint & timepoint) const -> shared_handle
{
    return lock_shared();
}
}
#endif
