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

#ifndef LIBGUARDED_COW_GUARDED_HPP
#define LIBGUARDED_COW_GUARDED_HPP

#include <atomic>
#include <memory>
#include <mutex>

#include <libguarded/lr_guarded.hpp>

namespace libguarded
{

/**
 \headerfile cow_guarded.hpp <libguarded/cow_guarded.hpp>

 This templated class wraps an object and allows only one thread at a
 time to modify the protected object. Any number of threads can read
 the protected object simultaneously and the version as of the time of
 reading will be maintained until no longer needed.

 When a thread locks the cow_guarded object for writing, a copy is
 made so that the writer may modify the stored data without race
 conditions. When the handle is released, the old copy is atomically
 replaced by the modified copy.

 The handle type supports a cancel() method, which will discard the
 modified copy and immediately unlock the data without applying the
 changes.

 This class will use std::mutex for the internal locking mechanism by
 default. Other classes which are useful for the mutex type are
 std::recursive_mutex, std::timed_mutex, and
 std::recursive_timed_mutex.

 The handle returned by the various lock methods is moveable but not
 copyable. The shared_handle type is moveable and copyable.

 The T class must be copy constructible.
*/
template <typename T, typename Mutex = std::mutex>
class cow_guarded
{
  private:
    class deleter;
    class shared_deleter;

  public:
    class handle;
    using shared_handle = std::shared_ptr<const T>;

    /**
     Construct a cow_guarded object. This constructor will accept any
     number of parameters, all of which are forwarded to the
     constructor of T.
    */
    template <typename... Us>
    cow_guarded(Us &&... data);

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
    shared_handle try_lock_shared_for(const Duration &duration) const;

    /**
     Acquire a shared_handle to the protected object. Always succeeds without
     blocking.
    */
    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint &timepoint) const;

  private:
    class deleter
    {
      public:
        using pointer = T *;

        deleter(std::unique_lock<Mutex> &&lock, cow_guarded &guarded)
            : m_lock(std::move(lock)), m_guarded(guarded), m_cancelled(false)
        {
        }

        void cancel()
        {
            m_cancelled = true;

            if (m_lock.owns_lock()) {
                m_lock.unlock();
            }
        }

        void operator()(T *ptr)
        {
            if (m_cancelled) {
                delete ptr;
            } else if (ptr) {
                std::shared_ptr<const T> newPtr(ptr);

                m_guarded.m_data.modify([newPtr](std::shared_ptr<const T> &ptr) { ptr = newPtr; });
            }

            if (m_lock.owns_lock()) {
                m_lock.unlock();
            }
        }

      private:
        std::unique_lock<Mutex> m_lock;
        cow_guarded &m_guarded;
        bool m_cancelled;
    };

  public:
    /**
       The handle class for cow_guarded is moveable but not copyable.
     */
    class handle : public std::unique_ptr<T, deleter>
    {
      public:
        using std::unique_ptr<T, deleter>::unique_ptr;

        /**
           Cancel all pending changes, reset the handle to null, and
           unlock the data.
         */
        void cancel()
        {
            this->get_deleter().cancel();
            this->reset();
        }
    };

  private:
    mutable lr_guarded<std::shared_ptr<const T>> m_data;
    mutable Mutex m_writeMutex;
};

template <typename T, typename M>
template <typename... Us>
cow_guarded<T, M>::cow_guarded(Us &&... data)
    : m_data(std::make_shared<T>(std::forward<Us>(data)...))
{
}

template <typename T, typename M>
auto cow_guarded<T, M>::lock() -> handle
{
    std::unique_lock<M> guard(m_writeMutex);

    auto data(m_data.lock_shared());
    std::unique_ptr<T> val(new T(**data));
    data.reset();

    return handle(val.release(), deleter(std::move(guard), *this));
}

template <typename T, typename M>
auto cow_guarded<T, M>::try_lock() -> handle
{
    std::unique_lock<M> guard(m_writeMutex);

    auto data(m_data.try_lock_shared());

    if (!data) {
        return handle();
    }

    std::unique_ptr<T> val(new T(**data));
    data.reset();

    return handle(val.release(), deleter(std::move(guard), *this));
}

template <typename T, typename M>
template <typename Duration>
auto cow_guarded<T, M>::try_lock_for(const Duration &duration) -> handle
{
    std::unique_lock<M> guard(m_writeMutex);
    auto data = m_data.try_lock_shared_for(duration);

    if (!data) {
        return handle();
    }

    std::unique_ptr<T> val(new T(**data));
    data.reset();

    return handle(val.release(), deleter(std::move(guard), *this));
}

template <typename T, typename M>
template <typename TimePoint>
auto cow_guarded<T, M>::try_lock_until(const TimePoint &timepoint) -> handle
{
    std::unique_lock<M> guard(m_writeMutex);

    auto data(m_data.try_lock_shared_until(timepoint));

    if (!data) {
        return handle();
    }

    std::unique_ptr<T> val(new T(**data));
    data.reset();

    return handle(val.release(), deleter(std::move(guard), *this));
}

template <typename T, typename M>
auto cow_guarded<T, M>::lock_shared() const -> shared_handle
{
    auto lock = m_data.lock_shared();
    return *lock;
}

template <typename T, typename M>
auto cow_guarded<T, M>::try_lock_shared() const -> shared_handle
{
    shared_handle retval;

    auto lock = m_data.try_lock_shared();
    if (lock) {
        retval = *lock;
    }

    return retval;
}

template <typename T, typename M>
template <typename Duration>
auto cow_guarded<T, M>::try_lock_shared_for(const Duration &duration) const -> shared_handle
{
    shared_handle retval;

    auto lock = m_data.try_lock_shared_for(duration);
    if (lock) {
        retval = *lock;
    }

    return retval;
}

template <typename T, typename M>
template <typename TimePoint>
auto cow_guarded<T, M>::try_lock_shared_until(const TimePoint &timepoint) const -> shared_handle
{
    shared_handle retval;

    auto lock = m_data.try_lock_shared_until(timepoint);
    if (lock) {
        retval = *lock;
    }

    return retval;
}
}

#endif
