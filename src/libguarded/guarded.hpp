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

#ifndef LIBGUARDED_GUARDED_HPP
#define LIBGUARDED_GUARDED_HPP

#include <memory>
#include <mutex>

namespace libguarded
{

/**
   \headerfile guarded.hpp <libguarded/guarded.hpp>

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
class guarded
{
  private:
    class deleter;

  public:
    using handle = std::unique_ptr<T, deleter>;

    /**
     Construct a guarded object. This constructor will accept any
     number of parameters, all of which are forwarded to the
     constructor of T.
    */
    template <typename... Us>
    guarded(Us &&... data);

    /**
     Acquire a handle to the protected object. As a side effect, the
     protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle lock();

    /**
     Attempt to acquire a handle to the protected object. Returns a
     null handle if the object is already locked. As a side effect,
     the protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle try_lock();

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
    handle try_lock_for(const Duration &duration);

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
    handle try_lock_until(const TimePoint &timepoint);

  private:
    T m_obj;
    M m_mutex;
};

template <typename T, typename M>
class guarded<T, M>::deleter
{
  public:
    using pointer = T *;

    deleter(std::unique_lock<M> lock);

    void operator()(T *ptr);

  private:
    std::unique_lock<M> m_lock;
};

template <typename T, typename M>
guarded<T, M>::deleter::deleter(std::unique_lock<M> lock) : m_lock(std::move(lock))
{
}

template <typename T, typename M>
void guarded<T, M>::deleter::operator()(T *)
{
    if (m_lock.owns_lock()) {
        m_lock.unlock();
    }
}

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
auto guarded<T, M>::try_lock_for(const Duration &d) -> handle
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
auto guarded<T, M>::try_lock_until(const TimePoint &tp) -> handle
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
