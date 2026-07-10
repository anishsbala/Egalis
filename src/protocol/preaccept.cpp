#include "egalis/protocol/preaccept.hpp"
#include "egalis/protocol/replica.hpp"

#include <utility>
#include <type_traits>

namespace egalis::protocol {
static_assert(std::is_nothrow_move_assignable_v<InstanceRecord>);
bool initial_preaccept_allowed(const InstanceState& state) noexcept {
  return state.ballots.current.initial() && state.phase == Phase::initial;
}
bool preaccept_reply_replayable(const InstanceState& state, const PreAccept& request) {
  const auto* command = std::get_if<kv::Command>(&state.command);
  return state.phase == Phase::preaccepted && state.ballots.current.initial() &&
      state.initial_command && *state.initial_command == request.command &&
      state.initial_dependencies == request.dependencies && command && *command == request.command;
}

DispatchResult Replica::receive_preaccept(ReplicaId source, const PreAccept& message) {
  const auto found = instances_.find(message.id);
  const auto acknowledge = [&](const Dependencies& dependencies, DispatchStatus status) {
    DispatchResult result{status, {}};
    result.effects.emplace_back(std::in_place_type<SendMessage>, id_, source,
        std::in_place_type<PreAcceptOk>, message.id, dependencies);
    return result;
  };
  if (found != instances_.end()) {
    const auto& state = found->second.state;
    if (state.phase == Phase::preaccepted && state.ballots.current.initial()) {
      if (!preaccept_reply_replayable(state, message)) return {DispatchStatus::rejected, {}};
      // dep is frozen by the first PreAccept, not recomputed from new knowledge.
      // It is the response cache while phase=preaccepted and bal=0; later
      // handlers must advance phase/ballot before changing it. No second table.
      return acknowledge(state.dependencies, DispatchStatus::replayed);
    }
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
  auto result = acknowledge(expanded, DispatchStatus::processed);
  if (found == instances_.end()) instances_.emplace(message.id, std::move(record));
  else found->second = std::move(record);
  if (message.id.coordinator() == id_) allocator_.observe(message.id);
  return result;
}
}  // namespace egalis::protocol
