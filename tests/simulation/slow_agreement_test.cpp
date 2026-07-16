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

// Every scenario reconstructs the captured artifact, rather than rerunning only
// with a matching seed. All crash cuts occur between serialized runner inputs.
template<class Scenario>
void captured_replay(Scenario scenario, FaultPolicy policy = {}) {
  Network original{0, policy};
  scenario(original);
  original.verify_replay_complete();
  const auto captured = serialize(original.trace());
  Network replay{parse_trace(captured), policy};
  scenario(replay);
  replay.verify_replay_complete();
  EXPECT_EQ(original.trace(), replay.trace());
}
void decisions(const Replicas& replicas, InstanceId target, const Dependencies& expected) {
  for (const auto* endpoint : replicas) {
    const auto found = endpoint->core().instances().find(target);
    if (found == endpoint->core().instances().end()) continue;
    const auto& record = found->second;
    if (record.state.phase == Phase::committed) {
      ASSERT_TRUE(record.committed_decision);
      EXPECT_EQ(record.state.command, kv::Payload{command});
      EXPECT_EQ(record.state.dependencies, expected);
    }
    if (record.committed_decision) {
      EXPECT_EQ(record.committed_decision->command, kv::Payload{command});
      EXPECT_EQ(record.committed_decision->dependencies, expected);
    }
  }
}
template<class Predicate>
bool until(Network& network, const Replicas& replicas, InstanceId target,
           const Dependencies& expected, Predicate ready) {
  for (unsigned events = 0; events < 500; ++events) {
    decisions(replicas, target, expected);
    if (ready()) return true;
    if (!network.scheduler().step()) return false;
  }
  return false;
}
void drain(Network& network, const Replicas& replicas, InstanceId target,
           const Dependencies& expected = {}) {
  decisions(replicas, target, expected);
  unsigned events = 0;
  while (network.scheduler().step()) {
    ASSERT_LT(++events, 500U);
    decisions(replicas, target, expected);
  }
}
std::vector<ReplicaId> broadcast_order(const Network& network, InstanceId target,
                                     const Dependencies& dependencies, bool commit) {
  std::vector<ReplicaId> result;
  for (const auto& decision : network.trace().decisions) {
    if (decision.kind != DecisionKind::protocol) continue;
    for (const auto peer : members()) {
      const Envelope packet = commit
          ? Envelope{target.coordinator(), peer, Commit{{}, target, command, dependencies}}
          : Envelope{target.coordinator(), peer, Accept{{}, target, command, dependencies}};
      if (decision.detail == "effect/send/" + canonical(packet)) result.push_back(peer);
    }
  }
  return result;
}
const InstanceId conflict{20, ReplicaId{4}};
void seed_conflict(Network& network, ReplicaId receiver) {
  network.send({ReplicaId{4}, receiver,
      Commit{{}, conflict, kv::Put{"key", "prior"}, {}}});
}
TEST(SlowFailureReplay, NonzeroCoordinatorDisagreesAndCommitsWithAcceptorsOutsideInitialQuorum) {
  captured_replay([](Network& network) {
    const auto replicas = attach(network);
    const ReplicaId coordinator{3};
    const InstanceId target{1, coordinator};
    seed_conflict(network, ReplicaId{1});
    network.scheduler().run();
    replicas[3]->dispatch(Submit{target, command});
    ASSERT_TRUE(until(network, replicas, target, {conflict}, [&] {
      return replicas[3]->core().instances().at(target).state.phase == Phase::accepted;
    }));
    const auto& selected = replicas[3]->core().instances().at(target);
    ASSERT_EQ(selected.initial_selection, InitialSelection::slow);
    ASSERT_TRUE(selected.preaccept_observations);
    EXPECT_EQ(selected.preaccept_observations->size(), 3U);
    EXPECT_TRUE(selected.preaccept_observations->contains(ReplicaId{0}));
    EXPECT_TRUE(selected.preaccept_observations->contains(ReplicaId{1}));
    EXPECT_FALSE(selected.preaccept_observations->contains(ReplicaId{2}));
    EXPECT_FALSE(selected.preaccept_observations->contains(ReplicaId{4}));
    ASSERT_TRUE(selected.slow_votes);
    EXPECT_EQ(selected.slow_votes->responders.size(), 1U);  // Real immediate self-OK.
    // Keep the two initial remote voters out of the AcceptOK quorum. The
    // remaining two replicas need not belong to the frozen PreAccept Q.
    network.set_link(ReplicaId{0}, coordinator, true);
    network.set_link(ReplicaId{1}, coordinator, true);
    drain(network, replicas, target, {conflict});
    EXPECT_TRUE(selected.slow_votes->sealed);
    EXPECT_EQ(selected.slow_votes->responders.size(), 3U);
    for (unsigned member : {2U, 3U, 4U})
      EXPECT_TRUE(selected.slow_votes->responders.contains(ReplicaId{member}));
    for (const auto* endpoint : replicas)
      EXPECT_EQ(endpoint->core().instances().at(target).state.phase, Phase::committed);
    const std::vector<ReplicaId> order{ReplicaId{3}, ReplicaId{0}, ReplicaId{1}, ReplicaId{2}, ReplicaId{4}};
    EXPECT_EQ(broadcast_order(network, target, {conflict}, false), order);
    EXPECT_EQ(broadcast_order(network, target, {conflict}, true), order);
    EXPECT_TRUE(selected.state.initial_dependencies.empty());
    EXPECT_EQ(selected.state.initial_command, command);
  });
}
TEST(SlowFailureReplay, TwoSurvivorsDuplicatedTrafficCannotSelectOrDecide) {
  captured_replay([](Network& network) {
    const auto replicas = attach(network);
    for (unsigned member : {2U, 3U, 4U}) network.crash(ReplicaId{member});
    replicas[0]->dispatch(Submit{instance, command});
    drain(network, replicas, instance);
    const auto& record = replicas[0]->core().instances().at(instance);
    ASSERT_TRUE(record.preaccept_observations);
    EXPECT_EQ(record.preaccept_observations->size(), 2U);
    EXPECT_EQ(record.initial_selection, InitialSelection::collecting);
    EXPECT_FALSE(record.slow_proposal);
    EXPECT_FALSE(record.slow_votes);
    EXPECT_TRUE(broadcast_order(network, instance, {}, false).empty());
    EXPECT_TRUE(broadcast_order(network, instance, {}, true).empty());
    for (unsigned survivor : {0U, 1U})
      EXPECT_EQ(replicas[survivor]->core().instances().at(instance).state.phase, Phase::preaccepted);
  }, FaultPolicy{{1}, {2}, 0, 1000000, {2}});
}
TEST(SlowFailureReplay, DuplicatedAndReorderedAcceptAndAcknowledgmentsKeepOneDecision) {
  captured_replay([](Network& network) {
    const auto replicas = attach(network);
    for (unsigned peer : {1U, 2U, 3U, 4U}) seed_conflict(network, ReplicaId{peer});
    network.scheduler().run();
    replicas[0]->dispatch(Submit{instance, command});
    ASSERT_TRUE(until(network, replicas, instance, {conflict}, [&] {
      return replicas[0]->core().instances().at(instance).state.phase == Phase::accepted;
    }));
    const auto& record = replicas[0]->core().instances().at(instance);
    ASSERT_TRUE(record.slow_proposal);
    EXPECT_EQ(record.initial_selection, InitialSelection::slow);
    ASSERT_TRUE(until(network, replicas, instance, {conflict}, [&] {
      const auto found = replicas[1]->core().instances().find(instance);
      return found != replicas[1]->core().instances().end() && found->second.state.phase == Phase::accepted;
    }));
    // Peer1 has actually accepted before any acknowledgment retry is sent.
    // Explicit retries overtake the original delayed retry; each receiver still
    // processes the same immutable proposal, and replies have no value fields.
    const Envelope request{owner, ReplicaId{1}, Accept{{}, instance, command, {conflict}}};
    const auto slow_request = network.send(request, DeliveryPlan{false, {8}, true, {9}});
    const auto fast_request = network.send(request, DeliveryPlan{false, {1}, true, {2}});
    const Envelope ack{ReplicaId{1}, owner, AcceptOk{{}, instance}};
    const auto slow_ack = network.send(ack, DeliveryPlan{false, {7}, true, {8}});
    const auto fast_ack = network.send(ack, DeliveryPlan{false, {1}, true, {2}});
    drain(network, replicas, instance, {conflict});
    EXPECT_TRUE(record.slow_votes->sealed);
    EXPECT_EQ(record.slow_votes->responders.size(), 3U);
    EXPECT_EQ(broadcast_order(network, instance, {conflict}, true).size(), 5U);
    for (const auto* endpoint : replicas)
      EXPECT_EQ(endpoint->core().instances().at(instance).state.phase, Phase::committed);
    unsigned requests = 0, acknowledgments = 0;
    for (const auto& decision : network.trace().decisions) {
      if (decision.kind != DecisionKind::protocol) continue;
      if (decision.detail == "input/peer/" + canonical(request)) ++requests;
      if (decision.detail == "input/peer/" + canonical(ack)) ++acknowledgments;
    }
    const auto delivered_index = [&](transport::MessageId packet) {
      for (std::size_t index = 0; index < network.trace().decisions.size(); ++index) {
        const auto& event = network.trace().decisions[index];
        if (event.kind == DecisionKind::delivered && event.fields[1] == packet.value && event.fields[2] == 0)
          return index;
      }
      return network.trace().decisions.size();
    };
    ASSERT_LT(delivered_index(slow_request), network.trace().decisions.size());
    ASSERT_LT(delivered_index(slow_ack), network.trace().decisions.size());
    EXPECT_LT(delivered_index(fast_request), delivered_index(slow_request));
    EXPECT_LT(delivered_index(fast_ack), delivered_index(slow_ack));
    EXPECT_GE(requests, 4U);
    EXPECT_GE(acknowledgments, 4U);
  }, FaultPolicy{{1}, {3}, 0, 1000000, {2}});
}
TEST(SlowFailureReplay, SupersededCollectorIgnoresOldAndLargerRepliesAndStaleCommit) {
  captured_replay([](Network& network) {
    const auto replicas = attach(network);
    network.crash(ReplicaId{3}); network.crash(ReplicaId{4});
    replicas[0]->dispatch(Submit{instance, command});
    const auto& record = replicas[0]->core().instances().at(instance);
    ASSERT_TRUE(until(network, replicas, instance, {}, [&] {
      return record.slow_votes && record.slow_votes->responders.size() == 2U;
    }));
    ASSERT_EQ(record.state.phase, Phase::accepted);
    const auto joined = Ballot::recovery(1, owner);
    // Typed incoming positive-ballot guard fixture, not recovery initiation.
    replicas[0]->dispatch(PeerMessage{{owner, owner, Accept{joined, instance, command, {}}}});
    EXPECT_FALSE(record.slow_proposal);
    EXPECT_FALSE(record.slow_votes);
    network.send({ReplicaId{1}, owner, AcceptOk{{}, instance}}, DeliveryPlan{false, {1}, true, {3}});
    network.send({ReplicaId{2}, owner, AcceptOk{Ballot::recovery(2, owner), instance}});
    replicas[0]->dispatch(PeerMessage{{owner, owner, Commit{{}, instance, command, {}}}});
    EXPECT_EQ(replicas[0]->last_status(), DispatchStatus::stale);
    drain(network, replicas, instance);
    EXPECT_EQ(record.state.phase, Phase::accepted);
    EXPECT_EQ(record.state.ballots.current, joined);
    EXPECT_TRUE(broadcast_order(network, instance, {}, true).empty());
    replicas[0]->dispatch(PeerMessage{{owner, owner, Commit{joined, instance, command, {}}}});
    decisions(replicas, instance, {});
    EXPECT_EQ(record.state.phase, Phase::committed);
    network.verify_replay_complete();
  });
}
TEST(SlowFailureReplay, CommitBeforeAcceptRetainsDecisionAcrossReorderedBallots) {
  captured_replay([](Network& network) {
    const auto replicas = attach(network);
    const ReplicaId receiver{1}, recoverer{2};
    const auto joined = Ballot::recovery(1, recoverer);
    network.send({owner, receiver, Commit{{}, instance, command, {conflict}}},
        DeliveryPlan{false, {1}, false, {1}});
    network.send({owner, receiver, Accept{{}, instance, command, {conflict}}},
        DeliveryPlan{false, {4}, true, {5}});
    network.send({recoverer, receiver, Accept{joined, instance, command, {conflict}}},
        DeliveryPlan{false, {6}, false, {6}});
    network.send({owner, receiver, Commit{{}, instance, command, {conflict}}},
        DeliveryPlan{false, {8}, false, {8}});
    network.send({recoverer, receiver, Commit{joined, instance, command, {conflict}}},
        DeliveryPlan{false, {9}, false, {9}});
    ASSERT_TRUE(until(network, replicas, instance, {conflict}, [&] {
      return replicas[1]->dispatch_count() == 1U;
    }));
    const auto& record = replicas[1]->core().instances().at(instance);
    EXPECT_EQ(record.state.phase, Phase::committed);
    EXPECT_FALSE(record.state.initial_command);
    ASSERT_TRUE(until(network, replicas, instance, {conflict}, [&] {
      return replicas[1]->dispatch_count() == 3U;
    }));
    EXPECT_EQ(replicas[1]->last_status(), DispatchStatus::stale);
    EXPECT_EQ(record.state.phase, Phase::committed);
    ASSERT_TRUE(until(network, replicas, instance, {conflict}, [&] {
      return replicas[1]->dispatch_count() == 4U;
    }));
    EXPECT_EQ(record.state.phase, Phase::accepted);  // Higher Accept is paper-allowed.
    EXPECT_EQ(record.state.ballots.current, joined);
    ASSERT_TRUE(record.committed_decision);
    EXPECT_EQ(record.committed_decision->dependencies, Dependencies{conflict});
    drain(network, replicas, instance, {conflict});
    EXPECT_EQ(record.state.phase, Phase::committed);
    EXPECT_EQ(record.state.ballots.accepted, joined);
    EXPECT_FALSE(record.state.initial_command);
  });
}
TEST(SlowFailureReplay, CoordinatorCrashAtLegalSlowBoundaries) {
  for (unsigned cut = 0; cut < 3; ++cut) {
    SCOPED_TRACE(cut);
    captured_replay([cut](Network& network) {
      const auto replicas = attach(network);
      network.crash(ReplicaId{3}); network.crash(ReplicaId{4});
      replicas[0]->dispatch(Submit{instance, command});
      const auto& record = replicas[0]->core().instances().at(instance);
      ASSERT_TRUE(until(network, replicas, instance, {}, [&] {
        if (cut == 0) return record.preaccept_observations->size() == 2U;
        if (cut == 1) return record.slow_votes && record.slow_votes->responders.size() == 2U;
        return record.state.phase == Phase::committed;
      }));
      if (cut == 0) {
        EXPECT_EQ(record.state.phase, Phase::preaccepted);
        EXPECT_FALSE(record.slow_proposal);
      } else if (cut == 1) {
        EXPECT_EQ(record.state.phase, Phase::accepted);
        EXPECT_FALSE(record.slow_votes->sealed);
      } else {
        ASSERT_TRUE(record.slow_votes->sealed);
        for (unsigned peer : {1U, 2U})
          EXPECT_EQ(replicas[peer]->core().instances().at(instance).state.phase, Phase::accepted);
      }
      network.crash(owner);
      drain(network, replicas, instance);
      EXPECT_FALSE(replicas[0]->core().active());
      if (cut < 2) {
        EXPECT_TRUE(broadcast_order(network, instance, {}, true).empty());
        for (unsigned survivor : {0U, 1U, 2U})
          EXPECT_FALSE(replicas[survivor]->core().instances().at(instance).committed_decision);
        EXPECT_EQ(broadcast_order(network, instance, {}, false).size(), cut == 0 ? 0U : 5U);
      } else {
        EXPECT_EQ(broadcast_order(network, instance, {}, true).size(), 5U);
        for (unsigned survivor : {0U, 1U, 2U})
          EXPECT_EQ(replicas[survivor]->core().instances().at(instance).state.phase, Phase::committed);
      }
    });
  }
}
}  // namespace
}  // namespace egalis::sim
