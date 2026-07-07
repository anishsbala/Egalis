#include "egalis/protocol/dependencies.hpp"

namespace egalis::protocol {

Dependencies scan_known_dependencies(const std::map<InstanceId, InstanceRecord>& instances,
                                     InstanceId instance, const kv::Command& command) {
  Dependencies result;
  for (const auto& [known_id, record] : instances) {
    // EPaxos* v1 Figure 3, lines 3 and 9: scan current cmd, without a phase,
    // ballot, coordinator, or executed-record filter. Exclude the queried ID
    // even when it is already installed (the message layer forbids self edges).
    if (known_id != instance && kv::conflicts(record.state.command, command))
      result.insert(result.end(), known_id);
  }
  return result;
}

Dependencies Replica::known_dependencies(InstanceId instance, const kv::Command& command) const {
  return scan_known_dependencies(instances_, instance, command);
}

}  // namespace egalis::protocol
