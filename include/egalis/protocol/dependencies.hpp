#pragma once

#include "egalis/protocol/replica.hpp"

namespace egalis::protocol {

// Read-only full-scan oracle over the authoritative table, not a derived index
// or a snapshot cache. Result owns its IDs and is ordered by InstanceId.
[[nodiscard]] Dependencies scan_known_dependencies(
    const std::map<InstanceId, InstanceRecord>& instances,
    InstanceId instance, const kv::Command& command);

}  // namespace egalis::protocol
