
#ifndef CDRC_INTERNAL_MEMORY_MANAGER_BASE_H
#define CDRC_INTERNAL_MEMORY_MANAGER_BASE_H

#include <cassert>
#include <cstddef>

#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include "counted_object.h"
#include "utils.h"

namespace cdrc {
namespace internal {

constexpr static size_t num_retire_types = 3;

enum class RetireType {
  decrement_strong_count,
  decrement_weak_count,
  dispose
};

template<typename U>
struct basic_acquired_pointer {
 public:

  basic_acquired_pointer() : value(nullptr) {}

  /* implicit */ basic_acquired_pointer(U value_) : value(value_) {}

  basic_acquired_pointer(basic_acquired_pointer&& other) noexcept : value(other.value) {
    other.value = nullptr;
  }

  basic_acquired_pointer &operator=(basic_acquired_pointer&& other) noexcept {
    value = other.value;
    other.value = nullptr;
  }

  void swap(basic_acquired_pointer &other) {
    std::swap(value, other.value);
  }

  U& get() { return value; }

  U get() const { return value; }

  [[nodiscard]] bool is_protected() const { return true; }

  void clear_protection() {}

  void clear() { value = nullptr; }

 private:
  U value;
};

template<typename T, typename Derived>
struct memory_manager_base {

  using counted_object_t = counted_object<T>;
  using counted_ptr_t = std::add_pointer_t<counted_object_t>;

  explicit memory_manager_base(size_t num_threads) : num_allocated(num_threads) {}

  void dispose(counted_ptr_t ptr) {
    assert(ptr->get_use_count() == 0);
    ptr->dispose();
    if (ptr->release_weak_refs(1)) destroy(ptr);
  }

  void destroy(counted_ptr_t ptr) {
    assert(ptr->get_use_count() == 0);
    assert(ptr->disposed.load() == true);
    static_cast<Derived *>(this)->delete_object(ptr);
  }

  void retire(counted_ptr_t ptr, RetireType type) {
    static_cast<Derived *>(this)->retire(ptr, type);
  }

  // Perform an eject action. This can correspond to any action that
  // should be delayed until the ptr is no longer protected
  void eject(counted_ptr_t ptr, RetireType type) {
    assert(ptr != nullptr);

    if (type == RetireType::decrement_strong_count) {
      decrement_ref_cnt(ptr);
    }
    else if (type == RetireType::decrement_weak_count) {
      decrement_weak_cnt(ptr);
    }
    else {
      assert(type == RetireType::dispose);
      dispose(ptr);
    }
  }

  bool increment_ref_cnt(counted_ptr_t ptr) {
    assert(ptr != nullptr);
    return ptr->add_refs(1);
  }

  bool increment_weak_cnt(counted_ptr_t ptr) {
    assert(ptr != nullptr);
    return ptr->add_weak_refs(1);
  }

  void decrement_ref_cnt(counted_ptr_t ptr) {
    assert(ptr != nullptr);
    assert(ptr->get_use_count() >= 1);
    auto result = ptr->release_refs(1);
    if (result == counted_object_t::EjectAction::destroy) {
      destroy(ptr);
    } else if (result == counted_object_t::EjectAction::delay) {
      retire(ptr, RetireType::dispose);
    }
  }

  void decrement_weak_cnt(counted_ptr_t ptr) {
    assert(ptr != nullptr);
    assert(ptr->get_weak_count() >= 1);
    if (ptr->release_weak_refs(1)) {
      destroy(ptr);
    }
  }

  void delayed_decrement_ref_cnt(counted_ptr_t ptr) {
    assert(ptr->get_use_count() >= 1);
    retire(ptr, RetireType::decrement_strong_count);
  }

  void delayed_decrement_weak_cnt(counted_ptr_t ptr) {
    assert(ptr->get_weak_count() >= 1);
    retire(ptr, RetireType::decrement_weak_count);
  }

  void decrement_allocations() {
    int tid = utils::ThreadID::GetThreadID().getTID();
    num_allocated[tid].store(num_allocated[tid].load(std::memory_order_seq_cst) - 1, std::memory_order_seq_cst);
  }

  void increment_allocations() {
    int tid = utils::ThreadID::GetThreadID().getTID();
    num_allocated[tid].store(num_allocated[tid].load(std::memory_order_seq_cst) + 1, std::memory_order_seq_cst);
  }

  size_t currently_allocated() {
    size_t total = 0;
    for (size_t t = 0; t < utils::num_threads(); t++) {
      total += num_allocated[t].load(std::memory_order_acquire);
    }
    return total;
  }

  std::vector<utils::Padded<std::atomic<std::ptrdiff_t>>> num_allocated;
};

}  // namespace internal
}  // namespace cdrc

// Specialize hash for the RetireType enum so it can be used in a hashtable
template<>
struct std::hash<typename cdrc::internal::RetireType> {
  std::size_t operator()(cdrc::internal::RetireType t) const { return static_cast<std::size_t>(t); }
};


#endif  // CDRC_INTERNAL_MEMORY_MANAGER_BASE_H
