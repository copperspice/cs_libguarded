/*
 * Copyright 2016-2017 Ansel Sermersheim
 *
 * All rights reserved
 *
 * This file is part of libguarded. Libguarded is free software
 * released under the BSD 2-clause license. For more information see
 * the LICENCE file provided with this project.
 */

#ifndef INCLUDED_LIBGUARDED_RCU_LIST_HPP
#define INCLUDED_LIBGUARDED_RCU_LIST_HPP

#include <memory>
#include <atomic>
#include <mutex>

namespace libguarded
{
template <typename T, typename M = std::mutex, typename Alloc = std::allocator<T>>
class rcu_list
{
  public:
    using value_type      = T;
    using allocator_type  = Alloc;
    using size_type       = ssize_t;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using pointer         = typename std::allocator_traits<Alloc>::pointer;
    using const_pointer   = typename std::allocator_traits<Alloc>::const_pointer;
    class iterator;
    class const_iterator;
    class reverse_iterator;
    class const_reverse_iterator;

    class rcu_guard;
    using rcu_write_guard = rcu_guard;
    using rcu_read_guard  = rcu_guard;

    rcu_list();
    explicit rcu_list(const Alloc & alloc);

    ~rcu_list();

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    void clear();

    iterator insert(const_iterator pos, T value);
    iterator insert(const_iterator pos, size_type count, const T & value);

    template <typename InputIter>
    iterator insert(const_iterator pos, InputIter first, InputIter last);
    iterator insert(const_iterator pos, std::initializer_list<T> ilist);

    template <typename... Us>
    iterator emplace(const_iterator pos, Us &&... vs);

    void push_front(T value);
    void push_back(T value);
    template <typename... Us>
    void emplace_front(Us &&... vs);
    template <typename... Us>
    void emplace_back(Us &&... vs);

    iterator erase(const_iterator pos);

  private:
    struct node {
        // uncopyable, unmoveable
        node(const node &) = delete;
        node(node &&) = delete;
        node & operator=(const node &) = delete;
        node & operator=(node &&) = delete;
        template <typename... Us>
        explicit node(Us &&... vs)
            : next(nullptr), back(nullptr), deleted(false), data(std::forward<Us>(vs)...)
        {
        }

        std::atomic<node *> next;
        std::atomic<node *> back;
        bool deleted;
        T data;
    };

    struct zombie_list_node {
        zombie_list_node() = default;
        // uncopyable, unmoveable
        zombie_list_node(const zombie_list_node &) = delete;
        zombie_list_node(zombie_list_node &&) = delete;
        zombie_list_node & operator=(const zombie_list_node &) = delete;
        zombie_list_node & operator=(zombie_list_node &&) = delete;

        ~zombie_list_node()
        {
            delete zombie_node;
        }

        std::shared_ptr<zombie_list_node> next{nullptr};
        std::atomic<void *> owner{nullptr};
        node * zombie_node{nullptr};
    };

    using alloc_trait             = std::allocator_traits<Alloc>;
    using node_alloc_t            = typename alloc_trait::template rebind_alloc<node>;
    using node_alloc_trait        = std::allocator_traits<node_alloc_t>;
    using zombie_node_alloc_t     = typename alloc_trait::template rebind_alloc<node>;
    using zombie_node_alloc_trait = std::allocator_traits<zombie_node_alloc_t>;

    std::atomic<node *> m_head;
    std::atomic<node *> m_tail;

    std::shared_ptr<zombie_list_node> m_zombie_head;

    M m_write_mutex;

    node_alloc_t m_alloc;
};

/*----------------------------------------*/

template <typename T, typename M, typename Alloc>
class rcu_list<T, M, Alloc>::rcu_guard
{

  public:
    void rcu_read_lock(const rcu_list<T, M, Alloc> & list);

    void rcu_read_unlock(const rcu_list<T, M, Alloc> & list);

    void rcu_write_lock(const rcu_list<T, M, Alloc> & list);

    void rcu_write_unlock(const rcu_list<T, M, Alloc> & list);

  private:
    void unlock();

