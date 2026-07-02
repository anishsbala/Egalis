#include "egalis/sim/replica_endpoint.hpp"

#include <gtest/gtest.h>
#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

namespace egalis::sim {
namespace {
using protocol::ReplicaId;
using protocol::InstanceId;
const ReplicaId local{0};
const InstanceId id{1, local};
protocol::Submit submission(InstanceId instance = id) {
  return {instance, kv::Put{"key", "value"}};
}
protocol::Envelope message(unsigned source, unsigned destination) {
  return {ReplicaId{source}, ReplicaId{destination}, protocol::Waiting{id}};
}
protocol::Effects sends(std::initializer_list<protocol::Envelope> envelopes) {
  protocol::Effects effects;
  for (const auto& envelope : envelopes) effects.push_back(protocol::SendMessage{envelope});
  return effects;
}
void scenario(Network& network) {
  std::array<ReplicaEndpoint*, 5> endpoints{};
  for (const auto replica : protocol::members()) {
    endpoints[replica.value()] = &ReplicaEndpoint::attach(network, replica);
    endpoints[replica.value()]->dispatch(submission(InstanceId{1, replica}));
  }
  endpoints[0]->arm_timer(id, {5});
  endpoints[0]->arm_timer(id, {2});
  endpoints[2]->arm_timer(InstanceId{1, ReplicaId{2}}, {3});
  endpoints[0]->execute(sends({message(0, 0), message(0, 1), message(0, 2)}));
  network.send(message(3, 4));
  const std::array left{ReplicaId{1}}, right{ReplicaId{2}};
  network.partition(left, right);
  network.send(message(2, 1));
  network.partition(left, right, false);
  network.crash(ReplicaId{2});
  network.restart(ReplicaId{2});
  endpoints[2]->arm_timer(InstanceId{1, ReplicaId{2}}, {1});
  network.scheduler().run(100);
  EXPECT_EQ(network.scheduler().pending(), 0U);
  for (const auto* endpoint : endpoints) {
    ASSERT_EQ(endpoint->core().instances().size(), 1U);
    EXPECT_EQ(endpoint->core().instances().begin()->second.state.phase, protocol::Phase::initial);
  }
  network.verify_replay_complete();
}
}

TEST(ReplicaEndpoint, EndpointDispatchUsesTheSameCoreAndNeverVotesYet) {
  Network network;
  auto& source = ReplicaEndpoint::attach(network, local);
  auto& target = ReplicaEndpoint::attach(network, ReplicaId{1});
  source.dispatch(submission());
  EXPECT_EQ(source.last_status(), protocol::DispatchStatus::processed);
  source.execute(sends({message(0, 1)}));
  EXPECT_EQ(target.dispatch_count(), 0U);
  EXPECT_EQ(network.scheduler().run(), 1U);
  EXPECT_EQ(target.last_status(), protocol::DispatchStatus::unsupported);
  EXPECT_TRUE(target.core().instances().empty());
  EXPECT_EQ(source.core().instances().at(id).state.phase, protocol::Phase::initial);
}
TEST(ReplicaEndpoint, SelfDeliveryCompletesBeforeFollowingEffectsAtTheSameTick) {
  Network network{Duration{99}};
  auto& source = ReplicaEndpoint::attach(network, local);
  ReplicaEndpoint::attach(network, ReplicaId{1});
  source.execute(sends({message(0, 0), message(0, 1)}));
  EXPECT_EQ(source.dispatch_count(), 1U);
  EXPECT_EQ(network.scheduler().now(), LogicalTime{0});
  EXPECT_EQ(network.scheduler().pending(), 1U);
  std::size_t self_result = 0, remote_send = 0;
  const auto& trace = network.trace().decisions;
  for (std::size_t i = 0; i < trace.size(); ++i) {
    if (trace[i].kind == DecisionKind::protocol && trace[i].detail.starts_with("result/"))
      self_result = i;
    if (trace[i].kind == DecisionKind::sent && trace[i].detail == protocol::canonical(message(0, 1)))
      remote_send = i;
  }
  EXPECT_GT(self_result, 0U);
  EXPECT_GT(remote_send, self_result);
}
TEST(ReplicaEndpoint, ManySelfEffectsDoNotRecursivelyPumpOrQueueNetworkEvents) {
  Network network;
  auto& endpoint = ReplicaEndpoint::attach(network, local);
  protocol::Effects effects;
  for (unsigned i = 0; i < 2000; ++i) effects.push_back(protocol::SendMessage{message(0, 0)});
  endpoint.execute(std::move(effects));
  EXPECT_EQ(endpoint.dispatch_count(), 2000U);
  EXPECT_EQ(network.scheduler().pending(), 0U);
}
TEST(ReplicaEndpoint, EffectBudgetBoundsSynchronousFeedbackAndFailsClosed) {
  Network network;
  EXPECT_THROW(ReplicaEndpoint::attach(network, local, 0), std::invalid_argument);
  auto& endpoint = ReplicaEndpoint::attach(network, local, 2);
  EXPECT_THROW(endpoint.execute(sends({message(0, 0), message(0, 0), message(0, 0)})),
               std::runtime_error);
  EXPECT_EQ(endpoint.dispatch_count(), 2U);
  EXPECT_TRUE(network.scheduler().failed());
  EXPECT_THROW(endpoint.execute({}), std::logic_error);
}
TEST(ReplicaEndpoint, TimerReplacementAndStaleCallbacksCannotConsumeTheCurrentTimer) {
  Network network;
  auto& endpoint = ReplicaEndpoint::attach(network, local);
  endpoint.dispatch(submission());
  endpoint.arm_timer(id, {9});
  const auto old = *endpoint.core().instances().at(id).timer;
  endpoint.arm_timer(id, {1});
  const auto current = *endpoint.core().instances().at(id).timer;
  endpoint.dispatch(protocol::TimerFired{old});
  EXPECT_EQ(endpoint.last_status(), protocol::DispatchStatus::stale);
  EXPECT_EQ(endpoint.core().instances().at(id).timer, current);
  EXPECT_EQ(network.scheduler().run(), 1U);
  EXPECT_EQ(network.scheduler().now(), LogicalTime{1});
  EXPECT_EQ(endpoint.last_status(), protocol::DispatchStatus::unsupported);
  EXPECT_FALSE(endpoint.core().instances().at(id).timer);
}
TEST(ReplicaEndpoint, CrashCancelsTimersAndOldDeliveriesStayStaleAfterRestart) {
  Network network;
  auto& endpoint = ReplicaEndpoint::attach(network, local);
  ReplicaEndpoint::attach(network, ReplicaId{1});
  endpoint.dispatch(submission());
  endpoint.arm_timer(id, {4});
  const auto old = *endpoint.core().instances().at(id).timer;
  network.send(message(1, 0), DeliveryPlan{false, {3}, false, {3}});
  network.crash(local);
  EXPECT_FALSE(endpoint.core().active());
  EXPECT_THROW(endpoint.dispatch(protocol::TimerFired{old}), std::logic_error);
  EXPECT_THROW(endpoint.execute(sends({message(0, 1)})), std::logic_error);
  network.restart(local);
  EXPECT_EQ(endpoint.core().instances().size(), 1U);
  endpoint.dispatch(protocol::TimerFired{old});
  EXPECT_EQ(endpoint.last_status(), protocol::DispatchStatus::stale);
  const auto before = endpoint.dispatch_count();
  EXPECT_EQ(network.scheduler().run(), 1U);  // Old message discarded, timer cancelled.
  EXPECT_EQ(endpoint.dispatch_count(), before);
  endpoint.arm_timer(id, {0});
  network.scheduler().run();
  EXPECT_EQ(endpoint.dispatch_count(), before + 1);
}
TEST(ReplicaEndpoint, DuplicateDeliveryDoesNotAllocateOrMutateProtocolRecords) {
  Network network;
  ReplicaEndpoint::attach(network, local);
  auto& target = ReplicaEndpoint::attach(network, ReplicaId{1});
  target.dispatch(submission(InstanceId{1, ReplicaId{1}}));
  network.send(message(0, 1), DeliveryPlan{false, {0}, true, {0}});
  EXPECT_EQ(network.scheduler().run(), 2U);
  EXPECT_EQ(target.dispatch_count(), 3U);
  EXPECT_EQ(target.core().instances().size(), 1U);
  EXPECT_EQ(target.core().instances().begin()->second.state.phase, protocol::Phase::initial);
}
TEST(ReplicaEndpoint, EffectBatchRejectsForeignSourceBeforeSendingAnything) {
  Network network;
  auto& endpoint = ReplicaEndpoint::attach(network, local);
  const auto before = network.trace();
  EXPECT_THROW(endpoint.execute(sends({message(0, 0), message(1, 0)})), std::invalid_argument);
  EXPECT_EQ(network.trace(), before);
  EXPECT_EQ(endpoint.dispatch_count(), 0U);
  EXPECT_FALSE(network.scheduler().failed());
}
TEST(ReplicaEndpoint, TimerOverflowFailsRunnerAndNetworkClosed) {
  Network network;
  auto& endpoint = ReplicaEndpoint::attach(network, local);
  endpoint.dispatch(submission());
  network.scheduler().schedule_at({std::numeric_limits<std::uint64_t>::max()}, "last", [] {});
  network.scheduler().run();
  EXPECT_THROW(endpoint.arm_timer(id, {1}), std::overflow_error);
  EXPECT_TRUE(network.scheduler().failed());
  EXPECT_THROW(endpoint.dispatch(protocol::TimerFired{{id, {}, {1}}}), std::logic_error);
}
TEST(ReplicaEndpoint, DestroyingNetworkWithPendingCoreTimersIsSafe) {
  {
    Network network;
    auto& endpoint = ReplicaEndpoint::attach(network, local);
    endpoint.dispatch(submission());
    endpoint.arm_timer(id, {100});
    network.send(message(0, 0));
  }
  SUCCEED();  // Callback ownership is exercised without dereferencing a dead runner.
}
TEST(ReplicaEndpoint, SeededCoreScenariosHaveIdenticalTracesAndReplayExactly) {
  const FaultPolicy faults{{1}, {4}, 250000, 1000000, {1}};
  for (std::uint64_t seed = 0; seed < 16; ++seed) {
    Network first{seed, faults}; scenario(first);
    Network second{seed, faults}; scenario(second);
    EXPECT_EQ(first.trace(), second.trace()) << seed;
    Network replay{parse_trace(serialize(first.trace())), faults}; scenario(replay);
    EXPECT_EQ(replay.trace(), first.trace());
  }
}
TEST(ReplicaEndpoint, ReplayDetectsChangedSubmissionBytesAndFailsClosed) {
  Network original;
  auto& source = ReplicaEndpoint::attach(original, local);
  source.dispatch(submission());
  Network replay{original.trace(), FaultPolicy{}};
  auto& target = ReplicaEndpoint::attach(replay, local);
  EXPECT_THROW(target.dispatch(protocol::Submit{id, kv::Put{"key", "different"}}), ReplayDivergence);
  EXPECT_TRUE(target.core().instances().empty());
  EXPECT_TRUE(replay.scheduler().failed());
  EXPECT_THROW(target.dispatch(submission()), std::logic_error);
}
TEST(ReplicaEndpoint, LifecycleHookFailureStopsNetwork) {
  class Broken final : public transport::Endpoint {
    void receive(const protocol::Envelope&) override {}
    void on_crash() override { throw std::runtime_error("crash hook failed"); }
  };
  Network network;
  network.attach(local, std::make_unique<Broken>());
  EXPECT_THROW(network.crash(local), std::runtime_error);
  EXPECT_TRUE(network.scheduler().failed());
}
TEST(ReplicaEndpoint, RestartHookFailureAlsoStopsNetwork) {
  class Broken final : public transport::Endpoint {
    void receive(const protocol::Envelope&) override {}
    void on_restart() override { throw std::runtime_error("restart hook failed"); }
  };
  Network network;
  network.attach(local, std::make_unique<Broken>());
  network.crash(local);
  EXPECT_THROW(network.restart(local), std::runtime_error);
  EXPECT_TRUE(network.scheduler().failed());
}
}  // namespace egalis::sim
