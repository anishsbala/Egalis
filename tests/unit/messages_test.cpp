#include "egalis/protocol/messages.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace egalis::protocol {
namespace {
const ReplicaId a{0};
const ReplicaId b{1};
const InstanceId id{1, a};
const InstanceId other{2, b};
const Ballot recovery = Ballot::recovery(1, a);
const kv::Command command = kv::Put{"k", "v"};
const kv::Payload payload = command;
Envelope wrap(Message message) { return Envelope{a, a, std::move(message)}; }
}  // namespace

TEST(Messages, EveryBaselineMessageHasAValidTypedRepresentation) {
  const std::vector<Message> messages{
      PreAccept{id, command, {other}}, PreAcceptOk{id, {other}},
      Accept{Ballot{}, id, payload, {}}, AcceptOk{Ballot{}, id},
      Commit{recovery, id, kv::Nop{}, {}}, Recover{recovery, id},
      RecoverOk{recovery, id, Ballot{}, payload, {other}, {}, Phase::preaccepted},
      Validate{recovery, id, command, {}},
      ValidateOk{recovery, id, {{other, Phase::initial}, {other, Phase::committed}}},
      Waiting{id}, TryRecover{id}};
  std::set<std::string> encodings;
  for (const auto& message : messages) {
    const auto envelope = wrap(message);
    EXPECT_NO_THROW(validate(envelope));
    encodings.insert(canonical(envelope));
  }
  EXPECT_EQ(encodings.size(), std::variant_size_v<Message>);
}

TEST(Messages, ProposalSenderAndReplyDestinationMustOwnBallot) {
  EXPECT_THROW(validate(Envelope{b, a, PreAccept{id, command, {}}}), std::invalid_argument);
  EXPECT_THROW(validate(Envelope{a, b, PreAcceptOk{id, {}}}), std::invalid_argument);
  for (const Message& msg : std::vector<Message>{Accept{recovery, id, payload, {}},
       Commit{recovery, id, payload, {}}, Recover{recovery, id}, Validate{recovery, id, command, {}}}) {
    EXPECT_THROW(validate(Envelope{b, a, msg}), std::invalid_argument);
  }
  EXPECT_THROW(validate(Envelope{a, b, AcceptOk{recovery, id}}), std::invalid_argument);
  EXPECT_THROW(validate(Envelope{a, b, RecoverOk{recovery, id, Ballot{}, {}, {}, {}, Phase::initial}}),
               std::invalid_argument);
  EXPECT_NO_THROW(validate(Envelope{b, b, ValidateOk{recovery, id, {}}}));
}

TEST(Messages, RecoveryRequiresPositiveBallots) {
  for (const Message& msg : std::vector<Message>{Recover{Ballot{}, id},
       RecoverOk{Ballot{}, id, Ballot{}, {}, {}, {}, Phase::initial},
       Validate{Ballot{}, id, command, {}}, ValidateOk{Ballot{}, id, {}}}) {
    EXPECT_THROW(validate(wrap(msg)), std::invalid_argument);
  }
}

TEST(Messages, DependenciesCannotReferToOwnInstance) {
  for (const Message& msg : std::vector<Message>{PreAccept{id, command, {id}}, PreAcceptOk{id, {id}},
       Accept{recovery, id, payload, {id}}, Commit{recovery, id, payload, {id}},
       Validate{recovery, id, command, {id}},
       RecoverOk{recovery, id, Ballot{}, payload, {id}, {}, Phase::preaccepted},
       RecoverOk{recovery, id, Ballot{}, payload, {}, {id}, Phase::preaccepted}}) {
    EXPECT_THROW(validate(wrap(msg)), std::invalid_argument);
  }
}

