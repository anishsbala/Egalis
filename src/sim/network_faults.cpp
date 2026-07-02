#include "egalis/sim/network.hpp"

#include <bitset>
#include <limits>
#include <stdexcept>
#include <utility>

namespace egalis::sim {

void Network::set_link(protocol::ReplicaId from, protocol::ReplicaId to, bool blocked) {
  healthy();
  if (from == to) throw std::invalid_argument("cannot partition self-delivery");
  trace_.record({DecisionKind::link, {scheduler_.now().ticks, from.value(), to.value(), blocked}, {}});
  blocked_[from.value()][to.value()] = blocked;
}

void Network::partition(std::span<const protocol::ReplicaId> left,
                        std::span<const protocol::ReplicaId> right, bool blocked) {
  healthy();
  std::bitset<protocol::Configuration::replicas> l, r;
  for (const auto member : left) {
    if (l.test(member.value())) throw std::invalid_argument("duplicate partition member");
    l.set(member.value());
  }
  for (const auto member : right) {
    if (r.test(member.value()) || l.test(member.value())) {
      throw std::invalid_argument("duplicate or overlapping partition member");
    }
    r.set(member.value());
  }
  // Canonical order, independent of caller group ordering; other links untouched.
  for (const auto from : protocol::members()) for (const auto to : protocol::members()) {
    if ((l.test(from.value()) && r.test(to.value())) ||
        (r.test(from.value()) && l.test(to.value()))) set_link(from, to, blocked);
  }
}

void Network::crash(protocol::ReplicaId replica) {
  auto& slot = live(replica);
  if (slot.generation == std::numeric_limits<std::uint64_t>::max()) {
    throw std::overflow_error("endpoint generation exhausted");
  }
  trace_.record({DecisionKind::crashed, {scheduler_.now().ticks, replica.value(), slot.generation}, {}});
  slot.crashed = true;
  ++slot.generation;
}

void Network::restart(protocol::ReplicaId replica) {
  healthy();
  auto& slot = endpoints_[replica.value()];
  if (!slot.endpoint || !slot.crashed) throw std::logic_error("only crashed endpoints can restart");
  trace_.record({DecisionKind::restarted, {scheduler_.now().ticks, replica.value(), slot.generation}, {}});
  // Reactivates the test endpoint. Durable protocol reconstruction is milestone 27.
  slot.crashed = false;
}

EventId Network::timer(protocol::ReplicaId replica, Duration delay, std::string label,
                       Scheduler::Callback callback) {
  const auto generation = live(replica).generation;
  if (!callback) throw std::invalid_argument("timer has no callback");
  return scheduler_.schedule_after(delay, std::move(label),
      [this, replica, generation, callback = std::move(callback)] {
        const auto& slot = endpoints_[replica.value()];
        const bool active = !slot.crashed && slot.generation == generation;
        trace_.record({DecisionKind::timer, {scheduler_.now().ticks, replica.value(), generation, active}, {}});
        if (active) callback();
      });
}

}  // namespace egalis::sim
