#pragma once
#include <cstddef>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include "compressed_pair.h"
#include "split_buffer.h"
#include "type_utils.h"
#include "algorithm.h"

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

    explicit vector(const allocator_type& alloc) noexcept
        : end_cap_allocator_(nullptr, alloc) {}

    explicit vector(size_type count,
                    const allocator_type& a = allocator_type())
        : vector(a) {
        allocate_n(count);
        end_ = construct(alloc(), begin_, begin_ + count);
    }

    vector(size_type count, const value_type& value,
           const allocator_type& a = allocator_type())
        : vector(a) {
        allocate_n(count);
        end_ = construct(alloc(), begin_, begin_ + count, value);
    }

    template<typename I,
             std::enable_if_t<is_forward_iter<I>::value, int> = 0>
    vector(I first, I last, const allocator_type& a = allocator_type())
        : vector(a) {
        create(first, last);
    }

    template<typename I,
             std::enable_if_t<is_input_iter<I>::value && !is_forward_iter<I>::value, int> = 0>
    vector(I first, I last, const allocator_type& a = allocator_type())
        : vector(a) {
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    vector(std::initializer_list<value_type> list, const allocator_type& a = allocator_type())
        : vector(a) {
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
        : vector(other.alloc()) {
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
            split_buffer<value_type, allocator_type&> buff(0, n, alloc());
            buff.construct_at_end(first, last);
            swap(buff);
        } else if (n < size()) {
            end_ = destroy(alloc(), std::copy(first, last, begin_), end_);
        } else {
            first = input_copy_n(first, size(), begin_);
            end_ = uninit_copy(alloc(), first, last, end_);
        }
    }

    void assign(size_type n, const value_type& value) {
        auto new_end = begin_ + n;
        if (new_end > end_cap()) {
            split_buffer<value_type, allocator_type&> buff(0, n, alloc());
            buff.construct_at_end(n, value);
            swap(buff);
        } else {
            std::fill(begin_, std::min(new_end, end_), value);
            end_ = (new_end < end_)
                ? destroy(alloc(), new_end, end_)
                : construct(alloc(), end_, new_end, value);
        }
    }

    template<typename I>
    std::enable_if_t<is_input_iter<I>::value && !is_forward_iter<I>::value, void>
    assign(I first, I last) {
        clear();
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    void assign(std::initializer_list<value_type> list) {
        assign(list.begin(), list.end());
    }

public: // other modification members
    void clear() noexcept {
        end_ = destroy(alloc(), begin_, end_);
    }

    void reserve(size_type n) {
        if (n > capacity()) {
            split_buffer<value_type, allocator_type&> buff(size(), n, alloc());
            swap_out_buffer(buff);
        }
    }

    void resize(size_type n) {
        resize_impl(n, [this](pointer begin, pointer end) {
                           return construct(alloc(), begin, end);
                       });
    }

    void resize(size_type n, const value_type& value) {
        resize_impl(n, [&](pointer begin, pointer end) {
                           return construct(alloc(), begin, end, value);
                       });
    }

    void push_back(const_reference elem) {
        emplace_back(elem);
    }

    void push_back(value_type&& elem) {
        emplace_back(std::move(elem));
    }

    template<typename... Args>
    reference emplace_back(Args&&... args) {
        if (end_ != end_cap()) {
            fast_push_back(std::forward<Args>(args)...);
        } else {
            split_buffer<value_type, allocator_type&> buff(size(), expand(size() + 1), alloc());
            buff.emplace_back(std::forward<Args>(args)...);
            swap_out_buffer(buff);
        }
        return back();
    }

    iterator insert(const_iterator pos, const value_type& value) {
        return insert_impl(pos - begin(), value);
    }

    iterator insert(const_iterator pos, value_type&& value) {
        return insert_impl(pos - begin(), std::move(value));
    }

    template<typename I>
    std::enable_if_t<is_forward_iter<I>::value, iterator>
    insert(const_iterator cpos, I first, I last) {
        auto n = std::distance(first, last);
        auto idx = cpos - begin();
        auto pos = begin_ + idx;
        if (end_cap() < n + end_) {
            split_buffer<value_type, allocator_type&> buff(idx, expand(size() + n), alloc());
            buff.construct_at_end(first, last);
            swap_out_buffer(buff, pos);
        } else {
            if (auto tail = end_ - pos; tail < n) {
                auto m = first;
                std::advance(m, tail);
                uninit_copy(alloc(), m, last, end_);
                last = m;
            }
            end_ = right_shift(pos, n);
            std::copy(first, last, pos);
        }
        return begin_ + idx;
    }

    iterator insert(const_iterator cpos, size_type n, const value_type& value) {
        auto idx = cpos - begin();
        auto pos = begin_ + idx;
        if (end_cap() < end_ + n) {
            split_buffer<value_type, allocator_type&> buff(idx, expand(size() + n), alloc());
            buff.construct_at_end(n, value);
            swap_out_buffer(buff, pos);
        } else {
            auto count = static_cast<difference_type>(n);
            if (auto tail = end_ - pos; tail < count) {
                construct(alloc(), end_, end_ + (n - tail), value);
                count = tail;
            }
            end_ = right_shift(pos, n);
            auto vr = std::pointer_traits<const_pointer>::pointer_to(value);
            if (pos <= vr && vr < end_) {
                vr += n;
            }
            std::fill(pos, pos + count, *vr);
        }
        return begin_ + idx;
    }

    iterator insert(const_iterator pos, std::initializer_list<value_type> list) {
        insert(pos, list.begin(), list.end());
    }

    template<typename I>
    std::enable_if_t<is_input_iter<I>::value && !is_forward_iter<I>::value, iterator>
    insert(const_iterator cpos, I first, I last) {
        auto idx = cpos - begin();
        auto old_size = size();
        for (; first != last && end_ != end_cap(); ++first) {
            fast_push_back(*first);
        }
        split_buffer<value_type, allocator_type&> buff(alloc());
        if (first != last) {
            buff.construct_at_end(first, last);
            reserve(expand(size() + buff.size()));
        }
        insert(std::rotate(begin_ + idx, begin_ + old_size, end_),
               std::make_move_iterator(buff.begin), std::make_move_iterator(buff.end));
        return begin() + idx;
    }

    template<typename... Args>
    iterator emplace(const_iterator cpos, Args&&... args) {
        auto idx = cpos - begin();
        auto pos = begin_ + idx;
        if (end_ == end_cap()) {
            split_buffer<value_type, allocator_type&> buff(idx, expand(size() + 1), alloc());
            buff.emplace_back(std::forward<Args>(args)...);
            swap_out_buffer(buff, pos);
        } else if (pos != end_) {
            end_ = right_shift(pos, 1);
            allocator_traits::destroy(alloc(), pos); // \todo
            allocator_traits::construct(alloc(), pos, std::forward<Args>(args)...);
        } else {
            fast_push_back(std::forward<Args>(args)...);
        }

        return begin() + idx;
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

    iterator erase(const_iterator cfirst, const_iterator clast) {
        auto first = begin_ + (cfirst - begin());
        auto last = begin_ + (clast - begin());
        std::move(last, end(), first);
        end_ = destroy(alloc(), end_ - (last - first), end_);
        return first;
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
        clear();
        allocator_traits::deallocate(alloc(), begin_, capacity());
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
        swap(buff);
    }

    void swap_out_buffer(split_buffer<value_type, allocator_type&>& buff, pointer pos) {
        uninit_move(buff.alloc(), begin_, pos, buff.begin);
        buff.end = uninit_move(buff.alloc(), pos, end_, buff.end);
        swap(buff);
    }

    void swap(split_buffer<value_type, allocator_type&>& buff) {
        std::swap(begin_, buff.begin);
        std::swap(end_, buff.end);
        std::swap(end_cap(), buff.end_cap());
    }

    template<typename... Args>
    void fast_push_back(Args&&... elem) {
        allocator_traits::construct(alloc(), end_++, std::forward<Args>(elem)...);
    }

    template<typename Constructor>
    void resize_impl(size_type n, const Constructor& constructor) {
        auto sz = size();
        if (n > capacity()) {
            split_buffer<value_type, allocator_type&> buff(n, expand(n), alloc());
            constructor(buff.begin + sz, buff.end);
            swap_out_buffer(buff);
        } else if (n < sz) {
            end_ = destroy(alloc(), begin_ + n, end_);
        } else if (n > sz) {
            end_ = constructor(begin_ + sz, begin_ + n);
        }
    }

    template<typename U>
    iterator insert_impl(difference_type idx, U&& value) {
        auto pos = begin_ + idx;
        if (end_ == end_cap()) {
            split_buffer<value_type, allocator_type&> buff(idx, expand(size() + 1), alloc());
            buff.emplace_back(std::forward<U>(value));
            swap_out_buffer(buff, pos);
        } else if (pos != end_) {
            end_ = right_shift(pos, 1);
            auto vr = std::pointer_traits<const_pointer>::pointer_to(value);
            if (pos <= vr && vr < end_) {
                ++vr;
            }
            *pos = std::forward<U>(*vr);
        } else {
            fast_push_back(std::forward<U>(value));
        }
        return begin() + idx;
    }

    void allocate_n(size_type n) {
        end_ = begin_ = allocator_traits::allocate(alloc(), n);
        end_cap() = begin_ + n;
    }

    template<typename I>
    void create(I first, I last) {
        allocate_n(std::distance(first, last));
        end_ = uninit_copy(alloc(), first, last, end_);
    }

    pointer right_shift(pointer pos, difference_type n) {
        auto part = end_ - std::min(n, end_ - pos);
        if (part != end_) {
            uninit_move(alloc(), part, end_, part + n);
            std::move_backward(pos, part, end_);
        }
        return end_ + n;
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
