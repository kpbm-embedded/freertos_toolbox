#include <gtest/gtest.h>
#include "filter.h"

TEST(Filter, EmptyIsZero) { filter_t f{}; EXPECT_EQ(filter_avg(&f), 0); }

TEST(Filter, PartialWindow) {
  filter_t f{};
  filter_add(&f, 10); filter_add(&f, 20);
  EXPECT_EQ(filter_avg(&f), 15);
}

TEST(Filter, SlidesOverOldSamples) {
  filter_t f{};
  for (int i = 0; i < FILTER_N; i++) filter_add(&f, 0);
  for (int i = 0; i < FILTER_N; i++) filter_add(&f, 80);
  EXPECT_EQ(filter_avg(&f), 80);
}

TEST(Filter, NegativeValues) {
  filter_t f{};
  filter_add(&f, -40); filter_add(&f, 0);
  EXPECT_EQ(filter_avg(&f), -20);
}
