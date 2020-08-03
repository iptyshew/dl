#pragma once

#include <type_traits>

namespace dl {

template<typename T,
         unsigned num,
         bool isEmpty = (std::is_empty_v<T> && !std::is_final_v<T>)>
class compressed_pair_elem
{
public:
    compressed_pair_elem() = default;
    compressed_pair_elem(T&& t) : elem(std::forward<T>(t)) {}
    const T& get() const { return elem; };
    T& get() { return elem; };
private:
    T elem;
};

template<typename T, unsigned num>
class compressed_pair_elem<T, num, true> : private T
{
public:
    compressed_pair_elem() = default;
    compressed_pair_elem(T&& t) { *this = std::forward<T>(t); }
    const T& get() const { return *this; };
    T& get() { return *this; };
};

template<typename T1, typename T2>
class compressed_pair : private compressed_pair_elem<T1, 0>
                      , private compressed_pair_elem<T2, 1>
{
public:
    using Base1 = compressed_pair_elem<T1, 0>;
    using Base2 = compressed_pair_elem<T2, 1>;
public:
    compressed_pair() : Base1(), Base2() {}
    compressed_pair(T1&& first, T2&& second)
        : Base1(std::forward<T1>(first))
        , Base2(std::forward<T2>(second)) {}

    const T1& first() const { return Base1::get(); }
    const T2& second() const { return Base2::get(); }

    T1& first() { return Base1::get(); }
    T2& second() { return Base2::get(); }
};

} // namespace dl
