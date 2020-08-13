#pragma once
#include <cstddef>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
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
public: // aliases
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

    vector(const allocator_type& alloc) noexcept
        : end_cap_allocator_(nullptr, alloc) {}

    explicit vector(size_type count,
                    const allocator_type& a = allocator_type())
        : vector(a) {
        create(count);
    }

    vector(size_type count, const value_type& value,
           const allocator_type& a = allocator_type())
        : vector(a) {
        create(count, value);
    }

    template<typename I>
    vector(I first,
           typename std::enable_if_t<is_forward_iter<I>::value, I> last,
           const allocator_type& a = allocator_type())
        : vector(a) {
        create(first, last);
    }

    template<typename I>
    vector(I first,
           typename std::enable_if_t<is_input_iter<I>::value &&
                                    !is_forward_iter<I>::value, I> last,
           const allocator_type& a = allocator_type())
        : vector(a) {
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    vector(std::initializer_list<value_type> list, const allocator_type& a = allocator_type())
        : end_cap_allocator_(nullptr, a) {
        create(list.begin(), list.end());
    }

    vector(const vector& other)
        : vector(allocator_traits::select_on_container_copy_construction(other.alloc())) {
        create(other.begin_, other.end_);
    }

    vector(const vector& other, const allocator_type& a)
        : vector(a) {
        create(other.begin_, other.end_);
    }

    vector(vector&& other)
        : vector(std::move(other.alloc())) {
        begin_ = other.begin_;
        end_ = other.end_;
        end_cap() = other.end_cap();
        other.begin_ = other.end_ = other.end_cap() = nullptr;
    }

    vector(vector&& other, const allocator_type& a)
        : vector(a) {
        if (other.alloc() == a) {
            begin_ = other.begin_;
            end_ = other.end_;
            end_cap() = other.end_cap();
            other.begin_ = other.end_ = other.end_cap() = nullptr;
        } else {
            create(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        }
    }

public: // access members
    const value_type* data() const noexcept { return begin_; }
    value_type* data() noexcept             { return begin_; }

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

public: // assigns
    template<typename I>
    std::enable_if_t<is_forward_iter<I>::value, void>
    assign(I first, I last) {
        auto n = static_cast<size_type>(std::distance(first, last));
        if (n > capacity()) {
            clear_and_free();
            create(first, last, n);
        } else if (n < size()) {
            end_ = clear(std::copy(first, last, begin_), end_);
        } else if (n > size()) {
            for (auto it = begin_; it != end_; ++it, ++first) {
                *it = *first;
            }
            end_ = uninit_copy(alloc(), first, last, end_);
        }
    }

    template<typename I>
    typename std::enable_if_t<is_input_iter<I>::value &&
                              !is_forward_iter<I>::value, void>
    assign(I first, I last) {
        clear();
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    void assign(std::initializer_list<value_type> list) {
        assign(list.begin(), list.end());
    }

    void assign(size_type n, const value_type& value) {
        if (n > capacity()) {
            clear_and_free();
            create(n, value);
        } else if (n < size()) {
            std::fill(begin_, begin_ + n, value);
            end_ = clear(begin_ + n, end_);
        } else if (n > size()) {
            std::fill(begin_, end_, value);
            end_ = construct_range(alloc(), end_, begin_ + n, value);
        }
    }

public: // other modification members
    void clear() noexcept {
        end_ = clear(begin_, end_);
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
            split_buffer<value_type, allocator_type&> buff(n, expand(n), alloc());
            construct_range(buff.alloc(), buff.begin + sz, buff.end);
            swap_out_buffer(buff);
        } else if (n < sz) {
            end_ = clear(begin_ + n, end_);
        } else if (n > sz) {
            end_ = construct_range(alloc(), begin_ + sz, begin_ + n);
        }
    }

    void resize(size_type n, const value_type& value) { // \todo :(
        auto sz = size();
        if (n > capacity()) {
            split_buffer<value_type, allocator_type&> buff(n, expand(n), alloc());
            construct_range(buff.alloc(), buff.begin + sz, buff.end, value);
            swap_out_buffer(buff);
        } else if (n < sz) {
            end_ = clear(begin_ + n, end_);
        } else if (n > sz) {
            end_ = construct_range(alloc(), begin_ + sz, begin_ + n, value);
        }
    }

    void push_back(const_reference elem) {
        reserve_if_full();
        unsafe_insert(end_, elem);
    }

    void push_back(value_type&& elem) {
        reserve_if_full();
        unsafe_insert(end_, std::move(elem));
    }

    template<typename... Args>
    reference emplace_back(Args&&... args) {
        reserve_if_full();
        unsafe_insert(end_, std::forward<Args>(args)...);
        return back();
    }

    iterator insert(const_iterator pos, const value_type& value) {
        return insert_impl(pos - begin(), value);
    }

    iterator insert(const_iterator pos, value_type&& value) {
        return insert_impl(pos - begin(), std::move(value));
    }

    template<class I>
    iterator insert(const_iterator pos, I first, I last) {

    }

    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        insert_impl(pos - begin(), std::forward<Args>(args)...);
    }


    void pop_back() {
        allocator_traits::destroy(alloc(), end_ - 1);
        --end_;
    }

    iterator erase(const_iterator pos) {
        auto n = pos - begin();
        std::move(begin_ + n + 1, end_, begin_ + n);
        pop_back();
        return begin() + n;
    }

    iterator erase(const_iterator first, const_iterator last) {
        auto f = first - begin();
        auto l = last - begin();
        std::move(begin_ + l, end_, begin_ + f);
        end_ = clear(begin_ + size() - (l - f), end_);
        return begin() + f;
    }

    void swap(vector& other) noexcept {
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(end_cap_allocator_, other.end_cap_allocator_);
    }

    void shrink_to_fit() {
        if (capacity() != size()) {
            split_buffer<value_type, allocator_type&> buff(size(), size(), alloc());
            swap_out_buffer(buff);
        }
    }

    ~vector() {
        clear_and_free();
    }

private:
    size_t expand(size_t new_size) const noexcept {
        return std::max(new_size, capacity() * 2);
    }

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

    void reserve_if_full() {
        if (end_ == end_cap()) {
            reserve(expand(size() + 1));
        }
    }

    template<typename... Args>
    void unsafe_insert(pointer p, Args&&... elem) {
        allocator_traits::construct(alloc(), p, std::forward<Args>(elem)...);
        ++end_;
    }

    template<typename... Args>
    iterator insert_impl(difference_type pos, Args&&... args) {
        if (size() == capacity()) {
            split_buffer<value_type, allocator_type&> buff(size(), expand(size() + 1), alloc());
            swap_out_buffer(buff, begin_ + pos);
        } else {
            std::move_backward(begin_ + pos, end_, end_ + 1);
        }
        unsafe_insert(begin_ + pos, std::forward<Args>(args)...);
        return begin() + pos;
    }

    void allocate_n(size_type n) {
        end_ = begin_ = allocator_traits::allocate(alloc(), n);
        end_cap() = begin_ + n;
    }

    void create(size_t n) {
        allocate_n(n);
        while(n-- != 0) {
            unsafe_insert(end_);
        }
    }

    void create(size_t n, const_reference value) {
        allocate_n(n);
        while (n-- != 0) {
            unsafe_insert(end_, value);
        }
    }

    template<typename I>
    void create(I first, I last, std::optional<size_type> n = std::nullopt) {
        allocate_n(n ? *n : std::distance(first, last));
        end_ = uninit_copy(alloc(), first, last, end_);
    }

    void clear_and_free() {
        clear();
        allocator_traits::deallocate(alloc(), begin_, capacity());
    }

    pointer clear(pointer begin, pointer end) {
        for (auto it = begin; it != end; ++it) {
            allocator_traits::destroy(alloc(), it);
        }
        return begin;
    }

private:
    template<typename I, typename O>
    static O uninit_move(allocator_type& alloc, I begin, I end, O res) {
        for (; begin != end; ++begin, ++res) {
            allocator_traits::construct(alloc, res, std::move_if_noexcept(*begin));
        }
        return res;
    }

    template<typename I, typename O>
    static O uninit_copy(allocator_type& alloc, I begin, I end, O res) {
        for (; begin != end; ++begin, ++res) {
            allocator_traits::construct(alloc, res, *begin);
        }
        return res;
    }

    static pointer construct_range(allocator_type& alloc, pointer begin, pointer end) {
        for (; begin != end; ++begin) {
            allocator_traits::construct(alloc, begin);
        }
        return begin;
    }

    static pointer construct_range(allocator_type& alloc, pointer begin, pointer end, const value_type& val) {
        for (; begin != end; ++begin) {
            allocator_traits::construct(alloc, begin, val);
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
