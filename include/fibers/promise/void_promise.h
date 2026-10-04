#pragma once

#include "include/fibers/promise/base_promise.h"

#include <exception>
#include <type_traits>
#include <variant>

namespace stratum::fibers::details {

template<bool IS_NOEXCEPT>
struct void_promise_t : base_promise_t {

  void return_void() noexcept {}

  void unhandled_exception() noexcept {
    if constexpr (IS_NOEXCEPT) {
      std::terminate();
    } else {
      result_ = std::current_exception();
    }
  }

  void result() noexcept(IS_NOEXCEPT) {
    if constexpr (!IS_NOEXCEPT) {
      if (result_) [[unlikely]] {
        std::rethrow_exception(result_);
      }
    }
  }

private:
  using result_t = std::conditional_t<IS_NOEXCEPT, std::monostate, std::exception_ptr>;

  result_t result_;
};

} // namespace stratum::fibers::details
