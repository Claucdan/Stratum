# Stratum

A small experimental C++20 coroutine framework for Linux, focused on explicit coroutine ownership and lightweight scheduling. Stratum is header-only and is not intended to become a full asynchronous runtime.

## What is available

Task types in `stratum::fibers`:

- `task_t<T>` — a coroutine that returns a value of type `T` or no value (`void`). Supports `co_await` and propagates exceptions to the awaiting coroutine.
- `noexcept_task_t<T>` — a task variant that calls `std::terminate()` on an unhandled exception.

Tasks own their coroutine frames, support move semantics, and release their frames on destruction. Execution starts when a task is awaited.

```cpp
#include "include/fibers/task.h"

stratum::fibers::task_t<int> answer() {
    co_return 42;
}

stratum::fibers::task_t<void> work() {
    const int value = co_await answer();
    // Use value here.
    co_return;
}
```

This defines lazy tasks; calling `work()` alone does not execute its body. There is currently no public `sync_wait()` or scheduler `spawn()` helper. The owning task must remain alive while its coroutine is suspended or referenced elsewhere. Awaiting a task does not transfer ownership of its frame.

Internal promise types in `stratum::fibers::details` handle result storage, exceptions, and symmetric transfer back to the awaiting coroutine at final suspension.

## Current scheduler prototype

[`round_robing_scheduler_t`](include/scheduler/fifo/fifo-scheduler.h) in `stratum::scheduler` uses one mutex-protected FIFO queue and can run worker threads. The type name above matches the current API.

- `add_task(scheduler_task_t)` queues a unit of work.
- `schedule(threads_count, reuse_thread = true)` starts processing. The count must be positive. By default, the calling thread runs a worker loop and the call blocks; with `reuse_thread = false`, all workers run on separate threads.
- `stop()` requests that worker loops stop. The destructor calls `stop()` and joins the worker threads.

An empty queue does not stop the scheduler: workers poll until stopped. Each queued item is executed once; suspended coroutines are not automatically requeued. Stopping does not guarantee that queued work is drained.

`scheduler_task_t` stores a non-owning reference to an lvalue callable or a coroutine handle. Callers must keep the callable or coroutine frame alive until execution finishes. When submitting a coroutine handle, callers must also ensure it is valid to resume and has any required continuation set up. The scheduler does not destroy coroutine frames or retrieve task results.

Exceptions derived from `std::exception` thrown by callables are logged by the worker, which then requests a stop. Coroutine task exceptions follow the promise behavior described above.

## Planned scheduler

A single-threaded cooperative scheduler inspired by Linux O(1) is planned. It will use multiple levels, each with its own queue of runnable tasks:

```text
scheduler
├── level 0: task → task → ...
├── level 1: task → task → ...
└── level N: task → task → ...
```

The scheduler will select a runnable task and resume its coroutine. Suspension points, such as a future `yield()`, will return control to the scheduler.

The goals are cheap task selection, minimal allocations in queue operations, and explicit coroutine lifetime management. Level queues will be maintained directly, without an `active` / `expired` pair or array swaps.

The current FIFO prototype does not implement this multi-level design. Level-selection and task-migration policies remain undefined, and `yield()` is not available yet.

## Building

Requirements: Linux, a C++20 compiler (Clang or GCC), and CMake 3.20 or newer.

```sh
cmake -S . -B build
cmake --build build
```

The CMake target is an `INTERFACE` library, so these commands configure the project but do not compile the headers or run checks. To use Stratum from another CMake project:

```cmake
add_subdirectory(path/to/Stratum)
target_link_libraries(your_target PRIVATE stratum::stratum)
```

The target supplies the repository root as an include directory and requires C++20. Applications using the threaded scheduler should also link CMake's `Threads::Threads` target after `find_package(Threads REQUIRED)`.

With direnv, run `direnv allow` to load `.envrc`. It selects Clang and Ninja and defaults the C and C++ compiler launchers to `ccache`, preserving existing launcher settings. These tools must be installed when using those defaults. Load the environment before the first CMake configuration; existing build directories retain their cached settings.

## Inspiration

Stratum draws inspiration from [cppcoro](https://github.com/lewissbaker/cppcoro), [libcoro](https://github.com/jbaldwin/libcoro), and [libunifex](https://github.com/facebookexperimental/libunifex).