    std::shared_ptr<zombie_list_node> m_zombie;
};

template <typename T, typename M, typename Alloc>
void rcu_list<T, M, Alloc>::rcu_guard::rcu_read_lock(const rcu_list<T, M, Alloc> & list)
{
    m_zombie = atomic_load_explicit(&(list.m_zombie_head), std::memory_order_relaxed);

    if (m_zombie == nullptr) {
        return;
    }

    void * current_owner = nullptr;
    if (m_zombie->owner.compare_exchange_strong(current_owner, this, std::memory_order_relaxed)) {

        std::shared_ptr<zombie_list_node> n =
            atomic_load_explicit(&(m_zombie->next), std::memory_order_relaxed);

        while (n) {
            current_owner = nullptr;
            if (!n->owner.compare_exchange_strong(current_owner, this, std::memory_order_relaxed)) {
                break;
            }

            n = atomic_load_explicit(&(m_zombie->next), std::memory_order_relaxed);
        }
    }
}

template <typename T, typename M, typename Alloc>
void rcu_list<T, M, Alloc>::rcu_guard::rcu_read_unlock(const rcu_list<T, M, Alloc> & list)
{
    unlock();
};

template <typename T, typename M, typename Alloc>
void rcu_list<T, M, Alloc>::rcu_guard::unlock()
{
    std::shared_ptr<zombie_list_node> n = m_zombie;

    if (n == nullptr) {
        return;
    }

    bool first = true;

    while (n) {
        if (n->owner.load(std::memory_order_relaxed) != this) {
            if (first) {
                return;
            } else {
                break;
            }
        }

	n = atomic_load(&(n->next));
    }

    // We are about to potentially cause some deletions. We need to do an acquire to make
    // sure that we see the zombie address which may have been just released by a writer
    // is visible before the zombie_node destructor runs.
    atomic_store(&(m_zombie->next), n);
    atomic_store(&(m_zombie->owner), static_cast<void *>(nullptr));
}

template <typename T, typename M, typename Alloc>
void rcu_list<T, M, Alloc>::rcu_guard::rcu_write_lock(const rcu_list<T, M, Alloc> & list)
{
    rcu_read_lock(list);
}

template <typename T, typename M, typename Alloc>
void rcu_list<T, M, Alloc>::rcu_guard::rcu_write_unlock(const rcu_list<T, M, Alloc> & list)
{
    rcu_read_unlock(list);
}

/*----------------------------------------*/

template <typename T, typename M, typename Alloc>
class rcu_list<T, M, Alloc>::iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = const T;
    using pointer           = const T *;
    using reference         = const T &;
    using difference_type   = size_t;

    iterator() : m_current(nullptr){};

    const T & operator*() const
    {
        return m_current->data;
    };
    const T * operator->() const
    {
        return &(m_current->data);
    };

    bool operator==(const const_iterator & other) const
    {
        return m_current == other.m_current;
    }

    bool operator!=(const const_iterator & other) const
    {
        return m_current != other.m_current;
    }

    iterator & operator++()
    {
        m_current = m_current->next;
        return *this;
    }

    const_iterator & operator--()
    {
        m_current = m_current->prev;
        return *this;
    }

    const_iterator operator++(int)
    {
        const_iterator old(*this);
        ++(*this);
        return old;
    }

    const_iterator operator--(int)
    {
        const_iterator old(*this);
        --(*this);
        return old;
    }

  private:
    friend rcu_list<T, M, Alloc>;
    friend rcu_list<T, M, Alloc>::const_iterator;

    explicit iterator(const rcu_list<T, M, Alloc>::const_iterator & it) : m_current(it.m_current){};

    explicit iterator(node * n) : m_current(n){};

    node * m_current;
};

/*----------------------------------------*/

template <typename T, typename M, typename Alloc>
class rcu_list<T, M, Alloc>::const_iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = const T;
    using pointer           = const T *;
    using reference         = const T &;
    using difference_type   = size_t;

    const_iterator() : m_current(nullptr){};
    const_iterator(const rcu_list<T, M, Alloc>::iterator & it) : m_current(it.m_current){};

    const T & operator*() const
    {
        return m_current->data;
    };
    const T * operator->() const
    {
        return &(m_current->data);
    };

    bool operator==(const const_iterator & other) const
    {
        return m_current == other.m_current;
    }

    bool operator!=(const const_iterator & other) const
    {
        return m_current != other.m_current;
    }

    const_iterator & operator++()
    {
        m_current = m_current->next;
        return *this;
    }

    const_iterator & operator--()
    {
        m_current = m_current->prev;
        return *this;
    }

    const_iterator operator++(int)
    {
        const_iterator old(*this);
        ++(*this);
        return old;
    }

    const_iterator operator--(int)
    {
        const_iterator old(*this);
        --(*this);
        return old;
    }

  private:
    friend rcu_list<T, M, Alloc>;

    explicit const_iterator(node * n) : m_current(n){};

    node * m_current;
};

/*----------------------------------------*/

template <typename T, typename M, typename Alloc>
rcu_list<T, M, Alloc>::rcu_list()
{
    m_head.store(nullptr);
    m_tail.store(nullptr);
}

