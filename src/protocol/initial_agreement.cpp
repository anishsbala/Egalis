#include "egalis/protocol/initial_agreement.hpp"
#include "egalis/protocol/replica.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace egalis::protocol {

bool initial_observation_allowed(const InstanceState& state) noexcept {
  return state.ballots.current.initial() && state.phase == Phase::preaccepted;
}

bool observations_match_initial_dependencies(const PreAcceptObservations& observations,
                                             const InstanceState& state) {
  return std::ranges::all_of(observations, [&](const auto& observation) {
    return observation.second == state.initial_dependencies;
  });
}

DispatchResult Replica::receive_preaccept_ok(ReplicaId source, const PreAcceptOk& message) {
  const auto found = instances_.find(message.id);
  if (found == instances_.end()) return {DispatchStatus::stale, {}};
  auto& record = found->second;
  if (!record.preaccept_observations || record.initial_selection != InitialSelection::collecting ||
      !initial_observation_allowed(record.state))
    return {DispatchStatus::stale, {}};
  const auto& observations = *record.preaccept_observations;
  if (const auto first = observations.find(source); first != observations.end()) {
    return {first->second == message.dependencies ? DispatchStatus::replayed
                                                  : DispatchStatus::rejected, {}};
  }
  auto next = observations;
  next.emplace(source, message.dependencies);
  return select_initial(message.id, std::move(next));
}

DispatchResult Replica::select_initial(InstanceId instance, PreAcceptObservations observations,
                                       bool grace_expired) {
  auto& record = instances_.at(instance);
  const auto& state = record.state;
  const bool matching = observations_match_initial_dependencies(observations, state);
  const bool fast = matching && observations.size() >= Configuration::fast_quorum && !grace_expired;
  const bool slow = observations.size() >= Configuration::slow_quorum && (!matching || grace_expired);
  DispatchResult result{DispatchStatus::processed, {}};
  auto grace = record.initial_grace;
  if (fast || slow) {
    result.effects.reserve(1 + Configuration::replicas);
    if (grace && !grace_expired)
      result.effects.emplace_back(std::in_place_type<CancelTimer>, CancelTimer{grace->token});
    if (slow) {
      result.effects.emplace_back(std::in_place_type<SlowPathReady>,
                                  SlowPathReady{instance, state.ballots.current, observations});
    } else {
      const auto send = [&](ReplicaId destination) {
        result.effects.emplace_back(std::in_place_type<SendMessage>, id_, destination,
            std::in_place_type<Commit>, Ballot{}, instance, state.command,
            state.initial_dependencies);
      };
      send(id_);
      for (const auto member : members()) if (member != id_) send(member);
    }
    grace.reset();
  } else if (observations.size() >= Configuration::slow_quorum && !grace) {
    if (next_timer_ == std::numeric_limits<std::uint64_t>::max())
      throw std::overflow_error("timer tokens exhausted");
    grace = TimerGuard{instance, state.ballots.current, {next_timer_}};
    result.effects.emplace_back(std::in_place_type<ScheduleTimer>, ScheduleTimer{*grace, {1}});
  }
  // Copies/effects above may throw. Publish the vote and selection atomically
  // before any self message or local handoff can be executed by the runner.
  if (grace && !record.initial_grace) ++next_timer_;
  record.preaccept_observations = std::move(observations);
  record.initial_selection = fast ? InitialSelection::fast
      : slow ? InitialSelection::slow : InitialSelection::collecting;
  record.initial_grace = grace;
  return result;
}

}  // namespace egalis::protocol
