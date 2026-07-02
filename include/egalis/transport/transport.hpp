#pragma once

#include "egalis/protocol/messages.hpp"

#include <compare>
#include <cstdint>

namespace egalis::transport {

struct MessageId {
  std::uint64_t value;
  auto operator<=>(const MessageId&) const = default;
};

class Endpoint {
 public:
  virtual ~Endpoint() = default;
  virtual void receive(const protocol::Envelope& envelope) = 0;
  // Serialized simulator lifecycle notifications; default inboxes have no state
  // to invalidate. They do not imply durable protocol recovery.
  virtual void on_crash() {}
  virtual void on_restart() {}
};

// Typed transport contract shared by simulated and future real effect runners.
// A successful send allocates an ID; it does not promise remote delivery.
class Transport {
 public:
  virtual ~Transport() = default;
  virtual MessageId send(protocol::Envelope envelope) = 0;
};

}  // namespace egalis::transport
