#pragma once

#include "egalis/protocol/types.hpp"

namespace egalis::protocol {
// Volatile high-water mark only. Suspend/resume preserves it; replacing a lost
// Replica under the same identity is unsafe until durable allocation exists.
class InstanceAllocator {
 public:
  explicit InstanceAllocator(ReplicaId owner, std::uint64_t first = 1);
  [[nodiscard]] InstanceId next() const;
  void observe(InstanceId id);
 private:
  ReplicaId owner_;
  std::uint64_t next_;
};
}  // namespace egalis::protocol
