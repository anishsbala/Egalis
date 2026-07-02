#include "egalis/sim/replica_endpoint.hpp"

#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace egalis::sim {
namespace {
using namespace protocol;
std::string guard_text(const TimerGuard& guard) {
  return std::to_string(guard.instance.number()) + "/" +
         std::to_string(guard.ballot.number()) + "/" + std::to_string(guard.token.value);
}
std::string fingerprint(const Event& event) {
  return std::visit([](const auto& input) -> std::string {
    using T = std::decay_t<decltype(input)>;
    if constexpr (std::is_same_v<T, Submit>) {
      // Reuse command byte framing, not a wire message or simulated preaccept.
      return "submit/" + canonical(Envelope{input.instance.coordinator(),
          input.instance.coordinator(), PreAccept{input.instance, input.command, {}}});
    } else if constexpr (std::is_same_v<T, PeerMessage>) {
      return "peer/" + canonical(input.envelope);
    } else if constexpr (std::is_same_v<T, TimerFired>) {
      return "timer/" + guard_text(input.guard);
    } else {
      return "completion/" + std::to_string(input.instance.number()) + "/" +
          std::to_string(input.ballot.number()) + "/" + std::to_string(input.token.value) +
          "/" + std::to_string(static_cast<unsigned>(input.kind)) + "/" +
          std::to_string(static_cast<unsigned>(input.status));
    }
  }, event);
}
std::string fingerprint(const Effect& effect) {
  return std::visit([](const auto& output) -> std::string {
    using T = std::decay_t<decltype(output)>;
    if constexpr (std::is_same_v<T, SendMessage>) return "send/" + canonical(output.envelope);
    else if constexpr (std::is_same_v<T, ScheduleTimer>)
      return "schedule/" + guard_text(output.guard) + "/" + std::to_string(output.delay.ticks);
    else if constexpr (std::is_same_v<T, CancelTimer>)
      return "cancel/" + std::to_string(output.token.value);
    else return "registered/" + std::to_string(output.instance.number());
  }, effect);
}
}

