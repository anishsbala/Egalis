#pragma once

#include "egalis/sim/scheduler.hpp"
#include "egalis/transport/transport.hpp"

#include <array>
#include <memory>

namespace egalis::sim {

class Network final : public transport::Transport {
 public:
  explicit Network(Duration delay = Duration{1});
  Network(const Network&) = delete;
  Network& operator=(const Network&) = delete;
  Network(Network&&) = delete;
  Network& operator=(Network&&) = delete;

  void attach(protocol::ReplicaId replica, std::unique_ptr<transport::Endpoint> endpoint);
  transport::MessageId send(protocol::Envelope envelope) override;
  Scheduler& scheduler() noexcept { return scheduler_; }
 private:
  std::array<std::unique_ptr<transport::Endpoint>, protocol::Configuration::replicas> endpoints_;
  Duration delay_;
  std::uint64_t next_message_ = 1;
  // Owned scheduler is destroyed before endpoints. Pending closures cannot outlive
  // the nonmovable network they reference.
  Scheduler scheduler_;
};

}  // namespace egalis::sim
