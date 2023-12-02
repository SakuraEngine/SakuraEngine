
#ifndef CDRC_ATOMIC_WEAK_PTR_H
#define CDRC_ATOMIC_WEAK_PTR_H

#include <cstddef>

#include <atomic>
#include <type_traits>

#include "internal/counted_object.h"
#include "internal/fwd_decl.h"
#include "internal/utils.h"

#include "snapshot_ptr.h"
#include "weak_ptr.h"
#include "weak_snapshot_ptr.h"

namespace cdrc {

template<typename T, typename memory_manager, typename pointer_policy>
class atomic_weak_ptr : public pointer_policy::template arc_ptr_policy<T> {

  // The pointer_policy template argument introduces a customization point
  // that allows atomic_rc_ptr to be used with various kinds of marked
  // pointers. pointer_policy should defined a typename pointer_type, which
  // is implicitly convertible to T* and support all of the usual pointer-like
  // operations. It should also define three member types arc_ptr_policy,
  // rc_ptr_policy, and snapshot_ptr_policy, which can add additional methods
  // to the corresponding pointer types. See marked_arc_ptr.h for an example.

 private:
  using counted_object_t = internal::counted_object<T>;
  using counted_ptr_t = typename pointer_policy::template pointer_type<counted_object_t>;

  using atomic_ptr_t = atomic_rc_ptr<T, memory_manager, pointer_policy>;
  using rc_ptr_t = rc_ptr<T, memory_manager, pointer_policy>;
  using weak_ptr_t = weak_ptr<T, memory_manager, pointer_policy>;
  using snapshot_ptr_t = snapshot_ptr<T, memory_manager, pointer_policy>;
  using weak_snapshot_ptr_t = weak_snapshot_ptr<T, memory_manager, pointer_policy>;

  friend rc_ptr_t;
  friend weak_ptr_t;
  friend snapshot_ptr_t;

  friend typename pointer_policy::template arc_ptr_policy<T>;

 public:
  atomic_weak_ptr() : atomic_ptr(nullptr) {}

  /* implicit */ atomic_weak_ptr(std::nullptr_t) : atomic_ptr(nullptr) {}

  /* implicit */ atomic_weak_ptr(weak_ptr_t desired) : atomic_ptr(desired.release()) {}

  ~atomic_weak_ptr() {
    auto ptr = atomic_ptr.load();
    if (ptr != nullptr) mm.delayed_decrement_weak_cnt(ptr);
  }

  atomic_weak_ptr(const atomic_weak_ptr &) = delete;

  atomic_weak_ptr &operator=(const atomic_weak_ptr &) = delete;

  atomic_weak_ptr(atomic_weak_ptr &&) = delete;

  atomic_weak_ptr &operator=(atomic_weak_ptr &&) = delete;

  [[nodiscard]] bool is_lock_free() const noexcept { return true; }

  static constexpr bool is_always_lock_free = true;

  void store(std::nullptr_t) noexcept {
    auto old_ptr = atomic_ptr.exchange(nullptr, std::memory_order_seq_cst);
    if (old_ptr != nullptr) mm.delayed_decrement_weak_cnt(old_ptr);
  }

  void store(weak_ptr_t desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
    auto new_ptr = desired.release();
    auto old_ptr = atomic_ptr.exchange(new_ptr, order);
    if (old_ptr != nullptr) mm.delayed_decrement_weak_cnt(old_ptr);
  }

  void store_non_racy(weak_ptr_t desired) noexcept {
    auto new_ptr = desired.release();
    auto old_ptr = atomic_ptr.load();
    atomic_ptr.store(new_ptr, std::memory_order_release);
    if (old_ptr != nullptr) mm.delayed_decrement_weak_cnt(old_ptr);
  }

  void store(const weak_snapshot_ptr_t &desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
    auto new_ptr = desired.get_counted();
    if (new_ptr != nullptr) mm.increment_weak_cnt(new_ptr);
    auto old_ptr = atomic_ptr.exchange(new_ptr, order);
    if (old_ptr != nullptr) mm.delayed_decrement_weak_cnt(old_ptr);
  }

