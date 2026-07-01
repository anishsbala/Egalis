#include "egalis/kv/command.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace egalis::kv {

TEST(Command, MissingEmptyAndBinaryValuesAreDistinct) {
  StateMachine state;
  EXPECT_EQ(state.apply(Get{"k"}), (Result{GetResult{std::nullopt}}));
  EXPECT_EQ(state.apply(Put{"k", ""}), Result{PutResult{}});
  EXPECT_EQ(state.apply(Get{"k"}), (Result{GetResult{""}}));
  const std::string binary_key{"a\0b", 3};
  const std::string binary_value{"\0x", 2};
  state.apply(Put{binary_key, binary_value});
  EXPECT_EQ(state.read(binary_key), binary_value);
  EXPECT_EQ(state.read("a"), std::nullopt);
}

TEST(Command, CasReturnsOriginalObservationAndMutatesOnlyOnMatch) {
  StateMachine state;
  EXPECT_EQ(state.apply(CompareAndSwap{"k", "", "no"}),
            (Result{CasResult{false, std::nullopt}}));
  EXPECT_EQ(state.apply(CompareAndSwap{"k", std::nullopt, ""}),
            (Result{CasResult{true, std::nullopt}}));
  EXPECT_EQ(state.apply(CompareAndSwap{"k", std::nullopt, "no"}),
            (Result{CasResult{false, ""}}));
  EXPECT_EQ(state.read("k"), "");
  EXPECT_EQ(state.apply(CompareAndSwap{"k", "", "next"}),
            (Result{CasResult{true, ""}}));
  EXPECT_EQ(state.read("k"), "next");
}

TEST(Command, ConflictRulesAreConservativeAndSeparateSentinels) {
  EXPECT_FALSE(conflicts(Command{Get{"k"}}, Command{Get{"k"}}));
  EXPECT_TRUE(conflicts(Command{Get{"k"}}, Command{Put{"k", "x"}}));
  EXPECT_TRUE(conflicts(Command{Put{"k", "x"}}, Command{Put{"k", "x"}}));
  EXPECT_FALSE(conflicts(Command{Put{"a", "x"}}, Command{CompareAndSwap{"b", "x", "y"}}));
  const std::vector<Payload> payloads{std::monostate{}, Nop{}, Command{Get{"k"}},
                                    Command{Put{"k", "x"}}, Command{Get{"other"}}};
  for (const auto& left : payloads) {
    for (const auto& right : payloads) {
      EXPECT_EQ(conflicts(left, right), conflicts(right, left));
      if (std::holds_alternative<std::monostate>(left)) {
        EXPECT_FALSE(conflicts(left, right));
      }
      if (std::holds_alternative<Nop>(left)) {
        EXPECT_EQ(conflicts(left, right), !std::holds_alternative<std::monostate>(right));
      }
    }
  }
}

TEST(Command, EveryDeclaredCommutingPairPreservesStateAndBothResults) {
  const std::vector<std::optional<std::string>> values{std::nullopt, "", "v"};
  std::vector<Command> commands;
  for (const auto& key : {"a", "b"}) {
    commands.emplace_back(Get{key});
    for (const auto& value : {"", "v"}) {
      commands.emplace_back(Put{key, value});
      for (const auto& expected : values) {
        commands.emplace_back(CompareAndSwap{key, expected, value});
      }
    }
  }
  std::size_t checked = 0;
  for (const auto& a : values) for (const auto& b : values) {
    StateMachine initial;
    if (a) initial.apply(Put{"a", *a});
    if (b) initial.apply(Put{"b", *b});
    for (const auto& left : commands) for (const auto& right : commands) {
      EXPECT_EQ(conflicts(left, right), conflicts(right, left));
      if (conflicts(left, right)) continue;
      auto forward = initial;
      auto reverse = initial;
      const auto left_first = forward.apply(left);
      const auto right_second = forward.apply(right);
      const auto right_first = reverse.apply(right);
      const auto left_second = reverse.apply(left);
      EXPECT_EQ(forward, reverse);
      EXPECT_EQ(left_first, left_second);
      EXPECT_EQ(right_first, right_second);
      ++checked;
    }
  }
  EXPECT_EQ(checked, 1476U);  // 9 states * (162 cross-key pairs + 2 read/read pairs).
}

}  // namespace egalis::kv
