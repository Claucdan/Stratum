#pragma once

#include <coroutine>
#include <functional>
#include <memory>

namespace stratum::scheduler {

namespace details {

inline void coroutine_resume_task(void* object) noexcept {
  std::coroutine_handle<>::from_address(object).resume();
}

template<typename F>
inline void invoke_resume_task(void* object) {
  std::invoke(*static_cast<F*>(object));
}

} // namespace details

class scheduler_task_t {
public:
  scheduler_task_t(std::coroutine_handle<> handle)
      : object_(handle.address()), func(details::coroutine_resume_task) {}

  template<typename F>
  scheduler_task_t(F& callable)
      : object_(std::addressof(callable)), func(details::invoke_resume_task<F>) {}

  void execute() {
    func(object_);
  }

private:
  void* object_;
  void (*func)(void*);
};

} // namespace stratum::scheduler
