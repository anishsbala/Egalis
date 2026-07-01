#include <array>
#include <ranges>
#include <span>

#include <gtest/gtest.h>

// A toolchain smoke test only; this does not exercise consensus behavior.
TEST(BuildContract, SupportsRequiredCxx20LibraryFeatures) {
  constexpr std::array values{1, 2, 3, 4};
  const std::span<const int> view{values};
  auto even = view | std::views::filter([](int value) { return value % 2 == 0; });
  int total = 0;
  for (const int value : even) {
    total += value;
  }
  EXPECT_EQ(total, 6);
}
