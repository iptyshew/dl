#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <sstream>
#include "vector.h"
#include "test_type.h"

using size_type = dl::vector<int>::size_type;
size_type cast(int n) {
    return static_cast<size_type>(n);
}

#define CHECK_TRACE(bc, clc, mrc, olc, orc, d)                      \
    do {                                                            \
        EXPECT_EQ(trace_int::basic_construct, cast(bc));            \
        EXPECT_EQ(trace_int::copy_lval_construct, cast(clc));       \
        EXPECT_EQ(trace_int::move_rval_construct, cast(mrc));       \
        EXPECT_EQ(trace_int::operator_lval_construct, cast(olc));   \
        EXPECT_EQ(trace_int::operator_rval_construct, cast(orc));   \
        EXPECT_EQ(trace_int::destruct, cast(d));                    \
    } while (0)

#define CHECK_VECTOR(vec, res, cap)             \
    do {                                        \
        EXPECT_EQ(vec, res);                    \
        EXPECT_EQ(vec.capacity(), cast(cap));   \
    } while (0)

auto makeVector(std::initializer_list<int> l = {}) {
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
    CHECK_VECTOR(vec, dl::vector<int>{}, 2);

    vec.shrink_to_fit();
    ASSERT_EQ(vec.capacity(), cast(0));
}

TEST(VectorTest, Constructors) {
    {
        dl::vector<int> vec(3);
        dl::vector<int> res{0, 0, 0};
        CHECK_VECTOR(vec, res, 3);
    }
    {
        dl::vector<int> vec(3, 3);
        dl::vector<int> res{3, 3, 3};
        CHECK_VECTOR(vec, res, 3);
    }
    {
        auto list = {1, 2, 3};
        dl::vector<int> vec(list.begin(), list.end());
        dl::vector<int> res{1, 2, 3};
        CHECK_VECTOR(vec, res, 3);
    }
    {
        dl::vector<int> temp{1, 2, 3};
        dl::vector<int> vec(temp);
        dl::vector<int> res{1, 2, 3};
        CHECK_VECTOR(vec, res, 3);
    }
    {
        dl::vector<int> temp{1, 2, 3};
        dl::vector<int> vec(std::move(temp));

        ASSERT_EQ(temp.size(), cast(0));
        ASSERT_EQ(temp.capacity(), cast(0));
        dl::vector<int> res{1, 2, 3};
        CHECK_VECTOR(vec, res, 3);
    }
    { // input iterator
        std::stringstream stream;
        stream << "1" << " 2";
        std::istream_iterator<int> first(stream);
        std::istream_iterator<int> last;
        dl::vector<int> vec(first, last);
        dl::vector<int> res{1, 2};
        CHECK_VECTOR(vec, res, 2);
    }
}

