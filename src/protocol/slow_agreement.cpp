#include "egalis/protocol/replica.hpp"

#include <type_traits>

namespace egalis::protocol {

DispatchResult Replica::receive_accept(ReplicaId source, const Accept& message) {
  const auto found = instances_.find(message.id);
  const InstanceRecord empty;
  const auto& record = found == instances_.end() ? empty : found->second;
  const auto& state = record.state;
  // Figure 3 right 1: current ballot, never the accepted ballot or phase order.
  if (state.ballots.current > message.ballot ||
      (state.ballots.current == message.ballot && state.phase == Phase::committed))
    return {DispatchStatus::stale, {}};
  const auto matches = [&](const auto& value) {
    return value.command == message.command && value.dependencies == message.dependencies;
  };
  // Invariant diagnostics for contradictory trusted-sender traffic. Preaccepted
  // dependencies are not a frozen Accept value and may legitimately shrink.
  if ((record.committed_decision && !matches(*record.committed_decision)) ||
      (state.phase == Phase::accepted && state.ballots.accepted == message.ballot && !matches(state)) ||
      (record.slow_proposal && record.slow_proposal->ballot == message.ballot &&
       !matches(*record.slow_proposal)))
    return {DispatchStatus::rejected, {}};
  const bool replay = state.phase == Phase::accepted &&
      state.ballots.current == message.ballot && state.ballots.accepted == message.ballot;
  DispatchResult result{replay ? DispatchStatus::replayed : DispatchStatus::processed, {}};
  if (!replay) {
    if (record.timer) result.effects.emplace_back(std::in_place_type<CancelTimer>,
        CancelTimer{record.timer->token});
    if (record.initial_grace) result.effects.emplace_back(std::in_place_type<CancelTimer>,
        CancelTimer{record.initial_grace->token});
  }
  result.effects.emplace_back(std::in_place_type<SendMessage>, id_, source,
      std::in_place_type<AcceptOk>, message.ballot, message.id);
  if (replay) return result;
  // Stage all throwing copies and effects before publishing one authoritative record.
  InstanceRecord next = record;
  next.state.command = message.command;
  next.state.dependencies = message.dependencies;
  next.state.ballots.current = message.ballot;
  next.state.ballots.accepted = message.ballot;
  next.state.phase = Phase::accepted;
  next.timer.reset();
  next.initial_grace.reset();
  if (next.slow_proposal && next.slow_proposal->ballot != message.ballot)
    next.slow_proposal.reset();
  static_assert(std::is_nothrow_move_assignable_v<InstanceRecord>);
  if (found == instances_.end()) instances_.emplace(message.id, std::move(next));
  else found->second = std::move(next);
  if (message.id.coordinator() == id_) allocator_.observe(message.id);
  return result;
}

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
