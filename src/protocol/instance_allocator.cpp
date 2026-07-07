#include "egalis/protocol/instance_allocator.hpp"

#include <algorithm>
#include <stdexcept>

namespace egalis::protocol {
InstanceAllocator::InstanceAllocator(ReplicaId owner, std::uint64_t first)
    : owner_(owner), next_(first) {
  if (first == 0 || first > InstanceId::max_counter)
    throw std::out_of_range("invalid initial instance counter");
}
InstanceId InstanceAllocator::next() const {
  if (next_ > InstanceId::max_counter) throw std::overflow_error("instance IDs exhausted");
  return {next_, owner_};
}
void InstanceAllocator::observe(InstanceId id) {
  if (id.coordinator() != owner_) throw std::invalid_argument("foreign instance allocation");
  // max_counter + 1 fits uint64_t and is an exhaustion sentinel, never an ID.
  next_ = std::max(next_, id.counter() + 1);
}
}  // namespace egalis::protocol
