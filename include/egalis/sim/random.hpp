#pragma once

#include <cstdint>

namespace egalis::sim {

// SplitMix64-v1 with explicitly defined unsigned arithmetic. No implementation-
// dependent standard-library distributions participate in simulator decisions.
class Random {
 public:
  explicit Random(std::uint64_t seed) : state_(seed) {}
  std::uint64_t next();
  std::uint64_t uniform(std::uint64_t lower, std::uint64_t upper);
 private:
  std::uint64_t state_;
};

}  // namespace egalis::sim
