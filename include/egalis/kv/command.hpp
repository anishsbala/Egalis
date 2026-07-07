#pragma once

#include <map>
#include <optional>
#include <string>
#include <variant>

namespace egalis::kv {

struct Get {
  std::string key;
  bool operator==(const Get&) const = default;
};
struct Put {
  std::string key;
  std::string value;
  bool operator==(const Put&) const = default;
};
struct CompareAndSwap {
  std::string key;
  std::optional<std::string> expected;  // nullopt means the key must be absent.
  std::string desired;
  bool operator==(const CompareAndSwap&) const = default;
};
using Command = std::variant<Get, Put, CompareAndSwap>;

struct Nop { bool operator==(const Nop&) const = default; };
// monostate is the paper's bottom payload, NOT a decided Nop.
using Payload = std::variant<std::monostate, Nop, Command>;

[[nodiscard]] bool conflicts(const Command& left, const Command& right);
[[nodiscard]] bool conflicts(const Payload& left, const Payload& right);
// Avoid copying a command into a Payload while scanning known protocol records.
[[nodiscard]] bool conflicts(const Payload& left, const Command& right);

struct GetResult {
  std::optional<std::string> value;
  bool operator==(const GetResult&) const = default;
};
struct PutResult { bool operator==(const PutResult&) const = default; };
struct CasResult {
  bool exchanged;
  std::optional<std::string> observed;  // Value before this operation, on both paths.
  bool operator==(const CasResult&) const = default;
};
using Result = std::variant<GetResult, PutResult, CasResult>;

// The deterministic application state machine. Replication/execution gating is
// deliberately outside this class; it can also serve as the history-check oracle.
class StateMachine {
 public:
  [[nodiscard]] std::optional<std::string> read(const std::string& key) const;
  Result apply(const Command& command);
  bool operator==(const StateMachine&) const = default;
 private:
  std::map<std::string, std::string> values_;
};

}  // namespace egalis::kv
