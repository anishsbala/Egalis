#pragma once

#include "egalis/protocol/messages.hpp"

namespace egalis::protocol {
// Figure 3 line 5, deliberately not an ordinal comparison of phases. Exposed
// separately so all advanced ballot/phase fixtures can test the exact predicate
// before later milestones provide transitions into those states.
[[nodiscard]] bool initial_preaccept_allowed(const InstanceState& state) noexcept;
}  // namespace egalis::protocol
