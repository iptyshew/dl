#pragma once
#include <memory>

namespace dl {

template<typename I, typename O, typename Allocator>
O uninit_move(Allocator& alloc, I begin, I end, O res) {
    for (; begin != end; ++begin, ++res) {
        std::allocator_traits<Allocator>::construct(alloc, res, std::move_if_noexcept(*begin));
    }
    return res;
}

template<typename I, typename O, typename Allocator>
O uninit_copy(Allocator& alloc, I begin, I end, O res) {
    for (; begin != end; ++begin, ++res) {
        std::allocator_traits<Allocator>::construct(alloc, res, *begin);
    }
    return res;
}

template<typename I, typename Allocator>
I construct(Allocator& alloc, I begin, I end) {
    for (; begin != end; ++begin) {
        std::allocator_traits<Allocator>::construct(alloc, begin);
    }
    return begin;
}

template<typename I, typename T, typename Allocator>
I construct(Allocator& alloc, I begin, I end, const T& val) {
    for (; begin != end; ++begin) {
        std::allocator_traits<Allocator>::construct(alloc, begin, val);
    }
    return begin;
}

template<typename I, typename Allocator>
I destroy(Allocator& alloc, I begin, I end) {
    for (auto it = begin; it != end; ++it) {
        std::allocator_traits<Allocator>::destroy(alloc, it);
    }
    return begin;
}

template<typename I, typename O>
I input_copy_n(I first, size_t n, O out) {
    while (n-- != 0) {
        *out = *first;
        ++first;
        ++out;
    }
    return first;
}

} // namespace dl
