#pragma once

#include "egalis/kv/command.hpp"
#include "egalis/protocol/types.hpp"

#include <optional>
#include <set>
#include <string>
#include <variant>

namespace egalis::protocol {

using Dependencies = std::set<InstanceId>;

struct InstanceState {
  BallotState ballots;
  Phase phase = Phase::initial;
  kv::Payload command;
  Dependencies dependencies;
  std::optional<kv::Command> initial_command;
  Dependencies initial_dependencies;
};

struct PreAccept {
  InstanceId id;
  kv::Command command;
  Dependencies dependencies;
  bool operator==(const PreAccept&) const = default;
};
struct PreAcceptOk {
  InstanceId id;
  Dependencies dependencies;
  bool operator==(const PreAcceptOk&) const = default;
};
struct Accept {
  Ballot ballot;
  InstanceId id;
  kv::Payload command;
  Dependencies dependencies;
  bool operator==(const Accept&) const = default;
};
struct AcceptOk {
  Ballot ballot;
  InstanceId id;
  bool operator==(const AcceptOk&) const = default;
};
struct Commit {
  Ballot ballot;
  InstanceId id;
  kv::Payload command;
  Dependencies dependencies;
  bool operator==(const Commit&) const = default;
};
struct Recover {
  Ballot ballot;
  InstanceId id;
  bool operator==(const Recover&) const = default;
};
struct RecoverOk {
  Ballot ballot;
  InstanceId id;
  Ballot accepted_ballot;
  kv::Payload command;
  Dependencies dependencies;
  Dependencies initial_dependencies;
  Phase phase;
  bool operator==(const RecoverOk&) const = default;
};
struct Validate {
  Ballot ballot;
  InstanceId id;
  kv::Command command;
  Dependencies dependencies;
  bool operator==(const Validate&) const = default;
};
struct Invalidator {
  InstanceId id;
  Phase phase;
  auto operator<=>(const Invalidator&) const = default;
};
struct ValidateOk {
  Ballot ballot;
  InstanceId id;
  std::set<Invalidator> invalidators;
  bool operator==(const ValidateOk&) const = default;
};
// Baseline Waiting has neither a ballot nor Section 4's R_max size.
struct Waiting {
  InstanceId id;
  bool operator==(const Waiting&) const = default;
};
struct TryRecover {
  InstanceId id;
  bool operator==(const TryRecover&) const = default;
};
using Message = std::variant<PreAccept, PreAcceptOk, Accept, AcceptOk, Commit,
                             Recover, RecoverOk, Validate, ValidateOk, Waiting, TryRecover>;
struct Envelope {
  ReplicaId source;
  ReplicaId destination;
  Message message;
  bool operator==(const Envelope&) const = default;
};

// Structural validation only. Receiver-state guards belong to the shared core.
void validate(const Envelope& envelope);
// Version 1 canonical representation for trace comparison, not a network codec.
[[nodiscard]] std::string canonical(const Envelope& envelope);

}  // namespace egalis::protocol
