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
  table.at(id(2)).state.ballots = {Ballot::recovery(1, local), Ballot::recovery(1, local)};
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
TEST(Dependencies, ExpansionPreservesUnknownAndNonconflictingCoordinatorKnowledge) {
  Table table;
  table.emplace(id(1), record(put()));
  table.emplace(id(2), record(put("other")));
  const Dependencies proposed{id(2), id(7, 4)};
  const auto result = expand_known_dependencies(table, target, get(), proposed);
  EXPECT_EQ(result, (Dependencies{id(1), id(2), id(7, 4)}));
  EXPECT_EQ(proposed, (Dependencies{id(2), id(7, 4)}));
  EXPECT_EQ(expand_known_dependencies(table, target, get(), {}),
            scan_known_dependencies(table, target, get()));
  EXPECT_EQ(expand_known_dependencies({}, target, get(), proposed), proposed);
}
TEST(Dependencies, InvalidSelfProposalIsRejectedWithoutChangingKnowledge) {
  Replica replica{local};
  replica.dispatch(Submit{id(1), put()});
  const Dependencies proposed{target, id(1)};
  EXPECT_THROW(static_cast<void>(replica.expand_dependencies(target, get(), proposed)),
               std::invalid_argument);
  EXPECT_EQ(proposed, (Dependencies{target, id(1)}));
  EXPECT_FALSE(replica.instances().contains(target));
  EXPECT_EQ(replica.known_dependencies(target, get()), (Dependencies{id(1)}));
  EXPECT_TRUE(replica.instances().at(id(1)).state.dependencies.empty());
}
TEST(Dependencies, CandidateNopKeepsVisibilityAndQueriesPreserveAllInitialMetadata) {
  Table table;
  auto& entry = table.emplace(id(1), record(put(), Phase::preaccepted)).first->second;
  entry.state.initial_command = put();
  entry.state.initial_dependencies = {id(7, 1)};
  entry.state.dependencies = {id(7, 1), id(8, 2)};
  entry.timer = TimerGuard{id(1), {}, {7}};
  const auto initial_command = entry.state.initial_command;
  const auto initial_dependencies = entry.state.initial_dependencies;
  for (const auto phase : {Phase::preaccepted, Phase::accepted, Phase::committed}) {
    entry.state.phase = phase;
    if (phase != Phase::preaccepted) {
      entry.state.ballots = {Ballot::recovery(1, local), Ballot::recovery(1, local)};
      entry.timer->ballot = entry.state.ballots.current;
      entry.state.command = kv::Nop{};
      entry.state.dependencies.clear();  // Recovery's Nop proposal has an empty dep.
    }
    const auto command = entry.state.command;
    const auto dependencies = entry.state.dependencies;
    const auto ballots = entry.state.ballots;
    const auto timer = entry.timer;
    EXPECT_EQ(expand_known_dependencies(table, target, get(), {}), (Dependencies{id(1)}));
    // Looking only at the original key would lose Nop's conflicts with other keys.
    EXPECT_EQ(expand_known_dependencies(table, target, get("other"), {}),
              phase == Phase::preaccepted ? Dependencies{} : Dependencies{id(1)});
    EXPECT_EQ(entry.state.command, command);
    EXPECT_EQ(entry.state.initial_command, initial_command);
    EXPECT_EQ(entry.state.initial_dependencies, initial_dependencies);
    EXPECT_EQ(entry.state.dependencies, dependencies);
    EXPECT_EQ(entry.state.phase, phase);
    EXPECT_EQ(entry.state.ballots.current, ballots.current);
    EXPECT_EQ(entry.state.ballots.accepted, ballots.accepted);
    EXPECT_EQ(entry.timer, timer);
  }
  // The fixture describes candidate records, not an implementation/test of the
  // corresponding recovery transitions (which remain later milestones).
}
TEST(Dependencies, ValidatedInitialRecordsAreVisibleButBottomIsNot) {
  Table table;
  auto& validated = table.emplace(id(1), record(put())).first->second;
  validated.state.initial_command = put();
  validated.state.initial_dependencies = {id(7, 1)};
  validated.state.ballots.current = Ballot::recovery(1, local);
  // Figure 5 validation may leave phase initial and abal zero. No phase filter
  // may erase its cmd knowledge. An untouched initial bottom record is different.
  table.emplace(id(2), record(std::monostate{}));
  EXPECT_EQ(expand_known_dependencies(table, target, get(), {}), (Dependencies{id(1)}));
  EXPECT_EQ(validated.state.phase, Phase::initial);
  EXPECT_TRUE(validated.state.ballots.accepted.initial());
  EXPECT_EQ(validated.state.initial_dependencies, (Dependencies{id(7, 1)}));
  EXPECT_EQ(validated.state.initial_command, put());
}
TEST(Dependencies, ExpansionIsNotTransitiveClosureAndDoesNotOverwriteStoredProposals) {
  Table table;
  auto& known = table.emplace(id(1), record(put(), Phase::preaccepted)).first->second;
  known.state.initial_command = put();
  known.state.initial_dependencies = {id(7, 1)};
  known.state.dependencies = {id(4), id(7, 1)};
  table.emplace(id(4), record(get("other")));
  const Dependencies proposed{id(8, 3)};
  EXPECT_EQ(expand_known_dependencies(table, target, get(), proposed),
            (Dependencies{id(1), id(8, 3)}));
  EXPECT_EQ(known.state.dependencies, (Dependencies{id(4), id(7, 1)}));
  EXPECT_EQ(known.state.initial_dependencies, (Dependencies{id(7, 1)}));
}
TEST(Dependencies, UnsupportedCandidateChangesCannotAlterTheLiveOracle) {
  Replica replica{local};
  replica.dispatch(Submit{id(1), put()});
  const auto ballot = Ballot::recovery(1, local);
  const Dependencies proposed{id(7, 4)};
  const auto before = replica.expand_dependencies(target, get(), proposed);
  for (const Message& message : std::vector<Message>{
      Accept{ballot, id(1), kv::Nop{}, {}}, Commit{ballot, id(1), kv::Nop{}, {}},
      Validate{ballot, id(1), put("other"), {}}}) {
    const auto result = replica.dispatch(PeerMessage{{local, local, message}});
    EXPECT_EQ(result.status, DispatchStatus::unsupported);
    EXPECT_TRUE(result.effects.empty());
    EXPECT_EQ(replica.expand_dependencies(target, get(), proposed), before);
    EXPECT_EQ(replica.instances().at(id(1)).state.command, (kv::Payload{put()}));
    EXPECT_FALSE(replica.instances().at(id(1)).state.initial_command);
  }
}
}  // namespace egalis::protocol
