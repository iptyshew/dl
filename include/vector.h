#pragma once
#include <cstddef>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include "compressed_pair.h"

namespace dl {

template<typename T, typename Allocator = std::allocator<T>>
class vector
{
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    using reference = value_type&;
    using const_reference = const value_type&;
    using allocator_type = Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using pointer = typename allocator_traits::pointer;
    using const_pointer = typename allocator_traits::const_pointer;
    using iterator = pointer;
    using const_iterator = const iterator;

public:
    iterator begin() {
        return begin_;
    }

    iterator end() {
        return end_;
    }

    const_iterator begin() const {
        return begin_;
    }

    const_iterator end() const {
        return end_;
    }
public:
    vector() = default;

    vector(std::initializer_list<value_type> list) {
        assign(list.begin(), list.end());
    }

    template<typename InputIt>
    void assign(InputIt b, InputIt e) {
        reserve(std::distance(b, e));
        while (b != e) {
            push_back(*b++);
        }
    }

    void assign(std::initializer_list<value_type> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    const_reference operator[](size_type i) const {
        return begin_[i];
    }

    reference operator[](size_type i) {
        return begin_[i];
    }

    bool operator==(const vector<value_type>& o) {
        return size() == o.size() && std::equal(begin(), end(), o.begin());
    }

    const_reference at(size_t i) const {
        if (i >= size()) {
            throw std::out_of_range("vector index out of bounds");
        }
        return begin_[i];
    }

    reference at(size_t i) {
        if (i >= size()) {
            throw std::out_of_range("vector index out of bounds");
        }
        return begin_[i];
    }

    size_type size() const {
        return static_cast<size_type>(end_ - begin_);
    }

    size_type capacity() const {
        return static_cast<size_type>(capacity_.first() - begin_);
    }

    bool empty() const {
        return begin_ == end_;
    }

    reference front() {
        assert(begin_ != nullptr);
        return *begin_;
    }

    const_reference front() const {
        assert(begin_ != nullptr);
        return *begin_;
    }

    reference back() {
        assert(end_ - 1 != nullptr);
        return *(end_ - 1);
    }

    const_reference back() const {
        assert(end_ - 1 != nullptr);
        return *(end_ - 1);
    }

    void clear() {
        for (auto i = begin_; i != end_; ++i) {
            allocator_traits::destroy(alloc(), i);
        }
    }

    void reserve(size_type n) {
        if (n <= capacity()) {
            return;
        }

        auto buff = allocator_traits::allocate(alloc(), n);
        for (auto i = begin_, ib = buff; i != end_; ++i) {
            allocator_traits::construct(alloc(), ib++, std::move_if_noexcept(*i));
            allocator_traits::destroy(alloc(), i);
        }

        auto sz = size();
        allocator_traits::deallocate(alloc(), begin_, capacity());
        begin_ = buff;
        end_ = buff + sz;
        capacity_.first() = buff + n;
    }

    void resize(size_type n) {
        if (n <= size()) {
            for (auto i = begin_ + n; i != end_; ++i) {
                allocator_traits::destroy(alloc(), i);
            }
            end_ = begin_ + n;
            return;
        }
        if (n <= capacity()) {
            for (auto i = begin_ + size(); i != (begin_ + n); ++i) {
                allocator_traits::construct(alloc(), i);
            }
            end_ = begin_ + n;
            return;
        }
        auto buff = allocator_traits::allocate(alloc(), n);
        for (auto i = begin_, ib = buff; i != end_; ++i) {
            allocator_traits::construct(alloc(), ib++, std::move_if_noexcept(*i));
            allocator_traits::destroy(alloc(), i);
        }

        allocator_traits::deallocate(alloc(), begin_, capacity());

        for (auto i = buff + size(); i != (buff + n); ++i) {
            allocator_traits::construct(alloc(), i);
        }

        begin_ = buff;
        end_ = begin_ + n;
        capacity_.first() = begin_ + n;
    }

    void push_back(const_reference elem) {
        if (end_ == capacity_.first()) {
            reserve(expand(size()));
        }
        allocator_traits::construct(alloc(), end_, elem);
        ++end_;
    }

    void push_back(value_type&& elem) {
        if (end_ == capacity_.first()) {
            reserve(expand(size()));
        }
        allocator_traits::construct(alloc(), end_, std::forward<value_type>(elem));
        ++end_;
    }

    iterator insert(const_iterator pos, const T& value) {
        size_t new_capacity = expand(size());
        auto buff = allocator_traits::allocate(alloc(), new_capacity);
        for (auto i = begin(), ib = buff; i < pos; ++i) {
            allocator_traits::construct(alloc(), ib++, std::move(*i));
            allocator_traits::destroy(alloc(), std::addressof(*i));
        }
        auto idx = pos - begin();
        allocator_traits::construct(alloc(), buff + idx, value);
        for (auto i = pos, ib = buff + idx + 1; i != end(); ++i) {
            allocator_traits::construct(alloc(), ib++, std::move(*i));
            allocator_traits::destroy(alloc(), std::addressof(*i));
        }
        auto sz = size();
        allocator_traits::deallocate(alloc(), begin_, capacity());
        begin_ = buff;
        end_ = begin_ + sz + 1;
        capacity_.first() = begin_ + new_capacity;
        return begin() + idx;
    }

    ~vector() {
        clear();
        allocator_traits::deallocate(alloc(), begin_, capacity());
    }

private:
    size_t expand(size_t sz) {
        return (sz == 0) ? 1 : sz * 2;
    }

    allocator_type& alloc() {
        return capacity_.second();
    }

private:
    pointer begin_ = nullptr;
    pointer end_ = nullptr;
    compressed_pair<pointer, allocator_type> capacity_;
};

} // namespace dl
