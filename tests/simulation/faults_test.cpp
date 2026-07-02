#include "egalis/sim/network.hpp"

#include <gtest/gtest.h>
#include <array>
#include <functional>
#include <stdexcept>
#include <vector>

namespace egalis::sim {
namespace {
using protocol::ReplicaId;
class Inbox final : public transport::Endpoint {
 public:
  void receive(const protocol::Envelope& envelope) override { messages.push_back(envelope); }
  std::vector<protocol::Envelope> messages;
};
protocol::Envelope message(unsigned from, unsigned to, std::uint64_t counter = 1) {
  return {ReplicaId{from}, ReplicaId{to}, protocol::Waiting{protocol::InstanceId{counter, ReplicaId{from}}}};
}
Inbox& attach(Network& network, unsigned id) {
  auto endpoint = std::make_unique<Inbox>();
  auto& inbox = *endpoint;
  network.attach(ReplicaId{id}, std::move(endpoint));
  return inbox;
}
std::size_t count(const Network& network, DecisionKind kind, const std::string& detail = "") {
  std::size_t total = 0;
  for (const auto& decision : network.trace().decisions) {
    if (decision.kind == kind && (detail.empty() || decision.detail == detail)) ++total;
  }
  return total;
}

std::array<std::vector<protocol::Envelope>, 5> scenario(Network& network) {
  std::array<Inbox*, 5> inboxes{};
  for (unsigned i = 0; i < 5; ++i) inboxes[i] = &attach(network, i);
  network.scheduler().schedule_at({2}, "partition", [&] {
    const std::array left{ReplicaId{0}, ReplicaId{1}};
    const std::array right{ReplicaId{2}, ReplicaId{3}};
    network.partition(left, right);
  });
  network.scheduler().schedule_at({3}, "crash", [&] { network.crash(ReplicaId{1}); });
  network.scheduler().schedule_at({6}, "restart", [&] { network.restart(ReplicaId{1}); });
  network.timer(ReplicaId{1}, {7}, "old timer", [] { FAIL() << "stale timer executed"; });
  for (unsigned i = 0; i < 50; ++i) network.send(message(i % 5, (i + 1) % 5, i + 1));
  network.scheduler().run();
  network.verify_replay_complete();
  std::array<std::vector<protocol::Envelope>, 5> result;
  for (unsigned i = 0; i < 5; ++i) result[i] = inboxes[i]->messages;
  return result;
}
}  // namespace

TEST(Faults, ExplicitDelaysLossAndDuplicatesHavePredictableOutcomes) {
  Network network;
  attach(network, 0);
  auto& inbox = attach(network, 1);
  network.send(message(0, 1, 1), DeliveryPlan{false, {9}, false, {0}});
  const auto duplicate = network.send(message(0, 1, 2), DeliveryPlan{false, {2}, true, {2}});
  network.send(message(0, 1, 3), DeliveryPlan{true, {0}, true, {0}});
  EXPECT_EQ(network.scheduler().pending(), 3U);
  EXPECT_EQ(network.scheduler().run(2), 2U);
  ASSERT_EQ(inbox.messages.size(), 2U);
  EXPECT_EQ(inbox.messages[0], message(0, 1, 2));
  EXPECT_EQ(inbox.messages[1], message(0, 1, 2));
  EXPECT_EQ(network.scheduler().now(), LogicalTime{2});
  network.scheduler().run();
  ASSERT_EQ(inbox.messages.size(), 3U);
  EXPECT_EQ(inbox.messages.back(), message(0, 1, 1));
  EXPECT_EQ(count(network, DecisionKind::discarded, "loss"), 1U);
  std::vector<std::uint64_t> copies;
  for (const auto& record : network.trace().decisions) {
    if (record.kind == DecisionKind::delivered && record.fields[1] == duplicate.value) {
      copies.push_back(record.fields[2]);
    }
  }
  EXPECT_EQ(copies, (std::vector<std::uint64_t>{0, 1}));
}

TEST(Faults, PartitionOnlyBlocksCrossGroupCommunicationAndCanHeal) {
  Network network;
  std::array<Inbox*, 5> inboxes{};
  for (unsigned i = 0; i < 5; ++i) inboxes[i] = &attach(network, i);
  const std::array left{ReplicaId{0}, ReplicaId{1}};
  const std::array right{ReplicaId{2}, ReplicaId{3}};
  network.partition(left, right);
  for (unsigned from = 0; from < 5; ++from) for (unsigned to = 0; to < 5; ++to) {
    network.send(message(from, to));
  }
  network.scheduler().run();
  for (unsigned i = 0; i < 4; ++i) EXPECT_EQ(inboxes[i]->messages.size(), 3U);
  EXPECT_EQ(inboxes[4]->messages.size(), 5U);
  EXPECT_EQ(count(network, DecisionKind::discarded, "partition at send"), 8U);
  network.partition(left, right, false);
  network.send(message(0, 2));
  network.scheduler().run();
  EXPECT_EQ(inboxes[2]->messages.size(), 4U);
}

TEST(Faults, DirectedPartitionsAndInFlightPartitionChangesAreDistinct) {
  Network network{Duration{5}};
  auto& zero = attach(network, 0);
  auto& one = attach(network, 1);
  network.send(message(0, 1));
  network.set_link(ReplicaId{0}, ReplicaId{1}, true);
  network.send(message(1, 0));
  network.scheduler().run();
  EXPECT_EQ(zero.messages.size(), 1U);
  EXPECT_TRUE(one.messages.empty());
  EXPECT_EQ(count(network, DecisionKind::discarded, "partition at delivery"), 1U);
  network.send(message(0, 1));  // Lost now; healing cannot resurrect it.
  network.set_link(ReplicaId{0}, ReplicaId{1}, false);
  network.scheduler().run();
  EXPECT_TRUE(one.messages.empty());
}

TEST(Faults, CrashBlocksReceivesSendsAndOldTimersUntilRestart) {
  Network network{Duration{5}};
  attach(network, 0);
  auto& one = attach(network, 1);
  int timers = 0;
  network.timer(ReplicaId{1}, {2}, "timer", [&] { ++timers; });
  network.send(message(0, 1));
  network.crash(ReplicaId{1});
  EXPECT_THROW(network.send(message(1, 0)), std::logic_error);
  EXPECT_THROW(network.timer(ReplicaId{1}, {0}, "down", [] {}), std::logic_error);
  network.scheduler().run();
  EXPECT_TRUE(one.messages.empty());
  EXPECT_EQ(timers, 0);
  network.restart(ReplicaId{1});
  network.send(message(0, 1));
  network.timer(ReplicaId{1}, {0}, "fresh timer", [&] { ++timers; });
  network.scheduler().run();
  EXPECT_EQ(one.messages.size(), 1U);
  EXPECT_EQ(timers, 1);
}

TEST(Faults, RestartRejectsPreCrashInFlightMessagesAndTimers) {
  Network network{Duration{5}};
  attach(network, 0);
  auto& one = attach(network, 1);
  int timers = 0;
  network.send(message(0, 1));
  network.timer(ReplicaId{1}, {4}, "old timer", [&] { ++timers; });
  network.crash(ReplicaId{1});
  network.restart(ReplicaId{1});
  network.scheduler().run();
  EXPECT_TRUE(one.messages.empty());
  EXPECT_EQ(timers, 0);
  EXPECT_EQ(count(network, DecisionKind::discarded, "stale destination generation"), 1U);
}

TEST(Faults, SourceCrashDoesNotRecallAlreadySentMessages) {
  Network network;
  attach(network, 0);
  auto& one = attach(network, 1);
  network.send(message(0, 1));
  network.crash(ReplicaId{0});
  network.scheduler().run();
  EXPECT_EQ(one.messages.size(), 1U);
}

TEST(Faults, LateEndpointRegistrationDoesNotReceivePreviouslyUndeliverablePackets) {
  Network network;
  attach(network, 0);
  network.send(message(0, 1));
  auto& one = attach(network, 1);
  network.scheduler().run();
  EXPECT_TRUE(one.messages.empty());
  EXPECT_EQ(count(network, DecisionKind::discarded, "stale destination generation"), 1U);
}

TEST(Faults, IdenticalSeedsAndInputsProduceIdenticalTracesAndReplay) {
  const FaultPolicy policy{{1}, {10}, 200000, 400000, {2}};
  Network first{UINT64_C(12345), policy};
  Network second{UINT64_C(12345), policy};
  const auto expected = scenario(first);
  EXPECT_EQ(scenario(second), expected);
  EXPECT_EQ(serialize(first.trace()), serialize(second.trace()));
  Network replay{parse_trace(serialize(first.trace())), policy};
  EXPECT_EQ(scenario(replay), expected);
  EXPECT_EQ(replay.trace(), first.trace());
  Network different{UINT64_C(12346), policy};
  scenario(different);
  EXPECT_NE(first.trace().decisions, different.trace().decisions);
}

TEST(Faults, PolicyExtremesDropOrDuplicateEveryRemoteMessage) {
  Network loss{1, FaultPolicy{{0}, {0}, 1000000, 0, {0}}};
  attach(loss, 0);
  auto& missing = attach(loss, 1);
  loss.send(message(0, 1));
  EXPECT_EQ(loss.scheduler().run(), 0U);
  EXPECT_TRUE(missing.messages.empty());
  Network duplication{1, FaultPolicy{{0}, {0}, 0, 1000000, {0}}};
  attach(duplication, 0);
  auto& twice = attach(duplication, 1);
  duplication.send(message(0, 1));
  EXPECT_EQ(duplication.scheduler().run(), 2U);
  EXPECT_EQ(twice.messages.size(), 2U);
}

TEST(Faults, InvalidPoliciesAndLifecycleOperationsAreRejectedWithoutMutation) {
  EXPECT_THROW((Network{0, FaultPolicy{{5}, {1}, 0, 0, {0}}}), std::invalid_argument);
  EXPECT_THROW((Network{0, FaultPolicy{{0}, {1}, 1000001, 0, {0}}}), std::invalid_argument);
  EXPECT_THROW((Network{0, FaultPolicy{{0}, {1}, 0, 1000001, {0}}}), std::invalid_argument);
  Network network;
  attach(network, 0);
  const std::array left{ReplicaId{0}, ReplicaId{1}};
  const std::array overlapping{ReplicaId{1}, ReplicaId{2}};
  const auto before = network.trace().decisions.size();
  EXPECT_THROW(network.partition(left, overlapping), std::invalid_argument);
  EXPECT_THROW(network.set_link(ReplicaId{0}, ReplicaId{0}, true), std::invalid_argument);
  EXPECT_THROW(network.restart(ReplicaId{0}), std::logic_error);
  EXPECT_THROW(network.crash(ReplicaId{4}), std::invalid_argument);
  EXPECT_EQ(network.trace().decisions.size(), before);
  network.crash(ReplicaId{0});
  EXPECT_THROW(network.crash(ReplicaId{0}), std::logic_error);
}

TEST(Faults, ReplayDetectsChangedPayloadAndScheduleBeforeEndpointDelivery) {
  const FaultPolicy policy{};
  Network original{42, policy};
  attach(original, 0);
  attach(original, 1);
  original.send(message(0, 1));
  original.scheduler().run();
  Network replay{original.trace(), policy};
  attach(replay, 0);
  auto& inbox = attach(replay, 1);
  EXPECT_THROW(replay.send(message(0, 1, 2)), ReplayDivergence);
  EXPECT_TRUE(inbox.messages.empty());
  EXPECT_THROW(replay.send(message(0, 1)), ReplayDivergence);
  auto changed = policy;
  changed.maximum_delay = Duration{2};
  EXPECT_THROW((Network{original.trace(), changed}), ReplayDivergence);
}

TEST(Faults, DuplicateTimeOverflowLeavesNoPartiallyQueuedDelivery) {
  Network network;
  attach(network, 0);
  attach(network, 1);
  network.scheduler().schedule_at({1}, "advance", [] {});
  network.scheduler().run();
  EXPECT_THROW(network.send(message(0, 1), DeliveryPlan{false, {1}, true,
               {std::numeric_limits<std::uint64_t>::max()}}), std::overflow_error);
  EXPECT_EQ(network.scheduler().pending(), 0U);
  EXPECT_TRUE(network.scheduler().failed());
}

TEST(Faults, ImmediateEndpointExceptionsFailClosedWithoutSchedulingWork) {
  class Throwing final : public transport::Endpoint {
    void receive(const protocol::Envelope&) override { throw std::runtime_error("handler failed"); }
  };
  Network network;
  network.attach(ReplicaId{0}, std::make_unique<Throwing>());
  EXPECT_THROW(network.send(message(0, 0)), std::runtime_error);
  EXPECT_TRUE(network.scheduler().failed());
  EXPECT_THROW(network.send(message(0, 1)), std::logic_error);
  EXPECT_EQ(network.scheduler().pending(), 0U);
}

TEST(Faults, ReplayDetectsChangedSchedulerEventsAndUnfinishedQueues) {
  Network original;
  original.scheduler().schedule_at({3}, "input", [] {});
  EXPECT_THROW(original.verify_replay_complete(), std::logic_error);
  original.scheduler().run();
  Network replay{original.trace(), FaultPolicy{}};
  EXPECT_THROW(replay.scheduler().schedule_at({4}, "input", [] {}), ReplayDivergence);
  EXPECT_TRUE(replay.scheduler().failed());
  EXPECT_THROW(replay.verify_replay_complete(), ReplayDivergence);
}

TEST(Faults, DifferentPartitionInputOrderProducesSameDecisions) {
  Network first, second;
  const std::array left{ReplicaId{0}, ReplicaId{1}};
  const std::array reversed_left{ReplicaId{1}, ReplicaId{0}};
  const std::array right{ReplicaId{2}, ReplicaId{3}};
  const std::array reversed_right{ReplicaId{3}, ReplicaId{2}};
  first.partition(left, right);
  second.partition(reversed_left, reversed_right);
  EXPECT_EQ(first.trace(), second.trace());
}

TEST(Faults, EndpointCanSendDuringDeliveryThroughTheSameTransportBoundary) {
  class Responder final : public transport::Endpoint {
   public:
    explicit Responder(transport::Transport& transport) : transport_(transport) {}
    void receive(const protocol::Envelope& envelope) override {
      transport_.send(message(envelope.destination.value(), envelope.source.value()));
    }
   private:
    transport::Transport& transport_;
  };
  Network network{Duration{2}};
  auto& inbox = attach(network, 0);
  network.attach(ReplicaId{1}, std::make_unique<Responder>(network));
  network.send(message(0, 1));
  EXPECT_EQ(network.scheduler().run(), 2U);
  ASSERT_EQ(inbox.messages.size(), 1U);
  EXPECT_EQ(inbox.messages.front(), message(1, 0));
  EXPECT_EQ(network.scheduler().now(), LogicalTime{4});
}

TEST(Faults, SeededFaultRunsReplayAcrossMultipleSchedules) {
  const FaultPolicy policy{{0}, {8}, 250000, 250000, {1}};
  for (std::uint64_t seed = 0; seed < 32; ++seed) {
    SCOPED_TRACE(seed);
    Network original{seed, policy};
    const auto expected = scenario(original);
    Network replay{parse_trace(serialize(original.trace())), policy};
    EXPECT_EQ(scenario(replay), expected);
  }
}

}  // namespace egalis::sim
