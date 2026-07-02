#include "egalis/sim/trace.hpp"
#include "egalis/sim/network.hpp"

#include <gtest/gtest.h>
#include <limits>

namespace egalis::sim {

TEST(Random, SplitMix64HasVersionedGoldenOutput) {
  Random random{0};
  EXPECT_EQ(random.next(), UINT64_C(0xe220a8397b1dcdaf));
  EXPECT_EQ(random.next(), UINT64_C(0x6e789e6aa1b965f4));
  EXPECT_EQ(random.next(), UINT64_C(0x06c45d188009454f));
  Random mapped{0};
  EXPECT_EQ(mapped.uniform(0, 9), 5U);
  EXPECT_EQ(mapped.uniform(0, 9), 0U);
}

TEST(Random, BoundedDrawsAreRepeatableAndHandleFullWidthAndSingletons) {
  Random first{917}, second{917};
  for (int i = 0; i < 10000; ++i) {
    const auto x = first.uniform(7, 23);
    EXPECT_EQ(x, second.uniform(7, 23));
    EXPECT_GE(x, 7U);
    EXPECT_LE(x, 23U);
  }
  Random full{0};
  EXPECT_EQ(full.uniform(0, std::numeric_limits<std::uint64_t>::max()), UINT64_C(0xe220a8397b1dcdaf));
  EXPECT_EQ(full.uniform(42, 42), 42U);
  EXPECT_THROW(full.uniform(2, 1), std::invalid_argument);
  // This interval exercises rejection rather than a biased remainder mapping.
  for (int i = 0; i < 1000; ++i) {
    EXPECT_LE(full.uniform(0, UINT64_C(0x8000000000000000)), UINT64_C(0x8000000000000000));
  }
}

TEST(Trace, StableArtifactPreservesAllBytesAndMaximumIntegers) {
  TraceData data;
  data.seed = std::numeric_limits<std::uint64_t>::max();
  data.decisions = {{DecisionKind::sent, {0, data.seed}, std::string{"a\0\n\r\xff", 5}},
                    {DecisionKind::discarded, {}, {}}};
  EXPECT_EQ(parse_trace(serialize(data)), data);
  EXPECT_EQ(serialize(parse_trace(serialize(data))), serialize(data));
  TraceData small;
  small.version = 1;  // Preserve the original artifact's golden bytes.
  small.decisions = {{DecisionKind::attached, {0, 1}, "x"}};
  EXPECT_EQ(serialize(small), "EGALIS_TRACE 1 1 0\n5 2 0 1 78\n");
}

TEST(Trace, MalformedAndUnsupportedArtifactsAreRejected) {
  for (const auto* text : {"", "WRONG 1 1 0\n", "EGALIS_TRACE 3 1 0\n",
       "EGALIS_TRACE 1 2 0\n", "EGALIS_TRACE 1 1 -1\n", "EGALIS_TRACE 1 1 0 extra\n",
       "EGALIS_TRACE 1 1 18446744073709551616\n", "EGALIS_TRACE 1 1 0\n99 0 -\n",
       "EGALIS_TRACE 1 1 0\n1 17 -\n", "EGALIS_TRACE 1 1 0\n1 1 -\n",
       "EGALIS_TRACE 1 1 0\n1 0 abc\n", "EGALIS_TRACE 1 1 0\n1 0 gg\n",
       "EGALIS_TRACE 1 1 0\n1 0 - extra\n"}) {
    EXPECT_THROW(static_cast<void>(parse_trace(text)), std::invalid_argument) << text;
  }
  TraceData unsupported;
  unsupported.version = 99;
  EXPECT_THROW(static_cast<void>(serialize(unsupported)), std::invalid_argument);
  EXPECT_THROW(Trace{unsupported}, std::invalid_argument);
}

TEST(Trace, ReplayConsumesSavedChoicesWithoutRegeneratingThem) {
  Random original_random{12};
  Trace original{12};
  const auto choice = original.choose(original_random, 0, 100, "delay");
  original.record({DecisionKind::attached, {0, 2}, {}});
  Trace replay{parse_trace(serialize(original.data()))};
  Random unrelated_random{999};
  EXPECT_EQ(replay.choose(unrelated_random, 0, 100, "delay"), choice);
  replay.record({DecisionKind::attached, {0, 2}, {}});
  EXPECT_NO_THROW(replay.verify_complete());
}

TEST(Trace, ProtocolDecisionsRequireV2AndLegacyArtifactsRemainReadable) {
  TraceData data;
  data.decisions = {{DecisionKind::protocol, {0, 1}, "result/unsupported"}};
  EXPECT_EQ(parse_trace(serialize(data)), data);
  data.version = 1;
  EXPECT_THROW(static_cast<void>(serialize(data)), std::invalid_argument);
  EXPECT_THROW(Trace{data}, std::invalid_argument);
  EXPECT_THROW(static_cast<void>(parse_trace("EGALIS_TRACE 1 1 0\n14 0 -\n")),
               std::invalid_argument);
  auto legacy = parse_trace("EGALIS_TRACE 1 1 0\n5 2 0 1 78\n");
  Trace replay{legacy};
  replay.record({DecisionKind::attached, {0, 1}, "x"});
  EXPECT_NO_THROW(replay.verify_complete());
}
TEST(Trace, LegacyTransportOnlyNetworkTraceReplaysWithoutProtocolRecords) {
  Network original;
  auto legacy = original.trace();
  legacy.version = 1;
  Network replay{parse_trace(serialize(legacy)), FaultPolicy{}};
  EXPECT_NO_THROW(replay.verify_replay_complete());
  EXPECT_EQ(replay.trace(), legacy);
}

TEST(Trace, ReplayFailsClosedOnDivergenceAndUnconsumedRecords) {
  Trace original{0};
  original.record({DecisionKind::attached, {0, 1}, "input"});
  Trace changed{original.data()};
  EXPECT_THROW(changed.record({DecisionKind::attached, {0, 1}, "changed"}), ReplayDivergence);
  EXPECT_THROW(changed.record({DecisionKind::attached, {0, 1}, "input"}), ReplayDivergence);
  Trace incomplete{original.data()};
  EXPECT_THROW(incomplete.verify_complete(), ReplayDivergence);
  Trace exceeded{original.data()};
  exceeded.record({DecisionKind::attached, {0, 1}, "input"});
  EXPECT_THROW(exceeded.record({DecisionKind::attached, {0, 1}, "input"}), ReplayDivergence);
}

TEST(Trace, ReplayRejectsChangedRandomRequestsAndOutOfRangeDecisions) {
  Random random{1};
  Trace original{1};
  original.choose(random, 2, 3, "range");
  auto damaged = original.data();
  damaged.decisions[0].fields[2] = 99;
  Trace invalid{damaged};
  EXPECT_THROW(invalid.choose(random, 2, 3, "range"), ReplayDivergence);
  Trace changed_range{original.data()};
  EXPECT_THROW(changed_range.choose(random, 0, 3, "range"), ReplayDivergence);
  Trace changed_label{original.data()};
  EXPECT_THROW(changed_label.choose(random, 2, 3, "other"), ReplayDivergence);
}

}  // namespace egalis::sim
