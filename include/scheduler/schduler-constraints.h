#pragma once

#include "include/scheduler/scheduler-task.h"

#include <concepts>

namespace stratum::scheduler {

template<typename T>
concept scheduler_c = requires(T scheduler, scheduler_task_t task, size_t thread_count) {
  { scheduler.add_task(task) } -> std::same_as<void>;
  { scheduler.schedule(thread_count) } -> std::same_as<void>;
};

} // namespace stratum::scheduler
