#pragma once

#include "egalis/sim/scheduler.hpp"
#include "egalis/sim/trace.hpp"
#include "egalis/transport/transport.hpp"

#include <array>
#include <memory>
#include <span>

namespace egalis::sim {

struct FaultPolicy {
  Duration minimum_delay{1};
  Duration maximum_delay{1};
  std::uint32_t drop_per_million = 0;
  std::uint32_t duplicate_per_million = 0;
  Duration duplicate_spacing{0};
};
struct DeliveryPlan {
  bool drop = false;
  Duration delay{1};
  bool duplicate = false;
  Duration duplicate_delay{1};  // Delay from send, not from the first copy.
};

class Network final : public transport::Transport {
 public:
  explicit Network(Duration delay = Duration{1});
  Network(std::uint64_t seed, FaultPolicy policy);
  Network(TraceData replay, FaultPolicy policy);
  Network(const Network&) = delete;
  Network& operator=(const Network&) = delete;
  Network(Network&&) = delete;
  Network& operator=(Network&&) = delete;

  void attach(protocol::ReplicaId replica, std::unique_ptr<transport::Endpoint> endpoint);
  transport::MessageId send(protocol::Envelope envelope) override;
  transport::MessageId send(protocol::Envelope envelope, DeliveryPlan plan);
  void set_link(protocol::ReplicaId from, protocol::ReplicaId to, bool blocked);
  void partition(std::span<const protocol::ReplicaId> left,
                 std::span<const protocol::ReplicaId> right, bool blocked = true);
  void crash(protocol::ReplicaId replica);
  void restart(protocol::ReplicaId replica);
  void protocol_decision(protocol::ReplicaId replica, std::string detail);
  EventId timer(protocol::ReplicaId replica, Duration delay, std::string label,
                Scheduler::Callback callback);
  [[nodiscard]] const TraceData& trace() const noexcept { return trace_.data(); }
  void verify_replay_complete() const {
    healthy();
    if (scheduler_.pending() != 0) throw std::logic_error("simulation still has pending events");
    trace_.verify_complete();
  }
  Scheduler& scheduler() noexcept { return scheduler_; }
 private:
  struct Slot {
    std::unique_ptr<transport::Endpoint> endpoint;
    std::uint64_t generation = 0;
    bool crashed = false;
  };
  void configure();
  void healthy() const;
  transport::MessageId begin_send(const protocol::Envelope& envelope);
  void enqueue(transport::MessageId id, protocol::Envelope envelope, DeliveryPlan plan);
  void deliver(transport::MessageId id, const protocol::Envelope& envelope,
               std::uint64_t generation, unsigned copy);
  void discard(transport::MessageId id, std::string reason, unsigned copy = 0);
  Slot& live(protocol::ReplicaId replica);
  Trace trace_;
  Random random_;
  FaultPolicy policy_;
  std::array<Slot, protocol::Configuration::replicas> endpoints_;
  std::array<std::array<bool, protocol::Configuration::replicas>, protocol::Configuration::replicas> blocked_{};
  std::uint64_t next_message_ = 1;
  // Owned scheduler is destroyed before endpoints. Pending closures cannot outlive
  // the nonmovable network they reference.
  Scheduler scheduler_;
};

}  // namespace egalis::sim
