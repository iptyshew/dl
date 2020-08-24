#include <gtest/gtest.h>
#include "memory.h"
#include "test_type.h"

TEST(UniquePtrTest, Basic) {
    dl::unique_ptr<int> ptr;
    EXPECT_EQ(sizeof(ptr), sizeof(void*));
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_FALSE(ptr);
}
