#include "egalis/protocol/replica.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

namespace egalis::protocol {

void Replica::require_active() const {
  if (!active_) throw std::logic_error("replica is suspended");
}
DispatchResult Replica::dispatch(const Event& event) {
  require_active();
  validate(event);
  return std::visit([this](const auto& input) { return handle(input); }, event);
}
DispatchResult Replica::handle(const Submit& input) {
  if (input.instance.coordinator() != id_)
    throw std::invalid_argument("submission instance is not locally owned");
  if (instances_.contains(input.instance))
    throw std::invalid_argument("submission reuses an instance ID");
  // Reserve a local proposal, NOT a preaccept vote. initCmd/initDep and phase
  // changes are the responsibility of the later baseline transition handlers.
  InstanceRecord record;
  record.state.command = input.command;
  DispatchResult result{DispatchStatus::processed, {SubmissionRegistered{input.instance}}};
  instances_.emplace(input.instance, std::move(record));
  return result;
}
DispatchResult Replica::handle(const PeerMessage& input) {
  if (input.envelope.destination != id_)
    throw std::invalid_argument("message delivered to wrong replica");
  // No missing record is materialized by an unsupported message, and no
  // reply implies that a vote was taken. Agreement starts at milestone 9.
  return {DispatchStatus::unsupported, {}};
}
DispatchResult Replica::handle(const TimerFired& input) {
  const auto found = instances_.find(input.guard.instance);
  if (found == instances_.end() || found->second.timer != input.guard ||
      found->second.state.ballots.current != input.guard.ballot)
    return {DispatchStatus::stale, {}};
  found->second.timer.reset();
  // Consume once, but do not invent a grace/recovery handler before its milestone.
  return {DispatchStatus::unsupported, {}};
}
DispatchResult Replica::handle(const Completed&) {
  // No persistence/application operation can be issued yet. Unsolicited (even
  // failed) completions cannot change state or resurrect a continuation.
  return {DispatchStatus::stale, {}};
}
Effects Replica::arm_timer(InstanceId instance, TimerDelay delay) {
  require_active();
  auto found = instances_.find(instance);
  if (found == instances_.end()) throw std::invalid_argument("timer instance is unknown");
  if (next_timer_ == std::numeric_limits<std::uint64_t>::max())
    throw std::overflow_error("timer tokens exhausted");
  const TimerGuard guard{instance, found->second.state.ballots.current, {next_timer_}};
  Effects effects;
  effects.reserve(2);
  if (found->second.timer) effects.push_back(CancelTimer{found->second.timer->token});
  effects.push_back(ScheduleTimer{guard, delay});
  found->second.timer = guard;
  ++next_timer_;
  return effects;
}
Effects Replica::suspend() {
  require_active();
  Effects effects;
  effects.reserve(instances_.size());
  for (const auto& [id, record] : instances_) {
    (void)id;
    if (record.timer) effects.push_back(CancelTimer{record.timer->token});
  }
  for (auto& [id, record] : instances_) { (void)id; record.timer.reset(); }
  active_ = false;
  return effects;
}
void Replica::resume() {
  if (active_) throw std::logic_error("replica is already active");
  active_ = true;
}
}  // namespace egalis::protocol
