#pragma once
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <initializer_list>
#include <iterator>

namespace dl {

template<typename T>
class vector
{
public:
    vector() = default;

    const T& operator[](size_t i) const { return (*this)[i]; }
    T& operator[](size_t i) { return *(begin_ + i); }
    size_t size() const { return end_ - begin_; }

    vector(std::initializer_list<T> list) {
        resize(list.size());
        for (const T& elem : list) {
            push_back(elem);
        }
    }

    void resize(size_t n) {
        begin_ = new T(n);
        end_ = begin_ + n;
        capacity_ = end_;
    }

    void push_back(const T& elem) {
        if (end_ == capacity_) {
            up_capacity();
        }
        *end_ = elem;
        ++end_;
    }

    ~vector() {
        delete begin_;
    }

    class iterator
    {
        friend vector;
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
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

private:
    void up_capacity() {
        size_t sz = size();
        size_t new_capacity = (sz == 0) ? 1 : sz * 2;
        auto new_begin = new T(new_capacity);
        std::copy(begin_, end_, new_begin);
        begin_ = new_begin;
        end_ = begin_ + sz;
        capacity_ = begin_ + new_capacity;
    }

private:
    T* begin_ = nullptr;
    T* end_ = nullptr;
    T* capacity_ = nullptr;
};

} // namespace dl
