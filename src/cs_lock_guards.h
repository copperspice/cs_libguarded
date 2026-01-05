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

#ifndef CSLIBGUARDED_LOCK_GUARDS_H
#define CSLIBGUARDED_LOCK_GUARDS_H

#include <algorithm>
#include <array>
#include <tuple>
#include <utility>

namespace libguarded
{

namespace detail
{

// type trait to get the address of a guard, specialization below
template <typename T>
class guard_address
{
  public:
    constexpr guard_address(const T &object) : value(&object){};

    const void *const value;
};

// Base class to sort by address and virtually invoke the actual try_lock method
class guard_locker_base
{
  public:
    guard_locker_base(const void *address) : m_address(address){};

    virtual ~guard_locker_base() = default;

    [[nodiscard]] virtual bool do_try_lock() = 0;
    virtual void reset()                     = 0;

    [[nodiscard]] bool operator<(const guard_locker_base &rhs) const noexcept
    {
        return m_address < rhs.m_address;
    }

  private:
    const void *const m_address;
};

// Concrete implementation, stores a reference to a guarded object and a handle from calling
// the underlying try_lock() method
template <typename T>
class guard_locker : public guard_locker_base
{
  public:
    using lock_type = std::decay_t<decltype(std::declval<T>().try_lock())>;

    guard_locker(T &guard) : guard_locker_base(guard_address<T>(guard).value), m_guard(guard)
    {
    }

    [[nodiscard]] bool do_try_lock() override
    {
        m_lock = m_guard.try_lock();
        return m_lock != nullptr;
    }

    void reset() override
    {
        m_lock.reset();
    }

    [[nodiscard]] lock_type take_lock() &&
    {
        return std::move(m_lock);
    }

  private:
    lock_type m_lock;
    T &m_guard;
};

// Adapter class which forwards the try_lock() method to the underlying type's try_lock_shared()
// method so we can get a read lock
template <typename T>
class guard_reader
{
  public:
    guard_reader(T &guard) : m_guard(guard)
    {
    }

    [[nodiscard]] auto try_lock()
    {
        return m_guard.try_lock_shared();
    }

  private:
    T &m_guard;

    friend class guard_address<guard_reader<T>>;
};

// Specialization for guard_address which returns the address of the underlying guard instead of the
// adapter object
template <typename T>
class guard_address<guard_reader<T>>
{
  public:
    constexpr guard_address(const guard_reader<T> &object) : value(&(object.m_guard)){};

    const void *const value;
};

} // namespace detail

template <typename T>
[[nodiscard]] auto as_reader(T &guard)
{
    return detail::guard_reader<T>(guard);
}

template <typename... Ts>
[[nodiscard]] auto lock_guards(Ts &&...Vs)
{
    using namespace libguarded::detail;

    std::tuple<guard_locker<Ts>...> lockers = {guard_locker<Ts>(Vs)...};

    auto lock_sequence = std::apply(
        [](auto &&...args) {
            std::array<guard_locker_base *, sizeof...(Ts)> retval = {(&args)...};
            return retval;
        },
        lockers);

    // sort the lock sequence based on the address of the underlying guarded object
    std::sort(lock_sequence.begin(), lock_sequence.end());

    auto iter = lock_sequence.begin();

    // walk through all locks from the beginning
    while (iter != lock_sequence.end()) {
        if ((*iter)->do_try_lock()) {
            // this lock succeeded, go on
            ++iter;
        } else {
            // a lock failed, release as we walk back to the beginning
            while (iter != lock_sequence.begin()) {
                --iter;
                (*iter)->reset();
            }
        }
    }

    // all locks succeeded

    return std::apply(
        [](auto &&...args) {
            // move locks out of the guard_locker objects
            return std::make_tuple(std::move(args).take_lock()...);
        },
        std::move(lockers));
}

} // namespace libguarded

#endif
