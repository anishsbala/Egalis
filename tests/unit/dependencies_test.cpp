#include "egalis/protocol/dependencies.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace egalis::protocol {
namespace {
using Table = std::map<InstanceId, InstanceRecord>;
const ReplicaId local{0};
const InstanceId target{9, local};
InstanceId id(unsigned counter, unsigned owner = 0) { return {counter, ReplicaId{owner}}; }
InstanceRecord record(kv::Payload command, Phase phase = Phase::initial) {
  InstanceRecord result;
  result.state.command = std::move(command);
  result.state.phase = phase;
  return result;
}
kv::Command get(std::string key = "key") { return kv::Get{std::move(key)}; }
kv::Command put(std::string key = "key") { return kv::Put{std::move(key), "value"}; }
}

TEST(Dependencies, EmptyAndAbsentTargetQueriesDoNotCreateRecords) {
  Table table;
  EXPECT_TRUE(scan_known_dependencies(table, target, put()).empty());
  EXPECT_TRUE(table.empty());
  table.emplace(id(1), record(put()));
  EXPECT_EQ(scan_known_dependencies(table, target, get()), (Dependencies{id(1)}));
  EXPECT_FALSE(table.contains(target));
}
TEST(Dependencies, SelectsMutationsButNotCommutingReadsOrOtherKeys) {
  Table table;
  table.emplace(id(1), record(get()));
  table.emplace(id(2), record(put()));
  table.emplace(id(3), record(kv::Command{kv::CompareAndSwap{"key", std::nullopt, "v"}}));
  table.emplace(id(4), record(put("different")));
  EXPECT_EQ(scan_known_dependencies(table, target, get()), (Dependencies{id(2), id(3)}));
  EXPECT_EQ(scan_known_dependencies(table, target, put()), (Dependencies{id(1), id(2), id(3)}));
}
TEST(Dependencies, ExcludesOnlyTheExactInstanceIdentity) {
  Table table;
  table.emplace(id(1), record(put()));
  table.emplace(id(1, 1), record(put()));
  table.emplace(id(2), record(put()));
  EXPECT_EQ(scan_known_dependencies(table, id(1), put()), (Dependencies{id(1, 1), id(2)}));
  EXPECT_EQ(scan_known_dependencies(table, id(1, 1), put()), (Dependencies{id(1), id(2)}));
}
TEST(Dependencies, EveryPhaseAndBallotRemainsVisible) {
  Table table;
  const std::array phases{Phase::initial, Phase::preaccepted, Phase::accepted, Phase::committed};
  for (unsigned i = 0; i < phases.size(); ++i) {
    auto entry = record(put(), phases[i]);
    if (i >= 2) entry.state.ballots = {Ballot::recovery(1, local), Ballot::recovery(1, local)};
    table.emplace(id(i + 1, i), std::move(entry));
  }
  EXPECT_EQ(scan_known_dependencies(table, target, get()),
            (Dependencies{id(1, 0), id(2, 1), id(3, 2), id(4, 3)}));
  // Applying a retained committed command in the KV model is not a reason to
  // prune it from protocol knowledge. The actual graph executor is not built yet.
  kv::StateMachine machine;
  machine.apply(put());
  EXPECT_EQ(scan_known_dependencies(table, target, get()).size(), 4U);
}
TEST(Dependencies, BottomIsAbsentButNopConflictsAcrossKeys) {
  Table table;
  table.emplace(id(1), record(std::monostate{}));
  table.emplace(id(2), record(kv::Nop{}, Phase::committed));
  table.emplace(id(3), record(put("different")));
  EXPECT_EQ(scan_known_dependencies(table, target, get()), (Dependencies{id(2)}));
  EXPECT_EQ(scan_known_dependencies(table, id(2), get()), (Dependencies{}));
}
TEST(Dependencies, AllInsertionOrdersProduceTheSameCanonicalResult) {
  std::array<unsigned, 4> order{0, 1, 2, 3};
  const std::array ids{id(4, 4), id(1, 2), id(1, 1), id(2, 3)};
  do {
    Table table;
    for (const auto i : order) table.emplace(ids[i], record(put()));
    const auto result = scan_known_dependencies(table, target, get());
    EXPECT_EQ((std::vector<InstanceId>{result.begin(), result.end()}),
              (std::vector<InstanceId>{id(1, 1), id(1, 2), id(2, 3), id(4, 4)}));
  } while (std::next_permutation(order.begin(), order.end()));
}
TEST(Dependencies, PayloadCommandOverloadMatchesTheExistingRelation) {
  const std::vector<kv::Command> commands{
      get(), put(), kv::CompareAndSwap{"key", std::nullopt, "v"},
      get("other"), put("other"), get(std::string{"a\0b", 3}), put("")};
  std::vector<kv::Payload> payloads{std::monostate{}, kv::Nop{}};
  for (const auto& command : commands) payloads.emplace_back(command);
  for (const auto& left : payloads) for (const auto& right : commands) {
    EXPECT_EQ(kv::conflicts(left, right), kv::conflicts(left, kv::Payload{right}));
    EXPECT_EQ(kv::conflicts(left, right), kv::conflicts(kv::Payload{right}, left));
  }
}
TEST(Dependencies, ReplicaQueriesUseTheLiveTableAndReturnIndependentSets) {
  Replica replica{local};
  EXPECT_TRUE(replica.known_dependencies(target, get()).empty());
  replica.dispatch(Submit{id(2), put()});
  auto earlier = replica.known_dependencies(target, get());
  replica.dispatch(Submit{id(1), put()});
  EXPECT_EQ(earlier, (Dependencies{id(2)}));
  earlier.clear();
  EXPECT_EQ(replica.known_dependencies(target, get()), (Dependencies{id(1), id(2)}));
  EXPECT_TRUE(replica.instances().at(id(1)).state.dependencies.empty());
  EXPECT_EQ(replica.instances().at(id(1)).state.phase, Phase::initial);
  replica.arm_timer(id(1), {1});
  replica.suspend();
  EXPECT_EQ(replica.known_dependencies(target, get()), (Dependencies{id(1), id(2)}));
  replica.resume();
}
}  // namespace egalis::protocol
