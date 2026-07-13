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
}  // namespace
}  // namespace egalis::protocol
