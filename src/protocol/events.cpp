#include "egalis/protocol/events.hpp"

#include <stdexcept>
#include <type_traits>

namespace egalis::protocol {
namespace {
void token(std::uint64_t value) {
  if (value == 0) throw std::invalid_argument("zero continuation token");
}
}
void validate(const Event& event) {
  std::visit([](const auto& input) {
    using T = std::decay_t<decltype(input)>;
    if constexpr (std::is_same_v<T, PeerMessage>) validate(input.envelope);
    if constexpr (std::is_same_v<T, SlowPathReady>) validate(Effect{input});
    if constexpr (std::is_same_v<T, TimerFired>) token(input.guard.token.value);
    if constexpr (std::is_same_v<T, Completed>) {
      token(input.token.value);
      if (input.kind != CompletionKind::persistence && input.kind != CompletionKind::application)
        throw std::invalid_argument("unknown completion kind");
      if (input.status != CompletionStatus::succeeded && input.status != CompletionStatus::failed)
        throw std::invalid_argument("unknown completion status");
    }
  }, event);
}
void validate(const Effect& effect) {
  std::visit([](const auto& output) {
    using T = std::decay_t<decltype(output)>;
    if constexpr (std::is_same_v<T, SendMessage>) validate(output.envelope);
    if constexpr (std::is_same_v<T, ScheduleTimer>) token(output.guard.token.value);
    if constexpr (std::is_same_v<T, CancelTimer>) token(output.token.value);
    if constexpr (std::is_same_v<T, SlowPathReady>) {
      if (!output.ballot.initial() || output.observations.size() < Configuration::slow_quorum)
        throw std::invalid_argument("invalid initial slow handoff");
      for (const auto& [sender, dependencies] : output.observations)
        validate(Envelope{sender, output.instance.coordinator(),
                          PreAcceptOk{output.instance, dependencies}});
    }
  }, effect);
}
}  // namespace egalis::protocol
