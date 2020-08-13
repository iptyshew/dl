#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include "vector.h"
#include "test_type.h"

using size_type = dl::vector<int>::size_type;
size_type cast(int n) {
    return static_cast<size_type>(n);
}

bool check_trace(size_t bc, size_t clc, size_t mrc, size_t olc, size_t orc, size_t d) {
    return
        trace_int::basic_construct == bc &&
        trace_int::copy_lval_construct == clc &&
        trace_int::move_rval_construct == mrc &&
        trace_int::operator_lval_construct == olc &&
        trace_int::operator_rval_construct == orc &&
        trace_int::destruct == d;
}

auto makeVector(std::initializer_list<int> l) {
    return dl::vector<trace_int>(l.begin(), l.end());
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

TEST(VectorTest, Constructors) {
    {
        dl::vector<int> vec(3);
        ASSERT_EQ(vec.size(), cast(3));
        ASSERT_EQ(vec.capacity(), cast(3));
        dl::vector<int> res{0, 0, 0};
        ASSERT_EQ(vec, res);
    }
    {
        dl::vector<int> vec(3, 3);
        ASSERT_EQ(vec.size(), cast(3));
        ASSERT_EQ(vec.capacity(), cast(3));
        dl::vector<int> res{3, 3, 3};
        ASSERT_EQ(vec, res);
    }
    {
        auto list = {1, 2, 3};
        dl::vector<int> vec(list.begin(), list.end());
        ASSERT_EQ(vec.size(), cast(3));
        ASSERT_EQ(vec.capacity(), cast(3));
        dl::vector<int> res{1, 2, 3};
        ASSERT_EQ(vec, res);
    }
    {
        dl::vector<int> temp{1, 2, 3};
        dl::vector<int> vec(temp);
        ASSERT_EQ(vec.size(), cast(3));
        ASSERT_EQ(vec.capacity(), cast(3));
        dl::vector<int> res{1, 2, 3};
        ASSERT_EQ(vec, res);
    }
    {
        dl::vector<int> temp{1, 2, 3};
        dl::vector<int> vec(std::move(temp));
        ASSERT_EQ(vec.size(), cast(3));
        ASSERT_EQ(vec.capacity(), cast(3));

        ASSERT_EQ(temp.size(), cast(0));
        ASSERT_EQ(temp.capacity(), cast(0));
        dl::vector<int> res{1, 2, 3};
        ASSERT_EQ(vec, res);
    }
    { // input iterator
        std::stringstream stream;
        stream << "1" << " 2";
        std::istream_iterator<int> first(stream);
        std::istream_iterator<int> last;
        dl::vector<int> vec(first, last);
        dl::vector<int> res{1, 2};
        ASSERT_EQ(vec, res);
    }
}

TEST(VectorTest, ChangeLastElement) {
    { // rvalue
        dl::vector<trace_int> vec;
        trace_int::init();
        vec.push_back({1});
        vec.push_back({2});
        vec.push_back({3});
        ASSERT_TRUE(check_trace(3, 0, 6, 0, 0, 6) &&
                    vec.size() == 3 &&
                    vec.capacity() == 4);
        ASSERT_EQ(vec, makeVector({1, 2, 3}));
    }

    { // lvalue
        dl::vector<trace_int> vec;
        trace_int v1(1), v2(2), v3(3);
        trace_int::init();
        vec.push_back(v1);
        vec.push_back(v2);
        vec.push_back(v3);
        ASSERT_TRUE(check_trace(0, 3, 3, 0, 0, 3) &&
                    vec.size() == 3 &&
                    vec.capacity() == 4);
        ASSERT_EQ(vec, makeVector({1, 2, 3}));
    }

    { // emplace_back
        auto vec = makeVector({1, 2});
        trace_int::init();
        vec.emplace_back(3);
        ASSERT_TRUE(check_trace(1, 0, 2, 0, 0, 2) &&
                    vec.size() == 3 &&
                    vec.capacity() == 4);
        auto res = makeVector({1, 2, 3});
        ASSERT_EQ(vec, res);
    }
    { // pop_back
        auto vec = makeVector({1, 2, 3});
        trace_int::init();
        vec.pop_back();
        ASSERT_TRUE(check_trace(0, 0, 0, 0, 0, 1) &&
                    vec.size() == 2 &&
                    vec.capacity() == 3);
        auto res = makeVector({1, 2});
        ASSERT_EQ(vec, res);
    }
}

TEST(VectorTest, Iterator) {
    {
        dl::vector<int> vec{1, 2, 3};
        auto it = vec.begin();

        ASSERT_EQ(++it, vec.begin() + 1);
        ASSERT_EQ(it++, vec.begin() + 1);
        ASSERT_EQ(it, vec.begin() + 2);
        ASSERT_EQ(*it, vec.begin()[2]);
        ASSERT_EQ(it + 1, vec.end());
        ASSERT_EQ(it - 2, vec.begin());

        ASSERT_EQ(it - vec.begin(), 2);
        ASSERT_TRUE(vec.begin() < it);

        auto reverse_list = {3, 2, 1};
        auto b1 = vec.rbegin();
        auto b2 = reverse_list.begin();
        while (b1 != vec.rend()) {
            ASSERT_EQ(*b1++, *b2++);
        }
        vec.clear();
        ASSERT_EQ(vec.begin(), vec.end());
    }
    {
        const dl::vector<int> vec{1, 2, 3};
        ASSERT_EQ(vec.cbegin(), vec.begin());
        ASSERT_EQ(vec.crbegin(), vec.rbegin());

        ASSERT_EQ(vec.cend(), vec.end());
        ASSERT_EQ(vec.crend(), vec.rend());
    }
}

TEST(VectorTest, reserve) {
    auto vec = makeVector({1, 2, 3, 4});
    auto res = vec;

    trace_int::init();
    vec.reserve(10);
    ASSERT_TRUE(check_trace(0, 0, 4, 0, 0, 4) &&
                vec.size() == 4 &&
                vec.capacity() == 10);
    ASSERT_EQ(vec, res);

    trace_int::init();
    vec.reserve(5);
    ASSERT_TRUE(check_trace(0, 0, 0, 0, 0, 0) &&
                vec.size() == 4 &&
                vec.capacity() == 10);
    ASSERT_EQ(vec, res);
}

TEST(VectorTest, resize) {
    { // n > capacity, n > size
        trace_int::init();
        auto vec = makeVector({1, 2});
        vec.reserve(4);
        trace_int::init();
        vec.resize(6);
        ASSERT_TRUE(check_trace(4, 0, 2, 0, 0, 2) &&
                    vec.size() == 6 &&
                    vec.capacity() == 6);
        auto res = makeVector({1, 2, 0, 0, 0, 0});
        ASSERT_EQ(vec, res);
    }

    { // n < capacity , n < size
        auto vec = makeVector({1, 2, 3, 4});
        vec.reserve(6);
        trace_int::init();
        vec.resize(2);
        ASSERT_TRUE(check_trace(0, 0, 0, 0, 0, 2) &&
                    vec.size() == 2 &&
                    vec.capacity() == 6);
        auto res = makeVector({1, 2});
        ASSERT_EQ(vec, res);
    }

    { // n < capacity, n > size
        auto vec = makeVector({1, 2});
        vec.reserve(6);
        trace_int::init();
        vec.resize(4);
        ASSERT_TRUE(check_trace(2, 0, 0, 0, 0, 0) &&
                    vec.size() == 4 &&
                    vec.capacity() == 6);
        auto res = makeVector({1, 2, 0, 0});
        ASSERT_EQ(vec, res);
    }

    { // n = capacity = size
        auto vec = makeVector({1, 2, 3, 4});
        auto res = vec;
        trace_int::init();
        vec.resize(4);
        ASSERT_TRUE(check_trace(0, 0, 0, 0, 0, 0) &&
                    vec.size() == 4 &&
                    vec.capacity() == 4);
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

TEST(VectorTest, insert) {
    { // some insert;
        dl::vector<trace_int> vec;
        trace_int v1(1), v2(2), v3(3), v4(4);
        trace_int::init();
        vec.insert(vec.begin(), v1);
        vec.insert(vec.begin(), v2);
        vec.insert(vec.begin(), v3);
        vec.insert(vec.begin(), v4);
        ASSERT_TRUE(check_trace(0, 4, 3, 0, 3, 3) &&
                    vec.size() == 4 &&
                    vec.capacity() == 4);
        auto res = makeVector({4, 3, 2, 1});
        ASSERT_EQ(vec, res);
    }
    { // push back
        auto vec = makeVector({1, 2});
        trace_int val(3);
        trace_int::init();
        vec.insert(vec.end(), val);
        ASSERT_TRUE(check_trace(0, 1, 2, 0, 0, 2) &&
                    vec.size() == 3 &&
                    vec.capacity() == 4);
        auto res = makeVector({1, 2, 3});
        ASSERT_EQ(vec, res);
    }
}

// TEST(VectorTest, assign) {
//     { // assign count > capacity
//         dl::vector<int> vec;
//         vec.reserve(2);
//         dl::vector<int> res{1, 2, 3};
//         vec.assign(res.begin(), res.end());
//         ASSERT_EQ(vec.size(), cast(3));
//         ASSERT_EQ(vec.capacity(), cast(3));
//         ASSERT_EQ(vec, res);
//     }
//     { // assign count < size
//         dl::vector<int> vec{1, 2, 3};
//         vec.reserve(4);
//         dl::vector<int> res{1, 2};
//         vec.assign(res.begin(), res.end());
//         ASSERT_EQ(vec.size(), res.size());
//         ASSERT_EQ(vec.capacity(), cast(4));
//         ASSERT_EQ(vec, res);
//     }
//     { // assign count > size, but < capacity
//         dl::vector<int> vec{1, 2, 3};
//         vec.reserve(5);
//         dl::vector<int> res{1, 2, 3, 4};
//         vec.assign(res.begin(), res.end());
//         ASSERT_EQ(vec.size(), res.size());
//         ASSERT_EQ(vec.capacity(), cast(5));
//         ASSERT_EQ(vec, res);
//     }
// }