TEST(Messages, AbsentAndNopProposalsAreCheckedSeparately) {
  EXPECT_THROW(validate(wrap(Accept{recovery, id, {}, {}})), std::invalid_argument);
  EXPECT_THROW(validate(wrap(Commit{recovery, id, {}, {}})), std::invalid_argument);
  EXPECT_THROW(validate(wrap(Accept{Ballot{}, id, kv::Nop{}, {}})), std::invalid_argument);
  EXPECT_THROW(validate(wrap(Commit{Ballot{}, id, kv::Nop{}, {}})), std::invalid_argument);
  EXPECT_NO_THROW(validate(wrap(Accept{recovery, id, kv::Nop{}, {}})));
}

TEST(Messages, RecoverySnapshotsPreserveInitialValidationWithoutInventingVotes) {
  EXPECT_NO_THROW(validate(wrap(RecoverOk{recovery, id, Ballot{}, {}, {}, {}, Phase::initial})));
  EXPECT_NO_THROW(validate(wrap(RecoverOk{recovery, id, Ballot{}, payload, {}, {other}, Phase::initial})));
  EXPECT_THROW(validate(wrap(RecoverOk{recovery, id, recovery, payload, {}, {}, Phase::accepted})),
               std::invalid_argument);
  EXPECT_THROW(validate(wrap(RecoverOk{recovery, id, Ballot{}, {}, {}, {}, Phase::accepted})),
               std::invalid_argument);
  EXPECT_THROW(validate(wrap(RecoverOk{recovery, id, Ballot{}, kv::Nop{}, {}, {}, Phase::preaccepted})),
               std::invalid_argument);
  auto later = recovery.next_owned_by(a);
  EXPECT_THROW(validate(wrap(RecoverOk{later, id, recovery, payload, {}, {}, Phase::preaccepted})),
               std::invalid_argument);
  EXPECT_THROW(validate(wrap(RecoverOk{later, id, Ballot{}, payload, {}, {}, static_cast<Phase>(99)})),
               std::invalid_argument);
}

TEST(Messages, InvalidatorsHaveCheckedIdentityAndPhase) {
  EXPECT_THROW(validate(wrap(ValidateOk{recovery, id, {{id, Phase::committed}}})), std::invalid_argument);
  EXPECT_THROW(validate(wrap(ValidateOk{recovery, id, {{other, static_cast<Phase>(99)}}})),
               std::invalid_argument);
}

TEST(Messages, CanonicalEncodingIsStableAndUnambiguousForBytesAndSets) {
  const InstanceId third{3, a};
  EXPECT_EQ(canonical(wrap(PreAccept{id, command, {third, other}})),
            canonical(wrap(PreAccept{id, command, {other, third}})));
  EXPECT_NE(canonical(wrap(PreAccept{id, kv::Put{"ab", "c"}, {}})),
            canonical(wrap(PreAccept{id, kv::Put{"a", "bc"}, {}})));
  EXPECT_NE(canonical(wrap(PreAccept{id, kv::Put{std::string{"a\0", 2}, "b"}, {}})),
            canonical(wrap(PreAccept{id, kv::Put{"a", "b"}, {}})));
  EXPECT_NE(canonical(wrap(Validate{recovery, id, kv::CompareAndSwap{"k", std::nullopt, "v"}, {}})),
            canonical(wrap(Validate{recovery, id, kv::CompareAndSwap{"k", "", "v"}, {}})));
  EXPECT_NE(canonical(Envelope{a, b, Waiting{id}}), canonical(wrap(Waiting{id})));
}

TEST(Messages, InstanceRecordStartsAtPaperDefaults) {
  const InstanceState state;
  EXPECT_TRUE(state.ballots.current.initial());
  EXPECT_TRUE(state.ballots.accepted.initial());
  EXPECT_EQ(state.phase, Phase::initial);
  EXPECT_TRUE(std::holds_alternative<std::monostate>(state.command));
  EXPECT_FALSE(state.initial_command);
  EXPECT_TRUE(state.dependencies.empty());
  EXPECT_TRUE(state.initial_dependencies.empty());
}

}  // namespace egalis::protocol
