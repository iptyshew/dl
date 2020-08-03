#pragma once
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
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

    iterator begin() { return iterator(begin_); }
    iterator end() { return iterator(end_); }

public:
    vector() = default;

    const_reference operator[](size_type i) const {
        return (*this)[i];
    }

    reference operator[](size_type i) {
        return *(begin_ + i);
    }

    size_type size() const {
        return static_cast<size_type>(end_ - begin_);
    }

    size_type capacity() const {
        return static_cast<size_type>(capacity_.first() - begin_);
    }

    vector(std::initializer_list<value_type> list) {
        reserve(list.size());
        for (auto&& elem : list) {
            push_back(std::forward<value_type>(elem));
        }
    }

    void reserve(size_type n) {
        auto new_begin = allocator_traits::allocate(capacity_.second(), n);
        std::copy(begin_, end_, new_begin);
        auto sz = size();
        delete begin_;
        begin_ = new_begin;
        end_ = begin_ + sz;
        capacity_.first() = begin_ + n;
    }

    void resize(size_type n) {
        begin_ = allocator_traits::allocate(capacity_.second(), n);
        end_ = begin_ + n;
        capacity_.first() = end_;
    }

    void push_back(const_reference elem) {
        if (end_ == capacity_.first()) {
            reserve(expand(size()));
        }
        *end_ = elem;
        ++end_;
    }

    void push_back(value_type&& elem) {
        if (end_ == capacity_.first()) {
            reserve(expand(size()));
        }
        *end_ = std::forward<value_type>(elem);
        ++end_;
    }

    ~vector() {
        allocator_traits::deallocate(capacity_.second(), begin_, capacity());
    }

private:
    size_t expand(size_t sz) {
        return (sz == 0) ? 1 : sz * 2;
    }

private:
    pointer begin_ = nullptr;
    pointer end_ = nullptr;
    compressed_pair<pointer, allocator_type> capacity_;
};

} // namespace dl
