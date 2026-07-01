#include "egalis/protocol/types.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>

namespace egalis::protocol {

static_assert(!std::is_convertible_v<unsigned, ReplicaId>);
static_assert(!std::is_default_constructible_v<InstanceId>);

TEST(Types, FixedBaselineRejectsOtherVariants) {
  EXPECT_NO_THROW(Configuration::validate(5, 2, 1));
  EXPECT_THROW(Configuration::validate(5, 2, 2), std::invalid_argument);
  EXPECT_THROW(Configuration::validate(3, 1, 1), std::invalid_argument);
  EXPECT_THROW(ReplicaId{5}, std::out_of_range);
  EXPECT_EQ(members().size(), 5U);
}

TEST(Types, InstanceOrderMatchesNumericalEncodingAndRetainsOwner) {
  for (const auto replica : members()) {
    InstanceId id{7, replica};
    EXPECT_EQ(id.coordinator(), replica);
    EXPECT_EQ(id.number(), 35 + replica.value());
    EXPECT_LT(id, (InstanceId{8, ReplicaId{0}}));
  }
  EXPECT_LT((InstanceId{7, ReplicaId{0}}), (InstanceId{7, ReplicaId{4}}));
  EXPECT_THROW((InstanceId{0, ReplicaId{0}}), std::out_of_range);
  EXPECT_THROW((InstanceId{InstanceId::max_counter + 1, ReplicaId{0}}), std::out_of_range);
  EXPECT_NO_THROW(static_cast<void>((InstanceId{InstanceId::max_counter, ReplicaId{4}}).number()));
}

TEST(Types, InitialAndRecoveryBallotsHaveUnambiguousOwnership) {
  InstanceId instance{1, ReplicaId{3}};
  Ballot zero;
  EXPECT_TRUE(zero.initial());
  EXPECT_EQ(zero.owner(instance), ReplicaId{3});
  for (const auto owner : members()) {
    auto b = Ballot::recovery(12, owner);
    EXPECT_EQ(b.round(), 12U);
    EXPECT_EQ(b.owner(instance), owner);
    for (const auto next_owner : members()) {
      EXPECT_GT(b.next_owned_by(next_owner), b);
      EXPECT_EQ(b.next_owned_by(next_owner).owner(instance), next_owner);
    }
  }
  EXPECT_THROW(static_cast<void>(Ballot::recovery(0, ReplicaId{0})), std::out_of_range);
  EXPECT_THROW(static_cast<void>(Ballot::recovery(Ballot::max_round + 1, ReplicaId{0})), std::out_of_range);
  EXPECT_THROW(static_cast<void>(Ballot::recovery(Ballot::max_round, ReplicaId{4}).next_owned_by(ReplicaId{0})),
               std::overflow_error);
  EXPECT_LT(zero, zero.next_owned_by(ReplicaId{0}));
  EXPECT_LT(Ballot::recovery(1, ReplicaId{4}), Ballot::recovery(2, ReplicaId{0}));
}

TEST(Types, CurrentAndAcceptedBallotsAreIndependent) {
  BallotState state;
  state.current = Ballot::recovery(2, ReplicaId{1});
  EXPECT_TRUE(state.accepted.initial());
  EXPECT_TRUE(state.valid());
  state.accepted = Ballot::recovery(3, ReplicaId{0});
  EXPECT_FALSE(state.valid());
}

TEST(Types, QuorumsCountDistinctMembersIncludingCoordinator) {
  Quorum quorum;
  EXPECT_EQ(quorum.size(), 0U);
  for (const auto member : members()) {
    EXPECT_TRUE(quorum.add(member));
    EXPECT_FALSE(quorum.add(member));
    EXPECT_TRUE(quorum.contains(member));
    EXPECT_EQ(quorum.size(), member.value() + 1U);
    EXPECT_EQ(quorum.slow(), quorum.size() >= 3);
    EXPECT_EQ(quorum.fast(), quorum.size() >= 4);
  }
}

}  // namespace egalis::protocol
