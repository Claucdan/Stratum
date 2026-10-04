# Stratum

A small experimental C++20 project with coroutine primitives and a planned lightweight scheduler.

## What is available

Task types in `stratum::fibers`:

- `task_t<T>` — a coroutine that returns a value of type `T` or no value (`void`). Supports `co_await` and propagates exceptions to the awaiting coroutine.
- `noexcept_task_t<T>` — a task variant that calls `std::terminate()` on an unhandled exception.

Tasks own their coroutine frames, support move semantics, and release their frames on destruction. Execution starts when a task is awaited.

Internal promise types in `stratum::fibers::details`:

- `base_promise_t` — initial and final suspension, and transfer of control back to the awaiting coroutine.
- `value_promise_t<T, IS_NOEXCEPT>` — result storage and exception handling.
- `void_promise_t<IS_NOEXCEPT>` — completion without a result and exception handling.

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

The scheduler is not implemented yet. Policies for selecting levels and moving tasks between them are still to be defined.

## Inspiration

Stratum draws inspiration from [cppcoro](https://github.com/lewissbaker/cppcoro), [libcoro](https://github.com/jbaldwin/libcoro), and [libunifex](https://github.com/facebookexperimental/libunifex).