TEST(VectorTest, BackOperations) {
    auto result = makeVector({1, 2, 3});
    { // rvalue
        auto vec = makeVector();
        trace_int vals[] = {trace_int(1), trace_int(2), trace_int(3)};
        trace_int::init();
        for (auto& v : vals) {
            vec.push_back(std::move(v));
        }
        CHECK_TRACE(0, 0, 6, 0, 0, 3);
        CHECK_VECTOR(vec, result, 4);
    }
    { // lvalue
        auto vec = makeVector();
        trace_int vals[] = {trace_int(1), trace_int(2), trace_int(3)};
        trace_int::init();
        for (const auto& v : vals) {
            vec.push_back(v);
        }
        CHECK_TRACE(0, 3, 3, 0, 0, 3);
        CHECK_VECTOR(vec, result, 4);
    }
    { // push lvalue reference from this
        auto vec = makeVector({1, 2});
        trace_int::init();
        vec.push_back(vec.back());
        CHECK_TRACE(0, 1, 2, 0, 0, 2);
        CHECK_VECTOR(vec, makeVector({1, 2, 2}), 4);
    }
    { // push rvalue reference from this
        auto vec = makeVector({1, 2});
        trace_int::init();
        vec.push_back(std::move(vec.back()));
        CHECK_TRACE(0, 0, 3, 0, 0, 2);
        CHECK_VECTOR(vec, makeVector({1, 0, 2}), 4);
    }
    { // emplace_back
        auto vec = makeVector({1, 2});
        trace_int::init();
        vec.emplace_back(3);
        CHECK_TRACE(1, 0, 2, 0, 0, 2);
        CHECK_VECTOR(vec, result, 4);
    }
    { // pop_back
        auto vec = makeVector({1, 2, 3, 4});
        trace_int::init();
        vec.pop_back();
        CHECK_TRACE(0, 0, 0, 0, 0, 1);
        CHECK_VECTOR(vec, result, 4);
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
    size_t n = 10;
    vec.reserve(n);
    CHECK_TRACE(0, 0, res.size(), 0, 0, res.size());
    CHECK_VECTOR(vec, res, n);

    trace_int::init();
    vec.reserve(5);
    CHECK_TRACE(0, 0, 0, 0, 0, 0);
    CHECK_VECTOR(vec, res, n);
}

TEST(VectorTest, resize) {
    { // n > capacity, n > size
        auto vec = makeVector({1, 2});
        vec.reserve(4);
        trace_int::init();
        vec.resize(6);
        CHECK_TRACE(4, 0, 2, 0, 0, 2);
        CHECK_VECTOR(vec, makeVector({1, 2, 0, 0, 0, 0}), 8);
    }
    { // n < capacity , n < size
        auto vec = makeVector({1, 2, 3, 4});
        vec.reserve(6);
        trace_int::init();
        vec.resize(2);
        CHECK_TRACE(0, 0, 0, 0, 0, 2);
        CHECK_VECTOR(vec, makeVector({1, 2}), 6);
    }
    { // n < capacity, n > size
        auto vec = makeVector({1, 2});
        vec.reserve(6);
        trace_int::init();
        vec.resize(4);
        CHECK_TRACE(2, 0, 0, 0, 0, 0);
        CHECK_VECTOR(vec, makeVector({1, 2, 0, 0}), 6);
    }
    { // n = capacity = size
        auto vec = makeVector({1, 2, 3, 4});
        auto res = vec;
        trace_int::init();
        vec.resize(4);
        CHECK_TRACE(0, 0, 0, 0, 0, 0);
        CHECK_VECTOR(vec, res, 4);
    }
    { // val is reference from this
        auto vec = makeVector({1, 2});
        vec.reserve(4);
        trace_int::init();
        vec.resize(6, vec.back());
        CHECK_TRACE(0, 4, 2, 0, 0, 2);
        CHECK_VECTOR(vec, makeVector({1, 2, 2, 2, 2, 2}), 8);
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

TEST(VectorTest, assign) {
    { // count > size, capacity
        auto vec = makeVector({1, 2});
        auto res = makeVector({10, 20, 30});
        auto old_size = vec.size();
        trace_int::init();
        vec.assign(res.begin(), res.end());
        CHECK_TRACE(0, res.size(), 0, 0, 0, old_size);
        CHECK_VECTOR(vec, res, vec.size());
    }
    { // count < size, capacity
        auto vec = makeVector({1, 2, 3});
        vec.reserve(4);
        auto res = makeVector({10, 20});
        trace_int::init();
        vec.assign(res.begin(), res.end());
        CHECK_TRACE(0, 0, 0, res.size(), 0, 1);
        CHECK_VECTOR(vec, res, 4);
    }
    { // size < count < capacity
        auto vec = makeVector({1, 2, 3});
        vec.reserve(6);
        auto res = makeVector({10, 20, 30, 40});
        trace_int::init();
        vec.assign(res.begin(), res.end());
        CHECK_TRACE(0, 1, 0, 3, 0, 0);
        CHECK_VECTOR(vec, res, 6);
    }
    { // assign range from self
        auto vec = makeVector({1, 2, 3});
        trace_int::init();
        vec.assign(vec.begin() + 1, vec.end());
        CHECK_TRACE(0, 0, 0, 2, 0, 1);
        CHECK_VECTOR(vec, makeVector({2, 3}), 3);
    }

    { // initializer list
        auto vec = makeVector();
        auto list = {trace_int(10), trace_int(20), trace_int(30)};
        vec.assign(list.begin(), list.end());
        auto res = makeVector({10, 20, 30});
        CHECK_VECTOR(vec, res, res.capacity());
    }

    trace_int val(10);
    { // count > size, capacity
        auto vec = makeVector({1, 2});
        auto res = makeVector({10, 10, 10});
        auto old_size = vec.size();
        trace_int::init();
        vec.assign(res.size(), val);
        CHECK_TRACE(0, res.size(), 0, 0, 0, old_size);
        CHECK_VECTOR(vec, res, vec.capacity());
    }
    { // count < size, capacity
        auto vec = makeVector({1, 2, 3});
        vec.reserve(4);
        auto res = makeVector({10, 10});
        trace_int::init();
        vec.assign(res.size(), val);
        CHECK_TRACE(0, 0, 0, res.size(), 0, 1);
        CHECK_VECTOR(vec, res, 4);
    }
    { // size < count < capacity
        auto vec = makeVector({1, 2, 3});
        vec.reserve(6);
        auto res = makeVector({10, 10, 10, 10});
        trace_int::init();
        vec.assign(res.size(), val);
        CHECK_TRACE(0, 1, 0, 3, 0, 0);
        CHECK_VECTOR(vec, res, 6);
    }
    { // assign val from self
        auto vec = makeVector({1, 2, 3});
        trace_int::init();
        vec.assign(3, vec.front());
        CHECK_TRACE(0, 0, 0, 3, 0, 0);
        CHECK_VECTOR(vec, makeVector({1, 1, 1}), 3);
    }
    { // input iterator
        std::stringstream stream;
        stream << "1" << " 2";
        std::istream_iterator<int> first(stream);
        std::istream_iterator<int> last;
        dl::vector<int> vec{-1, -2};
        vec.assign(first, last);
        dl::vector<int> res{1, 2};
        ASSERT_EQ(vec, res);
    }
}

TEST(VectorTest, erase) {
    { // erase middle
        auto vec = makeVector({1, 2, 3, 4});
        auto res = makeVector({1, 3, 4});
        trace_int::init();
        auto next = vec.erase(vec.begin() + 1);
        ASSERT_EQ(next, vec.begin() + 1);
        CHECK_TRACE(0, 0, 0, 0, 2, 1);
        CHECK_VECTOR(vec, res, 4);
    }
    { // erase first, last
        auto vec = makeVector({1, 2, 3, 4});
        auto res = makeVector({2, 3});
        auto next = vec.erase(vec.begin());
        ASSERT_EQ(next, vec.begin());
        next = vec.erase(vec.end() - 1);
        ASSERT_EQ(next, vec.end());
        ASSERT_EQ(vec, res);
    }
    { // erase one
        auto vec = makeVector({1});
        auto res = makeVector();
        auto next = vec.erase(vec.begin());
        ASSERT_EQ(next, vec.end());
        ASSERT_EQ(vec, res);
    }
    { // erase range
        auto vec = makeVector({1, 2, 3, 4, 5, 6, 7});
        auto res = makeVector({1, 2, 6, 7});
        trace_int::init();
        auto next = vec.erase(vec.begin() + 2, vec.begin() + 5);
        ASSERT_EQ(next, vec.begin() + 2);
        CHECK_TRACE(0, 0, 0, 0, 2, 3);
        CHECK_VECTOR(vec, res, 7);
    }
    { // erase range in end
        auto vec = makeVector({1, 2, 3, 4});
        auto res = makeVector({1, 2});
        auto next = vec.erase(vec.begin() + 2, vec.end());
        ASSERT_EQ(vec, res);
        ASSERT_EQ(next, vec.end());
    }
    { // erase range in begin
        auto vec = makeVector({1, 2, 3, 4});
        auto res = makeVector({4});
        auto next = vec.erase(vec.begin(), vec.begin() + 3);
        ASSERT_EQ(vec, res);
        ASSERT_EQ(next, vec.begin());
    }
    { // erase range in begin
        auto vec = makeVector({1, 2, 3, 4});
        auto res = makeVector();
        auto next = vec.erase(vec.begin(), vec.end());
        ASSERT_EQ(vec, res);
        ASSERT_EQ(next, vec.end());
    }
}

TEST(VectorTest, insert) {
    { // some inserts;
        auto vec = makeVector();
        std::initializer_list<trace_int> list{1, 2, 3, 4};
        trace_int::init();
        for (const auto& v : list) {
            vec.insert(vec.begin(), v);
        }
        CHECK_TRACE(0, 3, 4, 1, 2, 3);
        auto res = makeVector({4, 3, 2, 1});
        CHECK_VECTOR(vec, res, 4);
    }
    { // insert ref from self
        auto vec = makeVector({1, 2, 3});
        vec.reserve(5);
        trace_int::init();
        vec.insert(vec.begin(), vec.begin()[1]);
        CHECK_TRACE(0, 0, 1, 1, 2, 0);
        auto res = makeVector({2, 1, 2, 3});
        CHECK_VECTOR(vec, res, 5);
    }
    { // push back
        auto vec = makeVector({1, 2});
        trace_int val(3);
        trace_int::init();
        vec.insert(vec.end(), val);
        CHECK_TRACE(0, 1, 2, 0, 0, 2);
        CHECK_VECTOR(vec, makeVector({1, 2, 3}), 4);
    }
}

TEST(VectorTest, range_insert) {
    { // range len < end() - pos
        auto vec = makeVector({0, 3, 4, 5});
        vec.reserve(6);
        std::initializer_list<trace_int> list{1, 2};
        auto pos = vec.begin() + 1;
        trace_int::init();
        EXPECT_EQ(pos, vec.insert(vec.begin() + 1, list.begin(), list.end()));
        CHECK_TRACE(0, 0, 2, 2, 1, 0);
        CHECK_VECTOR(vec, makeVector({0, 1, 2, 3, 4, 5}), 6);
    }
    { // range_len > end() - pos
        auto vec = makeVector({0, 1, 2, 3, 4, 5});
        vec.reserve(12);
        dl::vector<trace_int> list(6, 6);
        auto pos = vec.begin() + 2;
        trace_int::init();
        EXPECT_EQ(pos, vec.insert(vec.begin() + 2, list.begin(), list.end()));
        CHECK_TRACE(0, 2, 4, 4, 0, 0);
        CHECK_VECTOR(vec, makeVector({0, 1, 6,6,6,6,6,6, 2, 3, 4, 5}), 12);
    }
    { // size() + range len > capacity()
        auto vec = makeVector({1, 4, 5});
        std::initializer_list<trace_int> list{2, 3};
        trace_int::init();
        auto res = vec.insert(vec.begin() + 1, list.begin(), list.end());
        EXPECT_EQ(res, vec.begin() + 1);
        CHECK_TRACE(0, 2, 3, 0, 0, 3);
        CHECK_VECTOR(vec, makeVector({1, 2, 3, 4, 5}), 6);
    }
    { // zero range
        auto vec = makeVector({1, 4, 5});
        std::initializer_list<trace_int> list;
        trace_int::init();
        auto res = vec.insert(vec.begin() + 1, list.begin(), list.end());
        EXPECT_EQ(res, vec.begin() + 1);
        CHECK_TRACE(0, 0, 0, 0, 0, 0);
        CHECK_VECTOR(vec, makeVector({1, 4, 5}), 3);
    }
}

TEST(VectorTest, insert_n_val) {
    { // range len < end() - pos
        auto vec = makeVector({0, 3, 4, 5});
        vec.reserve(6);
        trace_int val(10);
        trace_int::init();
        vec.insert(vec.begin() + 1, 2, val);
        CHECK_TRACE(0, 0, 2, 2, 1, 0);
        CHECK_VECTOR(vec, makeVector({0, 10, 10, 3, 4, 5}), 6);
    }
    { // range_len > end() - pos
        auto vec = makeVector({0, 1, 2, 3, 4, 5});
        vec.reserve(12);
        trace_int val(6);
        trace_int::init();
        vec.insert(vec.begin() + 2, 6, val);
        CHECK_TRACE(0, 2, 4, 4, 0, 0);
        CHECK_VECTOR(vec, makeVector({0, 1, 6,6,6,6,6,6, 2, 3, 4, 5}), 12);
    }
    { // size() + range len > capacity()
        auto vec = makeVector({1, 4, 5});
        trace_int val(6);
        trace_int::init();
        vec.insert(vec.begin() + 1, 2, val);
        CHECK_TRACE(0, 2, 3, 0, 0, 3);
        CHECK_VECTOR(vec, makeVector({1, 6, 6, 4, 5}), 6);
    }
    { // insert ref from self
        auto vec = makeVector({1, 4, 5});
        vec.reserve(5);
        trace_int::init();
        vec.insert(vec.begin(), 2, vec.begin()[1]);
        CHECK_TRACE(0, 0, 2, 2, 1, 0);
        CHECK_VECTOR(vec, makeVector({4, 4, 1, 4, 5}), 5);
    }
}

TEST(VectorTest, insert_input_iter) {
    { // range_len + size() <= capacity
        auto vec = makeVector({0, 3, 4, 5});
        vec.reserve(6);
        std::stringstream stream;
        stream << "1" << " 2";
        std::istream_iterator<int> first(stream);
        std::istream_iterator<int> last;
        trace_int::init();
        auto res = vec.insert(vec.begin() + 1, first, last);
        EXPECT_EQ(res, vec.begin() + 1);
        CHECK_TRACE(2, 0, 4, 0, 8, 4);
        CHECK_VECTOR(vec, makeVector({0, 1, 2, 3, 4, 5}), 6);
    }
    { // range_len + size() > capacity
        auto vec = makeVector({0, 3, 4, 5});
        vec.reserve(6);
        std::stringstream stream;
        stream << "10" << " 20" << " 30" << " 40";
        std::istream_iterator<int> first(stream);
        std::istream_iterator<int> last;
        trace_int::init();
        auto res = vec.insert(vec.begin() + 1, first, last);
        EXPECT_EQ(res, vec.begin() + 1);
        CHECK_TRACE(4, 0, 12, 0, 11, 12);
        CHECK_VECTOR(vec, makeVector({0, 10, 20, 30, 40, 3, 4, 5}), 12);
    }
}

TEST(VectorTest, emplace) {
    { // some insert;
        auto vec = makeVector();
        trace_int::init();
        for (int i = 1; i < 5; ++i) {
            vec.emplace(vec.begin(), i);
        }
        CHECK_TRACE(4, 0, 4, 0, 2, 4);
        CHECK_VECTOR(vec, makeVector({4, 3, 2, 1}), 4);
    }
    { // push back
        auto vec = makeVector({1, 2});
        trace_int val(3);
        trace_int::init();
        vec.emplace(vec.end(), val);
        CHECK_TRACE(0, 1, 2, 0, 0, 2);
        CHECK_VECTOR(vec, makeVector({1, 2, 3}), 4);
    }
}

#undef CHECK_TRACE
