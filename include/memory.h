#pragma once

#include "compressed_pair.h"
#include <type_traits>

namespace dl {

template<typename T>
class default_delete
{
public:
    void operator()(T* p) {
        delete p;
    }
};

template<typename T, typename Deleter = default_delete<T>>
class unique_ptr
{
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

public:
    unique_ptr() : pointer_deleter_(nullptr, Deleter()) {}
    unique_ptr(pointer p) : pointer_deleter_(p, Deleter()) {}

    std::add_lvalue_reference_t<element_type> operator*() const noexcept {
        return *pointer_deleter_.first();
    }

    pointer get() noexcept {
        return pointer_deleter_.first();
    }

    pointer get() const noexcept {
        return pointer_deleter_.first();
    }

    deleter_type& get_deleter() noexcept {
        return pointer_deleter_.second();
    }

    const deleter_type& get_deleter() const noexcept {
        return pointer_deleter_.second();
    }

    operator bool() const {
        return get() != nullptr;
    }

    void reset(pointer ptr = pointer()) {
        get_deleter()(get());
        pointer_deleter_.first() = ptr;
    }

    ~unique_ptr() {
        reset();
    }

private:
    compressed_pair<pointer, deleter_type> pointer_deleter_;
};

} // namespace dl
