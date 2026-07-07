#include "egalis/protocol/replica.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace egalis::protocol {
namespace {
const ReplicaId local{0};
const InstanceId id{1, local};
Submit submit(InstanceId instance = id) { return {instance, kv::Put{"key", "value"}}; }
TimerGuard arm(Replica& replica) {
  return std::get<ScheduleTimer>(replica.arm_timer(id, {4}).back()).guard;
}
}
TEST(Replica, SubmissionReservesExactlyOneInitialRecordWithoutVoting) {
  Replica replica{local};
  const auto result = replica.dispatch(submit());
  EXPECT_EQ(result.status, DispatchStatus::processed);
  ASSERT_EQ(result.effects.size(), 6U);
  EXPECT_EQ(std::get<SubmissionRegistered>(result.effects.front()).instance, id);
  ASSERT_EQ(replica.instances().size(), 1U);
  const auto& state = replica.instances().at(id).state;
  EXPECT_EQ(state.phase, Phase::initial);
  EXPECT_EQ(state.command, (kv::Payload{submit().command}));
  EXPECT_EQ(state.initial_command, submit().command);
  EXPECT_TRUE(state.dependencies.empty());
  EXPECT_TRUE(state.ballots.current.initial());
}
TEST(Replica, ReusedAndForeignIdsCannotOverwriteState) {
  Replica replica{local};
  replica.dispatch(submit());
  EXPECT_THROW(replica.dispatch(Submit{id, kv::Get{"other"}}), std::invalid_argument);
  EXPECT_THROW(replica.dispatch(submit(InstanceId{1, ReplicaId{1}})), std::invalid_argument);
  EXPECT_EQ(replica.instances().size(), 1U);
  EXPECT_EQ(replica.instances().at(id).state.command, (kv::Payload{submit().command}));
}
TEST(Replica, CanonicalInstanceIterationDoesNotDependOnSubmissionOrder) {
  Replica replica{local};
  replica.dispatch(submit(InstanceId{3, local}));
  replica.dispatch(submit());
  replica.dispatch(submit(InstanceId{2, local}));
  EXPECT_EQ(replica.instances().begin()->first, id);
}
TEST(Replica, ValidMessagesAreExplicitlyUnsupportedWithoutAllocatingState) {
  Replica replica{local};
  const auto positive = Ballot::recovery(1, local);
  const std::vector<Message> messages{
      PreAccept{id, kv::Get{"k"}, {}}, PreAcceptOk{id, {}},
      Accept{{}, id, kv::Command{kv::Get{"k"}}, {}}, AcceptOk{{}, id},
      Commit{{}, id, kv::Command{kv::Get{"k"}}, {}}, Recover{positive, id},
      RecoverOk{positive, id, {}, {}, {}, {}, Phase::initial},
      Validate{positive, id, kv::Get{"k"}, {}}, ValidateOk{positive, id, {}},
      Waiting{id}, TryRecover{id}};
  for (const auto& message : messages) {
    const auto result = replica.dispatch(PeerMessage{{local, local, message}});
    EXPECT_EQ(result.status, DispatchStatus::unsupported);
    EXPECT_TRUE(result.effects.empty());
  }
  EXPECT_TRUE(replica.instances().empty());
  EXPECT_THROW(replica.dispatch(PeerMessage{{local, ReplicaId{1}, Waiting{id}}}),
               std::invalid_argument);
}
TEST(Replica, ReplacingTimersCancelsBeforeSchedulingAndRejectsStaleEvents) {
  Replica replica{local};
  replica.dispatch(submit());
  const auto old = arm(replica);
  const auto effects = replica.arm_timer(id, {0});
  ASSERT_EQ(effects.size(), 2U);
  EXPECT_EQ(std::get<CancelTimer>(effects[0]).token, old.token);
  const auto current = std::get<ScheduleTimer>(effects[1]).guard;
  EXPECT_NE(current.token, old.token);
  EXPECT_EQ(replica.dispatch(TimerFired{old}).status, DispatchStatus::stale);
  auto wrong_ballot = current;
  wrong_ballot.ballot = Ballot::recovery(1, local);
  EXPECT_EQ(replica.dispatch(TimerFired{wrong_ballot}).status, DispatchStatus::stale);
  EXPECT_EQ(replica.dispatch(TimerFired{current}).status, DispatchStatus::unsupported);
  EXPECT_EQ(replica.dispatch(TimerFired{current}).status, DispatchStatus::stale);
}
TEST(Replica, UnknownTimersAndCompletionsCannotMaterializeInstances) {
  Replica replica{local};
  EXPECT_THROW(replica.arm_timer(id, {1}), std::invalid_argument);
  EXPECT_EQ(replica.dispatch(TimerFired{{id, {}, {1}}}).status, DispatchStatus::stale);
  for (const auto kind : {CompletionKind::persistence, CompletionKind::application})
    for (const auto status : {CompletionStatus::succeeded, CompletionStatus::failed})
      EXPECT_EQ(replica.dispatch(Completed{id, {}, {1}, kind, status}).status,
                DispatchStatus::stale);
  EXPECT_TRUE(replica.instances().empty());
}
TEST(Replica, SuspendPreservesRecordsButInvalidatesEveryTimerAcrossResume) {
  Replica replica{local};
  replica.dispatch(submit());
  const auto before = arm(replica);
  const auto cancelled = replica.suspend();
  ASSERT_EQ(cancelled.size(), 1U);
  EXPECT_EQ(std::get<CancelTimer>(cancelled.front()).token, before.token);
  EXPECT_THROW(replica.dispatch(TimerFired{before}), std::logic_error);
  EXPECT_THROW(replica.arm_timer(id, {1}), std::logic_error);
  EXPECT_THROW(replica.suspend(), std::logic_error);
  replica.resume();
  EXPECT_THROW(replica.resume(), std::logic_error);
  EXPECT_EQ(replica.instances().size(), 1U);
  EXPECT_EQ(replica.dispatch(TimerFired{before}).status, DispatchStatus::stale);
  EXPECT_GT(arm(replica).token, before.token);
}
}  // namespace egalis::protocol
