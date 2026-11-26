#include "egalis/sim/replica_endpoint.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace egalis::sim {
namespace {
using namespace protocol;

TEST(AgreementSimulation, FourthReplyCommitsBeforeFifthReplicaReceivesPreaccept) {
  Network network;
  const ReplicaId coordinator{3}, late{4};
  const InstanceId instance{1, coordinator};
  const kv::Command command = kv::Put{"key", "value"};
  std::array<ReplicaEndpoint*, Configuration::replicas> replicas{};
  for (const auto member : members())
    replicas[member.value()] = &ReplicaEndpoint::attach(network, member);

  network.set_link(coordinator, late, true);
  replicas[coordinator.value()]->dispatch(Submit{instance, command});
  network.set_link(coordinator, late, false);
  network.send({coordinator, late, PreAccept{instance, command, {}}},
               DeliveryPlan{false, {5}, false, {5}});

  // The other three remote requests and their replies run before the delayed request.
  for (unsigned event = 0; event < 6; ++event) ASSERT_TRUE(network.scheduler().step());
  EXPECT_EQ(replicas[coordinator.value()]->core().instances().at(instance).state.phase,
            Phase::committed);
  EXPECT_TRUE(replicas[late.value()]->core().instances().empty());
  network.scheduler().run();

  const Commit chosen{{}, instance, command, {}};
  const auto self_packet = canonical({coordinator, coordinator, chosen});
  const std::vector<std::string> expected_sends{
      "effect/send/" + self_packet,
      "effect/send/" + canonical({coordinator, ReplicaId{0}, chosen}),
      "effect/send/" + canonical({coordinator, ReplicaId{1}, chosen}),
      "effect/send/" + canonical({coordinator, ReplicaId{2}, chosen}),
      "effect/send/" + canonical({coordinator, late, chosen})};
  std::vector<std::string> commit_sends;
  std::size_t self_input = 0, self_result = 0, first_remote = 0;
  const auto& trace = network.trace().decisions;
  for (std::size_t index = 0; index < trace.size(); ++index) {
    const auto& decision = trace[index];
    if (decision.kind != DecisionKind::protocol) continue;
    if (std::ranges::find(expected_sends, decision.detail) != expected_sends.end())
      commit_sends.push_back(decision.detail);
    if (decision.detail == "input/peer/" + self_packet) self_input = index;
    if (self_input && !self_result && decision.detail.starts_with("result/"))
      self_result = index;
    if (decision.detail == expected_sends[1]) first_remote = index;
  }
  EXPECT_GT(self_input, 0U);
  EXPECT_GT(self_result, self_input);
  EXPECT_GT(first_remote, self_result);
  EXPECT_EQ(commit_sends, expected_sends);

  for (const auto* replica : replicas) {
    const auto& state = replica->core().instances().at(instance).state;
    EXPECT_EQ(state.phase, Phase::committed);
    EXPECT_EQ(state.command, (kv::Payload{command}));
    EXPECT_TRUE(state.dependencies.empty());
    EXPECT_EQ(state.initial_command.has_value(), replica->core().id() != late);
  }
  EXPECT_EQ(replicas[late.value()]->last_status(), DispatchStatus::stale);
  network.verify_replay_complete();
}
TEST(AgreementSimulation, RetainedGraceResumesThroughRunnerEffectsAndReplays) {
  const auto run = [](Network& network) {
    const ReplicaId coordinator{0};
    const InstanceId instance{1, coordinator};
    const kv::Command command = kv::Put{"key", "value"};
    auto& endpoint = ReplicaEndpoint::attach(network, coordinator);
    endpoint.dispatch(Submit{instance, command});
    endpoint.dispatch(PeerMessage{{ReplicaId{1}, coordinator, PreAcceptOk{instance, {}}}});
    endpoint.dispatch(PeerMessage{{ReplicaId{2}, coordinator, PreAcceptOk{instance, {}}}});
    const auto old = endpoint.core().instances().at(instance).initial_grace;
    ASSERT_TRUE(old);
    network.crash(coordinator);
    network.restart(coordinator);
    EXPECT_NE(endpoint.core().instances().at(instance).initial_grace, old);
    endpoint.dispatch(TimerFired{*old});
    EXPECT_EQ(endpoint.last_status(), DispatchStatus::stale);
    network.scheduler().run();
    const auto& record = endpoint.core().instances().at(instance);
    EXPECT_TRUE(record.initial_selection != InitialSelection::collecting);
    EXPECT_FALSE(record.initial_grace);
    EXPECT_EQ(record.state.phase, Phase::accepted);  // Real self-Accept follows slow selection.
    ASSERT_TRUE(record.preaccept_observations);
    EXPECT_EQ(record.preaccept_observations->size(), 3U);
    std::size_t handoffs = 0;
    for (const auto& decision : network.trace().decisions)
      if (decision.kind == DecisionKind::protocol && decision.detail.starts_with("effect/slow-ready/"))
        ++handoffs;
    EXPECT_EQ(handoffs, 1U);
    network.verify_replay_complete();
  };
  Network original; run(original);
  Network replay{parse_trace(serialize(original.trace())), FaultPolicy{}}; run(replay);
  EXPECT_EQ(original.trace(), replay.trace());
}
// These scenarios use real PreAccept/OK traffic and the shared Replica core.
// Their oracles catch premature fast selection, lost mismatching observations,
// changed chosen dependencies, duplicate decisions, and replay divergence.
using Replicas = std::array<ReplicaEndpoint*, Configuration::replicas>;
Replicas attach_all(Network& network) {
  Replicas replicas{};
  for (const auto member : members())
    replicas[member.value()] = &ReplicaEndpoint::attach(network, member);
  return replicas;
}
const ReplicaId owner{0};
const InstanceId target{1, owner};
const kv::Command target_command = kv::Put{"key", "target"};

void check_observed_commits(const Replicas& replicas, const Dependencies& expected) {
  for (const auto* endpoint : replicas) {
    const auto found = endpoint->core().instances().find(target);
    if (found == endpoint->core().instances().end() || found->second.state.phase != Phase::committed)
      continue;
    EXPECT_EQ(found->second.state.command, (kv::Payload{target_command}));
    EXPECT_EQ(found->second.state.dependencies, expected);
    EXPECT_TRUE(found->second.state.ballots.current.initial());
    EXPECT_TRUE(found->second.state.ballots.accepted.initial());
  }
}
void drain_checked(Network& network, const Replicas& replicas, const Dependencies& expected = {}) {
  std::size_t events = 0;
  while (network.scheduler().step()) {
    ASSERT_LT(++events, 100U);
    check_observed_commits(replicas, expected);
  }
  network.verify_replay_complete();
}
std::size_t handoffs(const Network& network) {
  return std::ranges::count_if(network.trace().decisions, [](const Decision& decision) {
    return decision.kind == DecisionKind::protocol && decision.detail.starts_with("effect/slow-ready/");
  });
}
void submit_controlled(Network& network, const Replicas& replicas) {
  // Suppress automatic remote requests, then explicitly schedule real retries.
  for (unsigned peer = 1; peer < Configuration::replicas; ++peer)
    network.set_link(owner, ReplicaId{peer}, true);
  replicas[0]->dispatch(Submit{target, target_command});
  for (unsigned peer = 1; peer < Configuration::replicas; ++peer)
    network.set_link(owner, ReplicaId{peer}, false);
  const auto& record = replicas[0]->core().instances().at(target);
  ASSERT_TRUE(record.preaccept_observations);
  EXPECT_EQ(record.preaccept_observations->size(), 1U);  // Actual immediate self-OK.
}
void request(Network& network, unsigned peer, std::uint64_t delay,
             const Dependencies& proposal = {}, bool duplicate = false) {
  network.send({owner, ReplicaId{peer}, PreAccept{target, target_command, proposal}},
               DeliveryPlan{false, {delay}, duplicate, {delay + 1}});
}
template<class Scenario>
void run_and_replay(Scenario scenario) {
  Network original;
  scenario(original);
  Network replay{parse_trace(serialize(original.trace())), FaultPolicy{}};
  scenario(replay);
  EXPECT_EQ(original.trace(), replay.trace());
}
void expect_no_commit(const Replicas& replicas) {
  for (const auto* endpoint : replicas) {
    const auto found = endpoint->core().instances().find(target);
    if (found != endpoint->core().instances().end()) {
      EXPECT_NE(found->second.state.phase, Phase::committed);
    }
  }
}

TEST(AgreementSimulation, NonemptyInitialDependenciesCommitWithFourOriginalVotesAndReplay) {
  run_and_replay([](Network& network) {
    const auto replicas = attach_all(network);
    const InstanceId dependency{1, ReplicaId{4}};
    for (const auto member : members())
      network.send({ReplicaId{4}, member, Commit{{}, dependency, kv::Put{"key", "prior"}, {}}},
                   DeliveryPlan{false, {0}, false, {0}});
    network.scheduler().run();
    submit_controlled(network, replicas);
    const Dependencies proposal{dependency};
    EXPECT_EQ(replicas[0]->core().instances().at(target).state.initial_dependencies, proposal);
    for (unsigned peer : {1U, 2U, 3U}) request(network, peer, 1, proposal);
    drain_checked(network, replicas, proposal);
    for (const auto* endpoint : replicas)
      EXPECT_EQ(endpoint->core().instances().at(target).state.phase, Phase::committed);
    EXPECT_EQ(replicas[0]->core().instances().at(target).preaccept_observations->size(), 4U);
    EXPECT_EQ(handoffs(network), 0U);
  });
}

TEST(AgreementSimulation, ThreeSurvivorsExpireToOneFrozenHandoffWithoutAnotherInputAndReplay) {
  run_and_replay([](Network& network) {
    const auto replicas = attach_all(network);
    network.crash(ReplicaId{3});
    network.crash(ReplicaId{4});
    submit_controlled(network, replicas);
    request(network, 1, 1);
    request(network, 2, 1);
    drain_checked(network, replicas);
    // C019 adds remote Accept deliveries after the handoff. Preserve the grace
    // bound by checking the handoff event itself, rather than final queue time.
    for (const auto& decision : network.trace().decisions) {
      if (decision.kind == DecisionKind::protocol && decision.detail.starts_with("effect/slow-ready/")) {
        EXPECT_EQ(decision.fields.at(0), 3U);  // Requests1, OK2, grace3.
      }
    }
    const auto& record = replicas[0]->core().instances().at(target);
    EXPECT_TRUE(record.initial_selection != InitialSelection::collecting);
    EXPECT_FALSE(record.initial_grace);
    EXPECT_EQ(*record.preaccept_observations,
              (PreAcceptObservations{{owner, {}}, {ReplicaId{1}, {}}, {ReplicaId{2}, {}}}));
    EXPECT_EQ(handoffs(network), 1U);
    for (unsigned survivor : {0U, 1U, 2U})
      EXPECT_EQ(replicas[survivor]->core().instances().at(target).state.phase, Phase::committed);
  });
}

TEST(AgreementSimulation, TwoSurvivorsAndDuplicateTrafficCannotSelectEitherBranchAndReplay) {
  run_and_replay([](Network& network) {
    const auto replicas = attach_all(network);
    for (unsigned peer : {2U, 3U, 4U}) network.crash(ReplicaId{peer});
    submit_controlled(network, replicas);
    request(network, 1, 1, {}, true);
    drain_checked(network, replicas);
    const auto& record = replicas[0]->core().instances().at(target);
    EXPECT_EQ(record.preaccept_observations->size(), 2U);
    EXPECT_FALSE(record.initial_selection != InitialSelection::collecting);
    EXPECT_FALSE(record.initial_grace);
    EXPECT_EQ(handoffs(network), 0U);
    expect_no_commit(replicas);
  });
}

void mismatch_scenario(Network& network, bool fourth) {
  const auto replicas = attach_all(network);
  const unsigned expanded_peer = fourth ? 3U : 1U;
  const InstanceId conflict{1, ReplicaId{4}};
  network.send({ReplicaId{4}, ReplicaId{expanded_peer},
                Commit{{}, conflict, kv::Put{"key", "conflict"}, {}}},
               DeliveryPlan{false, {0}, false, {0}});
  network.scheduler().run();
  submit_controlled(network, replicas);
  request(network, 1, 1);
  request(network, 2, 1);
  if (fourth) request(network, 3, 1);
  // A late original reply cannot remove the mismatch or reopen fast selection.
  request(network, 4, 5);
  drain_checked(network, replicas, {conflict});
  const auto& record = replicas[0]->core().instances().at(target);
  ASSERT_TRUE(record.preaccept_observations);
  EXPECT_EQ(record.preaccept_observations->size(), fourth ? 4U : 3U);
  EXPECT_EQ(record.preaccept_observations->at(ReplicaId{expanded_peer}), (Dependencies{conflict}));
  EXPECT_TRUE(record.state.initial_dependencies.empty());
  EXPECT_TRUE(record.preaccept_observations->at(owner).empty());  // Frozen local vote is untouched.
  EXPECT_EQ(record.state.dependencies, Dependencies{conflict});  // Real self-Accept installs the union.
  EXPECT_EQ(record.state.phase, Phase::committed);
  EXPECT_TRUE(record.initial_selection != InitialSelection::collecting);
  EXPECT_FALSE(record.initial_grace);
  EXPECT_EQ(handoffs(network), 1U);
  for (const auto* endpoint : replicas)
    EXPECT_EQ(endpoint->core().instances().at(target).state.phase, Phase::committed);
}
TEST(AgreementSimulation, MismatchBeforeThirdResponseSelectsSlowAndReplays) {
  run_and_replay([](Network& network) { mismatch_scenario(network, false); });
}
TEST(AgreementSimulation, FourthExpandedResponseCannotManufactureMatchingSubsetAndReplays) {
  run_and_replay([](Network& network) { mismatch_scenario(network, true); });
}

TEST(AgreementSimulation, SameTimestampFourthReplyAndExpiryRespectBothInsertionOrdersAndReplay) {
  for (const bool reply_first : {false, true}) {
    SCOPED_TRACE(reply_first ? "fourth reply first" : "expiry first");
    run_and_replay([reply_first](Network& network) {
      const auto replicas = attach_all(network);
      submit_controlled(network, replicas);
      request(network, 1, 1);
      request(network, 2, 1);
      if (reply_first) {
        // This request's t2 event predates the t2 replies, so its t3 OK
        // is inserted before the grace those replies will schedule.
        request(network, 3, 2);
      } else {
        // At t1, enqueue a t2 request-producing event behind the two OKs.
        // Its zero-delay request and t3 OK therefore follow grace insertion.
        network.scheduler().schedule_at({1}, "enqueue-fourth-after-first-replies", [&] {
          network.scheduler().schedule_at({2}, "fourth-request", [&] { request(network, 3, 0); });
        });
      }
      drain_checked(network, replicas);
      const auto& record = replicas[0]->core().instances().at(target);
      EXPECT_TRUE(record.initial_selection != InitialSelection::collecting);
      EXPECT_EQ(record.preaccept_observations->size(), reply_first ? 4U : 3U);
      EXPECT_EQ(handoffs(network), reply_first ? 0U : 1U);
      for (const auto* endpoint : replicas)
        EXPECT_EQ(endpoint->core().instances().at(target).state.phase, Phase::committed);
      const auto fourth_input = "input/peer/" + canonical(Envelope{ReplicaId{3}, owner, PreAcceptOk{target, {}}});
      std::size_t fourth_at_three = 0;
      for (const auto& decision : network.trace().decisions)
        if (decision.kind == DecisionKind::protocol && decision.detail == fourth_input && decision.fields[0] == 3)
          ++fourth_at_three;
      EXPECT_EQ(fourth_at_three, 1U);  // Both schedules really hit the tie.
    });
  }
}

TEST(AgreementSimulation, DuplicateRequestsAndLatePreacceptCannotRepeatFastDecisionAndReplay) {
  run_and_replay([](Network& network) {
    const auto replicas = attach_all(network);
    submit_controlled(network, replicas);
    for (unsigned peer : {1U, 2U, 3U}) request(network, peer, 1, {}, true);
    request(network, 4, 6, {}, true);  // Commit arrives before these requests.
    drain_checked(network, replicas);
    EXPECT_EQ(replicas[0]->core().instances().at(target).preaccept_observations->size(), 4U);
    EXPECT_EQ(handoffs(network), 0U);
    for (const auto member : members()) {
      const auto packet = "effect/send/" + canonical(Envelope{owner, member, Commit{{}, target, target_command, {}}});
      EXPECT_EQ(std::ranges::count_if(network.trace().decisions, [&](const Decision& decision) {
        return decision.kind == DecisionKind::protocol && decision.detail == packet;
      }), 1);
      EXPECT_EQ(replicas[member.value()]->core().instances().at(target).state.phase, Phase::committed);
    }
    EXPECT_EQ(replicas[4]->last_status(), DispatchStatus::stale);
    EXPECT_FALSE(replicas[4]->core().instances().at(target).state.initial_command);
  });
}

TEST(AgreementSimulation, CoordinatorCrashBeforeAndAfterSelfCommitPreservesObservedDecisionsAndReplay) {
  for (const bool after_commit : {false, true}) {
    SCOPED_TRACE(after_commit ? "after self commit" : "before decision");
    run_and_replay([after_commit](Network& network) {
      const auto replicas = attach_all(network);
      submit_controlled(network, replicas);
      for (unsigned peer : {1U, 2U, 3U}) request(network, peer, 1);
      if (after_commit) {
        std::size_t events = 0;
        while (replicas[0]->core().instances().at(target).state.phase != Phase::committed) {
          ASSERT_TRUE(network.scheduler().step());
          ASSERT_LT(++events, 20U);
          check_observed_commits(replicas, {});
        }
        // Self delivery completed synchronously. Remote Commit effects are
        // already queued and remain deliverable after the sender crash.
        for (unsigned peer : {1U, 2U, 3U, 4U}) {
          const auto found = replicas[peer]->core().instances().find(target);
          EXPECT_TRUE(found == replicas[peer]->core().instances().end() || found->second.state.phase != Phase::committed);
        }
        network.crash(owner);
      } else {
        network.scheduler().schedule_at({1}, "crash-before-replies", [&] { network.crash(owner); });
      }
      drain_checked(network, replicas);
      EXPECT_FALSE(replicas[0]->core().active());
      EXPECT_EQ(handoffs(network), 0U);
      if (after_commit) {
        for (const auto* endpoint : replicas)
          EXPECT_EQ(endpoint->core().instances().at(target).state.phase, Phase::committed);
      } else {
        EXPECT_FALSE(replicas[0]->core().instances().at(target).initial_selection != InitialSelection::collecting);
        expect_no_commit(replicas);
      }
    });
  }
}
}  // namespace
}  // namespace egalis::sim
