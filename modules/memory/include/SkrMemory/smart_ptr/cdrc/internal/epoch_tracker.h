
#ifndef CDRC_INTERNAL_EPOCH_TRACKER_H
#define CDRC_INTERNAL_EPOCH_TRACKER_H

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <atomic>
#include <algorithm>
#include <functional>
#include <memory>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "utils.h"

namespace cdrc {
namespace internal {

struct epoch_tracker {

  using epoch_type = uint64_t;
  constexpr static epoch_type no_epoch = std::numeric_limits<epoch_type>::max();

  struct alignas(128) Epoch : public std::atomic<epoch_type> {
    Epoch() : std::atomic<epoch_type>(no_epoch) {}

    explicit Epoch(epoch_type x) : std::atomic<epoch_type>(x) {}

    Epoch(const Epoch &other) : std::atomic<epoch_type>(other.load()) {}

    ~Epoch() = default;
  };

  static epoch_tracker& instance() {
    static epoch_tracker tracker;
    return tracker;
  }

  // Return the value of the current global epoch
  epoch_type get_current_epoch() {
    return global_epoch.load();
  }

  // Increment the global epoch
  auto advance_global_epoch() {
    return global_epoch.fetch_add(1);
  }

  // Return the value of the earliest announced epoch. Returns
  // numeric_limits<epoch_type>::max() if no threads has an
  // active announcement.
  epoch_type get_min_announced_epoch() {
    epoch_type answer = std::numeric_limits<epoch_type>::max();
    auto nt = utils::num_threads();
    for (size_t i = 0; i < nt; i++) {
      answer = std::min(answer, local_epoch[i].load(std::memory_order_acquire));
    }
    return answer;
  }

  // Apply the function f(i, e) to every currently announced epoch e at index i
  template<typename F>
  void scan_announced_epochs(F &&f) {
    std::atomic_thread_fence(std::memory_order_seq_cst); // TODO: is this necessary?
    auto nt = utils::num_threads();
    for (size_t i = 0; i < nt; i++) {
      epoch_type e = local_epoch[i].load(std::memory_order_acquire);
      if(e != no_epoch) f(i, e);
    }
  }

  bool begin_critical_section() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    if (critical_section[id]) {
      return false;
    } else {
      auto current = global_epoch.load(std::memory_order_acquire);
      local_epoch[id].exchange(current);
      critical_section[id] = true;
      return true;
    }
  }

  void end_critical_section() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    assert(critical_section[id]);
    assert(local_epoch[id].load() != no_epoch);
    local_epoch[id].store(no_epoch, std::memory_order_release);
    critical_section[id] = false;
  }

  bool in_critical_section() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    return critical_section[id];
  }

private:
  epoch_tracker() : global_epoch(0), local_epoch(utils::num_threads()), critical_section(utils::num_threads()) {}

  Epoch global_epoch;
  std::vector <Epoch> local_epoch;
  std::vector <utils::Padded<bool>> critical_section;
};

}  // namespace internal

struct epoch_guard {
  epoch_guard() : engaged(internal::epoch_tracker::instance().begin_critical_section()) {}

  ~epoch_guard() {
    if (engaged) { internal::epoch_tracker::instance().end_critical_section(); }
  }

  epoch_guard(const epoch_guard &) = delete;

  epoch_guard(epoch_guard &&) = delete;

  epoch_guard &operator=(const epoch_guard &) = delete;

  epoch_guard &operator=(epoch_guard &&) = delete;

private:
  bool engaged;
};

template<typename F>
std::invoke_result_t <F> with_epoch_guard(F &&f) {
  epoch_guard g;
  return std::invoke(std::forward<F>(f));
}

}  // namespace cdrc

#endif  // CDRC_INTERNAL_EPOCH_TRACKER_H
