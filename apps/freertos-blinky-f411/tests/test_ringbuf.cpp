// Host unit tests for lib/ringbuf (GoogleTest, run by CTest / CLion's test runner).
#include <gtest/gtest.h>

#include "ringbuf.h"

namespace {

TEST(Ringbuf, StartsEmpty)
{
    uint8_t mem[4];
    ringbuf_t rb;
    ringbuf_init(&rb, mem, sizeof mem);
    uint8_t b = 0;
    EXPECT_EQ(ringbuf_count(&rb), 0u);
    EXPECT_FALSE(ringbuf_get(&rb, &b));
}

TEST(Ringbuf, KeepsFifoOrder)
{
    uint8_t mem[4];
    ringbuf_t rb;
    ringbuf_init(&rb, mem, sizeof mem);
    for (uint8_t i = 1; i <= 3; i++) {
        ASSERT_TRUE(ringbuf_put(&rb, i));
    }
    for (uint8_t i = 1; i <= 3; i++) {
        uint8_t b = 0;
        ASSERT_TRUE(ringbuf_get(&rb, &b));
        EXPECT_EQ(b, i);
    }
}

TEST(Ringbuf, RejectsWhenFullAndWrapsAround)
{
    uint8_t mem[3];
    ringbuf_t rb;
    ringbuf_init(&rb, mem, sizeof mem);
    EXPECT_TRUE(ringbuf_put(&rb, 10));
    EXPECT_TRUE(ringbuf_put(&rb, 11));
    EXPECT_TRUE(ringbuf_put(&rb, 12));
    EXPECT_TRUE(ringbuf_full(&rb));
    EXPECT_FALSE(ringbuf_put(&rb, 13));

    uint8_t b = 0;
    ASSERT_TRUE(ringbuf_get(&rb, &b));
    EXPECT_EQ(b, 10);
    EXPECT_TRUE(ringbuf_put(&rb, 14)); // wraps to index 0
    for (uint8_t want : {11, 12, 14}) {
        ASSERT_TRUE(ringbuf_get(&rb, &b));
        EXPECT_EQ(b, want);
    }
    EXPECT_EQ(ringbuf_count(&rb), 0u);
}

} // namespace
