#include "egalis/sim/scheduler.hpp"

#include <algorithm>
#include <stdexcept>

namespace egalis::sim {

LogicalTime after(LogicalTime time, Duration delay) {
  if (delay.ticks > std::numeric_limits<std::uint64_t>::max() - time.ticks) {
    throw std::overflow_error("logical time exhausted");
  }
  return LogicalTime{time.ticks + delay.ticks};
}

Scheduler::Scheduler(Observer observer) : observer_(std::move(observer)) {}

void Scheduler::check_healthy() const {
  if (failed_) throw std::logic_error("scheduler failed; start a fresh simulation");
  if (observing_) throw std::logic_error("observers cannot mutate or run the scheduler");
}

void Scheduler::observe(EventAction action, const EventInfo& info) {
  if (!observer_) return;
  observing_ = true;
  try {
    observer_(action, info);
    observing_ = false;
  } catch (...) {
    observing_ = false;
    failed_ = true;
    throw;
  }
}

EventId Scheduler::schedule_at(LogicalTime time, std::string label, Callback callback) {
  check_healthy();
  if (time < now_) throw std::invalid_argument("cannot schedule in the past");
  if (!callback) throw std::invalid_argument("event has no callback");
  if (next_id_ == std::numeric_limits<std::uint64_t>::max() ||
      next_sequence_ == std::numeric_limits<std::uint64_t>::max()) {
    throw std::overflow_error("event identity space exhausted");
  }
  EventInfo info{EventId{next_id_}, EventSequence{next_sequence_}, time, std::move(label)};
  const auto id = info.id;
  const Key key{time, info.sequence};
  auto [it, inserted] = events_.emplace(key, Event{std::move(info), std::move(callback)});
  if (!inserted) throw std::logic_error("event sequence reused");
  ++next_id_;
  ++next_sequence_;
  observe(EventAction::scheduled, it->second.info);
  return id;
}

EventId Scheduler::schedule_after(Duration delay, std::string label, Callback callback) {
  return schedule_at(after(now_, delay), std::move(label), std::move(callback));
}

bool Scheduler::cancel(EventId id) {
  check_healthy();
  const auto it = std::find_if(events_.begin(), events_.end(),
                             [id](const auto& entry) { return entry.second.info.id == id; });
  if (it == events_.end()) return false;
  observe(EventAction::cancelled, it->second.info);
  events_.erase(it);
  return true;
}

bool Scheduler::step() {
  check_healthy();
  if (executing_) throw std::logic_error("recursive event execution is forbidden");
  if (events_.empty()) return false;
  // Extract ownership before invoking user code: it can safely enqueue/cancel.
  auto node = events_.extract(events_.begin());
  auto& event = node.mapped();
  now_ = event.info.time;
  executing_ = true;
  try {
    observe(EventAction::executed, event.info);
    event.callback();
    executing_ = false;
  } catch (...) {
    executing_ = false;
    failed_ = true;
    throw;
  }
  return true;
}

std::size_t Scheduler::run(std::size_t max_events) {
  check_healthy();
  if (executing_) throw std::logic_error("recursive event execution is forbidden");
  std::size_t count = 0;
  while (count < max_events && step()) ++count;
  return count;
}

}  // namespace egalis::sim
