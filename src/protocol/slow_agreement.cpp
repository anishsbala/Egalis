#include "egalis/protocol/replica.hpp"

#include <type_traits>

namespace egalis::protocol {

DispatchResult Replica::handle(const SlowPathReady& input) {
  const auto found = instances_.find(input.instance);
  if (input.instance.coordinator() != id_ || found == instances_.end())
    return {DispatchStatus::stale, {}};
  auto& record = found->second;
  if (record.initial_selection != InitialSelection::slow || record.slow_proposal ||
      !initial_observation_allowed(record.state) || record.state.ballots.current != input.ballot ||
      !record.preaccept_observations || *record.preaccept_observations != input.observations)
    return {DispatchStatus::stale, {}};

  SlowProposal proposal{input.ballot, record.state.command, {}};
  for (const auto& [sender, dependencies] : input.observations) {
    (void)sender;
    proposal.dependencies.insert(dependencies.begin(), dependencies.end());
  }
  DispatchResult result{DispatchStatus::processed, {}};
  result.effects.reserve(Configuration::replicas);
  const auto send = [&](ReplicaId destination) {
    result.effects.emplace_back(std::in_place_type<SendMessage>, id_, destination,
        std::in_place_type<Accept>, proposal.ballot, input.instance,
        proposal.command, proposal.dependencies);
  };
  send(id_);
  for (const auto member : members()) if (member != id_) send(member);
  // All allocation precedes publication. Self-Accept has not happened yet;
  // command/dependencies in InstanceState still describe our preaccept vote.
  static_assert(std::is_nothrow_move_constructible_v<SlowProposal>);
  record.slow_proposal.emplace(std::move(proposal));
  return result;
}

}  // namespace egalis::protocol
