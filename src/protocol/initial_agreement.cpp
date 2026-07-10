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
  if (!record.preaccept_observations || !initial_observation_allowed(record.state))
    return {DispatchStatus::stale, {}};

  const auto [first, inserted] =
      record.preaccept_observations->try_emplace(source, message.dependencies);
  if (inserted) return {DispatchStatus::processed, {}};
  if (first->second == message.dependencies) return {DispatchStatus::replayed, {}};
  return {DispatchStatus::rejected, {}};
}

}  // namespace egalis::protocol
