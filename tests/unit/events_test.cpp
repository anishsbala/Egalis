#include "egalis/protocol/events.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>

namespace egalis::protocol {
namespace {
const InstanceId id{1, ReplicaId{0}};
const Envelope envelope{ReplicaId{0}, ReplicaId{1}, Waiting{id}};
}
TEST(ReplicaContract, InputsOwnTheirPayloadsAndAreDistinct) {
  kv::Command command = kv::Put{"key", "value"};
  Event input = Submit{id, command};
  command = kv::Get{"other"};
  EXPECT_EQ(std::get<Submit>(input).command, (kv::Command{kv::Put{"key", "value"}}));
  EXPECT_NO_THROW(validate(input));
  EXPECT_NO_THROW(validate(Event{PeerMessage{envelope}}));
  EXPECT_NO_THROW(validate(Event{TimerFired{{id, {}, {1}}}}));
  EXPECT_NO_THROW(validate(Event{Completed{id, {}, {1}, CompletionKind::persistence,
                                          CompletionStatus::succeeded}}));
  static_assert(!std::is_convertible_v<TimerToken, CompletionToken>);
  static_assert(!std::is_convertible_v<TimerDelay, std::uint64_t>);
}
TEST(ReplicaContract, EffectsPreserveOrderAndValidateEveryAlternative) {
  Effects effects{SendMessage{envelope}, ScheduleTimer{{id, {}, {1}}, {0}},
                  CancelTimer{{1}}, SubmissionRegistered{id}};
  for (const auto& effect : effects) EXPECT_NO_THROW(validate(effect));
  EXPECT_TRUE(std::holds_alternative<SendMessage>(effects[0]));
  EXPECT_TRUE(std::holds_alternative<ScheduleTimer>(effects[1]));
}
TEST(ReplicaContract, RejectsInvalidTokensAndCompletionTags) {
  EXPECT_THROW(validate(Event{TimerFired{{id, {}, {0}}}}), std::invalid_argument);
  EXPECT_THROW(validate(Effect{ScheduleTimer{{id, {}, {0}}, {1}}}), std::invalid_argument);
  EXPECT_THROW(validate(Effect{CancelTimer{{0}}}), std::invalid_argument);
  EXPECT_THROW(validate(Event{Completed{id, {}, {0}, CompletionKind::application,
                                       CompletionStatus::failed}}), std::invalid_argument);
  EXPECT_THROW(validate(Event{Completed{id, {}, {1}, static_cast<CompletionKind>(9),
                                       CompletionStatus::succeeded}}), std::invalid_argument);
  EXPECT_THROW(validate(Event{Completed{id, {}, {1}, CompletionKind::application,
                                       static_cast<CompletionStatus>(9)}}), std::invalid_argument);
}
TEST(ReplicaContract, MessageValidationIsNotBypassed) {
  Envelope malformed{ReplicaId{1}, ReplicaId{0}, PreAccept{id, kv::Get{"k"}, {}}};
  EXPECT_THROW(validate(Event{PeerMessage{malformed}}), std::invalid_argument);
  EXPECT_THROW(validate(Effect{SendMessage{malformed}}), std::invalid_argument);
}
}  // namespace egalis::protocol
