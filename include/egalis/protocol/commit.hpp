#pragma once

#include "egalis/protocol/messages.hpp"

namespace egalis::protocol {

// A missing instance has the default current ballot (zero).
[[nodiscard]] bool commit_allowed(const InstanceState& state, Ballot ballot) noexcept;

}  // namespace egalis::protocol
