#include "egalis/sim/random.hpp"

#include <stdexcept>

namespace egalis::sim {

std::uint64_t Random::next() {
  auto value = (state_ += UINT64_C(0x9e3779b97f4a7c15));
  value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
  return value ^ (value >> 31);
}

std::uint64_t Random::uniform(std::uint64_t lower, std::uint64_t upper) {
  if (upper < lower) throw std::invalid_argument("inverted random range");
  const std::uint64_t width = upper - lower + 1;
  if (width == 0) return next();  // The entire uint64 range, not division by zero.
  const std::uint64_t threshold = (std::uint64_t{0} - width) % width;
  std::uint64_t value;
  do { value = next(); } while (value < threshold);
  return lower + value % width;
}

}  // namespace egalis::sim
