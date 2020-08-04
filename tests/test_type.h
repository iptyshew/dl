#pragma once
#include <cstddef>

class test_type
{
public:
    test_type() {
        ++default_construct;
    }
    test_type(const test_type&) {
        ++copy_lval_construct;
    }
    test_type(test_type&&) {
        ++move_rval_construct;
    }

    test_type& operator=(const test_type&) {
        ++operator_lval_construct;
        return *this;
    }

    test_type& operator=(test_type&&) {
        ++operator_rval_construct;
        return *this;
    }

    ~test_type() {
        ++destruct;
    }

    static void init() {
        default_construct = 0;
        copy_lval_construct = 0;
        move_rval_construct = 0;
        operator_lval_construct = 0;
        operator_rval_construct = 0;
        destruct = 0;
    }

    static size_t default_construct;
    static size_t copy_lval_construct;
    static size_t move_rval_construct;
    static size_t operator_lval_construct;
    static size_t operator_rval_construct;
    static size_t destruct;
};

size_t test_type::default_construct = 0;
size_t test_type::copy_lval_construct = 0;
size_t test_type::move_rval_construct = 0;
size_t test_type::operator_lval_construct = 0;
size_t test_type::operator_rval_construct = 0;
size_t test_type::destruct = 0;
