#include <gtest/gtest.h>
#include "vector.h"
#include "test_type.h"

using namespace dl;

using size_type = dl::vector<int>::size_type;
size_type cast(int n) {
    return static_cast<size_type>(n);
}

bool check_test_type(size_t dc, size_t clc, size_t mrc, size_t olc, size_t orc, size_t d) {
    return
        test_type::default_construct == dc &&
        test_type::copy_lval_construct == clc &&
        test_type::move_rval_construct == mrc &&
        test_type::operator_lval_construct == olc &&
        test_type::operator_rval_construct == orc &&
        test_type::destruct == d;
}

TEST(VectorTest, PushBack) {
    test_type::init();
    {
        dl::vector<test_type> vec;
        test_type temp;
        vec.push_back(std::move(temp));
        ASSERT_EQ(vec.size(), cast(1));
        ASSERT_EQ(vec.capacity(), cast(1));
    }
    ASSERT_TRUE(check_test_type(1, 0, 1, 0, 0, 2));

    test_type::init();
    {
        dl::vector<test_type> vec;
        test_type temp;
        vec.push_back(temp);
        vec.push_back(temp);
        vec.push_back(temp);
        ASSERT_EQ(vec.size(), cast(3));
        ASSERT_EQ(vec.capacity(), cast(4));
    }
    ASSERT_TRUE(check_test_type(1, 3, 3, 0, 0, 7));
}
