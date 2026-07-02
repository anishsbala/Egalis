#pragma once

#include "egalis/sim/random.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace egalis::sim {

enum class DecisionKind : std::uint32_t {
  configured, scheduled, executed, cancelled, random, attached, sent, plan,
  link, crashed, restarted, delivered, discarded, timer, protocol
};
struct Decision {
  DecisionKind kind;
  std::vector<std::uint64_t> fields;
  std::string detail;
  bool operator==(const Decision&) const = default;
};
struct TraceData {
  std::uint32_t version = 2;
  std::uint32_t random_version = 1;
  std::uint64_t seed = 0;
  std::vector<Decision> decisions;
  bool operator==(const TraceData&) const = default;
};
class ReplayDivergence : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// Stable ASCII artifact with hex-encoded byte strings, independent of locale.
[[nodiscard]] std::string serialize(const TraceData& data);
[[nodiscard]] TraceData parse_trace(std::string_view text);

class Trace {
 public:
  explicit Trace(std::uint64_t seed = 0);
  explicit Trace(TraceData replay);
  void record(Decision decision);
  std::uint64_t choose(Random& random, std::uint64_t lower, std::uint64_t upper,
                       std::string label);
  void verify_complete() const;
  void check_healthy() const;
  [[nodiscard]] const TraceData& data() const noexcept { return data_; }
 private:
  [[noreturn]] void diverge(const std::string& reason) const;
  TraceData data_;
  bool replay_ = false;
  std::size_t cursor_ = 0;
  mutable bool failed_ = false;
};

}  // namespace egalis::sim
