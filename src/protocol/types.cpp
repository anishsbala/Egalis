#include "egalis/protocol/types.hpp"

#include <stdexcept>

namespace egalis::protocol {

void Configuration::validate(std::size_t n, std::size_t f, std::size_t e) {
  if (n != replicas || f != failures || e != fast_failures) {
    throw std::invalid_argument("only baseline EPaxos* n=5,f=2,e=1 is supported");
  }
}

ReplicaId::ReplicaId(std::uint32_t value) : value_(value) {
  if (value >= Configuration::replicas) {
    throw std::out_of_range("replica is outside fixed membership");
  }
}

std::array<ReplicaId, Configuration::replicas> members() {
  return {ReplicaId{0}, ReplicaId{1}, ReplicaId{2}, ReplicaId{3}, ReplicaId{4}};
}

InstanceId::InstanceId(std::uint64_t counter, ReplicaId coordinator)
    : counter_(counter), coordinator_(coordinator) {
  if (counter == 0 || counter > max_counter) {
    throw std::out_of_range("instance counter is zero or exhausted");
  }
}

Ballot Ballot::recovery(std::uint64_t round, ReplicaId owner) {
  if (round == 0 || round > max_round) {
    throw std::out_of_range("recovery ballot round is zero or exhausted");
  }
  return Ballot{(round - 1) * 5 + owner.value() + 1};
}

Ballot Ballot::next_owned_by(ReplicaId owner) const {
  if (round() == max_round) {
    throw std::overflow_error("ballot space exhausted");
  }
  return recovery(round() + 1, owner);
}

std::uint64_t Ballot::round() const noexcept {
  return initial() ? 0 : (number_ - 1) / 5 + 1;
}

ReplicaId Ballot::owner(const InstanceId& instance) const {
  return initial() ? instance.coordinator()
                   : ReplicaId{static_cast<std::uint32_t>((number_ - 1) % 5)};
}

bool Quorum::add(ReplicaId replica) {
  const bool fresh = !votes_.test(replica.value());
  votes_.set(replica.value());
  return fresh;
}

bool Quorum::contains(ReplicaId replica) const { return votes_.test(replica.value()); }

}  // namespace egalis::protocol
