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
}  // namespace egalis::protocol
