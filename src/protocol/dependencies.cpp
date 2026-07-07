#include "egalis/protocol/dependencies.hpp"

#include <stdexcept>

namespace egalis::protocol {

Dependencies scan_known_dependencies(const std::map<InstanceId, InstanceRecord>& instances,
                                     InstanceId instance, const kv::Command& command) {
  Dependencies result;
  for (const auto& [known_id, record] : instances) {
    // EPaxos* v1 Figure 3, lines 3 and 9: scan current cmd, without a phase,
    // ballot, coordinator, or executed-record filter. initCmd is NOT a second
    // candidate here: after preaccept/validation, cmd stays c or Nop (Figure 9,
    // Invariant 6). Nop's universal conflicts retain visibility even for a
    // different query key; original metadata must remain intact for recovery.
    // Exclude the queried ID
    // even when it is already installed (the message layer forbids self edges).
    if (known_id != instance && kv::conflicts(record.state.command, command))
      result.insert(result.end(), known_id);
  }
  return result;
}

Dependencies Replica::known_dependencies(InstanceId instance, const kv::Command& command) const {
  return scan_known_dependencies(instances_, instance, command);
}

Dependencies expand_known_dependencies(const std::map<InstanceId, InstanceRecord>& instances,
    InstanceId instance, const kv::Command& command, const Dependencies& proposed) {
  if (proposed.contains(instance))
    throw std::invalid_argument("dependency proposal contains its own instance");
  auto result = scan_known_dependencies(instances, instance, command);
  // D is coordinator knowledge, not a hint to intersect with this replica's
  // table. Missing vertices and currently nonconflicting entries must survive.
  // Neither initDep nor dep is modified or replaced by this derived value.
  result.insert(proposed.begin(), proposed.end());
  return result;
}

Dependencies Replica::expand_dependencies(InstanceId instance, const kv::Command& command,
                                         const Dependencies& proposed) const {
  return expand_known_dependencies(instances_, instance, command, proposed);
}

}  // namespace egalis::protocol
