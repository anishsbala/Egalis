#pragma once

#include <array>
#include <bitset>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace egalis::protocol {

// Only the reviewed Section 3 baseline configuration is supported.
struct Configuration {
  static constexpr std::size_t replicas = 5;
  static constexpr std::size_t failures = 2;
  static constexpr std::size_t fast_failures = 1;
  static constexpr std::size_t slow_quorum = replicas - failures;
  static constexpr std::size_t fast_quorum = replicas - fast_failures;
  static void validate(std::size_t n, std::size_t f, std::size_t e);
};

class ReplicaId {
 public:
  explicit ReplicaId(std::uint32_t value);
  [[nodiscard]] std::uint32_t value() const noexcept { return value_; }
  auto operator<=>(const ReplicaId&) const = default;
 private:
  std::uint32_t value_;
};

[[nodiscard]] std::array<ReplicaId, Configuration::replicas> members();

class InstanceId {
 public:
  static constexpr std::uint64_t max_counter =
      (std::numeric_limits<std::uint64_t>::max() - 4) / 5;
  InstanceId(std::uint64_t counter, ReplicaId coordinator);
  [[nodiscard]] std::uint64_t counter() const noexcept { return counter_; }
  [[nodiscard]] ReplicaId coordinator() const noexcept { return coordinator_; }
  [[nodiscard]] std::uint64_t number() const noexcept {
    return counter_ * 5 + coordinator_.value();
  }
  // Counter first is the numerical order used to break SCC ties.
  auto operator<=>(const InstanceId&) const = default;
 private:
  std::uint64_t counter_;
  ReplicaId coordinator_;
};

class Ballot {
 public:
  static constexpr std::uint64_t max_round =
      std::numeric_limits<std::uint64_t>::max() / 5;
  Ballot() = default;  // Ballot zero belongs to the instance's initial coordinator.
  [[nodiscard]] static Ballot recovery(std::uint64_t round, ReplicaId owner);
  [[nodiscard]] Ballot next_owned_by(ReplicaId owner) const;
  [[nodiscard]] bool initial() const noexcept { return number_ == 0; }
  [[nodiscard]] std::uint64_t number() const noexcept { return number_; }
  [[nodiscard]] std::uint64_t round() const noexcept;
  [[nodiscard]] ReplicaId owner(const InstanceId& instance) const;
  auto operator<=>(const Ballot&) const = default;
 private:
  explicit Ballot(std::uint64_t number) : number_(number) {}
  std::uint64_t number_ = 0;
};

enum class Phase : std::uint8_t { initial, preaccepted, accepted, committed };

// These variables are deliberately distinct: joining a recovery ballot is not
// a vote to accept a proposal in that ballot (EPaxos* bal versus abal).
struct BallotState {
  Ballot current;
  Ballot accepted;
  [[nodiscard]] bool valid() const noexcept { return accepted <= current; }
};

class Quorum {
 public:
  bool add(ReplicaId replica);
  [[nodiscard]] bool contains(ReplicaId replica) const;
  [[nodiscard]] std::size_t size() const noexcept { return votes_.count(); }
  [[nodiscard]] bool slow() const noexcept { return size() >= Configuration::slow_quorum; }
  [[nodiscard]] bool fast() const noexcept { return size() >= Configuration::fast_quorum; }
 private:
  std::bitset<Configuration::replicas> votes_;
};

}  // namespace egalis::protocol
