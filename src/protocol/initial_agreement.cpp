#include "egalis/protocol/initial_agreement.hpp"
#include "egalis/protocol/replica.hpp"

#include <algorithm>

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
  // Dispatch has checked delivery to this replica and to the instance's initial owner.
  const auto found = instances_.find(message.id);
  if (found == instances_.end()) return {DispatchStatus::stale, {}};
  auto& record = found->second;
  const auto& state = record.state;
  if (!record.preaccept_observations || record.initial_quorum_selected ||
      !initial_observation_allowed(state))
    return {DispatchStatus::stale, {}};

  auto& observations = *record.preaccept_observations;
  if (const auto first = observations.find(source); first != observations.end()) {
    return {first->second == message.dependencies ? DispatchStatus::replayed
                                                  : DispatchStatus::rejected, {}};
  }

  const bool fast = observations.size() + 1 >= Configuration::fast_quorum &&
      message.dependencies == state.initial_dependencies &&
      observations_match_initial_dependencies(observations, state);
  DispatchResult result{DispatchStatus::processed, {}};
  if (fast) {
    result.effects.reserve(Configuration::replicas);
    const auto send = [&](ReplicaId destination) {
      result.effects.emplace_back(std::in_place_type<SendMessage>, id_, destination,
          std::in_place_type<Commit>, Ballot{}, message.id, state.command,
          state.initial_dependencies);
    };
    send(id_);
    for (const auto member : members()) if (member != id_) send(member);
  }
  // Complete the fallible effect construction before recording the deciding vote.
  // Seal the quorum before the runner can deliver any Commit, including self.
  observations.emplace(source, message.dependencies);
  record.initial_quorum_selected = fast;
  return result;
}

}  // namespace egalis::protocol
