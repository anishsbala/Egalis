#include "egalis/sim/replica_endpoint.hpp"

#include <gtest/gtest.h>
#include <array>

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
}  // namespace egalis::sim
