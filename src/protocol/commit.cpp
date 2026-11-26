#include "egalis/protocol/commit.hpp"
#include "egalis/protocol/replica.hpp"

#include <type_traits>
#include <utility>

namespace egalis::protocol {

bool commit_allowed(const InstanceState& state, Ballot ballot) noexcept {
  return state.ballots.current == ballot;
}

DispatchResult Replica::receive_commit(const Commit& message) {
  const auto found = instances_.find(message.id);
  const InstanceState default_state;
  const auto& state = found == instances_.end() ? default_state : found->second.state;
  if (!commit_allowed(state, message.ballot)) return {DispatchStatus::stale, {}};
  if (found != instances_.end() && found->second.committed_decision &&
      (found->second.committed_decision->command != message.command ||
       found->second.committed_decision->dependencies != message.dependencies))
    return {DispatchStatus::rejected, {}};
  if (state.phase == Phase::committed) {
    if (state.command != message.command || state.dependencies != message.dependencies)
      return {DispatchStatus::rejected, {}};
    if (state.ballots.accepted == message.ballot) return {DispatchStatus::replayed, {}};
  }

  // Copy the decision before opening or modifying its record. The moves below
  // cannot throw, and the initial proposal and current ballot stay untouched.
  static_assert(std::is_nothrow_move_assignable_v<kv::Payload>);
  static_assert(std::is_nothrow_move_assignable_v<Dependencies>);
  static_assert(std::is_nothrow_move_constructible_v<CommittedDecision>);
  CommittedDecision decision{message.command, message.dependencies};
  auto command = message.command;
  auto dependencies = message.dependencies;
  DispatchResult result{DispatchStatus::processed, {}};
  if (found != instances_.end() && found->second.initial_grace)
    result.effects.emplace_back(std::in_place_type<CancelTimer>,
        CancelTimer{found->second.initial_grace->token});
  auto& target = found == instances_.end() ? instances_.try_emplace(message.id).first->second
                                           : found->second;
  target.committed_decision.emplace(std::move(decision));
  target.state.ballots.accepted = message.ballot;
  target.state.command = std::move(command);
  target.state.dependencies = std::move(dependencies);
  target.state.phase = Phase::committed;
  target.initial_grace.reset();
  if (message.id.coordinator() == id_) allocator_.observe(message.id);
  return result;
}

}  // namespace egalis::protocol
