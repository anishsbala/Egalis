#include "egalis/protocol/replica.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace egalis::protocol {
namespace {
const ReplicaId owner{0};
const InstanceId id{1, owner}, first{1, ReplicaId{1}}, second{2, ReplicaId{2}},
    third{3, ReplicaId{3}};
const kv::Command command = kv::Put{"key", "value"};
void start(Replica& replica) {
  const auto submit = replica.dispatch(Submit{id, command});
  const auto preaccept = replica.dispatch(PeerMessage{std::get<SendMessage>(submit.effects[1]).envelope});
  replica.dispatch(PeerMessage{std::get<SendMessage>(preaccept.effects[0]).envelope});
}
DispatchResult reply(Replica& replica, ReplicaId sender, Dependencies dependencies = {}) {
  return replica.dispatch(PeerMessage{{sender, owner, PreAcceptOk{id, std::move(dependencies)}}});
}
SlowPathReady select_slow(Replica& replica) {
  start(replica);
  reply(replica, ReplicaId{1}, {first, second});
  const auto result = reply(replica, ReplicaId{2}, {second, third});
  return std::get<SlowPathReady>(result.effects.back());
}
TEST(SlowProposal, FreezesWholeUnionWithoutReplacingLocalVoteOrInitialProposal) {
  Replica replica{owner};
  const auto ready = select_slow(replica);
  const auto selected = replica.dispatch(Event{ready});
  ASSERT_EQ(selected.effects.size(), Configuration::replicas);
  EXPECT_EQ(selected.status, DispatchStatus::processed);
  const Accept expected{{}, id, command, {first, second, third}};
  for (unsigned index = 0; index < Configuration::replicas; ++index) {
    const auto& envelope = std::get<SendMessage>(selected.effects[index]).envelope;
    EXPECT_EQ(envelope, (Envelope{owner, ReplicaId{index}, expected}));
  }
  const auto& record = replica.instances().at(id);
  ASSERT_TRUE(record.slow_proposal);
  EXPECT_EQ(record.slow_proposal->dependencies, (Dependencies{first, second, third}));
  EXPECT_EQ(record.slow_proposal->command, (kv::Payload{command}));
  EXPECT_EQ(record.state.initial_command, command);
  EXPECT_TRUE(record.state.initial_dependencies.empty());
  EXPECT_TRUE(record.state.dependencies.empty());
  EXPECT_EQ(record.state.phase, Phase::preaccepted);
  EXPECT_EQ(record.preaccept_observations, ready.observations);
  EXPECT_EQ(replica.dispatch(Event{ready}), (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_EQ(reply(replica, ReplicaId{4}, {}).status, DispatchStatus::stale);
  EXPECT_EQ(record.slow_proposal->dependencies, expected.dependencies);
}
TEST(SlowProposal, RejectsTamperedSnapshotWithoutConsumingAuthenticContinuation) {
  Replica replica{owner};
  const auto ready = select_slow(replica);
  auto tampered = ready;
  tampered.observations.at(ReplicaId{1}).clear();
  EXPECT_EQ(replica.dispatch(Event{tampered}), (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_FALSE(replica.instances().at(id).slow_proposal);
  EXPECT_EQ(replica.dispatch(Event{ready}).effects.size(), Configuration::replicas);
}
TEST(SlowProposal, FastSelectionCannotAuthorizeForgedHandoffBeforeSelfCommit) {
  Replica replica{owner};
  start(replica);
  reply(replica, ReplicaId{1}); reply(replica, ReplicaId{2});
  const auto fast = reply(replica, ReplicaId{3});
  ASSERT_EQ(fast.effects.size(), 1U + Configuration::replicas);
  ASSERT_EQ(replica.instances().at(id).state.phase, Phase::preaccepted);
  const SlowPathReady forged{id, {}, *replica.instances().at(id).preaccept_observations};
  EXPECT_EQ(replica.dispatch(Event{forged}), (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_FALSE(replica.instances().at(id).slow_proposal);
}
TEST(SlowProposal, UnselectedForeignAndSupersededContinuationsAreInert) {
  Replica replica{owner};
  const SlowPathReady unknown{id, {}, {{owner, {}}, {ReplicaId{1}, {}}, {ReplicaId{2}, {}}}};
  EXPECT_EQ(replica.dispatch(Event{unknown}).status, DispatchStatus::stale);
  Replica collecting{owner};
  start(collecting);
  EXPECT_EQ(collecting.dispatch(Event{unknown}).status, DispatchStatus::stale);
  EXPECT_FALSE(collecting.instances().at(id).slow_proposal);
  const auto ready = select_slow(replica);
  Replica foreign{ReplicaId{1}};
  EXPECT_EQ(foreign.dispatch(Event{ready}).status, DispatchStatus::stale);
  replica.dispatch(PeerMessage{{owner, owner, Commit{{}, id, command, {first}}}});
  EXPECT_EQ(replica.dispatch(Event{ready}), (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_FALSE(replica.instances().at(id).slow_proposal);
}
TEST(SlowProposal, SuspendResumeCannotRebroadcastAnAlreadyFrozenProposal) {
  Replica replica{owner};
  const auto ready = select_slow(replica);
  ASSERT_EQ(replica.dispatch(Event{ready}).effects.size(), Configuration::replicas);
  const auto before = replica.instances().at(id).slow_proposal;
  replica.suspend();
  EXPECT_THROW(replica.dispatch(Event{ready}), std::logic_error);
  EXPECT_TRUE(replica.resume().empty());
  EXPECT_EQ(replica.dispatch(Event{ready}), (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_EQ(replica.instances().at(id).slow_proposal, before);
}

DispatchResult accept(Replica& replica, Ballot ballot = {}, Dependencies dependencies = {},
                      kv::Payload payload = command, InstanceId instance = id) {
  return replica.dispatch(PeerMessage{{ballot.owner(instance), replica.id(),
      Accept{ballot, instance, std::move(payload), std::move(dependencies)}}});
}
TEST(AcceptTransition, MaterializesUnknownAndAdvancesLocalAllocator) {
  Replica replica{owner};
  const InstanceId high{40, owner};
  const auto result = accept(replica, {}, {first}, command, high);
  ASSERT_EQ(result.status, DispatchStatus::processed);
  ASSERT_EQ(result.effects.size(), 1U);
  EXPECT_EQ(std::get<SendMessage>(result.effects[0]).envelope,
      (Envelope{owner, owner, AcceptOk{{}, high}}));
  const auto& state = replica.instances().at(high).state;
  EXPECT_EQ(state.phase, Phase::accepted);
  EXPECT_EQ(state.command, kv::Payload{command});
  EXPECT_EQ(state.dependencies, Dependencies{first});
  EXPECT_FALSE(state.initial_command);
  EXPECT_TRUE(state.initial_dependencies.empty());
  const auto submitted = replica.dispatch(SubmitCommand{command});
  EXPECT_EQ(std::get<SubmissionRegistered>(submitted.effects[0]).instance, (InstanceId{41, owner}));
}
TEST(AcceptTransition, InstallsSelectedUnionEvenWhenPreacceptedDependenciesShrink) {
  Replica replica{owner};
  replica.dispatch(PeerMessage{{ReplicaId{1}, owner, PreAccept{first, command, {}}}});
  replica.dispatch(PeerMessage{{owner, owner, PreAccept{id, command, {second}}}});
  ASSERT_EQ(replica.instances().at(id).state.dependencies, (Dependencies{first, second}));
  ASSERT_EQ(accept(replica, {}, {second}).status, DispatchStatus::processed);
  const auto& state = replica.instances().at(id).state;
  EXPECT_EQ(state.dependencies, Dependencies{second});
  EXPECT_EQ(state.initial_dependencies, Dependencies{second});
  EXPECT_EQ(state.initial_command, command);
  EXPECT_EQ(state.phase, Phase::accepted);
}
TEST(AcceptTransition, ReplaysIdenticalAndRejectsEqualBallotContradictionsAtomically) {
  Replica replica{owner};
  const auto ballot = Ballot::recovery(1, ReplicaId{2});
  ASSERT_EQ(accept(replica, ballot, {first}).status, DispatchStatus::processed);
  const auto timer = replica.arm_timer(id, {7});
  const auto replay = accept(replica, ballot, {first});
  EXPECT_EQ(replay.status, DispatchStatus::replayed);
  ASSERT_EQ(replay.effects.size(), 1U);
  EXPECT_EQ(std::get<SendMessage>(replay.effects[0]).envelope,
      (Envelope{owner, ReplicaId{2}, AcceptOk{ballot, id}}));
  EXPECT_EQ(accept(replica, ballot, {second}), (DispatchResult{DispatchStatus::rejected, {}}));
  EXPECT_EQ(accept(replica, ballot, {first}, kv::Nop{}),
      (DispatchResult{DispatchStatus::rejected, {}}));
  const auto& record = replica.instances().at(id);
  EXPECT_EQ(record.state.dependencies, Dependencies{first});
  EXPECT_EQ(record.state.ballots.accepted, ballot);
  EXPECT_EQ(record.timer, std::get<ScheduleTimer>(timer.back()).guard);
  EXPECT_EQ(accept(replica), (DispatchResult{DispatchStatus::stale, {}}));
}
TEST(AcceptTransition, UsesCurrentRatherThanAcceptedBallotAndExactCommittedGuard) {
  Replica replica{owner};
  replica.dispatch(PeerMessage{{owner, owner, Commit{{}, id, command, {first}}}});
  EXPECT_EQ(accept(replica, {}, {first}), (DispatchResult{DispatchStatus::stale, {}}));
  const auto ballot = Ballot::recovery(1, ReplicaId{2});
  EXPECT_EQ(accept(replica, ballot, {second}), (DispatchResult{DispatchStatus::rejected, {}}));
  ASSERT_EQ(accept(replica, ballot, {first}).status, DispatchStatus::processed);
  EXPECT_EQ(replica.instances().at(id).state.phase, Phase::accepted);
  EXPECT_EQ(replica.instances().at(id).committed_decision->dependencies, Dependencies{first});
  EXPECT_EQ(replica.instances().at(id).committed_decision->command, kv::Payload{command});
  EXPECT_EQ(replica.dispatch(PeerMessage{{ReplicaId{2}, owner,
      Commit{ballot, id, kv::Nop{}, {first}}}}), (DispatchResult{DispatchStatus::rejected, {}}));
  EXPECT_EQ(replica.dispatch(PeerMessage{{ReplicaId{2}, owner,
      Commit{ballot, id, command, {second}}}}), (DispatchResult{DispatchStatus::rejected, {}}));
  EXPECT_EQ(accept(replica, {}, {first}), (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_EQ(accept(replica, Ballot::recovery(2, owner), {}, kv::Nop{}),
      (DispatchResult{DispatchStatus::rejected, {}}));
  // A delayed old commit cannot cross the newly joined current ballot.
  EXPECT_EQ(replica.dispatch(PeerMessage{{owner, owner, Commit{{}, id, command, {first}}}}).status,
      DispatchStatus::stale);
  EXPECT_EQ(replica.dispatch(PeerMessage{{ReplicaId{2}, owner,
      Commit{ballot, id, command, {first}}}}).status, DispatchStatus::processed);
}
TEST(AcceptTransition, CurrentBallotGuardIsIndependentOfAcceptedBallot) {
  Replica replica{owner};
  ASSERT_EQ(accept(replica, {}, {first}).status, DispatchStatus::processed);
  // Fixture of a joined ballot, not a recovery implementation or transition.
  auto& state = const_cast<InstanceState&>(replica.instances().at(id).state);
  const auto joined = Ballot::recovery(2, owner);
  state.ballots.current = joined;
  ASSERT_EQ(state.ballots.accepted, Ballot{});
  EXPECT_EQ(accept(replica, Ballot::recovery(1, owner), {first}),
      (DispatchResult{DispatchStatus::stale, {}}));
  EXPECT_EQ(state.ballots.accepted, Ballot{});
  EXPECT_EQ(accept(replica, joined, {second}).status, DispatchStatus::processed);
  EXPECT_EQ(state.ballots.current, joined);
  EXPECT_EQ(state.ballots.accepted, joined);
  EXPECT_EQ(state.dependencies, Dependencies{second});
}
TEST(AcceptTransition, SupersedesTimersAndCollectorButKeepsMatchingFrozenProposal) {
  Replica replica{owner};
  start(replica);
  reply(replica, ReplicaId{1});
  const auto grace = reply(replica, ReplicaId{2});
  const auto guard = std::get<ScheduleTimer>(grace.effects.back()).guard;
  const auto timer = replica.arm_timer(id, {7});
  const auto result = accept(replica, {}, {first});
  ASSERT_EQ(result.status, DispatchStatus::processed);
  ASSERT_EQ(result.effects.size(), 3U);
  EXPECT_EQ(std::get<CancelTimer>(result.effects[0]).token,
      std::get<ScheduleTimer>(timer.back()).guard.token);
  EXPECT_EQ(std::get<CancelTimer>(result.effects[1]).token, guard.token);
  EXPECT_EQ(std::get<SendMessage>(result.effects[2]).envelope,
      (Envelope{owner, owner, AcceptOk{{}, id}}));
  EXPECT_FALSE(replica.instances().at(id).initial_grace);
  EXPECT_FALSE(replica.instances().at(id).timer);
  EXPECT_EQ(replica.dispatch(TimerFired{guard}).status, DispatchStatus::stale);
  EXPECT_EQ(replica.dispatch(TimerFired{std::get<ScheduleTimer>(timer.back()).guard}).status,
      DispatchStatus::stale);
  EXPECT_EQ(reply(replica, ReplicaId{3}).status, DispatchStatus::stale);
  Replica selected{owner};
  const auto ready = select_slow(selected);
  selected.dispatch(Event{ready});
  const auto frozen = selected.instances().at(id).slow_proposal;
  EXPECT_EQ(accept(selected, {}, {}), (DispatchResult{DispatchStatus::rejected, {}}));
  ASSERT_EQ(accept(selected, {}, {first, second, third}).status, DispatchStatus::processed);
  EXPECT_EQ(selected.instances().at(id).slow_proposal, frozen);
  EXPECT_EQ(accept(selected, Ballot::recovery(1, owner), {}, kv::Nop{}).status, DispatchStatus::processed);
  EXPECT_EQ(selected.instances().at(id).state.command, kv::Payload{kv::Nop{}});
  EXPECT_EQ(selected.instances().at(id).state.initial_command, command);
  EXPECT_TRUE(selected.instances().at(id).state.initial_dependencies.empty());
  EXPECT_FALSE(selected.instances().at(id).slow_proposal);
}
}  // namespace
}  // namespace egalis::protocol
