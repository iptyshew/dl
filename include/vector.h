#pragma once
#include <cstddef>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include "compressed_pair.h"
#include "split_buffer.h"

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
    using const_iterator = const_pointer;

public: // constructors
    vector() = default;

    vector(std::initializer_list<value_type> list) {
        assign(list.begin(), list.end());
    }

public: // simple members, operators
    const value_type* data() const noexcept { return begin_; }
    value_type* data() noexcept             { return begin_; }

    iterator begin() noexcept { return begin_; }
    iterator end() noexcept   { return end_; }

    const_iterator begin() const noexcept { return begin_; }
    const_iterator end() const noexcept   { return end_; }

    const_reference operator[](size_type i) const noexcept{ return begin_[i]; }
    reference operator[](size_type i) noexcept            { return begin_[i]; }

    size_type size() const noexcept { return static_cast<size_type>(end_ - begin_); }

    size_type capacity() const noexcept { return static_cast<size_type>(end_cap() - begin_); }

    bool empty() const noexcept { return begin_ == end_; }

    reference front() noexcept             { return begin_[0]; }
    const_reference front() const noexcept { return begin_[0]; }

    reference back() noexcept             { return end_[-1]; }
    const_reference back() const noexcept { return end_[-1]; }

    const_reference at(size_t i) const {
        if (i >= size()) throw std::out_of_range("vector index out of bounds");
        return begin_[i];
    }

    reference at(size_t i) {
        if (i >= size()) throw std::out_of_range("vector index out of bounds");
        return begin_[i];
    }

public:
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

    void clear() noexcept {
        end_ = destroy_range(alloc(), begin_, end_);
    }

    void reserve(size_type n) {
        if (n > capacity()) {
            split_buffer<value_type, allocator_type&> buff(size(), n, alloc());
            swap_out_buffer(buff);
        }
    }

    void resize(size_type n) {
        auto sz = size();
        if (n > capacity()) {
            split_buffer<value_type, allocator_type&> buff(n, n, alloc());
            construct_range(buff.alloc(), buff.begin + sz, buff.end);
            swap_out_buffer(buff);
        } else if (n < sz) {
            end_ = destroy_range(alloc(), begin_ + n, end_);
        } else if (n > sz) {
            end_ = construct_range(alloc(), begin_ + sz, begin_ + n);
        }
    }

    void resize(size_type n, const value_type& value) { // \todo :(
        auto sz = size();
        if (n > capacity()) {
            split_buffer<value_type, allocator_type&> buff(n, n, alloc());
            construct_range(buff.alloc(), buff.begin + sz, buff.end, value);
            swap_out_buffer(buff);
        } else if (n < sz) {
            end_ = destroy_range(alloc(), begin_ + n, end_);
        } else if (n > sz) {
            end_ = construct_range(alloc(), begin_ + sz, begin_ + n, value);
        }
    }

    void push_back(const_reference elem) {
        if (end_ == end_cap()) {
            reserve(expand(size()));
        }
        allocator_traits::construct(alloc(), end_, elem);
        ++end_;
    }

    void push_back(value_type&& elem) {
        if (end_ == end_cap()) {
            reserve(expand(size()));
        }
        allocator_traits::construct(alloc(), end_, std::forward<value_type>(elem));
        ++end_;
    }

    template<typename... Args>
    reference emplace_back(Args&&... args) {
        if (end_ == end_cap()) {
            reserve(expand(size()));
        }
        allocator_traits::construct(alloc(), end_, std::forward<Args>(args)...);
        ++end_;
        return back();
    }

    void pop_back() {
        allocator_traits::destroy(alloc(), end_ - 1);
        --end_;
    }

    iterator insert(const_iterator pos, const T& value) {
        // split_buffer<value_type, allocator_type&> buff(size() + 1, expand(size()), alloc());

        // for (auto i = begin(), ib = buff; i < pos; ++i) {
        //     allocator_traits::construct(alloc(), ib++, std::move(*i));
        //     allocator_traits::destroy(alloc(), std::addressof(*i));
        // }
        // auto idx = pos - begin();
        // allocator_traits::construct(alloc(), buff + idx, value);
        // for (auto i = pos, ib = buff + idx + 1; i != end(); ++i) {
        //     allocator_traits::construct(alloc(), ib++, std::move(*i));
        //     allocator_traits::destroy(alloc(), std::addressof(*i));
        // }
        // allocator_traits::deallocate(alloc(), begin_, capacity());

        // swap_out_buffer(buff);
        // return begin() + idx;
        return begin();
    }

    void swap(vector& other) noexcept {
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(end_cap_allocator_, other.end_cap_allocator_);
    }

    ~vector() {
        clear();
        allocator_traits::deallocate(alloc(), begin_, capacity());
    }

private:
    static size_t expand(size_t sz) { return (sz == 0) ? 1 : sz * 2; }

    allocator_type& alloc() { return end_cap_allocator_.second(); }

    pointer& end_cap()             { return end_cap_allocator_.first(); }
    const pointer& end_cap() const { return end_cap_allocator_.first(); }

    void swap_out_buffer(split_buffer<value_type, allocator_type&>& buff) {
        uninit_move(buff.alloc(), begin_, end_, buff.begin);
        std::swap(begin_, buff.begin);
        std::swap(end_, buff.end);
        std::swap(end_cap(), buff.end_cap());
    }

    template<typename InputIt, typename OutIt>
    static void uninit_move(allocator_type& alloc, InputIt b, InputIt e, OutIt res) {
        while (b != e) {
            allocator_traits::construct(alloc, res++, std::move_if_noexcept(*(b++)));
        }
    }

    static pointer destroy_range(allocator_type& alloc, pointer begin, pointer end) {
        auto it = begin;
        while (it != end) {
            allocator_traits::destroy(alloc, it++);
        }
        return begin;
    }

    static pointer construct_range(allocator_type& alloc, pointer begin, pointer end) {
        while (begin != end) {
            allocator_traits::construct(alloc, begin++);
        }
        return begin;
    }

    static pointer construct_range(allocator_type& alloc, pointer begin, pointer end, const value_type& val) {
        while (begin != end) {
            allocator_traits::construct(alloc, begin++, val);
        }
        return begin;
    }

private:
    pointer begin_ = nullptr;
    pointer end_ = nullptr;
    compressed_pair<pointer, allocator_type> end_cap_allocator_;
};

template<typename T, typename Alloc>
bool operator==(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<typename T, typename Alloc>
bool operator!=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
    return !(lhs == rhs);
}

template<typename T, typename Alloc>
bool operator<(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename T, typename Alloc>
bool operator<=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
    return !(lhs > rhs);
}

template<typename T, typename Alloc>
bool operator>(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
    return rhs < lhs;
}

template<typename T, typename Alloc>
bool operator>=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
    return !(lhs < rhs);
}

} // namespace dl
