#pragma once

#include "include/fibers/exceptions.h"
#include "include/fibers/promise/value_promise.h"
#include "include/fibers/promise/void_promise.h"

#include <coroutine>
#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

namespace stratum::fibers {

namespace details {

template<typename T, bool IS_NOEXCEPT>
struct task_t {
  using promise_base_t = std::conditional_t<std::is_void_v<T>,
                                            details::void_promise_t<IS_NOEXCEPT>,
                                            details::value_promise_t<T, IS_NOEXCEPT>>;

public:
  struct promise_type : promise_base_t {
    using promise_base_t::promise_base_t;

    task_t get_return_object() noexcept {
      return task_t{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
  };

  struct awaitable_t {
    [[nodiscard]] bool await_ready() const noexcept {
      return !handle_ || handle_.done();
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept {
      handle_.promise().set_continuation(handle);
      return handle_;
    }

    decltype(auto) await_resume() noexcept(IS_NOEXCEPT) {
      if (!handle_) [[unlikely]] {
        if constexpr (IS_NOEXCEPT) {
          std::terminate();
        } else {
          throw bad_task_access_t{};
        }
      }
      return std::move(handle_.promise()).result();
    }

    std::coroutine_handle<promise_type> handle_;
  };

  task_t() noexcept
      : handle_(std::noop_coroutine()) {}

  task_t(const task_t&) = delete;

  task_t(task_t&& other) noexcept
      : handle_(std::exchange(other.handle_, nullptr)) {}

  explicit task_t(std::coroutine_handle<promise_type> handle) noexcept
      : handle_(handle) {}

  task_t& operator=(const task_t&) = delete;

  task_t& operator=(task_t&& other) noexcept {
    if (this == std::addressof(other)) {
      return *this;
    }

    if (handle_) {
      handle_.destroy();
    }

    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
  }

  ~task_t() {
    if (handle_) {
      handle_.destroy();
    }
  }

  awaitable_t operator co_await() const noexcept {
    return awaitable_t{handle_};
  }

  [[nodiscard]] bool is_ready() const noexcept {
    return !handle_ || handle_.done();
  }

private:
  std::coroutine_handle<promise_type> handle_;
};

} // namespace details

template<typename T>
using task_t = details::task_t<T, false>;

template<typename T>
using noexcept_task_t = details::task_t<T, true>;

} // namespace stratum::fibers
