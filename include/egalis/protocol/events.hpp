#pragma once

#include "egalis/protocol/messages.hpp"

#include <cstdint>
#include <variant>
#include <vector>

namespace egalis::protocol {

// Protocol tokens do not alias scheduler IDs or transport message IDs. Zero is
// reserved as invalid; allocation and outstanding-token ownership belong to Replica.
struct TimerToken {
  std::uint64_t value;
  auto operator<=>(const TimerToken&) const = default;
};
struct CompletionToken {
  std::uint64_t value;
  auto operator<=>(const CompletionToken&) const = default;
};
// Abstract runner ticks, not a wall-clock deadline or a simulator event timestamp.
struct TimerDelay {
  std::uint64_t ticks;
  auto operator<=>(const TimerDelay&) const = default;
};
struct TimerGuard {
  InstanceId instance;
  Ballot ballot;
  TimerToken token;
  bool operator==(const TimerGuard&) const = default;
};
struct Submit {
  InstanceId instance;
  kv::Command command;
  bool operator==(const Submit&) const = default;
};
// Normal client input. Submit above supports explicitly preallocated local IDs;
// both forms take the same coordinator path, never bypassing PreAccept.
struct SubmitCommand {
  kv::Command command;
  bool operator==(const SubmitCommand&) const = default;
};
struct PeerMessage {
  Envelope envelope;
  bool operator==(const PeerMessage&) const = default;
};
struct TimerFired {
  TimerGuard guard;
  bool operator==(const TimerFired&) const = default;
};
enum class CompletionKind { persistence, application };
enum class CompletionStatus { succeeded, failed };
struct Completed {
  InstanceId instance;
  Ballot ballot;
  CompletionToken token;
  CompletionKind kind;
  CompletionStatus status;
  bool operator==(const Completed&) const = default;
};
using Event = std::variant<Submit, PeerMessage, TimerFired, Completed, SubmitCommand>;

struct SendMessage {
  Envelope envelope;
  bool operator==(const SendMessage&) const = default;
};
struct ScheduleTimer {
  TimerGuard guard;
  TimerDelay delay;
  bool operator==(const ScheduleTimer&) const = default;
};
struct CancelTimer {
  TimerToken token;
  bool operator==(const CancelTimer&) const = default;
};
// Registration is NOT a commit, application result, or successful client reply.
struct SubmissionRegistered {
  InstanceId instance;
  bool operator==(const SubmissionRegistered&) const = default;
};
using Effect = std::variant<SendMessage, ScheduleTimer, CancelTimer, SubmissionRegistered>;
using Effects = std::vector<Effect>;

// Structural checks are usable by both real and simulated runners. Validation
// does not claim that a completion corresponds to an outstanding operation.
void validate(const Event& event);
void validate(const Effect& effect);

}  // namespace egalis::protocol
