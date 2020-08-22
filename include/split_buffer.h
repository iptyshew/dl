#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>
#include "algorithm.h"
#include "compressed_pair.h"
#include "type_utils.h"

namespace dl {

template<typename T, typename Allocator>
class split_buffer
{
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    using reference = value_type&;
    using const_reference = const value_type&;
    using allocator_type = Allocator;
    using allocator_rr = typename std::remove_reference_t<allocator_type>;
    using allocator_traits = std::allocator_traits<allocator_rr>;
    using pointer = typename allocator_traits::pointer;
    using const_pointer = typename allocator_traits::const_pointer;

public:
    split_buffer(allocator_rr& a) : end_cap_allocator(nullptr, a) {}

    split_buffer(size_type size, size_type cap, allocator_rr& a) : split_buffer(a) {
        begin = cap != 0 ? allocator_traits::allocate(alloc(), cap) : nullptr;
        end = begin + size;
        end_cap() = begin + cap;
    }

    void clear() {
        while (begin != end) {
            allocator_traits::destroy(alloc(), end-- - 1);
        }
    }

    template<typename... Args>
    void emplace_back(Args&&... u) {
        allocator_traits::construct(alloc(), end, std::forward<Args>(u)...);
        ++end;
    }

    void construct_at_end(size_type n, const value_type& value) {
        while (n-- != 0) {
            allocator_traits::construct(alloc(), end, value);
            ++end;
        }
    }

    template<typename I>
    std::enable_if_t<is_forward_iter<I>::value, void>
    construct_at_end(I first, I last) {
        for (; first != last; ++first, ++end) {
            allocator_traits::construct(alloc(), end, *first);
        }
    }

    template<typename I>
    std::enable_if_t<is_input_iter<I>::value && !is_forward_iter<I>::value, void>
    construct_at_end(I first, I last) {
        for (; first != last; ++first, ++end) {
            if (end == end_cap()) {
                split_buffer buff(0, (capacity() + 1) * 2, alloc());
                buff.end = uninit_copy(alloc(), begin, end, buff.end);
                swap(buff);
            }
            allocator_traits::construct(alloc(), end, *first);
        }
    }

    void swap(split_buffer& other) {
        std::swap(begin, other.begin);
        std::swap(end, other.end);
        std::swap(end_cap(), other.end_cap());
    }

    ~split_buffer() {
        clear();
        allocator_traits::deallocate(alloc(), begin, capacity());
    }

    size_type capacity() const noexcept { return end_cap() - begin; }
    size_type size() const noexcept { return end - begin; }

    allocator_rr& alloc() { return end_cap_allocator.second(); }

    pointer& end_cap()             { return end_cap_allocator.first(); }
    const pointer& end_cap() const { return end_cap_allocator.first(); }

public:
    pointer begin = nullptr;
    pointer end = nullptr;
    compressed_pair<pointer, allocator_type> end_cap_allocator;
};

} // namespace dl
