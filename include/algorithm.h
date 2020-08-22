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

} // namespace dl
