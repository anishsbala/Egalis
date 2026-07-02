#include "egalis/sim/network.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace egalis::sim {
namespace {
class Inbox final : public transport::Endpoint {
 public:
  void receive(const protocol::Envelope& envelope) override { messages.push_back(envelope); }
  std::vector<protocol::Envelope> messages;
};
protocol::Envelope message(unsigned source, unsigned destination) {
  return {protocol::ReplicaId{source}, protocol::ReplicaId{destination},
          protocol::Waiting{protocol::InstanceId{1, protocol::ReplicaId{0}}}};
}
Inbox& attach(Network& network, unsigned id) {
  auto inbox = std::make_unique<Inbox>();
  auto& result = *inbox;
  network.attach(protocol::ReplicaId{id}, std::move(inbox));
  return result;
}
}  // namespace

TEST(Network, TypedTransportDeliversWithLogicalDelayAndStableOrder) {
  Network network{Duration{7}};
  attach(network, 0);
  auto& receiver = attach(network, 1);
  transport::Transport& transport = network;
  const auto first = transport.send(message(0, 1));
  const auto second = transport.send(message(0, 1));
  EXPECT_NE(first, second);
  EXPECT_TRUE(receiver.messages.empty());
  EXPECT_EQ(network.scheduler().run(), 2U);
  EXPECT_EQ(network.scheduler().now(), LogicalTime{7});
  ASSERT_EQ(receiver.messages.size(), 2U);
  EXPECT_EQ(receiver.messages.front(), message(0, 1));
}

TEST(Network, SelfDeliveryIsImmediateAndDoesNotQueueEvents) {
  Network network{Duration{100}};
  auto& receiver = attach(network, 0);
  network.send(message(0, 0));
  EXPECT_EQ(receiver.messages.size(), 1U);
  EXPECT_EQ(network.scheduler().pending(), 0U);
  EXPECT_EQ(network.scheduler().now(), LogicalTime{0});
}

TEST(Network, MissingDestinationsDoNotDereferenceInvalidEndpoints) {
  Network network;
  attach(network, 0);
  network.send(message(0, 4));
  EXPECT_EQ(network.scheduler().run(), 1U);
  EXPECT_THROW(network.send(message(1, 0)), std::invalid_argument);
  EXPECT_THROW(network.attach(protocol::ReplicaId{0}, std::make_unique<Inbox>()), std::invalid_argument);
  EXPECT_THROW(network.attach(protocol::ReplicaId{1}, nullptr), std::invalid_argument);
}

TEST(Network, SelfDeliveryDoesNotOverflowUnusedNetworkDelay) {
  Network network{Duration{1}};
  auto& inbox = attach(network, 0);
  network.scheduler().schedule_at({std::numeric_limits<std::uint64_t>::max()}, "last", [] {});
  network.scheduler().run();
  EXPECT_NO_THROW(network.send(message(0, 0)));
  EXPECT_EQ(inbox.messages.size(), 1U);
  EXPECT_THROW(network.send(message(0, 1)), std::overflow_error);
}

TEST(Network, DestroyingWithPendingDeliveriesReleasesOwnedEndpoints) {
  struct Lifetime final : transport::Endpoint {
    explicit Lifetime(bool& destroyed) : destroyed(destroyed) {}
    ~Lifetime() override { destroyed = true; }
    void receive(const protocol::Envelope&) override {}
    bool& destroyed;
  };
  bool destroyed = false;
  {
    Network network;
    network.attach(protocol::ReplicaId{0}, std::make_unique<Lifetime>(destroyed));
    network.send(message(0, 1));
  }
  EXPECT_TRUE(destroyed);
}
}  // namespace egalis::sim
