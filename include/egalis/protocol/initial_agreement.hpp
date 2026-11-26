#pragma once

#include "egalis/protocol/messages.hpp"

#include <map>

namespace egalis::protocol {

using PreAcceptObservations = std::map<ReplicaId, Dependencies>;

[[nodiscard]] bool initial_observation_allowed(const InstanceState& state) noexcept;
// Compare values with the frozen proposal. A caller checks quorum size separately.
[[nodiscard]] bool observations_match_initial_dependencies(
    const PreAcceptObservations& observations, const InstanceState& state);

}  // namespace egalis::protocol
