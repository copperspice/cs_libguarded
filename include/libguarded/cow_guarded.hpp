/*
 * Copyright 2016 Ansel Sermersheim
 *
 * All rights reserved
 *
 * This file is part of libguarded. Libguarded is free software
 * released under the BSD 2-clause license. For more information see
 * the LICENCE file provided with this project.
 */

#ifndef INCLUDED_LIBGUARDED_COW_GUARDED_HPP
#define INCLUDED_LIBGUARDED_COW_GUARDED_HPP

#include <atomic>
#include <memory>
#include <mutex>

#include <libguarded/lr_guarded.hpp>

namespace libguarded
{

template <typename T, typename Mutex = std::mutex>
class cow_guarded
{
  private:
    class deleter;
    class shared_deleter;

  public:
    class handle;
    using shared_handle = std::shared_ptr<const T>;

    template <typename... Us>
    cow_guarded(Us &&... data);

    // exclusive access
    handle lock();
    handle try_lock();

    template <class Duration>
    handle try_lock_for(const Duration & duration);

    template <class TimePoint>
    handle try_lock_until(const TimePoint & timepoint);

    // shared access, note "shared" in method names
    shared_handle lock_shared() const;
    shared_handle try_lock_shared() const;

    template <class Duration>
    shared_handle try_lock_shared_for(const Duration & duration) const;

    template <class TimePoint>
    shared_handle try_lock_shared_until(const TimePoint & timepoint) const;

  private:
    class deleter
    {
      public:
        using pointer = T *;

        deleter(std::unique_lock<Mutex> && lock, cow_guarded & guarded)
            : m_lock(std::move(lock)), m_guarded(guarded), m_cancelled(false)
        {
        }

        void cancel()
        {
            m_cancelled = true;
        }

        void operator()(T * ptr)
        {
            if (m_cancelled) {
                delete ptr;
            } else if (ptr) {
                std::shared_ptr<const T> newPtr(ptr);

                m_guarded.m_data.modify([newPtr](std::shared_ptr<const T> & ptr) { ptr = newPtr; });
            }

            if (m_lock.owns_lock()) {
                m_lock.unlock();
            }
        }

      private:
        std::unique_lock<Mutex> m_lock;
        cow_guarded &           m_guarded;
        bool                    m_cancelled;
    };

  public:
    class handle : public std::unique_ptr<T, deleter>
    {
      public:
        using std::unique_ptr<T, deleter>::unique_ptr;

        void cancel()
        {
            this->get_deleter().cancel();
            this->reset();
        }
    };

  private:
    mutable lr_guarded<std::shared_ptr<const T>> m_data;
    mutable Mutex                                m_writeMutex;
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

    auto               data(m_data.lock_shared());
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
auto cow_guarded<T, M>::try_lock_for(const Duration & duration) -> handle
{
    std::unique_lock<M> guard(m_writeMutex);
    auto                data = m_data.try_lock_shared_for(duration);

    if (!data) {
        return handle();
    }

    std::unique_ptr<T> val(new T(**data));
    data.reset();

    return handle(val.release(), deleter(std::move(guard), *this));
}

template <typename T, typename M>
template <typename TimePoint>
auto cow_guarded<T, M>::try_lock_until(const TimePoint & timepoint) -> handle
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
auto cow_guarded<T, M>::try_lock_shared_for(const Duration & duration) const -> shared_handle
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
auto cow_guarded<T, M>::try_lock_shared_until(const TimePoint & timepoint) const -> shared_handle
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
