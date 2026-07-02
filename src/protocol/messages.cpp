#include "egalis/protocol/messages.hpp"

#include <stdexcept>
#include <type_traits>

namespace egalis::protocol {
namespace {
void require(bool condition, const char* reason) {
  if (!condition) throw std::invalid_argument(reason);
}
bool valid_phase(Phase phase) {
  return phase == Phase::initial || phase == Phase::preaccepted ||
         phase == Phase::accepted || phase == Phase::committed;
}
void dependencies(const Dependencies& set, InstanceId id) {
  require(!set.contains(id), "self dependency is invalid");
}

// Length framing preserves arbitrary byte strings and avoids ambiguous traces.
void number(std::string& out, std::uint64_t value) { out += std::to_string(value) + ":"; }
void bytes(std::string& out, const std::string& value) {
  number(out, value.size());
  out += value;
}
void encode(std::string& out, const kv::Command& command) {
  number(out, command.index());
  std::visit([&](const auto& op) {
    using T = std::decay_t<decltype(op)>;
    bytes(out, op.key);
    if constexpr (std::is_same_v<T, kv::Put>) bytes(out, op.value);
    if constexpr (std::is_same_v<T, kv::CompareAndSwap>) {
      number(out, op.expected.has_value());
      if (op.expected) bytes(out, *op.expected);
      bytes(out, op.desired);
    }
  }, command);
}
void encode(std::string& out, const kv::Payload& payload) {
  number(out, payload.index());
  if (const auto* command = std::get_if<kv::Command>(&payload)) encode(out, *command);
}
void encode(std::string& out, const Dependencies& set) {
  number(out, set.size());
  for (const auto& id : set) number(out, id.number());
}
}  // namespace

void validate(const Envelope& envelope) {
  std::visit([&](const auto& msg) {
    using T = std::decay_t<decltype(msg)>;
    if constexpr (requires { msg.dependencies; }) dependencies(msg.dependencies, msg.id);
    if constexpr (std::is_same_v<T, PreAccept>) {
      require(envelope.source == msg.id.coordinator(), "PreAccept must come from initial coordinator");
    }
    if constexpr (std::is_same_v<T, PreAcceptOk>) {
      require(envelope.destination == msg.id.coordinator(), "PreAcceptOK target is not coordinator");
    }
    if constexpr (std::is_same_v<T, Accept> || std::is_same_v<T, Commit> ||
                  std::is_same_v<T, Recover> || std::is_same_v<T, Validate>) {
      require(envelope.source == msg.ballot.owner(msg.id), "sender does not own ballot");
    }
    if constexpr (std::is_same_v<T, AcceptOk> || std::is_same_v<T, RecoverOk>) {
      require(envelope.destination == msg.ballot.owner(msg.id), "reply target does not own ballot");
    }
    if constexpr (std::is_same_v<T, Recover> || std::is_same_v<T, RecoverOk> ||
                  std::is_same_v<T, Validate> || std::is_same_v<T, ValidateOk>) {
      require(!msg.ballot.initial(), "recovery messages require a positive ballot");
    }
    if constexpr (std::is_same_v<T, Accept> || std::is_same_v<T, Commit>) {
      require(!std::holds_alternative<std::monostate>(msg.command), "proposal has absent payload");
      require(!msg.ballot.initial() || !std::holds_alternative<kv::Nop>(msg.command),
              "Nop is only proposed during recovery");
    }
    if constexpr (std::is_same_v<T, RecoverOk>) {
      dependencies(msg.initial_dependencies, msg.id);
      require(valid_phase(msg.phase), "invalid recovery phase");
      require(msg.accepted_ballot < msg.ballot, "RecoverOK must report an earlier accepted ballot");
      require(!std::holds_alternative<kv::Nop>(msg.command) || !msg.accepted_ballot.initial(),
              "Nop cannot have been accepted at ballot zero");
      if (msg.phase != Phase::initial) {
        require(!std::holds_alternative<std::monostate>(msg.command), "noninitial record lacks payload");
      }
      if (msg.phase == Phase::initial || msg.phase == Phase::preaccepted) {
        require(msg.accepted_ballot.initial(), "initial/preaccepted state has accepted recovery ballot");
        require(!std::holds_alternative<kv::Nop>(msg.command), "unaccepted record contains Nop");
      }
      // Validate can populate cmd/initDep while phase remains initial; do not
      // reject that state or falsely report it as a preaccept vote.
    }
    if constexpr (std::is_same_v<T, ValidateOk>) {
      for (const auto& entry : msg.invalidators) {
        require(entry.id != msg.id, "instance invalidates itself");
        require(valid_phase(entry.phase), "invalid invalidator phase");
      }
    }
  }, envelope.message);
}

std::string canonical(const Envelope& envelope) {
  validate(envelope);
  std::string out;
  number(out, 1);  // Canonical message schema version.
  number(out, envelope.source.value());
  number(out, envelope.destination.value());
  number(out, envelope.message.index());
  std::visit([&](const auto& msg) {
    number(out, msg.id.number());
    if constexpr (requires { msg.ballot; }) number(out, msg.ballot.number());
    if constexpr (requires { msg.accepted_ballot; }) number(out, msg.accepted_ballot.number());
    if constexpr (requires { msg.command; }) encode(out, msg.command);
    if constexpr (requires { msg.dependencies; }) encode(out, msg.dependencies);
    if constexpr (requires { msg.initial_dependencies; }) encode(out, msg.initial_dependencies);
    if constexpr (requires { msg.phase; }) number(out, static_cast<unsigned>(msg.phase));
    if constexpr (requires { msg.invalidators; }) {
      number(out, msg.invalidators.size());
      for (const auto& entry : msg.invalidators) {
        number(out, entry.id.number());
        number(out, static_cast<unsigned>(entry.phase));
      }
    }
  }, envelope.message);
  return out;
}

}  // namespace egalis::protocol
