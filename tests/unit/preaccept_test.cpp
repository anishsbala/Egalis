#include "egalis/protocol/preaccept.hpp"
#include "egalis/protocol/replica.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace egalis::protocol {
namespace {
const ReplicaId owner{0}, receiver{1};
const InstanceId id{1, owner};
PeerMessage request(PreAccept message, ReplicaId destination = receiver) {
  return {{message.id.coordinator(), destination, std::move(message)}};
}
PreAccept proposal() { return {id, kv::Put{"k", "v"}, {}}; }
}
TEST(PreAcceptTransition, ExactInitialBallotAndPhaseGuard) {
  for (const auto phase : {Phase::initial, Phase::preaccepted, Phase::accepted, Phase::committed}) {
    InstanceState state;
    state.phase = phase;
    EXPECT_EQ(initial_preaccept_allowed(state), phase == Phase::initial);
    state.ballots.current = Ballot::recovery(1, receiver);
    EXPECT_FALSE(initial_preaccept_allowed(state));
  }
}
TEST(PreAcceptTransition, InitializesMetadataAndExpandsWithoutOverwritingInitialProposal) {
  Replica replica{receiver};
  const InstanceId local{10, receiver}, unknown{7, ReplicaId{4}};
  replica.dispatch(Submit{local, kv::Get{"k"}});
  const auto result = replica.dispatch(request({id, kv::Put{"k", "v"}, {unknown}}));
  ASSERT_EQ(result.status, DispatchStatus::processed);
  const auto& state = replica.instances().at(id).state;
  EXPECT_EQ(state.phase, Phase::preaccepted);
  EXPECT_TRUE(state.ballots.current.initial());
  EXPECT_TRUE(state.ballots.accepted.initial());
  EXPECT_EQ(state.initial_command, (kv::Command{kv::Put{"k", "v"}}));
  EXPECT_EQ(state.initial_dependencies, (Dependencies{unknown}));
  EXPECT_EQ(state.dependencies, (Dependencies{local, unknown}));
  ASSERT_EQ(result.effects.size(), 1U);
  const auto& reply = std::get<SendMessage>(result.effects[0]).envelope;
  EXPECT_EQ(reply, (Envelope{receiver, owner, PreAcceptOk{id, {local, unknown}}}));
  EXPECT_NO_THROW(validate(reply));
}
TEST(PreAcceptTransition, InvalidSenderAndSelfDependenciesCannotCreateOrConsumeState) {
  Replica replica{receiver};
  EXPECT_THROW(replica.dispatch(PeerMessage{{receiver, receiver, proposal()}}), std::invalid_argument);
  EXPECT_THROW(replica.dispatch(request({id, kv::Get{"k"}, {id}})), std::invalid_argument);
  EXPECT_TRUE(replica.instances().empty());
  const auto allocated = replica.dispatch(SubmitCommand{kv::Get{"k"}});
  EXPECT_EQ(std::get<SubmissionRegistered>(allocated.effects[0]).instance, (InstanceId{1, receiver}));
}
TEST(PreAcceptTransition, PendingSelfProposalCannotBeReplacedByAnotherPayloadOrDependencySet) {
  Replica replica{owner};
  const auto submitted = replica.dispatch(SubmitCommand{kv::Put{"k", "v"}});
  const auto original = std::get<PreAccept>(std::get<SendMessage>(submitted.effects[1]).envelope.message);
  auto different = original;
  different.command = kv::Get{"other"};
  EXPECT_EQ(replica.dispatch(request(different, owner)).status, DispatchStatus::rejected);
  different = original;
  different.dependencies.insert(InstanceId{2, receiver});
  EXPECT_EQ(replica.dispatch(request(different, owner)).status, DispatchStatus::rejected);
  EXPECT_EQ(replica.instances().at(id).state.phase, Phase::initial);
  EXPECT_EQ(replica.instances().at(id).state.command, (kv::Payload{original.command}));
  EXPECT_EQ(replica.dispatch(request(original, owner)).status, DispatchStatus::processed);
}
TEST(PreAcceptTransition, PhaseGuardPreventsSecondTransition) {
  Replica replica{receiver};
  const auto first = replica.dispatch(request(proposal()));
  const auto before = replica.instances().at(id).state.dependencies;
  const auto duplicate = replica.dispatch(request(proposal()));
  EXPECT_EQ(duplicate.status, DispatchStatus::stale);
  EXPECT_TRUE(duplicate.effects.empty());
  EXPECT_EQ(replica.instances().at(id).state.phase, Phase::preaccepted);
  EXPECT_EQ(replica.instances().at(id).state.dependencies, before);
  EXPECT_EQ(first.effects.size(), 1U);
}
TEST(PreAcceptTransition, ObservedLocallyOwnedIdsAdvanceTheSameAllocator) {
  Replica replica{owner};
  replica.dispatch(request({InstanceId{100, owner}, kv::Get{"k"}, {}}, owner));
  const auto next = replica.dispatch(SubmitCommand{kv::Get{"k"}});
  EXPECT_EQ(std::get<SubmissionRegistered>(next.effects[0]).instance, (InstanceId{101, owner}));
}
TEST(PreAcceptTransition, SameKeyReadsRemainCommuting) {
  Replica replica{receiver};
  replica.dispatch(Submit{InstanceId{1, receiver}, kv::Get{"k"}});
  replica.dispatch(request({id, kv::Get{"k"}, {}}));
  EXPECT_TRUE(replica.instances().at(id).state.dependencies.empty());
}
}  // namespace egalis::protocol
