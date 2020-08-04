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
    using pointer = T*;
    using const_pointer = const T*;
    using allocator_type = Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;

public:
    class iterator
    {
        friend vector;
    public:
        using difference_type = difference_type;
        using value_type = value_type;
        using pointer = pointer;
        using reference = reference;
        using iterator_category = std::random_access_iterator_tag;

    public:
        iterator& operator++() { ++i; return *this; }
        iterator operator++(int) { iterator r(i); ++(*this); return r; }
        bool operator==(iterator other) const { return i == other.i; }
        bool operator!=(iterator other) const { return !(*this == other); }
        reference operator*() const { return *i; }

    private:
        iterator(pointer i) : i(i) {}

    private:
        pointer i;
    };

    using const_iterator = const iterator;

    iterator begin() {
        return iterator(begin_);
    }

    iterator end() {
        return iterator(end_);
    }

    const_iterator begin() const {
        return iterator(begin_);
    }

    const_iterator end() const {
        return iterator(end_);
    }

public:
    vector() = default;

    const_reference operator[](size_type i) const {
        return begin_[i];
    }

    reference operator[](size_type i) {
        return begin_[i];
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

    vector(std::initializer_list<value_type> list) {
        reserve(list.size());
        for (auto&& elem : list) {
            push_back(std::forward<value_type>(elem));
        }
    }

    void reserve(size_type n) {
        if (n <= capacity()) {
            return;
        }
        auto buff = allocator_traits::allocate(alloc(), n);
        for (auto i = begin_; i != end_; ++i) {
            allocator_traits::construct(alloc(), buff, std::move(*i));
            allocator_traits::destroy(alloc(), i);
        }

        auto sz = size();
        allocator_traits::deallocate(alloc(), begin_, capacity());
        begin_ = buff;
        end_ = begin_ + sz;
        capacity_.first() = begin_ + n;
    }

    void resize(size_type n) {
        begin_ = allocator_traits::allocate(alloc(), n);
        end_ = begin_ + n;
        capacity_.first() = end_;
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
        allocator_traits::costruct(alloc(), end_, std::forward<value_type>(elem));
        ++end_;
    }

    ~vector() {
        for (auto i = begin_; i != end_; ++i) {
            allocator_traits::destroy(alloc(), i);
        }
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
