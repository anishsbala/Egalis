#pragma once

#include "egalis/protocol/events.hpp"

#include <map>
#include <optional>

namespace egalis::protocol {

enum class DispatchStatus { processed, unsupported, stale };
struct DispatchResult {
  DispatchStatus status;
  Effects effects;
  bool operator==(const DispatchResult&) const = default;
};
struct InstanceRecord {
  InstanceState state;
  std::optional<TimerGuard> timer;
};

// Serialized, non-reentrant ownership boundary. No callbacks, threads, transport,
// clock or effect execution occur inside Replica. Runners execute returned effects
// only after dispatch returns, so synchronous self-delivery cannot reenter mutation.
class Replica {
 public:
  explicit Replica(ReplicaId id) : id_(id) {}
  Replica(const Replica&) = delete;
  Replica& operator=(const Replica&) = delete;
  Replica(Replica&&) = delete;
  Replica& operator=(Replica&&) = delete;

  DispatchResult dispatch(const Event& event);
  // Mechanism only: no grace/recovery timeout policy is selected here.
  Effects arm_timer(InstanceId instance, TimerDelay delay);
  Effects suspend();
  void resume();
  [[nodiscard]] bool active() const noexcept { return active_; }
  [[nodiscard]] ReplicaId id() const noexcept { return id_; }
  [[nodiscard]] const std::map<InstanceId, InstanceRecord>& instances() const noexcept {
    return instances_;
  }
 private:
  void require_active() const;
  DispatchResult handle(const Submit& input);
  DispatchResult handle(const PeerMessage& input);
  DispatchResult handle(const TimerFired& input);
  DispatchResult handle(const Completed& input);
  ReplicaId id_;
  std::map<InstanceId, InstanceRecord> instances_;
  std::uint64_t next_timer_ = 1;
  bool active_ = true;
};

}  // namespace egalis::protocol
