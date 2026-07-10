#pragma once

#include "egalis/protocol/messages.hpp"

namespace egalis::protocol {
// Figure 3 line 5, deliberately not an ordinal comparison of phases. Exposed
// separately so all advanced ballot/phase fixtures can test the exact predicate
// before later milestones provide transitions into those states.
[[nodiscard]] bool initial_preaccept_allowed(const InstanceState& state) noexcept;
// A response replay is equivalent to duplicating the original reply on a link,
// not executing Figure 3's transition again. No reply after phase/ballot advance.
[[nodiscard]] bool preaccept_reply_replayable(const InstanceState& state,
                                             const PreAccept& request);
}  // namespace egalis::protocol
