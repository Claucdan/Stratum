#pragma once

#include "include/fibers/promise/base_promise.h"

#include <concepts>
#include <exception>
#include <type_traits>
#include <variant>

namespace stratum::fibers::details {

template<typename T, bool IS_NOEXCEPT>
struct value_promise_t : base_promise_t {

  template<typename VALUE>
    requires std::constructible_from<T, VALUE&&>
  void return_value(VALUE&& value) noexcept(std::is_nothrow_constructible_v<VALUE&&, T>) {
    result_.template emplace<T>(std::forward<VALUE>(value));
  }

  void unhandled_exception() noexcept {
    if constexpr (IS_NOEXCEPT) {
      std::terminate();
    } else {
      result_.template emplace<std::exception_ptr>(std::current_exception());
    }
  }

  T result() noexcept(IS_NOEXCEPT) {
    if constexpr (!IS_NOEXCEPT) {
      if (auto* exception_ptr = std::get_if<std::exception_ptr>(&result_)) [[unlikely]] {
        std::rethrow_exception(*exception_ptr);
      }
    }
    return std::move(std::get<T>(result_));
  }

private:
  using result_t = std::conditional_t<IS_NOEXCEPT,
                                      std::variant<std::monostate, T>,
                                      std::variant<std::monostate, T, std::exception_ptr>>;

  result_t result_;
};

} // namespace stratum::fibers::details
