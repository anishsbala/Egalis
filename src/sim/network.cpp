#include "egalis/sim/network.hpp"

#include <limits>
#include <stdexcept>

namespace egalis::sim {

Network::Network(Duration delay) : delay_(delay) {}

void Network::attach(protocol::ReplicaId replica, std::unique_ptr<transport::Endpoint> endpoint) {
  if (!endpoint) throw std::invalid_argument("null endpoint");
  auto& slot = endpoints_[replica.value()];
  if (slot) throw std::invalid_argument("replica endpoint already attached");
  slot = std::move(endpoint);
}

transport::MessageId Network::send(protocol::Envelope envelope) {
  protocol::validate(envelope);
  if (!endpoints_[envelope.source.value()]) throw std::invalid_argument("source endpoint missing");
  if (next_message_ == std::numeric_limits<std::uint64_t>::max()) {
    throw std::overflow_error("message identity space exhausted");
  }
  if (envelope.source == envelope.destination) {
    const transport::MessageId id{next_message_++};
    // Protocol self-messages are synchronous, not subject to network delay.
    endpoints_[envelope.source.value()]->receive(envelope);
    return id;
  }
  const auto delivery_time = after(scheduler_.now(), delay_);
  const transport::MessageId id{next_message_++};
  scheduler_.schedule_at(delivery_time, "message", [this, envelope = std::move(envelope)] {
    if (auto& endpoint = endpoints_[envelope.destination.value()]) endpoint->receive(envelope);
  });
  return id;
}

}  // namespace egalis::sim
