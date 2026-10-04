#pragma once

#include <exception>

namespace stratum::fibers {

class bad_task_access_t final : public std::exception {
public:
  [[nodiscard]] const char* what() const noexcept override {
    return "bad task access";
  }
};

} // namespace stratum::fibers
