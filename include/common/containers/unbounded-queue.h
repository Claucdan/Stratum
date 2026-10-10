#pragma once

#include <deque>
#include <mutex>
#include <optional>

namespace stratum::common {

template<typename T>
class unbounded_queue_t {
public:
  void push(T value) {
    std::lock_guard lock(mutex_);
    queue_.push_back(std::move(value));
  }

  std::optional<T> pop() {
    std::lock_guard lock(mutex_);

    if (queue_.empty()) {
      return std::nullopt;
    }

    T value = std::move(queue_.front());
    queue_.pop_front();

    return value;
  }

private:
  mutable std::mutex mutex_;
  std::deque<T> queue_;
};

} // namespace stratum::common
