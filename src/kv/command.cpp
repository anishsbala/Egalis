#include "egalis/kv/command.hpp"

#include <string_view>
#include <type_traits>

namespace egalis::kv {
namespace {
std::string_view key(const Command& command) {
  return std::visit([](const auto& operation) -> std::string_view { return operation.key; }, command);
}
}  // namespace

bool conflicts(const Command& left, const Command& right) {
  return key(left) == key(right) &&
         !(std::holds_alternative<Get>(left) && std::holds_alternative<Get>(right));
}

bool conflicts(const Payload& left, const Payload& right) {
  if (std::holds_alternative<std::monostate>(right)) return false;
  if (std::holds_alternative<Nop>(right))
    return !std::holds_alternative<std::monostate>(left);
  return conflicts(left, std::get<Command>(right));
}

bool conflicts(const Payload& left, const Command& right) {
  if (const auto* command = std::get_if<Command>(&left)) return conflicts(*command, right);
  return std::holds_alternative<Nop>(left);
}

std::optional<std::string> StateMachine::read(const std::string& key) const {
  const auto it = values_.find(key);
  return it == values_.end() ? std::nullopt : std::optional{it->second};
}

Result StateMachine::apply(const Command& command) {
  return std::visit([this](const auto& operation) -> Result {
    using T = std::decay_t<decltype(operation)>;
    if constexpr (std::is_same_v<T, Get>) {
      return GetResult{read(operation.key)};
    } else if constexpr (std::is_same_v<T, Put>) {
      values_.insert_or_assign(operation.key, operation.value);
      return PutResult{};
    } else {
      const auto observed = read(operation.key);
      const bool exchanged = observed == operation.expected;
      if (exchanged) {
        values_.insert_or_assign(operation.key, operation.desired);
      }
      return CasResult{exchanged, observed};
    }
  }, command);
}

}  // namespace egalis::kv
