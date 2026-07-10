#include "egalis/sim/replica_endpoint.hpp"

#include <gtest/gtest.h>
#include <array>
#include <vector>

namespace egalis::sim {
namespace {
using protocol::Dependencies;
using protocol::InstanceId;
using protocol::ReplicaId;
void scenario(Network& network) {
  std::array<ReplicaEndpoint*, 5> replicas{};
  const kv::Command read{kv::Get{"shared"}}, write{kv::Put{"shared", "value"}};
  for (const auto member : protocol::members()) {
    auto& replica = ReplicaEndpoint::attach(network, member);
    replicas[member.value()] = &replica;
    replica.dispatch(protocol::Submit{InstanceId{1, member}, write});
    replica.dispatch(protocol::Submit{InstanceId{2, member}, read});
    replica.dispatch(protocol::Submit{InstanceId{3, member}, kv::Put{"unrelated", "v"}});
  }
  const auto inspect = [&] {
    const auto before = network.trace();
    for (const auto member : protocol::members()) {
      const auto& core = replicas[member.value()]->core();
      const InstanceId target{9, member};
      const Dependencies proposed{InstanceId{100, ReplicaId{4}}};
      Dependencies expected_read, expected_write = proposed;
      for (const auto& [known, record] : core.instances()) {
        // The workload assigns counter 1 to a shared PUT, 2 to a shared GET,
        // and 3 to an unrelated PUT. Peer PreAccept now installs real knowledge.
        if (known.counter() == 1) expected_read.insert(known);
        if (known.counter() == 1 || known.counter() == 2) expected_write.insert(known);
        EXPECT_LE(known.counter(), 3U);
        EXPECT_EQ(record.state.phase, protocol::Phase::preaccepted);
      }
      EXPECT_EQ(core.known_dependencies(target, read), expected_read);
      EXPECT_EQ(core.expand_dependencies(target, write, proposed), expected_write);
      EXPECT_GE(core.instances().size(), 3U);
      EXPECT_LE(core.instances().size(), 15U);
      for (std::uint64_t counter = 1; counter <= 3; ++counter)
        EXPECT_TRUE(core.instances().contains(InstanceId{counter, member}));
    }
    EXPECT_EQ(network.trace(), before);  // Queries introduce no events or effects.
  };
  inspect();
  for (const auto source : protocol::members()) {
    for (const auto destination : protocol::members()) {
      replicas[source.value()]->execute({protocol::SendMessage{
          {source, destination, protocol::Waiting{InstanceId{1, source}}}}});
    }
  }
  const std::array left{ReplicaId{0}};
  const std::array right{ReplicaId{1}, ReplicaId{2}};
  network.partition(left, right);
  replicas[0]->arm_timer(InstanceId{1, ReplicaId{0}}, {3});
  network.crash(ReplicaId{0});
  inspect();  // Read-only inspection is not processing an input while crashed.
  network.scheduler().run(1000);
  inspect();  // Check every live table after faulted remote deliveries, not just before them.
  network.restart(ReplicaId{0});
  network.partition(left, right, false);
  replicas[0]->dispatch(protocol::Submit{InstanceId{4, ReplicaId{0}}, kv::Get{"unrelated"}});
  Dependencies expected;
  for (const auto member : protocol::members())
    if (replicas[0]->core().instances().contains(InstanceId{1, member}))
      expected.insert(InstanceId{1, member});
  EXPECT_EQ(replicas[0]->core().known_dependencies(InstanceId{9, ReplicaId{0}}, read), expected);
  network.scheduler().run(1000);
  network.verify_replay_complete();
}
}
TEST(DependencySimulation, EqualTimeSubmissionsExposeOnlyPreviouslyKnownConflicts) {
  Network network;
  const ReplicaId local{0};
  auto& replica = ReplicaEndpoint::attach(network, local);
  const InstanceId first{1, local}, second{2, local}, target{9, local};
  const kv::Command write{kv::Put{"key", "v"}};
  std::vector<Dependencies> observed;
  network.timer(local, {2}, "submit-first", [&] {
    observed.push_back(replica.core().known_dependencies(first, write));
    replica.dispatch(protocol::Submit{first, write});
  });
  network.timer(local, {2}, "submit-second", [&] {
    observed.push_back(replica.core().known_dependencies(second, write));
    replica.dispatch(protocol::Submit{second, write});
  });
  network.timer(local, {2}, "inspect-both", [&] {
    const auto before = network.trace();
    observed.push_back(replica.core().expand_dependencies(target, write, {}));
    EXPECT_EQ(network.trace(), before);
  });
  for (int i = 0; i < 3; ++i) EXPECT_TRUE(network.scheduler().step());
  EXPECT_EQ(observed, (std::vector<Dependencies>{{}, {first}, {first, second}}));
  EXPECT_EQ(replica.core().instances().at(second).state.phase, protocol::Phase::preaccepted);
  EXPECT_EQ(replica.core().instances().at(second).state.dependencies, (Dependencies{first}));
  network.scheduler().run();
}
TEST(DependencySimulation, FaultedLiveCoreQueriesPreserveSeededTracesAndReplay) {
  const FaultPolicy policy{{1}, {5}, 200000, 700000, {1}};
  for (std::uint64_t seed = 0; seed < 8; ++seed) {
    Network original{seed, policy}; scenario(original);
    Network repeated{seed, policy}; scenario(repeated);
    EXPECT_EQ(original.trace(), repeated.trace());
    Network replay{parse_trace(serialize(original.trace())), policy}; scenario(replay);
    EXPECT_EQ(original.trace(), replay.trace());
  }
}
}  // namespace egalis::sim
