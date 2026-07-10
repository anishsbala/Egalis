#include "egalis/sim/replica_endpoint.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace egalis::sim {
namespace {
using namespace protocol;

TEST(AgreementSimulation, FourthReplyCommitsBeforeFifthReplicaReceivesPreaccept) {
  Network network;
  const ReplicaId coordinator{3}, late{4};
  const InstanceId instance{1, coordinator};
  const kv::Command command = kv::Put{"key", "value"};
  std::array<ReplicaEndpoint*, Configuration::replicas> replicas{};
  for (const auto member : members())
    replicas[member.value()] = &ReplicaEndpoint::attach(network, member);

  network.set_link(coordinator, late, true);
  replicas[coordinator.value()]->dispatch(Submit{instance, command});
  network.set_link(coordinator, late, false);
  network.send({coordinator, late, PreAccept{instance, command, {}}},
               DeliveryPlan{false, {5}, false, {5}});

  // The other three remote requests and their replies run before the delayed request.
  for (unsigned event = 0; event < 6; ++event) ASSERT_TRUE(network.scheduler().step());
  EXPECT_EQ(replicas[coordinator.value()]->core().instances().at(instance).state.phase,
            Phase::committed);
  EXPECT_TRUE(replicas[late.value()]->core().instances().empty());
  network.scheduler().run();

  const Commit chosen{{}, instance, command, {}};
  const auto self_packet = canonical({coordinator, coordinator, chosen});
  const std::vector<std::string> expected_sends{
      "effect/send/" + self_packet,
      "effect/send/" + canonical({coordinator, ReplicaId{0}, chosen}),
      "effect/send/" + canonical({coordinator, ReplicaId{1}, chosen}),
      "effect/send/" + canonical({coordinator, ReplicaId{2}, chosen}),
      "effect/send/" + canonical({coordinator, late, chosen})};
  std::vector<std::string> commit_sends;
  std::size_t self_input = 0, self_result = 0, first_remote = 0;
  const auto& trace = network.trace().decisions;
  for (std::size_t index = 0; index < trace.size(); ++index) {
    const auto& decision = trace[index];
    if (decision.kind != DecisionKind::protocol) continue;
    if (std::ranges::find(expected_sends, decision.detail) != expected_sends.end())
      commit_sends.push_back(decision.detail);
    if (decision.detail == "input/peer/" + self_packet) self_input = index;
    if (self_input && !self_result && decision.detail.starts_with("result/"))
      self_result = index;
    if (decision.detail == expected_sends[1]) first_remote = index;
  }
  EXPECT_GT(self_input, 0U);
  EXPECT_GT(self_result, self_input);
  EXPECT_GT(first_remote, self_result);
  EXPECT_EQ(commit_sends, expected_sends);

  for (const auto* replica : replicas) {
    const auto& state = replica->core().instances().at(instance).state;
    EXPECT_EQ(state.phase, Phase::committed);
    EXPECT_EQ(state.command, (kv::Payload{command}));
    EXPECT_TRUE(state.dependencies.empty());
    EXPECT_EQ(state.initial_command.has_value(), replica->core().id() != late);
  }
  EXPECT_EQ(replicas[late.value()]->last_status(), DispatchStatus::stale);
  network.verify_replay_complete();
}
}  // namespace
}  // namespace egalis::sim
