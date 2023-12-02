
#ifndef CDRC_ATOMIC_RC_PTR_H
#define CDRC_ATOMIC_RC_PTR_H

#include <cstddef>

#include <atomic>

#include "internal/counted_object.h"
#include "internal/fwd_decl.h"
#include "internal/utils.h"

#include "rc_ptr.h"
#include "snapshot_ptr.h"

namespace cdrc {

template<typename T, typename memory_manager, typename pointer_policy>
class atomic_rc_ptr : public pointer_policy::template arc_ptr_policy<T> {

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

  using rc_ptr_t = rc_ptr<T, memory_manager, pointer_policy>;
  using snapshot_ptr_t = snapshot_ptr<T, memory_manager, pointer_policy>;
  using weak_ptr_t = weak_ptr<T, memory_manager, pointer_policy>;
  using atomic_weak_ptr_t = atomic_weak_ptr<T, memory_manager, pointer_policy>;
  using weak_snapshot_ptr_t = weak_snapshot_ptr<T, memory_manager, pointer_policy>;

  friend rc_ptr_t;
  friend snapshot_ptr_t;
  friend weak_ptr_t;
  friend atomic_weak_ptr_t;
  friend weak_snapshot_ptr_t;

  friend typename pointer_policy::template arc_ptr_policy<T>;

 public:
  atomic_rc_ptr() : atomic_ptr(nullptr) {}

  /* implicit */ atomic_rc_ptr(std::nullptr_t) : atomic_ptr(nullptr) {}

  /* implicit */ atomic_rc_ptr(rc_ptr_t desired) : atomic_ptr(desired.release()) {}

  ~atomic_rc_ptr() {
    auto ptr = atomic_ptr.load();
    if (ptr != nullptr) mm.delayed_decrement_ref_cnt(ptr);
  }

  atomic_rc_ptr(const atomic_rc_ptr &) = delete;

  atomic_rc_ptr &operator=(const atomic_rc_ptr &) = delete;

  atomic_rc_ptr(atomic_rc_ptr &&) = delete;

  atomic_rc_ptr &operator=(atomic_rc_ptr &&) = delete;

  [[nodiscard]] bool is_lock_free() const noexcept { return true; }

  static constexpr bool is_always_lock_free = true;

  void store(std::nullptr_t) noexcept {
    auto old_ptr = atomic_ptr.exchange(nullptr, std::memory_order_seq_cst);
    if (old_ptr != nullptr) mm.delayed_decrement_ref_cnt(old_ptr);
  }

  void store(rc_ptr_t desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
    auto new_ptr = desired.release();
    auto old_ptr = atomic_ptr.exchange(new_ptr, order);
    if (old_ptr != nullptr) mm.delayed_decrement_ref_cnt(old_ptr);
  }

  void store_non_racy(rc_ptr_t desired) noexcept {
    auto new_ptr = desired.release();
    auto old_ptr = atomic_ptr.load();
    atomic_ptr.store(new_ptr, std::memory_order_release);
    if (old_ptr != nullptr) mm.delayed_decrement_ref_cnt(old_ptr);
  }

  void store(const snapshot_ptr_t &desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
    auto new_ptr = desired.get_counted();

    // If desired is protected, a small optimization opportunity is to not
    // increment/decrement the reference count of the new/old value if they
    // turn out to be the same. If desired isn't protected, we must proactively
    // increment, though, otherwise it could be decremented after we exchange
    // but before we perform the increment.
    if (desired.is_protected()) {
      auto old_ptr = atomic_ptr.exchange(new_ptr, order);
      if (old_ptr != new_ptr) {
        if (new_ptr != nullptr) mm.increment_ref_cnt(new_ptr);
        if (old_ptr != nullptr) mm.delayed_decrement_ref_cnt(old_ptr);
      }
    }
    else {
      if (new_ptr != nullptr) mm.increment_ref_cnt(new_ptr);
      auto old_ptr = atomic_ptr.exchange(new_ptr, order);
      if (old_ptr != nullptr) mm.delayed_decrement_ref_cnt(old_ptr);
    }
  }

  rc_ptr_t load() const noexcept {
    auto acquired_ptr = mm.acquire(&atomic_ptr);
    rc_ptr_t result(acquired_ptr.get(), rc_ptr_t::AddRef::yes);
    return result;
  }

  snapshot_ptr_t get_snapshot() const noexcept {
    return snapshot_ptr_t(mm.protect_snapshot(&atomic_ptr));
  }

  bool compare_exchange_weak(rc_ptr_t &expected, const rc_ptr_t &desired) noexcept {
    if (!compare_and_swap(expected, desired)) {
      expected = load();
      return false;
    } else
      return true;
  }

  bool compare_exchange_weak(snapshot_ptr_t &expected, const rc_ptr_t &desired) noexcept {
    if (!compare_and_swap(expected, desired)) {
      expected = get_snapshot();
      return false;
    } else
      return true;
  }

  // Atomically compares the underlying rc_ptr with expected, and if they refer to
  // the same managed object, replaces the current rc_ptr with a copy of desired
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
      if (desired_ptr != nullptr) mm.increment_ref_cnt(desired_ptr);
      return true;
    } else {
      return false;
    }
  }

  // Atomically compares the underlying rc_ptr with expected, and if they are equal,
  // replaces the current rc_ptr with desired by move assignment, hence leaving its
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

  // Swaps the currently stored shared pointer with the given
  // shared pointer. This operation does not affect the reference
  // counts of either shared pointer.
  //
  // Note that it is not safe to concurrently access desired
  // while this operation is taking place, since desired is a
  // non-atomic shared pointer!
  void swap(rc_ptr_t &desired) noexcept {
    auto desired_ptr = desired.release();
    auto current_ptr = atomic_ptr.load();
    desired = rc_ptr_t(current_ptr, rc_ptr_t::AddRef::no);
    while (!atomic_ptr.compare_exchange_weak(desired.ptr, desired_ptr)) { }
  }

  rc_ptr_t exchange(rc_ptr_t desired) noexcept {
    auto new_ptr = desired.release();
    auto old_ptr = atomic_ptr.exchange(new_ptr, std::memory_order_seq_cst);
    return rc_ptr_t(old_ptr, rc_ptr_t::AddRef::no);
  }

  atomic_rc_ptr& operator=(rc_ptr_t desired) noexcept {
    store(std::move(desired));
    return *this;
  }

  /* implicit */ operator rc_ptr_t() const noexcept { return load(); }

  bool friend operator==(const atomic_rc_ptr& p, std::nullptr_t) noexcept {
    return p.atomic_ptr.load() == nullptr;
  }

  static size_t currently_allocated() {
    return mm.currently_allocated();
  }

 protected:

  bool compare_and_swap_impl(counted_ptr_t expected_ptr, counted_ptr_t desired_ptr) noexcept {
    if (atomic_ptr.compare_exchange_strong(expected_ptr, desired_ptr, std::memory_order_seq_cst)) {
      if (expected_ptr != nullptr) {
        mm.delayed_decrement_ref_cnt(expected_ptr);
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

#endif  // CDRC_ATOMIC_RC_PTR_H
