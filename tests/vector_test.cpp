#include <algorithm>
#include <gtest/gtest.h>
#include "vector.h"
#include "test_type.h"

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

TEST(VectorTest, Basic) {
    dl::vector<int> vec;
    // check standart size
    ASSERT_EQ(sizeof(vec), 3 * sizeof(dl::vector<int>::size_type));

    vec.push_back(1);
    vec.push_back(2);

    ASSERT_EQ(vec.front(), 1);
    ASSERT_EQ(vec.back(), 2);
    ASSERT_EQ(vec[0], 1);
    ASSERT_EQ(vec.at(1), 2);
    ASSERT_EQ(vec.size(), cast(2));
    ASSERT_EQ(vec.data()[0], 1);

    vec.clear();
    ASSERT_EQ(vec.size(), cast(0));
}

TEST(VectorTest, ChangeLastElement) {
    { // rvalue
        test_type::init();
        dl::vector<test_type> vec;
        vec.push_back(test_type());
        ASSERT_EQ(vec.size(), cast(1));
        ASSERT_EQ(vec.capacity(), cast(1));
    }
    ASSERT_TRUE(check_test_type(1, 0, 1, 0, 0, 2));

    { // lvalue, expand capacity
        test_type::init();
        dl::vector<test_type> vec;
        test_type temp;
        vec.push_back(temp);
        vec.push_back(temp);
        vec.push_back(temp);
        ASSERT_EQ(vec.size(), cast(3));
        ASSERT_EQ(vec.capacity(), cast(4));
    }
    ASSERT_EQ(cast(3), test_type_exception::copy_lval_construct);
    ASSERT_EQ(cast(3), test_type_exception::move_rval_construct);
    ASSERT_EQ(cast(7), test_type_exception::destruct);

    ASSERT_TRUE(check_test_type(1, 3, 3, 0, 0, 7));

    { // don't use move has't noexcept
        test_type_exception::init();
        dl::vector<test_type_exception> vec;
        test_type_exception temp;
        vec.push_back(temp);
        vec.push_back(temp);
        vec.push_back(temp);
        ASSERT_EQ(cast(6), test_type_exception::copy_lval_construct);
        ASSERT_EQ(cast(0), test_type_exception::move_rval_construct);
    }
    { // check content
        dl::vector<int> vec;
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        dl::vector<int> res{1, 2, 3};
        ASSERT_EQ(vec, res);
    }
    { // emplacs_back
        dl::vector<std::pair<int, int>> vec;
        vec.emplace_back(1, 2);
        ASSERT_EQ(vec.front().first, 1);
        ASSERT_EQ(vec.front().second, 2);
    }
    { // pop_back
        dl::vector<int> vec{1, 2, 3};
        vec.pop_back();
        ASSERT_EQ(vec.size(), cast(2));
        ASSERT_EQ(vec.capacity(), cast(3));
        ASSERT_EQ(vec.back(), 2);
    }
}

TEST(VectorTest, Iterator) {
    dl::vector<int> vec;
    ASSERT_EQ(vec.begin(), vec.end());

    vec.assign({1, 2, 3,});
    auto it = vec.begin();

    ASSERT_EQ(++it, vec.begin() + 1);
    ASSERT_EQ(it++, vec.begin() + 1);
    ASSERT_EQ(it, vec.begin() + 2);
    ASSERT_EQ(*it, vec.begin()[2]);
    ASSERT_EQ(it + 1, vec.end());
    ASSERT_EQ(it - 2, vec.begin());

    ASSERT_EQ(it - vec.begin(), 2);
    ASSERT_TRUE(vec.begin() < it);
}

TEST(VectorTest, reserve) {
    test_type::init();
    dl::vector<test_type> vec;

    vec.reserve(100);
    ASSERT_EQ(vec.capacity(), cast(100));

    // capacity no down
    vec.reserve(50);
    ASSERT_EQ(vec.capacity(), cast(100));

    ASSERT_EQ(test_type::default_construct, cast(0));
    ASSERT_EQ(test_type::destruct, cast(0));
}

TEST(VectorTest, resize) {
    { // n > capacity, n > size
        test_type::init();
        dl::vector<test_type> vec;
        vec.reserve(10);
        vec.resize(5);
        vec.resize(15);
        ASSERT_EQ(vec.capacity(), cast(15));
        ASSERT_EQ(vec.size(), cast(15));
    }
    ASSERT_TRUE(check_test_type(15, 0, 5, 0, 0, 20));

    { // n < capacity , n < size
        test_type::init();
        dl::vector<test_type> vec;
        vec.reserve(10);
        vec.resize(7);
        vec.resize(3);
        ASSERT_EQ(vec.capacity(), cast(10));
        ASSERT_EQ(vec.size(), cast(3));
    }
    ASSERT_TRUE(check_test_type(7, 0, 0, 0, 0, 7));

    { // n < capacity, n > size
        test_type::init();
        dl::vector<test_type> vec;
        vec.reserve(10);
        vec.resize(1);
        vec.resize(5);
        ASSERT_EQ(vec.capacity(), cast(10));
        ASSERT_EQ(vec.size(), cast(5));
    }
    ASSERT_TRUE(check_test_type(5, 0, 0, 0, 0, 5));

    { // n = capacity = size
        test_type::init();
        dl::vector<test_type> vec;
        vec.reserve(2);
        vec.resize(2);
        vec.resize(2);
    }
    ASSERT_TRUE(check_test_type(2, 0, 0, 0, 0, 2));

    { // resize val
        test_type::init();
        dl::vector<int> vec;
        vec.resize(3, 3);
        dl::vector<int> res{3, 3, 3};
        ASSERT_EQ(vec, res);
    }
}

TEST(VectorTest, compare) {
    dl::vector<int> a{1, 2, 3};
    dl::vector<int> b{1, 2, 3};
    dl::vector<int> c{2, 3, 4};

    ASSERT_EQ(a, b);
    ASSERT_NE(b, c);

    ASSERT_GT(c, a);
    ASSERT_GE(b, a);

    ASSERT_LT(b, c);
    ASSERT_LE(a, b);

    ASSERT_FALSE(a < b);
    ASSERT_FALSE(b > a);
}
