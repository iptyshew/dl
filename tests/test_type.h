#pragma once
#include <cstddef>
#include <iterator>
#include <ostream>

template<typename T>
class trace_type
{
public:
    trace_type() : value(T()) {
        ++basic_construct;
    }

    trace_type(T v) : value(v) {
        ++basic_construct;
    }

    trace_type(const trace_type& o) : value(o.value) {
        ++copy_lval_construct;
    }

    trace_type(trace_type&& o) noexcept
        : value(std::move(o.value)) {
        ++move_rval_construct;
        o.value = T();
    }

    trace_type& operator=(const trace_type& o) {
        value = o.value;
        ++operator_lval_construct;
        return *this;
    }

    trace_type& operator=(trace_type&& o) noexcept {
        value = std::move(o.value);
        ++operator_rval_construct;
        return *this;
    }

    ~trace_type() {
        ++destruct;
        value = T();
    }

    static void init() {
        basic_construct = 0;
        copy_lval_construct = 0;
        move_rval_construct = 0;
        operator_lval_construct = 0;
        operator_rval_construct = 0;
        destruct = 0;
    }
    T value;

    static size_t basic_construct;
    static size_t copy_lval_construct;
    static size_t move_rval_construct;
    static size_t operator_lval_construct;
    static size_t operator_rval_construct;
    static size_t destruct;
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const trace_type<T>& t) {
    stream << t.value;
    return stream;
}

template<typename T>
bool operator==(const trace_type<T>& lhs, const trace_type<T>& rhs) {
    return lhs.value == rhs.value;
}

template<typename T>
size_t trace_type<T>::basic_construct = 0;

template<typename T>
size_t trace_type<T>::copy_lval_construct = 0;

template<typename T>
size_t trace_type<T>::move_rval_construct = 0;

template<typename T>
size_t trace_type<T>::operator_lval_construct = 0;

template<typename T>
size_t trace_type<T>::operator_rval_construct = 0;

template<typename T>
size_t trace_type<T>::destruct = 0;

using trace_int = trace_type<int>;
template class trace_type<int>;
