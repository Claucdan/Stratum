#pragma once

#include <coroutine>

namespace stratum::fibers::details {

struct base_promise_t {
  struct awaitable_t {
    [[nodiscard]] bool await_ready() const noexcept {
      return false;
    }

    template<typename PROMISE>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<PROMISE> handle) noexcept {
      return handle.promise().continuation_;
    }

    void await_resume() const noexcept {}
  };

  base_promise_t() = default;

  std::suspend_always initial_suspend() {
    return {};
  }

  awaitable_t final_suspend() noexcept {
    return awaitable_t{};
  }

  void set_continuation(std::coroutine_handle<> continuation) noexcept {
    continuation_ = continuation;
  }

private:
  std::coroutine_handle<> continuation_;

  friend struct awaitable_t;
};

} // namespace stratum::fibers::details
