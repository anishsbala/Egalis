#include "egalis/sim/network.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

namespace egalis::sim {

namespace {
Decision event_decision(EventAction action, const EventInfo& info) {
  DecisionKind kind = DecisionKind::scheduled;
  if (action == EventAction::executed) kind = DecisionKind::executed;
  if (action == EventAction::cancelled) kind = DecisionKind::cancelled;
  return {kind, {info.id.value, info.sequence.value, info.time.ticks}, info.label};
}
}  // namespace

Network::Network(Duration delay) : Network(0, FaultPolicy{delay, delay, 0, 0, {0}}) {}

Network::Network(std::uint64_t seed, FaultPolicy policy)
    : trace_(seed), random_(seed), policy_(policy),
      scheduler_([this](auto action, const auto& info) { trace_.record(event_decision(action, info)); }) {
  configure();
}

Network::Network(TraceData replay, FaultPolicy policy)
    : trace_(std::move(replay)), random_(trace_.data().seed), policy_(policy),
      scheduler_([this](auto action, const auto& info) { trace_.record(event_decision(action, info)); }) {
  configure();
}

void Network::configure() {
  if (policy_.maximum_delay < policy_.minimum_delay || policy_.drop_per_million > 1000000 ||
      policy_.duplicate_per_million > 1000000) {
    throw std::invalid_argument("invalid fault policy");
  }
  trace_.record({DecisionKind::configured,
                 {policy_.minimum_delay.ticks, policy_.maximum_delay.ticks,
                  policy_.drop_per_million, policy_.duplicate_per_million,
                  policy_.duplicate_spacing.ticks}, {}});
}

void Network::healthy() const {
  trace_.check_healthy();
  if (scheduler_.failed()) throw std::logic_error("network scheduler failed");
}

Network::Slot& Network::live(protocol::ReplicaId replica) {
  healthy();
  auto& slot = endpoints_[replica.value()];
  if (!slot.endpoint) throw std::invalid_argument("replica endpoint missing");
  if (slot.crashed) throw std::logic_error("replica endpoint is crashed");
  return slot;
}

void Network::attach(protocol::ReplicaId replica, std::unique_ptr<transport::Endpoint> endpoint) {
  healthy();
  if (!endpoint) throw std::invalid_argument("null endpoint");
  auto& slot = endpoints_[replica.value()];
  if (slot.endpoint) throw std::invalid_argument("replica endpoint already attached");
  trace_.record({DecisionKind::attached, {scheduler_.now().ticks, replica.value()}, {}});
  slot.endpoint = std::move(endpoint);
  slot.generation = 1;
}

transport::MessageId Network::begin_send(const protocol::Envelope& envelope) {
  healthy();
  protocol::validate(envelope);
  live(envelope.source);
  if (next_message_ == std::numeric_limits<std::uint64_t>::max()) {
    throw std::overflow_error("message identity space exhausted");
  }
  const transport::MessageId id{next_message_};
  trace_.record({DecisionKind::sent, {scheduler_.now().ticks, id.value}, protocol::canonical(envelope)});
  ++next_message_;
  return id;
}

transport::MessageId Network::send(protocol::Envelope envelope) {
  const auto id = begin_send(envelope);
  try {
    DeliveryPlan plan;
    if (envelope.source != envelope.destination) {
      plan.drop = trace_.choose(random_, 0, 999999, "loss") < policy_.drop_per_million;
      plan.duplicate = trace_.choose(random_, 0, 999999, "duplication") < policy_.duplicate_per_million;
      plan.delay = Duration{trace_.choose(random_, policy_.minimum_delay.ticks,
                                         policy_.maximum_delay.ticks, "delay")};
      plan.duplicate_delay = plan.delay;
      if (!plan.drop && plan.duplicate) {
        plan.duplicate_delay = Duration{after(LogicalTime{plan.delay.ticks}, policy_.duplicate_spacing).ticks};
      }
    }
    enqueue(id, std::move(envelope), plan);
  } catch (...) {
    scheduler_.fail();
    throw;
  }
  return id;
}

transport::MessageId Network::send(protocol::Envelope envelope, DeliveryPlan plan) {
  const auto id = begin_send(envelope);
  try {
    enqueue(id, std::move(envelope), plan);
  } catch (...) {
    scheduler_.fail();
    throw;
  }
  return id;
}

void Network::discard(transport::MessageId id, std::string reason, unsigned copy) {
  trace_.record({DecisionKind::discarded, {scheduler_.now().ticks, id.value, copy}, std::move(reason)});
}

void Network::enqueue(transport::MessageId id, protocol::Envelope envelope, DeliveryPlan plan) {
  if (envelope.source == envelope.destination) {
    // No faults, RNG draws, or event delays for protocol self-delivery.
    trace_.record({DecisionKind::delivered, {scheduler_.now().ticks, id.value, 0}, "self"});
    endpoints_[envelope.source.value()].endpoint->receive(envelope);
    return;
  }
  trace_.record({DecisionKind::plan,
                 {id.value, plan.drop, plan.delay.ticks, plan.duplicate, plan.duplicate_delay.ticks}, {}});
  if (plan.drop) { discard(id, "loss"); return; }
  if (blocked_[envelope.source.value()][envelope.destination.value()]) {
    discard(id, "partition at send");
    return;
  }
  // Check both times before queuing either copy. Overflow cannot leave a half plan.
  const auto first = after(scheduler_.now(), plan.delay);
  const auto second = plan.duplicate ? after(scheduler_.now(), plan.duplicate_delay) : first;
  const auto generation = endpoints_[envelope.destination.value()].generation;
  const auto shared = std::make_shared<const protocol::Envelope>(std::move(envelope));
  const auto schedule = [&](LogicalTime when, unsigned copy) {
    scheduler_.schedule_at(when, "message/" + std::to_string(id.value) + "/" + std::to_string(copy),
                          [this, id, shared, generation, copy] { deliver(id, *shared, generation, copy); });
  };
  schedule(first, 0);
  if (plan.duplicate) schedule(second, 1);
}

void Network::deliver(transport::MessageId id, const protocol::Envelope& envelope,
                      std::uint64_t generation, unsigned copy) {
  healthy();
  const auto& slot = endpoints_[envelope.destination.value()];
  if (!slot.endpoint) { discard(id, "missing endpoint", copy); return; }
  if (slot.crashed) { discard(id, "crashed endpoint", copy); return; }
  if (slot.generation != generation) { discard(id, "stale destination generation", copy); return; }
  if (blocked_[envelope.source.value()][envelope.destination.value()]) {
    discard(id, "partition at delivery", copy);
    return;
  }
  trace_.record({DecisionKind::delivered, {scheduler_.now().ticks, id.value, copy}, {}});
  slot.endpoint->receive(envelope);
}

}  // namespace egalis::sim