template <typename T, typename M, typename Alloc>
rcu_list<T, M, Alloc>::~rcu_list()
{
    node * n = m_head.load();

    while (n != nullptr) {
        node * current = n;
        n = n->next.load();
        node_alloc_trait::destroy(m_alloc, current);
        node_alloc_trait::deallocate(m_alloc, current, 1);
    }
}

template <typename T, typename M, typename Alloc>
auto rcu_list<T, M, Alloc>::begin() -> iterator
{
    return iterator(m_head.load());
}

template <typename T, typename M, typename Alloc>
auto rcu_list<T, M, Alloc>::end() -> iterator
{
    return iterator();
}

template <typename T, typename M, typename Alloc>
auto rcu_list<T, M, Alloc>::begin() const -> const_iterator
{
    return const_iterator(m_head.load());
}

template <typename T, typename M, typename Alloc>
auto rcu_list<T, M, Alloc>::end() const -> const_iterator
{
    return const_iterator();
}

template <typename T, typename M, typename Alloc>
void rcu_list<T, M, Alloc>::push_front(T data)
{
    std::lock_guard<M> guard(m_write_mutex);
    std::unique_ptr<node> newNode(new node(std::move(data)));

    node * oldHead = m_head.load(std::memory_order_relaxed);

    if (oldHead == nullptr) {
        m_head.store(newNode.get());
        m_tail.store(newNode.release());
    } else {
        newNode->next.store(oldHead);
        oldHead->back.store(newNode.get());
        m_head.store(newNode.release());
    }
}

template <typename T, typename M, typename Alloc>
template <typename... Us>
void rcu_list<T, M, Alloc>::emplace_front(Us &&... vs)
{
    std::lock_guard<M> guard(m_write_mutex);
    std::unique_ptr<node> newNode(new node(std::forward<Us>(vs)...));

    node * oldHead = m_tail.load(std::memory_order_relaxed);

    if (oldHead == nullptr) {
        m_head.store(newNode.get());
        m_tail.store(newNode.release());
    } else {
        newNode->back.store(oldHead);
        oldHead->next.store(newNode.get());
        m_head.store(newNode.release());
    }
}

template <typename T, typename M, typename Alloc>
void rcu_list<T, M, Alloc>::push_back(T data)
{
    std::lock_guard<M> guard(m_write_mutex);
    std::unique_ptr<node> newNode(new node(std::move(data)));

    node * oldTail = m_tail.load(std::memory_order_relaxed);

    if (oldTail == nullptr) {
        m_head.store(newNode.get());
        m_tail.store(newNode.release());
    } else {
        newNode->back.store(oldTail);
        oldTail->next.store(newNode.get());
        m_tail.store(newNode.release());
    }
}

template <typename T, typename M, typename Alloc>
template <typename... Us>
void rcu_list<T, M, Alloc>::emplace_back(Us &&... vs)
{
    std::lock_guard<M> guard(m_write_mutex);
    std::unique_ptr<node> newNode(new node(std::forward<Us>(vs)...));

    node * oldTail = m_tail.load(std::memory_order_relaxed);

    if (oldTail == nullptr) {
        m_head.store(newNode.get());
        m_tail.store(newNode.release());
    } else {
        newNode->back.store(oldTail);
        oldTail->next.store(newNode.get());
        m_tail.store(newNode.release());
    }
}

template <typename T, typename M, typename Alloc>
auto rcu_list<T, M, Alloc>::erase(const_iterator iter) -> iterator
{

    // Make sure the node has not already been marked for deletion
    if (!iter.m_current->deleted) {
        iter.m_current->deleted = true;

        std::shared_ptr<zombie_list_node> newZombie = std::make_shared<zombie_list_node>();
        newZombie->zombie_node                      = iter.m_current;

        std::shared_ptr<zombie_list_node> oldZombie = atomic_load(&m_zombie_head);

        do {
            newZombie->next = oldZombie;
        } while (!atomic_compare_exchange_weak(&m_zombie_head, &oldZombie, newZombie));

        node * oldPrev = iter.m_current->back.load();
        node * oldNext = iter.m_current->next.load();

        if (oldPrev) {
            oldPrev->next.store(oldNext);
        } else {
            // no previous node, we must have been the head
            m_head.store(oldNext);
        }

        if (oldNext) {
            oldNext->back.store(oldPrev);
        } else {
            // no next node, we must have been the tail
            m_tail.store(oldPrev);
        }
    }

    return iterator(++iter);
}
}

#endif