  void store(const snapshot_ptr_t &desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
    auto new_ptr = desired.get_counted();
    if (new_ptr != nullptr) mm.increment_weak_cnt(new_ptr);
    auto old_ptr = atomic_ptr.exchange(new_ptr, order);
    if (old_ptr != nullptr) mm.delayed_decrement_weak_cnt(old_ptr);
  }

  weak_ptr_t load() const noexcept {
    auto acquired_ptr = mm.acquire(&atomic_ptr);
    weak_ptr_t result(acquired_ptr.get(), weak_ptr_t::AddRef::yes);
    return result;
  }

  weak_snapshot_ptr_t get_snapshot() const noexcept {
    while (true) {
      auto p = mm.protect_snapshot(&atomic_ptr);
      auto ptr = p.get();
      if (ptr && ptr->get_use_count() > 0) return weak_snapshot_ptr_t(std::move(p));
      else if (ptr == nullptr || atomic_ptr.load() == ptr) {
        return weak_snapshot_ptr_t(nullptr);
      }
    }
  }

  bool compare_exchange_weak(weak_ptr_t &expected, const weak_ptr_t &desired) noexcept {
    if (!compare_and_swap(expected, desired)) {
      expected = load();
      return false;
    } else
      return true;
  }

  bool compare_exchange_weak(weak_snapshot_ptr_t &expected, const weak_ptr_t &desired) noexcept {
    if (!compare_and_swap(expected, desired)) {
      expected = get_snapshot();
      return false;
    } else
      return true;
  }

  // Atomically compares the underlying weak_ptr with expected, and if they refer to
  // the same managed object, replaces the current weak_ptr with a copy of desired
  // (incrementing its reference count) and returns true. Otherwise, returns false.
  template<typename P1, typename P2>
  bool compare_and_swap(const P1& expected, const P2& desired) noexcept {

    // We need to make a reservation if the desired snapshot pointer no longer has
    // an announcement slot. Otherwise, desired is protected, assuming that another
    // thread can not clear the announcement slot (this might change one day!)
    [[maybe_unused]] auto reservation = !desired.is_protected() ? mm.reserve(desired.get_counted()) :
                                                                  mm.template reserve_nothing<counted_ptr_t>();

    if (compare_and_swap_impl(expected.get_counted(), desired.get_counted())) {
      auto desired_ptr = desired.get_counted();
      if (desired_ptr != nullptr) mm.increment_weak_cnt(desired_ptr);
      return true;
    } else {
      return false;
    }
  }

  // Atomically compares the underlying weak_ptr with expected, and if they are equal,
  // replaces the current weak_ptr with desired by move assignment, hence leaving its
  // reference count unchanged. Otherwise returns false and leaves desired unmodified.
  template<typename P1, typename P2>
  auto compare_and_swap(const P1& expected, P2&& desired) noexcept
      -> std::enable_if_t<std::is_rvalue_reference_v<decltype(desired)>, bool> {
    if (compare_and_swap_impl(expected.get_counted(), desired.get_counted())) {
      desired.release();
      return true;
    } else {
      return false;
    }
  }

  weak_ptr_t exchange(weak_ptr_t desired) noexcept {
    auto new_ptr = desired.release();
    auto old_ptr = atomic_ptr.exchange(new_ptr, std::memory_order_seq_cst);
    return weak_ptr_t(old_ptr, weak_ptr_t::AddRef::no);
  }

  atomic_weak_ptr& operator=(weak_ptr_t desired) noexcept {
    store(std::move(desired));
    return *this;
  }

  /* implicit */ operator weak_ptr_t() const noexcept { return load(); }

  static size_t currently_allocated() {
    return mm.currently_allocated();
  }

 protected:

  bool compare_and_swap_impl(counted_ptr_t expected_ptr, counted_ptr_t desired_ptr) noexcept {
    if (atomic_ptr.compare_exchange_strong(expected_ptr, desired_ptr, std::memory_order_seq_cst)) {
      if (expected_ptr != nullptr) {
        mm.delayed_decrement_weak_cnt(expected_ptr);
      }
      return true;
    } else {
      return false;
    }
  }

  static inline memory_manager& mm = memory_manager::instance();

  std::atomic<counted_ptr_t> atomic_ptr;
};

}  // namespace cdrc

#endif  // CDRC_ATOMIC_WEAK_PTR_H
