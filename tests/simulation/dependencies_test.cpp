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
      EXPECT_EQ(core.known_dependencies(target, read), (Dependencies{InstanceId{1, member}}));
      EXPECT_EQ(core.expand_dependencies(target, write, proposed),
                (Dependencies{InstanceId{1, member}, InstanceId{2, member}, InstanceId{100, ReplicaId{4}}}));
      EXPECT_EQ(core.instances().size(), 3U);
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
  EXPECT_EQ(replicas[0]->core().known_dependencies(InstanceId{9, ReplicaId{0}}, read),
            (Dependencies{InstanceId{1, ReplicaId{0}}}));
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
  EXPECT_EQ(network.scheduler().run(), 3U);
  EXPECT_EQ(observed, (std::vector<Dependencies>{{}, {first}, {first, second}}));
  EXPECT_EQ(replica.core().instances().at(second).state.phase, protocol::Phase::initial);
  EXPECT_TRUE(replica.core().instances().at(second).state.dependencies.empty());
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
