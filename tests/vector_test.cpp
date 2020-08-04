#include <algorithm>
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

template<typename T>
bool check_vec(const dl::vector<T>& vec, std::initializer_list<T> list) {
    return vec.size() == list.size() && std::equal(vec.begin(), vec.end(), list.begin(), list.end());
}

TEST(VectorTest, Basic) {
    dl::vector<int> vec;
    ASSERT_EQ(sizeof(vec), 3 * sizeof(dl::vector<int>::size_type));
}

TEST(VectorTest, PushBack) {
    test_type::init();
    {
        dl::vector<test_type> vec;
        vec.push_back(test_type());
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
