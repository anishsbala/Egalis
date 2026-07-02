#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <utility>

namespace egalis::sim {

struct LogicalTime {
  std::uint64_t ticks = 0;
  auto operator<=>(const LogicalTime&) const = default;
};
struct Duration {
  std::uint64_t ticks = 0;
  auto operator<=>(const Duration&) const = default;
};
struct EventId {
  std::uint64_t value;
  auto operator<=>(const EventId&) const = default;
};
struct EventSequence {
  std::uint64_t value;
  auto operator<=>(const EventSequence&) const = default;
};
[[nodiscard]] LogicalTime after(LogicalTime time, Duration delay);

struct EventInfo {
  EventId id;
  EventSequence sequence;
  LogicalTime time;
  std::string label;
  bool operator==(const EventInfo&) const = default;
};
enum class EventAction { scheduled, executed, cancelled };

class Scheduler {
 public:
  using Callback = std::function<void()>;
  using Observer = std::function<void(EventAction, const EventInfo&)>;
  explicit Scheduler(Observer observer = {});
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;

  EventId schedule_at(LogicalTime time, std::string label, Callback callback);
  EventId schedule_after(Duration delay, std::string label, Callback callback);
  bool cancel(EventId id);
  bool step();
  std::size_t run(std::size_t max_events = std::numeric_limits<std::size_t>::max());
  [[nodiscard]] LogicalTime now() const noexcept { return now_; }
  [[nodiscard]] std::size_t pending() const noexcept { return events_.size(); }
  [[nodiscard]] bool failed() const noexcept { return failed_; }
 private:
  struct Event { EventInfo info; Callback callback; };
  using Key = std::pair<LogicalTime, EventSequence>;
  void check_healthy() const;
  void observe(EventAction action, const EventInfo& info);
  std::map<Key, Event> events_;
  LogicalTime now_;
  std::uint64_t next_id_ = 1;
  std::uint64_t next_sequence_ = 0;
  Observer observer_;
  bool executing_ = false;
  bool failed_ = false;
};

}  // namespace egalis::sim
