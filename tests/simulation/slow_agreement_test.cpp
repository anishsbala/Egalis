#include "egalis/sim/replica_endpoint.hpp"

#include <gtest/gtest.h>
#include <array>
#include <vector>

namespace egalis::sim {
namespace {
using namespace protocol;
const ReplicaId owner{0};
const InstanceId instance{1, owner};
const kv::Command command = kv::Put{"key", "value"};
using Replicas = std::array<ReplicaEndpoint*, Configuration::replicas>;
Replicas attach(Network& network) {
  Replicas replicas{};
  for (const auto member : members())
    replicas[member.value()] = &ReplicaEndpoint::attach(network, member);
  return replicas;
}
std::vector<std::string> accepts(const Network& network, const Dependencies& dependencies) {
  std::vector<std::string> result;
  for (const auto& decision : network.trace().decisions) {
    if (decision.kind != DecisionKind::protocol) continue;
    for (const auto member : members())
      if (decision.detail == "effect/send/" + canonical({owner, member,
          Accept{{}, instance, command, dependencies}})) result.push_back(decision.detail);
  }
  return result;
}
TEST(SlowProposalSimulation, ThreeSurvivorsAutomaticallyBroadcastFrozenAcceptAndReplay) {
  const auto run = [](Network& network) {
    const auto replicas = attach(network);
    network.crash(ReplicaId{3}); network.crash(ReplicaId{4});
    replicas[0]->dispatch(Submit{instance, command});
    network.scheduler().run(100);
    const auto sent = accepts(network, {});
    ASSERT_EQ(sent.size(), Configuration::replicas);
    EXPECT_EQ(sent.front(), "effect/send/" + canonical({owner, owner, Accept{{}, instance, command, {}}}));
    const auto& state = replicas[0]->core().instances().at(instance).state;
    EXPECT_EQ(state.phase, Phase::committed);  // Three real AcceptOKs, including self.
    for (unsigned survivor : {0U, 1U, 2U})
      EXPECT_EQ(replicas[survivor]->core().instances().at(instance).state.phase, Phase::committed);
    EXPECT_TRUE(state.dependencies.empty());
    EXPECT_TRUE(state.ballots.accepted.initial());
    network.verify_replay_complete();
  };
  Network original; run(original);
  Network replay{parse_trace(serialize(original.trace())), FaultPolicy{}}; run(replay);
  EXPECT_EQ(original.trace(), replay.trace());
}
TEST(SlowQuorumSimulation, RealSelfAndOnePeerWaitForThirdReply) {
  Network network;
  const auto replicas = attach(network);
  network.crash(ReplicaId{3}); network.crash(ReplicaId{4});
  replicas[0]->dispatch(Submit{instance, command});
  const auto votes = [&] {
    unsigned count = 0;
    for (const auto& decision : network.trace().decisions) {
      if (decision.kind != DecisionKind::protocol) continue;
      for (unsigned sender : {0U, 1U, 2U})
        if (decision.detail == "input/peer/" + canonical({ReplicaId{sender}, owner,
            AcceptOk{{}, instance}})) ++count;
    }
    return count;
  };
  unsigned events = 0;
  while (votes() < 2U) {
    ASSERT_TRUE(network.scheduler().step());
    ASSERT_LT(++events, 100U);
  }
  EXPECT_EQ(votes(), 2U);
  EXPECT_EQ(replicas[0]->core().instances().at(instance).state.phase, Phase::accepted);
  network.scheduler().run(100);
  EXPECT_EQ(votes(), 3U);
  EXPECT_EQ(replicas[0]->core().instances().at(instance).state.phase, Phase::committed);
  network.verify_replay_complete();
}
TEST(SlowProposalSimulation, FourthMismatchUnionAndLateRepliesCannotChangeBroadcast) {
  Network network;
  const auto replicas = attach(network);
  for (unsigned peer = 1; peer < Configuration::replicas; ++peer)
    network.set_link(owner, ReplicaId{peer}, true);
  replicas[0]->dispatch(Submit{instance, command});
  const InstanceId first{1, ReplicaId{3}}, second{2, ReplicaId{4}};
  replicas[0]->dispatch(PeerMessage{{ReplicaId{1}, owner, PreAcceptOk{instance, {}}}});
  replicas[0]->dispatch(PeerMessage{{ReplicaId{2}, owner, PreAcceptOk{instance, {}}}});
  replicas[0]->dispatch(PeerMessage{{ReplicaId{3}, owner, PreAcceptOk{instance, {first}}}});
  ASSERT_EQ(accepts(network, {first}).size(), Configuration::replicas);
  replicas[0]->dispatch(PeerMessage{{ReplicaId{4}, owner, PreAcceptOk{instance, {second}}}});
  network.scheduler().run(100);
  EXPECT_EQ(accepts(network, {first}).size(), Configuration::replicas);
  EXPECT_TRUE(accepts(network, {first, second}).empty());
  EXPECT_EQ(replicas[0]->core().instances().at(instance).preaccept_observations->size(), 4U);
  EXPECT_EQ(replicas[0]->core().instances().at(instance).state.dependencies, Dependencies{first});
  network.verify_replay_complete();
}
}  // namespace
}  // namespace egalis::sim
