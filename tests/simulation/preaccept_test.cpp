#include "egalis/sim/replica_endpoint.hpp"

#include <gtest/gtest.h>
#include <array>
#include <memory>
#include <vector>

namespace egalis::sim {
namespace {
using protocol::ReplicaId;
using protocol::InstanceId;
std::array<ReplicaEndpoint*, 5> attach(Network& network) {
  std::array<ReplicaEndpoint*, 5> replicas{};
  for (auto member : protocol::members()) replicas[member.value()] = &ReplicaEndpoint::attach(network, member);
  return replicas;
}
void seeded(Network& network) {
  const auto replicas = attach(network);
  for (auto member : protocol::members())
    replicas[member.value()]->dispatch(protocol::SubmitCommand{kv::Put{"shared", "v"}});
  network.scheduler().run(1000);
  for (auto member : protocol::members()) {
    const auto& core = replicas[member.value()]->core();
    EXPECT_TRUE(core.instances().contains(InstanceId{1, member}));
    for (const auto& [id, record] : core.instances()) {
      EXPECT_EQ(record.state.phase, protocol::Phase::preaccepted);
      EXPECT_TRUE(record.state.ballots.accepted.initial());
      EXPECT_TRUE(record.state.initial_dependencies.empty());
      EXPECT_FALSE(record.state.dependencies.contains(id));
    }
  }
  network.verify_replay_complete();
}
}
TEST(PreAcceptSimulation, SimultaneousConflictingCoordinatorsPreacceptWithoutCommitting) {
  Network network;
  const auto replicas = attach(network);
  for (unsigned member : {0U, 1U}) {
    network.scheduler().schedule_at({0}, "submit/" + std::to_string(member), [&, member] {
      replicas[member]->dispatch(protocol::SubmitCommand{kv::Put{"shared", std::to_string(member)}});
    });
  }
  network.scheduler().run();
  const InstanceId a{1, ReplicaId{0}}, b{1, ReplicaId{1}};
  for (const auto* replica : replicas) {
    ASSERT_EQ(replica->core().instances().size(), 2U);
    const auto& left = replica->core().instances().at(a).state;
    const auto& right = replica->core().instances().at(b).state;
    EXPECT_EQ(left.phase, protocol::Phase::preaccepted);
    EXPECT_EQ(right.phase, protocol::Phase::preaccepted);
    EXPECT_TRUE(left.initial_dependencies.empty());
    EXPECT_TRUE(right.initial_dependencies.empty());
    EXPECT_TRUE(left.dependencies.contains(b) || right.dependencies.contains(a));
  }
}
TEST(PreAcceptSimulation, ReorderedReceiptsPreserveInitialProposalAndLocalConflictOrder) {
  Network network;
  const auto replicas = attach(network);
  const InstanceId a{1, ReplicaId{0}}, b{1, ReplicaId{1}};
  network.send({ReplicaId{0}, ReplicaId{2}, protocol::PreAccept{a, kv::Put{"k", "a"}, {}}},
               DeliveryPlan{false, {5}, false, {5}});
  network.send({ReplicaId{1}, ReplicaId{2}, protocol::PreAccept{b, kv::Put{"k", "b"}, {}}},
               DeliveryPlan{false, {1}, false, {1}});
  network.scheduler().run();
  const auto& table = replicas[2]->core().instances();
  EXPECT_TRUE(table.at(b).state.dependencies.empty());
  EXPECT_EQ(table.at(a).state.dependencies, (protocol::Dependencies{b}));
  EXPECT_TRUE(table.at(a).state.initial_dependencies.empty());
}
TEST(PreAcceptSimulation, SeededInitialVotesAndFaultsReplayExactly) {
  const FaultPolicy faults{{1}, {6}, 200000, 700000, {1}};
  for (std::uint64_t seed = 0; seed < 8; ++seed) {
    Network original{seed, faults}; seeded(original);
    Network repeated{seed, faults}; seeded(repeated);
    EXPECT_EQ(original.trace(), repeated.trace());
    Network replay{parse_trace(serialize(original.trace())), faults}; seeded(replay);
    EXPECT_EQ(original.trace(), replay.trace());
  }
}
namespace {
class Replies final : public transport::Endpoint {
 public:
  void receive(const protocol::Envelope& envelope) override { messages.push_back(envelope); }
  std::vector<protocol::Envelope> messages;
};
Replies& inbox(Network& network, ReplicaId id) {
  auto endpoint = std::make_unique<Replies>();
  auto& reference = *endpoint;
  network.attach(id, std::move(endpoint));
  return reference;
}
protocol::Envelope packet(InstanceId id, ReplicaId target, std::string value = "v") {
  return {id.coordinator(), target, protocol::PreAccept{id, kv::Put{"k", std::move(value)}, {}}};
}
void cached_scenario(Network& network) {
  const ReplicaId a{0}, target{1}, b{2};
  auto& source = inbox(network, a);
  inbox(network, b);
  auto& replica = ReplicaEndpoint::attach(network, target);
  const InstanceId first{1, a}, other{1, b};
  network.send(packet(first, target), DeliveryPlan{false, {1}, true, {5}});
  network.send(packet(other, target), DeliveryPlan{false, {2}, false, {2}});
  EXPECT_EQ(network.scheduler().run(), 6U);  // Three requests, three responses.
  ASSERT_EQ(source.messages.size(), 2U);
  EXPECT_EQ(source.messages[0], source.messages[1]);
  EXPECT_TRUE(std::get<protocol::PreAcceptOk>(source.messages[0].message).dependencies.empty());
  EXPECT_TRUE(replica.core().instances().at(first).state.dependencies.empty());
  EXPECT_EQ(replica.core().instances().at(other).state.dependencies, (protocol::Dependencies{first}));
  EXPECT_EQ(replica.last_status(), protocol::DispatchStatus::replayed);
  EXPECT_EQ(replica.dispatch_count(), 3U);
  network.verify_replay_complete();
}
}
TEST(PreAcceptSimulation, DuplicateAfterInterveningConflictReplaysExactFrozenResponse) {
  Network original; cached_scenario(original);
  Network repeated; cached_scenario(repeated);
  EXPECT_EQ(original.trace(), repeated.trace());
  Network replay{parse_trace(serialize(original.trace())), FaultPolicy{}}; cached_scenario(replay);
  EXPECT_EQ(original.trace(), replay.trace());
}
TEST(PreAcceptSimulation, OldGenerationIsDiscardedButFreshRetryAfterResumeReplaysResponse) {
  Network network;
  const ReplicaId owner{0}, target{1};
  auto& source = inbox(network, owner);
  auto& replica = ReplicaEndpoint::attach(network, target);
  const InstanceId id{1, owner};
  network.send(packet(id, target), DeliveryPlan{false, {1}, true, {5}});
  EXPECT_TRUE(network.scheduler().step());
  network.crash(target);
  network.restart(target);
  network.scheduler().run();
  EXPECT_EQ(replica.dispatch_count(), 1U);
  ASSERT_EQ(source.messages.size(), 1U);
  network.send(packet(id, target), DeliveryPlan{false, {0}, false, {0}});
  network.scheduler().run();
  ASSERT_EQ(source.messages.size(), 2U);
  EXPECT_EQ(source.messages[0], source.messages[1]);
  EXPECT_EQ(replica.last_status(), protocol::DispatchStatus::replayed);
  EXPECT_EQ(replica.dispatch_count(), 2U);
}
TEST(PreAcceptSimulation, ConflictingDuplicateIsRejectedWithoutReplyOrNetworkFailure) {
  Network network;
  const ReplicaId owner{0}, target{1};
  auto& source = inbox(network, owner);
  auto& replica = ReplicaEndpoint::attach(network, target);
  const InstanceId id{1, owner};
  network.send(packet(id, target));
  network.scheduler().run();
  network.send(packet(id, target, "different"));
  network.scheduler().run();
  EXPECT_EQ(source.messages.size(), 1U);
  EXPECT_EQ(replica.last_status(), protocol::DispatchStatus::rejected);
  EXPECT_FALSE(network.scheduler().failed());
  EXPECT_EQ(replica.core().instances().at(id).state.command,
            (kv::Payload{kv::Command{kv::Put{"k", "v"}}}));
}
}  // namespace egalis::sim
