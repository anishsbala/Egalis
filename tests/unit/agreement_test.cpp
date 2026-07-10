#include "egalis/protocol/commit.hpp"
#include "egalis/protocol/replica.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <utility>

namespace egalis::protocol {
namespace {
const ReplicaId owner{0}, peer{1};
const InstanceId id{1, owner}, dependency{2, peer}, newer_conflict{3, peer};
const kv::Command command = kv::Put{"key", "value"};

Envelope submit(Replica& replica, InstanceId instance = id,
                const kv::Command& submitted = command) {
  const auto result = replica.dispatch(Submit{instance, submitted});
  return std::get<SendMessage>(result.effects.at(1)).envelope;
}
Envelope preaccept_self(Replica& replica, const Envelope& preaccept) {
  const auto result = replica.dispatch(PeerMessage{preaccept});
  return std::get<SendMessage>(result.effects.at(0)).envelope;
}
PeerMessage observation(ReplicaId sender, InstanceId instance, Dependencies dependencies = {}) {
  return {{sender, owner, PreAcceptOk{instance, std::move(dependencies)}}};
}
void expect_observation(Replica& replica, const PeerMessage& reply, DispatchStatus expected) {
  const auto result = replica.dispatch(reply);
  EXPECT_EQ(result.status, expected);
  EXPECT_TRUE(result.effects.empty());
}
void learn_conflict(Replica& replica, InstanceId conflict) {
  ASSERT_EQ(replica.dispatch(PeerMessage{{conflict.coordinator(), owner,
      PreAccept{conflict, command, {}}}}).status, DispatchStatus::processed);
}
PeerMessage commit(InstanceId instance, ReplicaId destination, Dependencies dependencies = {},
                   Ballot ballot = {}, const kv::Payload& payload = command) {
  return {{ballot.owner(instance), destination,
           Commit{ballot, instance, payload, std::move(dependencies)}}};
}
}  // namespace

TEST(InitialAgreement, CountsTheRealSelfReplyExactlyOnce) {
  Replica replica{owner};
  const auto preaccept = submit(replica);
  const auto& observations = replica.instances().at(id).preaccept_observations;
  EXPECT_EQ(observations, PreAcceptObservations{});
  const auto reply = preaccept_self(replica, preaccept);
  EXPECT_EQ(reply, (Envelope{owner, owner, PreAcceptOk{id, {}}}));
  EXPECT_EQ(observations, PreAcceptObservations{});
  expect_observation(replica, PeerMessage{reply}, DispatchStatus::processed);
  expect_observation(replica, PeerMessage{reply}, DispatchStatus::replayed);
  ASSERT_TRUE(observations);
  EXPECT_EQ(*observations, (PreAcceptObservations{{owner, {}}}));
  EXPECT_TRUE(observations_match_initial_dependencies(*observations, replica.instances().at(id).state));
}

TEST(InitialAgreement, ThreeCopiesOfOneSenderYieldOneObservation) {
  Replica replica{owner};
  preaccept_self(replica, submit(replica));
  const auto reply = observation(peer, id, {dependency});
  expect_observation(replica, reply, DispatchStatus::processed);
  expect_observation(replica, reply, DispatchStatus::replayed);
  expect_observation(replica, reply, DispatchStatus::replayed);
  EXPECT_EQ(replica.instances().at(id).preaccept_observations,
            (PreAcceptObservations{{peer, {dependency}}}));
}

TEST(InitialAgreement, ConflictingDuplicateLeavesTheFirstObservationAndRecordIntact) {
  Replica replica{owner};
  const auto preaccept = submit(replica);
  const auto self_reply = preaccept_self(replica, preaccept);
  const auto first = observation(peer, id, {dependency});
  expect_observation(replica, first, DispatchStatus::processed);
  replica.arm_timer(id, {4});
  const auto before = replica.instances().at(id);

  expect_observation(replica, observation(peer, id, {newer_conflict}), DispatchStatus::rejected);
  expect_observation(replica, first, DispatchStatus::replayed);
  const auto& after = replica.instances().at(id);
  EXPECT_EQ(after.preaccept_observations, (PreAcceptObservations{{peer, {dependency}}}));
  EXPECT_EQ(after.preaccept_observations, before.preaccept_observations);
  EXPECT_EQ(after.state.command, before.state.command);
  EXPECT_EQ(after.state.dependencies, before.state.dependencies);
  EXPECT_EQ(after.state.initial_command, before.state.initial_command);
  EXPECT_EQ(after.state.initial_dependencies, before.state.initial_dependencies);
  EXPECT_EQ(after.state.ballots.current, before.state.ballots.current);
  EXPECT_EQ(after.state.ballots.accepted, before.state.ballots.accepted);
  EXPECT_EQ(after.state.phase, before.state.phase);
  EXPECT_EQ(after.timer, before.timer);
  const auto replay = replica.dispatch(PeerMessage{preaccept});
  EXPECT_EQ(replay.status, DispatchStatus::replayed);
  ASSERT_EQ(replay.effects.size(), 1U);
  EXPECT_EQ(std::get<SendMessage>(replay.effects.front()).envelope, self_reply);
}

TEST(InitialAgreement, UnknownAndUnsubmittedIdsCannotCollect) {
  Replica replica{owner};
  preaccept_self(replica, submit(replica));
  const InstanceId unknown{7, owner}, unsubmitted{8, owner};
  expect_observation(replica, observation(peer, unknown), DispatchStatus::stale);
  EXPECT_EQ(replica.instances().size(), 1U);
  replica.dispatch(PeerMessage{{owner, owner, PreAccept{unsubmitted, command, {}}}});
  ASSERT_EQ(replica.instances().at(unsubmitted).state.phase, Phase::preaccepted);
  expect_observation(replica, observation(peer, unsubmitted), DispatchStatus::stale);
  EXPECT_EQ(replica.instances().size(), 2U);
  EXPECT_FALSE(replica.instances().at(unsubmitted).preaccept_observations);
  EXPECT_EQ(replica.instances().at(id).preaccept_observations, PreAcceptObservations{});
  expect_observation(replica, observation(peer, id), DispatchStatus::processed);
}

TEST(InitialAgreement, StructuralDestinationAndSelfDependencyChecksStillApply) {
  Replica replica{owner};
  preaccept_self(replica, submit(replica));
  EXPECT_THROW(replica.dispatch(PeerMessage{{peer, peer, PreAcceptOk{id, {}}}}),
               std::invalid_argument);
  EXPECT_THROW(replica.dispatch(PeerMessage{{peer, peer, PreAcceptOk{dependency, {}}}}),
               std::invalid_argument);
  EXPECT_THROW(replica.dispatch(observation(peer, id, {id})), std::invalid_argument);
  EXPECT_EQ(replica.instances().at(id).preaccept_observations, PreAcceptObservations{});
  expect_observation(replica, observation(peer, id, {dependency}), DispatchStatus::processed);
}

TEST(InitialAgreement, PendingSelfPreacceptCannotCollect) {
  Replica replica{owner};
  const auto preaccept = submit(replica);
  const auto reply = observation(peer, id, {dependency});
  expect_observation(replica, reply, DispatchStatus::stale);
  EXPECT_EQ(replica.instances().at(id).state.phase, Phase::initial);
  EXPECT_EQ(replica.instances().at(id).preaccept_observations, PreAcceptObservations{});
  preaccept_self(replica, preaccept);
  expect_observation(replica, reply, DispatchStatus::processed);
}

TEST(InitialAgreement, ConcurrentSubmissionsCollectIndependently) {
  Replica replica{owner};
  const InstanceId other{2, owner};
  const auto original_preaccept = submit(replica);
  const auto other_preaccept = submit(replica, other, kv::Get{"other"});
  preaccept_self(replica, original_preaccept);
  preaccept_self(replica, other_preaccept);
  expect_observation(replica, observation(peer, id, {dependency}), DispatchStatus::processed);
  expect_observation(replica, observation(peer, other, {newer_conflict}), DispatchStatus::processed);
  expect_observation(replica, observation(peer, id, {newer_conflict}), DispatchStatus::rejected);
  expect_observation(replica, observation(peer, other, {newer_conflict}), DispatchStatus::replayed);
  EXPECT_EQ(replica.instances().at(id).preaccept_observations,
            (PreAcceptObservations{{peer, {dependency}}}));
  EXPECT_EQ(replica.instances().at(other).preaccept_observations,
            (PreAcceptObservations{{peer, {newer_conflict}}}));
}

TEST(InitialAgreement, FourEqualExpandedObservationsPreserveTheOriginalProposal) {
  Replica replica{owner};
  learn_conflict(replica, dependency);
  const auto preaccept = submit(replica);
  EXPECT_EQ(std::get<PreAccept>(preaccept.message).dependencies, (Dependencies{dependency}));
  // Direct core dispatch lets the test expand self's reply after submission.
  learn_conflict(replica, newer_conflict);
  const Dependencies expanded{dependency, newer_conflict};
  expect_observation(replica, PeerMessage{preaccept_self(replica, preaccept)}, DispatchStatus::processed);
  for (const auto sender : {peer, ReplicaId{2}, ReplicaId{3}})
    expect_observation(replica, observation(sender, id, expanded), DispatchStatus::processed);
  const auto& record = replica.instances().at(id);
  ASSERT_TRUE(record.preaccept_observations);
  EXPECT_EQ(*record.preaccept_observations, (PreAcceptObservations{{owner, expanded},
      {peer, expanded}, {ReplicaId{2}, expanded}, {ReplicaId{3}, expanded}}));
  const auto& state = record.state;
  EXPECT_FALSE(observations_match_initial_dependencies(*record.preaccept_observations, state));
  EXPECT_EQ(state.phase, Phase::preaccepted);
  EXPECT_EQ(state.command, (kv::Payload{command}));
  EXPECT_EQ(state.initial_command, command);
  EXPECT_EQ(state.initial_dependencies, (Dependencies{dependency}));
  EXPECT_EQ(state.dependencies, expanded);
}

TEST(InitialAgreement, GuardNeedsPreacceptedPhaseAndCurrentBallotZero) {
  for (const auto phase : {Phase::initial, Phase::preaccepted, Phase::accepted, Phase::committed}) {
    InstanceState state;
    state.phase = phase;
    EXPECT_EQ(initial_observation_allowed(state), phase == Phase::preaccepted);
    state.ballots.current = Ballot::recovery(1, owner);
    EXPECT_FALSE(initial_observation_allowed(state));
  }
}

TEST(InitialAgreement, FourthDistinctOriginalVoteSealsOneCommitBeforeSelfDelivery) {
  Replica replica{owner};
  learn_conflict(replica, dependency);
  const auto preaccept = submit(replica);
  const auto self_reply = preaccept_self(replica, preaccept);
  const Dependencies proposed{dependency};
  const auto first = observation(peer, id, proposed);
  const auto fourth = observation(ReplicaId{3}, id, proposed);
  expect_observation(replica, PeerMessage{self_reply}, DispatchStatus::processed);
  expect_observation(replica, first, DispatchStatus::processed);
  expect_observation(replica, observation(ReplicaId{2}, id, proposed), DispatchStatus::processed);
  expect_observation(replica, first, DispatchStatus::replayed);
  ASSERT_EQ(replica.instances().at(id).state.phase, Phase::preaccepted);
  ASSERT_EQ(replica.instances().at(id).preaccept_observations->size(), 3U);

  const auto decision = replica.dispatch(fourth);
  EXPECT_EQ(decision.status, DispatchStatus::processed);
  ASSERT_EQ(decision.effects.size(), 5U);
  for (unsigned index = 0; index < 5; ++index) {
    EXPECT_EQ(std::get<SendMessage>(decision.effects[index]).envelope,
              commit(id, ReplicaId{index}, proposed).envelope);
  }
  const auto& record = replica.instances().at(id);
  EXPECT_EQ(record.state.phase, Phase::preaccepted);
  ASSERT_EQ(record.preaccept_observations->size(), 4U);
  const auto selected = *record.preaccept_observations;
  expect_observation(replica, fourth, DispatchStatus::stale);
  expect_observation(replica, observation(ReplicaId{4}, id, {dependency, newer_conflict}),
                     DispatchStatus::stale);
  EXPECT_EQ(record.preaccept_observations, selected);

  const auto self_commit = std::get<SendMessage>(decision.effects.front()).envelope;
  EXPECT_EQ(replica.dispatch(PeerMessage{self_commit}), (DispatchResult{DispatchStatus::processed, {}}));
  EXPECT_EQ(record.state.phase, Phase::committed);
  EXPECT_EQ(record.state.dependencies, proposed);
  EXPECT_EQ(record.preaccept_observations, selected);
  expect_observation(replica, first, DispatchStatus::stale);
}

TEST(InitialAgreement, ASelectedQuorumCannotDiscardAnExpandedObservation) {
  Replica replica{owner};
  expect_observation(replica, PeerMessage{preaccept_self(replica, submit(replica))},
                     DispatchStatus::processed);
  expect_observation(replica, observation(peer, id, {dependency}), DispatchStatus::processed);
  for (const auto sender : {ReplicaId{2}, ReplicaId{3}, ReplicaId{4}})
    expect_observation(replica, observation(sender, id), DispatchStatus::processed);
  const auto& record = replica.instances().at(id);
  ASSERT_EQ(record.preaccept_observations->size(), 5U);
  EXPECT_EQ(record.preaccept_observations->at(peer), (Dependencies{dependency}));
  EXPECT_EQ(record.state.phase, Phase::preaccepted);
}

TEST(CommitAgreement, BallotZeroMaterializesWithoutPriorPreaccept) {
  Replica replica{peer};
  EXPECT_EQ(replica.dispatch(commit(id, peer, {dependency})),
            (DispatchResult{DispatchStatus::processed, {}}));
  ASSERT_TRUE(replica.instances().contains(id));
  const auto& record = replica.instances().at(id);
  EXPECT_EQ(record.state.ballots.current, Ballot{});
  EXPECT_EQ(record.state.ballots.accepted, Ballot{});
  EXPECT_EQ(record.state.phase, Phase::committed);
  EXPECT_EQ(record.state.command, (kv::Payload{command}));
  EXPECT_EQ(record.state.dependencies, (Dependencies{dependency}));
  EXPECT_FALSE(record.state.initial_command);
  EXPECT_TRUE(record.state.initial_dependencies.empty());
  EXPECT_FALSE(record.preaccept_observations);
  EXPECT_EQ(replica.dispatch(PeerMessage{{owner, peer, PreAccept{id, command, {dependency}}}}),
            (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_FALSE(record.state.initial_command);
  EXPECT_TRUE(record.state.initial_dependencies.empty());
}

TEST(CommitAgreement, HigherBallotCannotCreateAnInstanceOrJoinAnExistingOne) {
  Replica replica{owner};
  const InstanceId future{20, owner};
  const Ballot recovery = Ballot::recovery(1, peer);
  EXPECT_EQ(replica.dispatch(commit(future, owner, {}, recovery)),
            (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_TRUE(replica.instances().empty());
  const auto preaccept = submit(replica);
  const auto reply = preaccept_self(replica, preaccept);
  EXPECT_EQ(replica.dispatch(commit(id, owner, {dependency}, recovery)),
            (DispatchResult{DispatchStatus::stale, {}}));
  const auto& state = replica.instances().at(id).state;
  EXPECT_EQ(state.ballots.current, Ballot{});
  EXPECT_EQ(state.ballots.accepted, Ballot{});
  EXPECT_EQ(state.phase, Phase::preaccepted);
  EXPECT_EQ(replica.dispatch(PeerMessage{preaccept}).effects, (Effects{SendMessage{reply}}));
  const auto result = replica.dispatch(SubmitCommand{kv::Get{"another"}});
  EXPECT_EQ(std::get<SubmissionRegistered>(result.effects.front()).instance, (InstanceId{2, owner}));
}

TEST(CommitAgreement, ExactCurrentBallotGuardAppliesInEveryPhase) {
  const auto earlier = Ballot::recovery(1, owner);
  const auto current = Ballot::recovery(2, peer);
  const auto different_owner = Ballot::recovery(2, owner);
  const auto later = Ballot::recovery(3, owner);
  for (const auto phase : {Phase::initial, Phase::preaccepted, Phase::accepted, Phase::committed}) {
    InstanceState state;
    state.phase = phase;
    state.ballots = {current, {}};
    EXPECT_TRUE(commit_allowed(state, current));
    EXPECT_FALSE(commit_allowed(state, Ballot{}));
    EXPECT_FALSE(commit_allowed(state, earlier));
    EXPECT_FALSE(commit_allowed(state, different_owner));
    EXPECT_FALSE(commit_allowed(state, later));
  }
}

TEST(CommitAgreement, InstallChosenDependenciesOverExpandedLocalDependencies) {
  Replica replica{ReplicaId{4}};
  const auto recipient = replica.id();
  const kv::Command original = kv::Put{"key", "local context"};
  const InstanceId proposed_dependency{6, peer};
  replica.dispatch(PeerMessage{{peer, recipient, PreAccept{dependency, original, {}}}});
  const auto result = replica.dispatch(PeerMessage{{owner, recipient,
      PreAccept{id, original, {proposed_dependency}}}});
  ASSERT_EQ(result.status, DispatchStatus::processed);
  const auto& state = replica.instances().at(id).state;
  ASSERT_EQ(state.dependencies, (Dependencies{dependency, proposed_dependency}));

  const auto decision = commit(id, recipient, {proposed_dependency});
  EXPECT_EQ(replica.dispatch(decision), (DispatchResult{DispatchStatus::processed, {}}));
  EXPECT_EQ(state.phase, Phase::committed);
  EXPECT_EQ(state.dependencies, (Dependencies{proposed_dependency}));
  EXPECT_EQ(state.command, (kv::Payload{command}));
  EXPECT_EQ(state.initial_command, original);
  EXPECT_EQ(state.initial_dependencies, (Dependencies{proposed_dependency}));
}

TEST(CommitAgreement, EquivalentDuplicatesReplayAndContradictionsPreserveTheDecision) {
  Replica replica{peer};
  const auto decision = commit(id, peer, {dependency});
  ASSERT_EQ(replica.dispatch(decision).status, DispatchStatus::processed);
  EXPECT_EQ(replica.dispatch(decision), (DispatchResult{DispatchStatus::replayed, {}}));
  EXPECT_EQ(replica.dispatch(commit(id, peer)), (DispatchResult{DispatchStatus::rejected, {}}));
  EXPECT_EQ(replica.dispatch(commit(id, peer, {dependency}, {}, kv::Command{kv::Get{"key"}})),
            (DispatchResult{DispatchStatus::rejected, {}}));
  const auto& state = replica.instances().at(id).state;
  EXPECT_EQ(state.phase, Phase::committed);
  EXPECT_EQ(state.command, (kv::Payload{command}));
  EXPECT_EQ(state.dependencies, (Dependencies{dependency}));
}

TEST(CommitAgreement, LocalCommitAdvancesTheSharedInstanceAllocator) {
  Replica replica{owner};
  const InstanceId learned{7, owner};
  ASSERT_EQ(replica.dispatch(commit(learned, owner)).status, DispatchStatus::processed);
  const auto submitted = replica.dispatch(SubmitCommand{kv::Get{"another"}});
  EXPECT_EQ(std::get<SubmissionRegistered>(submitted.effects.front()).instance,
            (InstanceId{8, owner}));
  EXPECT_EQ(replica.instances().at(learned).state.phase, Phase::committed);
}

TEST(CommitAgreement, MalformedMessagesCannotAllocateOrChangeProtocolState) {
  Replica replica{peer};
  ASSERT_EQ(replica.dispatch(commit(id, peer)).status, DispatchStatus::processed);
  const InstanceId unknown{2, owner};
  auto wrong_sender = commit(unknown, peer);
  wrong_sender.envelope.source = peer;
  EXPECT_THROW(replica.dispatch(wrong_sender), std::invalid_argument);
  EXPECT_THROW(replica.dispatch(commit(unknown, owner)), std::invalid_argument);
  EXPECT_THROW(replica.dispatch(commit(id, peer, {id})), std::invalid_argument);
  EXPECT_THROW(replica.dispatch(commit(id, peer, {}, {}, kv::Payload{})), std::invalid_argument);
  EXPECT_THROW(replica.dispatch(commit(id, peer, {}, {}, kv::Nop{})), std::invalid_argument);
  ASSERT_EQ(replica.instances().size(), 1U);
  const auto& state = replica.instances().at(id).state;
  EXPECT_EQ(state.phase, Phase::committed);
  EXPECT_EQ(state.command, (kv::Payload{command}));
  EXPECT_TRUE(state.dependencies.empty());
}
}  // namespace egalis::protocol
