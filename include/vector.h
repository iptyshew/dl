#pragma once
#include <cstddef>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "compressed_pair.h"
#include "split_buffer.h"
#include "type_utils.h"

namespace dl {

template<typename T, typename Allocator = std::allocator<T>>
class vector
{
public:
    using value_type = T;
    using allocator_type = Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using pointer = typename allocator_traits::pointer;
    using const_pointer = typename allocator_traits::const_pointer;
    using reference = value_type&;
    using const_reference = const value_type&;

    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = pointer;
    using const_iterator = const_pointer;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public: // constructors
    vector() noexcept = default;

    vector(const Allocator& alloc) noexcept
        : end_cap_allocator_(nullptr, alloc) {}

    explicit vector(size_type count, const Allocator& a = Allocator())
        : vector(a) {
        allocate_n(count);
        construct_at_end(count);
    }

    vector(size_type count, const T& value, const Allocator& a = Allocator())
        : vector(a) {
        allocate_n(count);
        construct_at_end(count, value);
    }

    template<typename InputIt>
    vector(InputIt first,
           typename std::enable_if_t<is_forward_iter<InputIt>::value, InputIt> last,
           const Allocator& a = Allocator())
        : vector(a) {
        allocate_n(std::distance(first, last));
        construct_at_end(first, last);
    }

    template<typename InputIt>
    vector(InputIt first,
           typename std::enable_if_t<is_input_iter<InputIt>::value &&
                                    !is_forward_iter<InputIt>::value, InputIt> last,
           const Allocator& a = Allocator())
        : vector(a) {
        for (;first != last; ++first) {
            emplace_back(*first);
        }
    }

    vector(std::initializer_list<value_type> list)
        : vector(list.begin(), list.end()) {
    }

    vector(std::initializer_list<value_type> list, const Allocator& a)
        : vector(a) {
        allocate_n(list.size());
        construct_at_end(list.begin(), list.end());
    }

    vector(const vector& other)
        : vector(allocator_traits::select_on_container_copy_construction(other.alloc())) {
        allocate_n(other.capacity());
        construct_at_end(other.begin_, other.end_);
    }

    vector(const vector& other, const Allocator& a)
        : vector(a) {
        allocate_n(other.capacity());
        construct_at_end(other.begin_, other.end_);
    }

    vector(vector&& other)
        : vector(std::move(other.alloc())) {
        begin_ = other.begin_;
        end_ = other.end_;
        end_cap() = other.end_cap();
        other.begin_ = other.end_ = other.end_cap() = nullptr;
    }

    vector(vector&& other, const Allocator& a)
        : vector(a) {
        if (other.alloc() == a) {
            begin_ = other.begin_;
            end_ = other.end_;
            end_cap() = other.end_cap();
            other.begin_ = other.end_ = other.end_cap() = nullptr;
        } else {
            assign(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        }
    }

public:
    const T* data() const noexcept { return begin_; }
    T* data() noexcept             { return begin_; }

    iterator begin() noexcept { return begin_; }
    iterator end() noexcept   { return end_; }

    const_iterator begin() const noexcept { return begin_; }
    const_iterator end() const noexcept   { return end_; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept   { return end(); }

    reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end_);   }
    reverse_iterator rend() noexcept   { return std::make_reverse_iterator(begin_); }

    const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end_);   }
    const_reverse_iterator rend() const noexcept   { return std::make_reverse_iterator(begin_); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept   { return rend();   }

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
        if (i >= size())
            throw std::out_of_range("vector index out of bounds");
        return begin_[i];
    }

    reference at(size_t i) {
        if (i >= size())
            throw std::out_of_range("vector index out of bounds");
        return begin_[i];
    }

    allocator_type get_allocator() const noexcept {
        return alloc();
    }

public:
    template<typename InputIt>
    void assign(InputIt b, InputIt e) { // \todo
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
        check_reserve();
        unsafe_push_back(elem);
    }

    void push_back(value_type&& elem) {
        check_reserve();
        unsafe_push_back(std::move(elem));
    }

    template<typename... Args>
    reference emplace_back(Args&&... args) {
        check_reserve();
        allocator_traits::construct(alloc(), end_, std::forward<Args>(args)...);
        ++end_;
        return back();
    }

    void pop_back() {
        allocator_traits::destroy(alloc(), end_ - 1);
        --end_;
    }

    iterator insert(const_iterator pos, const T& value) {
        return insert_impl(pos - begin(), value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return insert_impl(pos - begin(), std::move(value));
    }

    template<class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        auto len = std::distance(first, last);
        if (size() == capacity()) {
            split_buffer<value_type, allocator_type&> buff(size() + len, expand(size()), alloc());
            swap_out_buffer(buff, begin_ + pos);
        } else {

        }
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

    allocator_type& alloc()             { return end_cap_allocator_.second(); }
    const allocator_type& alloc() const { return end_cap_allocator_.second(); }

    pointer& end_cap()             { return end_cap_allocator_.first(); }
    const pointer& end_cap() const { return end_cap_allocator_.first(); }

    void swap_out_buffer(split_buffer<value_type, allocator_type&>& buff) {
        uninit_move(buff.alloc(), begin_, end_, buff.begin);
        std::swap(begin_, buff.begin);
        std::swap(end_, buff.end);
        std::swap(end_cap(), buff.end_cap());
    }

    void swap_out_buffer(split_buffer<value_type, allocator_type&>& buff, pointer pos) {
        uninit_move(buff.alloc(), begin_, pos, buff.begin);
        uninit_move(buff.alloc(), pos, end_, buff.begin + (pos - begin_ + 1));
        std::swap(begin_, buff.begin);
        std::swap(end_, buff.end);
        std::swap(end_cap(), buff.end_cap());
    }
    void check_reserve() {
        if (end_ == end_cap()) {
            reserve(expand(size()));
        }
    }

    template<typename U>
    void unsafe_push_back(U&& elem) {
        allocator_traits::construct(alloc(), end_, std::forward<U>(elem));
        ++end_;
    }

    template<typename U>
    iterator insert_impl(difference_type pos, U&& value) {
        if (size() == capacity()) {
            split_buffer<value_type, allocator_type&> buff(size(), expand(size()), alloc());
            swap_out_buffer(buff, begin_ + pos);
        } else {
            std::move_backward(begin_ + pos, end_, end_ + 1);
        }
        allocator_traits::construct(alloc(), begin_ + pos, std::forward<U>(value));
        ++end_;
        return begin() + pos;
    }

    void allocate_n(size_type n) {
        end_ = begin_ = allocator_traits::allocate(alloc(), n);
        end_cap() = begin_ + n;
    }

    void construct_at_end(size_t n) {
        while (n-- != 0) {
            allocator_traits::construct(alloc(), end_++);
        }
    }

    void construct_at_end(size_t n, const_reference value) {
        while (n-- != 0) {
            allocator_traits::construct(alloc(), end_++, value);
        }
    }

    template<typename InputIt>
    void construct_at_end(InputIt first, InputIt last) {
        end_ = uninit_copy(alloc(), first, last, end_);
    }

private:
    template<typename InputIt, typename OutIt>
    static OutIt uninit_move(allocator_type& alloc, InputIt begin, InputIt end, OutIt res) {
        while (begin != end) {
            allocator_traits::construct(alloc, res++, std::move_if_noexcept(*(begin++)));
        }
        return res;
    }

    template<typename InputIt, typename OutIt>
    static OutIt uninit_copy(allocator_type& alloc, InputIt begin, InputIt end, OutIt res) {
        while (begin != end) {
            allocator_traits::construct(alloc, res++, *(begin++));
        }
        return res;
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
