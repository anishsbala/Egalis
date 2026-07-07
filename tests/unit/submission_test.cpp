#include "egalis/protocol/replica.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace egalis::protocol {
TEST(Submission, AllocatesLocalIdsAndFreezesKnownConflictProposal) {
  Replica replica{ReplicaId{2}};
  const auto first = replica.dispatch(SubmitCommand{kv::Put{"k", "v"}});
  const auto first_id = std::get<SubmissionRegistered>(first.effects.front()).instance;
  EXPECT_EQ(first_id, (InstanceId{1, ReplicaId{2}}));
  const auto second = replica.dispatch(SubmitCommand{kv::Get{"k"}});
  ASSERT_EQ(second.effects.size(), 6U);
  const auto id = std::get<SubmissionRegistered>(second.effects[0]).instance;
  EXPECT_EQ(id, (InstanceId{2, ReplicaId{2}}));
  EXPECT_EQ(std::get<SendMessage>(second.effects[1]).envelope.destination, ReplicaId{2});
  std::set<ReplicaId> destinations;
  for (std::size_t i = 1; i < second.effects.size(); ++i) {
    const auto& envelope = std::get<SendMessage>(second.effects[i]).envelope;
    destinations.insert(envelope.destination);
    EXPECT_EQ(envelope.source, ReplicaId{2});
    EXPECT_EQ(std::get<PreAccept>(envelope.message).dependencies, (Dependencies{first_id}));
    EXPECT_NO_THROW(validate(envelope));
  }
  EXPECT_EQ(destinations.size(), 5U);
  const auto& state = replica.instances().at(id).state;
  EXPECT_EQ(state.initial_dependencies, (Dependencies{first_id}));
  EXPECT_EQ(state.phase, Phase::initial);  // Receive transition has not run yet.
  EXPECT_TRUE(state.dependencies.empty());
}
TEST(Submission, ExplicitReservationsAdvanceAllocationAndCannotBeReused) {
  Replica replica{ReplicaId{0}};
  const InstanceId reserved{20, ReplicaId{0}};
  replica.dispatch(Submit{reserved, kv::Get{"k"}});
  EXPECT_THROW(replica.dispatch(Submit{reserved, kv::Get{"other"}}), std::invalid_argument);
  EXPECT_THROW(replica.dispatch(Submit{InstanceId{100, ReplicaId{1}}, kv::Get{"k"}}),
               std::invalid_argument);
  const auto result = replica.dispatch(SubmitCommand{kv::Get{"k"}});
  EXPECT_EQ(std::get<SubmissionRegistered>(result.effects[0]).instance,
            (InstanceId{21, ReplicaId{0}}));
}
TEST(Submission, LastIdIsUsableAndExhaustionDoesNotMutateRecords) {
  Replica replica{ReplicaId{4}, InstanceId::max_counter};
  const auto result = replica.dispatch(SubmitCommand{kv::Get{"k"}});
  EXPECT_EQ(std::get<SubmissionRegistered>(result.effects[0]).instance.counter(), InstanceId::max_counter);
  EXPECT_THROW(replica.dispatch(SubmitCommand{kv::Get{"other"}}), std::overflow_error);
  EXPECT_EQ(replica.instances().size(), 1U);
  EXPECT_THROW((Replica{ReplicaId{0}, 0}), std::out_of_range);
  EXPECT_THROW((Replica{ReplicaId{0}, InstanceId::max_counter + 1}), std::out_of_range);
}
TEST(Submission, SuspendResumeRetainsAllocationAndFrozenEffects) {
  Replica replica{ReplicaId{0}};
  const auto first = replica.dispatch(SubmitCommand{kv::Get{"k"}});
  replica.suspend();
  EXPECT_THROW(replica.dispatch(SubmitCommand{kv::Get{"k"}}), std::logic_error);
  replica.resume();
  const auto second = replica.dispatch(SubmitCommand{kv::Put{"k", "v"}});
  EXPECT_EQ(std::get<SubmissionRegistered>(second.effects[0]).instance.counter(), 2U);
  EXPECT_TRUE(std::get<PreAccept>(std::get<SendMessage>(first.effects[1]).envelope.message).dependencies.empty());
}
}  // namespace egalis::protocol
