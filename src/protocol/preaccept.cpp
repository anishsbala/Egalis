#include "egalis/protocol/preaccept.hpp"
#include "egalis/protocol/replica.hpp"

#include <utility>
#include <type_traits>

namespace egalis::protocol {
static_assert(std::is_nothrow_move_assignable_v<InstanceRecord>);
bool initial_preaccept_allowed(const InstanceState& state) noexcept {
  return state.ballots.current.initial() && state.phase == Phase::initial;
}

DispatchResult Replica::receive_preaccept(ReplicaId source, const PreAccept& message) {
  const auto found = instances_.find(message.id);
  if (found != instances_.end()) {
    const auto& state = found->second.state;
    if (!initial_preaccept_allowed(state)) return {DispatchStatus::stale, {}};
    // Local submission already froze its self-request. Reject an inconsistent
    // delivery without overwriting the initial proposal or consuming another ID.
    if (state.initial_command && (*state.initial_command != message.command ||
                                 state.initial_dependencies != message.dependencies))
      return {DispatchStatus::rejected, {}};
  }
  const auto expanded = expand_dependencies(message.id, message.command, message.dependencies);
  InstanceRecord record = found == instances_.end() ? InstanceRecord{} : found->second;
  record.state.command = message.command;
  record.state.initial_command = message.command;
  record.state.initial_dependencies = message.dependencies;
  record.state.dependencies = expanded;
  record.state.phase = Phase::preaccepted;
  // All allocations, including the reply, precede mutation of the authoritative
  // record. The initial accepted ballot remains zero; this is not slow Accept.
  DispatchResult result{DispatchStatus::processed, {}};
  result.effects.emplace_back(std::in_place_type<SendMessage>,
      SendMessage{{id_, source, PreAcceptOk{message.id, expanded}}});
  if (found == instances_.end()) instances_.emplace(message.id, std::move(record));
  else found->second = std::move(record);
  if (message.id.coordinator() == id_) allocator_.observe(message.id);
  return result;
}
}  // namespace egalis::protocol