ReplicaEndpoint& ReplicaEndpoint::attach(Network& network, protocol::ReplicaId id,
                                         std::size_t effect_budget) {
  if (effect_budget == 0) throw std::invalid_argument("zero effect budget");
  auto endpoint = std::unique_ptr<ReplicaEndpoint>(new ReplicaEndpoint(network, id, effect_budget));
  auto& reference = *endpoint;
  network.attach(id, std::move(endpoint));
  try { network.protocol_decision(id, "runner/1/" + std::to_string(effect_budget)); }
  catch (...) { network.scheduler().fail(); throw; }
  return reference;
}
void ReplicaEndpoint::receive(const protocol::Envelope& envelope) {
  // Only immediate self-delivery can occur inside the effect pump. It dispatches
  // after the preceding core call returned, then pushes consequences above the
  // parent frame. No recursive core mutation or recursive effect pumping occurs.
  if (pumping_ && envelope.source != core_.id())
    throw std::logic_error("external delivery reentered effect execution");
  dispatch_one(protocol::PeerMessage{envelope});
  if (!pumping_) pump();
}
void ReplicaEndpoint::dispatch(protocol::Event event) {
  if (pumping_) throw std::logic_error("external input reentered effect execution");
  dispatch_one(event);
  pump();
}
void ReplicaEndpoint::dispatch_one(const protocol::Event& event) {
  if (failed_ || !core_.active()) throw std::logic_error("replica runner is inactive");
  protocol::validate(event);
  if (dispatch_count_ == std::numeric_limits<std::uint64_t>::max())
    throw std::overflow_error("dispatch counter exhausted");
  try {
    network_.protocol_decision(core_.id(), "input/" + fingerprint(event));
    auto result = core_.dispatch(event);
    last_status_ = result.status;
    ++dispatch_count_;
    network_.protocol_decision(core_.id(), "result/" +
        std::to_string(static_cast<unsigned>(result.status)) + "/" +
        std::to_string(result.effects.size()));
    if (!result.effects.empty()) frames_.push_back({std::move(result.effects), 0});
  } catch (...) { failed_ = true; network_.scheduler().fail(); throw; }
}
void ReplicaEndpoint::execute(protocol::Effects effects) {
  if (failed_ || !core_.active() || pumping_)
    throw std::logic_error("replica runner cannot accept effects");
  // Validate the whole batch before starting. Once execution starts, failures
  // are terminal; a partially executed batch cannot safely be retried.
  for (const auto& effect : effects) {
    protocol::validate(effect);
    if (const auto* send = std::get_if<protocol::SendMessage>(&effect)) {
      if (send->envelope.source != core_.id())
        throw std::invalid_argument("effect sends as another replica");
    }
  }
  frames_.push_back({std::move(effects), 0});
  pump();
}
void ReplicaEndpoint::pump() {
  if (failed_) throw std::logic_error("replica runner failed");
  pumping_ = true;
  std::size_t executed = 0;
  try {
    while (!frames_.empty()) {
      auto& frame = frames_.back();
      if (frame.next == frame.effects.size()) { frames_.pop_back(); continue; }
      if (executed == effect_budget_)
        throw std::runtime_error("replica effect budget exhausted; possible self-message feedback");
      ++executed;
      // Move before apply: synchronous self-delivery may grow/reallocate frames_.
      auto effect = std::move(frame.effects[frame.next++]);
      network_.protocol_decision(core_.id(), "effect/" + fingerprint(effect));
      apply(effect);
    }
    pumping_ = false;
  } catch (...) {
    pumping_ = false; failed_ = true; network_.scheduler().fail(); throw;
  }
}
void ReplicaEndpoint::cancel(protocol::TimerToken token) {
  const auto found = timers_.find(token);
  if (found == timers_.end()) return;
  network_.scheduler().cancel(found->second);
  timers_.erase(found);
}
void ReplicaEndpoint::apply(const protocol::Effect& effect) {
  std::visit([this](const auto& output) {
    using T = std::decay_t<decltype(output)>;
    if constexpr (std::is_same_v<T, protocol::SendMessage>) {
      network_.send(output.envelope);
    } else if constexpr (std::is_same_v<T, protocol::CancelTimer>) {
      cancel(output.token);
    } else if constexpr (std::is_same_v<T, protocol::ScheduleTimer>) {
      const auto found = core_.instances().find(output.guard.instance);
      if (found == core_.instances().end() || found->second.timer != output.guard ||
          found->second.state.ballots.current != output.guard.ballot) return;
      if (timers_.contains(output.guard.token))
        throw std::logic_error("timer effect executed twice");
      const auto guard = output.guard;
      const auto event = network_.timer(core_.id(), Duration{output.delay.ticks},
          "protocol-timer/" + guard_text(guard), [this, guard] {
            timers_.erase(guard.token);
            dispatch(protocol::TimerFired{guard});
          });
      timers_.emplace(guard.token, event);
    }
    // SubmissionRegistered is diagnostic only; never reply as if KV applied.
  }, effect);
}
void ReplicaEndpoint::arm_timer(protocol::InstanceId instance, protocol::TimerDelay delay) {
  if (failed_ || pumping_) throw std::logic_error("replica runner cannot arm timer");
  try { execute(core_.arm_timer(instance, delay)); }
  catch (...) { failed_ = true; network_.scheduler().fail(); throw; }
}
void ReplicaEndpoint::on_crash() {
  if (pumping_) throw std::logic_error("crash reentered effect execution");
  const auto effects = core_.suspend();
  for (const auto& effect : effects) cancel(std::get<protocol::CancelTimer>(effect).token);
  // Cancel even runner-only pending timers; never let callbacks outlive a lifetime.
  while (!timers_.empty()) cancel(timers_.begin()->first);
  frames_.clear();
}
void ReplicaEndpoint::on_restart() {
  if (failed_) throw std::logic_error("failed runner cannot restart");
  core_.resume();
}

}  // namespace egalis::sim
