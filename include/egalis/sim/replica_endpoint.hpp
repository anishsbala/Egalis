#pragma once

#include "egalis/protocol/replica.hpp"
#include "egalis/sim/network.hpp"

#include <map>
#include <optional>
#include <vector>

namespace egalis::sim {

// Constructed only through attach: Network owns both this endpoint and its queued
// callbacks. A runner cannot be destroyed while the network retains its timers.
class ReplicaEndpoint final : public transport::Endpoint {
 public:
  static ReplicaEndpoint& attach(Network& network, protocol::ReplicaId id,
                                 std::size_t effect_budget = 65536);
  void receive(const protocol::Envelope& envelope) override;
  void on_crash() override;
  void on_restart() override;
  void dispatch(protocol::Event event);
  void arm_timer(protocol::InstanceId instance, protocol::TimerDelay delay);
  // Explicit runner entry point, also usable to test transport effects without
  // inventing consensus handlers. Effects must originate from this replica.
  void execute(protocol::Effects effects);
  [[nodiscard]] const protocol::Replica& core() const noexcept { return core_; }
  [[nodiscard]] std::optional<protocol::DispatchStatus> last_status() const noexcept {
    return last_status_;
  }
  [[nodiscard]] std::uint64_t dispatch_count() const noexcept { return dispatch_count_; }
 private:
  ReplicaEndpoint(Network& network, protocol::ReplicaId id, std::size_t effect_budget)
      : network_(network), core_(id), effect_budget_(effect_budget) {}
  struct Frame { protocol::Effects effects; std::size_t next = 0; };
  void dispatch_one(const protocol::Event& event);
  void pump();
  void apply(const protocol::Effect& effect);
  void cancel(protocol::TimerToken token);
  Network& network_;
  protocol::Replica core_;
  std::map<protocol::TimerToken, EventId> timers_;
  std::vector<Frame> frames_;
  std::optional<protocol::DispatchStatus> last_status_;
  std::uint64_t dispatch_count_ = 0;
  std::size_t effect_budget_;
  bool pumping_ = false;
  bool failed_ = false;
};

}  // namespace egalis::sim
