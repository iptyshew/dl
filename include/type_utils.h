#pragma once
#include <type_traits>
#include <iterator>

namespace dl {

template<typename T>
using is_forward_iter =
    typename std::is_convertible<typename std::iterator_traits<T>::iterator_category,
                                 std::forward_iterator_tag>;

template<typename T>
using is_input_iter =
    typename std::is_convertible<typename std::iterator_traits<T>::iterator_category,
                                 std::input_iterator_tag>;


} // namespace dl
