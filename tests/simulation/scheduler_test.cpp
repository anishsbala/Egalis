#include "egalis/sim/scheduler.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace egalis::sim {
static_assert(!std::is_convertible_v<LogicalTime, Duration>);
static_assert(!std::is_convertible_v<EventId, EventSequence>);

TEST(Scheduler, EmptyAndBoundedRunsDoNotAdvanceTime) {
  Scheduler scheduler;
  EXPECT_FALSE(scheduler.step());
  EXPECT_EQ(scheduler.run(), 0U);
  scheduler.schedule_at({5}, "later", [] {});
  EXPECT_EQ(scheduler.run(0), 0U);
  EXPECT_EQ(scheduler.now(), LogicalTime{0});
  EXPECT_EQ(scheduler.run(1), 1U);
  EXPECT_EQ(scheduler.now(), LogicalTime{5});
}

TEST(Scheduler, SimultaneousEventsHaveStableInsertionOrderIncludingNewEvents) {
  Scheduler scheduler;
  std::vector<int> observed;
  auto first = scheduler.schedule_at({10}, "first", [&] {
    observed.push_back(1);
    scheduler.schedule_after({0}, "child", [&] { observed.push_back(3); });
  });
  auto second = scheduler.schedule_at({10}, "second", [&] { observed.push_back(2); });
  scheduler.schedule_at({1}, "earlier", [&] { observed.push_back(0); });
  EXPECT_NE(first, second);
  EXPECT_EQ(scheduler.run(), 4U);
  EXPECT_EQ(observed, (std::vector<int>{0, 1, 2, 3}));
  EXPECT_EQ(scheduler.now(), LogicalTime{10});
}

TEST(Scheduler, CancellationDoesNotReuseIdentityOrExecuteCallback) {
  Scheduler scheduler;
  int calls = 0;
  const auto id = scheduler.schedule_after({1}, "cancel", [&] { ++calls; });
  EXPECT_TRUE(scheduler.cancel(id));
  EXPECT_FALSE(scheduler.cancel(id));
  const auto next = scheduler.schedule_after({1}, "next", [&] { ++calls; });
  EXPECT_GT(next, id);
  EXPECT_EQ(scheduler.run(), 1U);
  EXPECT_EQ(calls, 1);
}

TEST(Scheduler, RejectsPastEmptyCallbacksAndLogicalTimeOverflow) {
  Scheduler scheduler;
  scheduler.schedule_at({5}, "advance", [] {});
  scheduler.run();
  EXPECT_THROW(scheduler.schedule_at({4}, "past", [] {}), std::invalid_argument);
  EXPECT_THROW(scheduler.schedule_after({0}, "empty", {}), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(after({std::numeric_limits<std::uint64_t>::max()}, {1})),
               std::overflow_error);
  scheduler.schedule_at({std::numeric_limits<std::uint64_t>::max()}, "last", [] {});
  scheduler.run();
  EXPECT_THROW(scheduler.schedule_after({1}, "overflow", [] {}), std::overflow_error);
}

TEST(Scheduler, CallbackFailureStopsSimulationAndNestedExecutionIsForbidden) {
  Scheduler scheduler;
  scheduler.schedule_after({0}, "nested", [&] { scheduler.run(); });
  EXPECT_THROW(scheduler.run(), std::logic_error);
  EXPECT_TRUE(scheduler.failed());
  EXPECT_THROW(scheduler.step(), std::logic_error);
  EXPECT_THROW(scheduler.schedule_after({0}, "after failure", [] {}), std::logic_error);
}

TEST(Scheduler, IdenticalInputsReproduceCompleteEventDecisions) {
  const auto scenario = [] {
    std::vector<std::pair<EventAction, EventInfo>> trace;
    Scheduler scheduler{[&](auto action, const auto& info) { trace.emplace_back(action, info); }};
    scheduler.schedule_at({8}, "a", [] {});
    auto cancelled = scheduler.schedule_at({3}, "b", [] {});
    scheduler.schedule_at({8}, "c", [] {});
    scheduler.cancel(cancelled);
    scheduler.run();
    return trace;
  };
  EXPECT_EQ(scenario(), scenario());
}
}  // namespace egalis::sim
