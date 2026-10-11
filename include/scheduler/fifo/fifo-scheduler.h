#pragma once

#include "include/common/containers/unbounded-queue.h"
#include "include/scheduler/schduler-constraints.h"
#include "include/scheduler/scheduler-task.h"
#include <atomic>
#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace stratum::scheduler {

class round_robing_scheduler_t {
public:
  round_robing_scheduler_t() = default;
  round_robing_scheduler_t(const round_robing_scheduler_t&) = delete;
  round_robing_scheduler_t(round_robing_scheduler_t&&) = delete;

  round_robing_scheduler_t& operator=(const round_robing_scheduler_t&) = delete;
  round_robing_scheduler_t& operator=(round_robing_scheduler_t&&) = delete;

  ~round_robing_scheduler_t() {
    stop();
    for (auto& worker : workers_) {
      worker.join();
    }
  }

  void add_task(scheduler_task_t task) {
    queue_.push(task);
  }

  void schedule(size_t threads_count, bool reuse_thread = true) {
    if (threads_count == 0) {
      throw std::runtime_error("invalid threads count");
    }

    const size_t base_threads_count = threads_count - 1 * reuse_thread;
    workers_.reserve(base_threads_count);
    work_.store(true, std::memory_order_relaxed);
    for (size_t i = 0; i < base_threads_count; ++i) {
      workers_.emplace_back([this] { worker_loop(); });
    }
    if (reuse_thread) {
      worker_loop();
    }
  }

  void stop() {
    work_.store(false, std::memory_order_relaxed);
  }

private:
  static constexpr auto DEFAULT_THROTLE_TIME = std::chrono::nanoseconds(100);

  void worker_loop() {
    while (work_.load(std::memory_order_relaxed)) {
      auto task = queue_.pop();
      if (!task) {
        std::this_thread::sleep_for(DEFAULT_THROTLE_TIME);
        continue;
      }
      try {
        task->execute();
      } catch (const std::exception& ex) {
        std::cerr << "[SCHEDULER] some worker throw exception this msg: [" << ex.what() << "]";
        work_.store(false, std::memory_order_relaxed);
      }
    }
  }

  std::atomic_bool work_;
  std::vector<std::thread> workers_;
  common::unbounded_queue_t<scheduler_task_t> queue_;
};

static_assert(scheduler_c<round_robing_scheduler_t>);

} // namespace stratum::scheduler
