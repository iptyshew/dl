#pragma once
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>

namespace dl {

template<typename T>
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

    const_reference operator[](size_type i) const { return (*this)[i]; }
    reference operator[](size_type i) { return *(begin_ + i); }
    size_type size() const { return end_ - begin_; }

    vector(std::initializer_list<value_type> list) {
        reserve(list.size());
        for (const_reference elem : list) {
            push_back(elem);
        }
    }

    void reserve(size_type n) {
        auto new_begin = new T(n);
        std::copy(begin_, end_, new_begin);
        auto sz = size();
        delete begin_;
        begin_ = new_begin;
        end_ = begin_ + sz;
        capacity_ = begin_ + n;
    }

    void resize(size_type n) {
        begin_ = new T(n);
        end_ = begin_ + n;
        capacity_ = end_;
    }

    void push_back(const_reference elem) {
        if (end_ == capacity_) {
            reserve(expand(size()));
        }
        *end_ = elem;
        ++end_;
    }

    ~vector() {
        delete begin_;
    }

private:
    size_t expand(size_t sz) {
        return (sz == 0) ? 1 : sz * 2;
    }

private:
    pointer begin_ = nullptr;
    pointer end_ = nullptr;
    pointer capacity_ = nullptr;
};

} // namespace dl
