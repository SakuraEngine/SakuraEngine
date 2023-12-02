
#ifndef CDRC_INTERNAL_COUNTED_OBJECT_H
#define CDRC_INTERNAL_COUNTED_OBJECT_H


#include <cassert>
#include <cstdint>

#include <atomic>
#include <new>
#include <type_traits>
#include <utility>

#include "utils.h"

namespace cdrc {
namespace internal {

// An instance of an object of type T with an atomic reference count.
template<typename T>
struct counted_object {
  alignas(alignof(T)) unsigned char storage[sizeof(T)];
  utils::StickyCounter<uint32_t> ref_cnt;
  utils::StickyCounter<uint32_t> weak_cnt;

// In debug mode only, keep track of whether the object has been
// destroyed yet, to ensure that it is correctly destroyed
#ifndef NDEBUG
  std::atomic<bool> disposed;
#endif

  template<typename... Args>
  explicit counted_object(Args &&... args) : ref_cnt(1), weak_cnt(1) {
    new (&storage) T(std::forward<Args>(args)...);
  }

  counted_object(const counted_object &) = delete;
  counted_object(counted_object &&) = delete;

#ifndef NDEBUG
  ~counted_object() {
    assert(disposed.load() == true);
  }
#else
  ~counted_object() = default;
#endif

  T *get() { return std::launder(reinterpret_cast<T*>(&storage)); }
  const T *get() const { return std::launder(reinterpret_cast<const T*>(&storage)); }

  // Destroy the managed object, but keep the control data intact
  void dispose() {
    get()->~T();
#ifndef NDEBUG
    disposed.store(true);
#endif
  }

  auto get_use_count() const { return ref_cnt.load(); }
  auto get_weak_count() const { return weak_cnt.load(); }

  bool add_refs(uint64_t count) { return ref_cnt.increment(count, std::memory_order_relaxed); }

  enum class EjectAction {
    nothing,
    delay,
    destroy
  };

  // Release strong references to the object. If the strong reference count reaches zero,
  // the managed object will be destroyed, and the weak reference count will be decremented
  // by one. If this causes the weak reference count to hit zero, returns true, indicating
  // that the caller should delete this object.
  EjectAction release_refs(uint64_t count) {

    // A decrement-release + an acquire fence is recommended by Boost's documentation:
    // https://www.boost.org/doc/libs/1_57_0/doc/html/atomic/usage_examples.html
    // Alternatively, an acquire-release decrement would work, but might be less efficient since the
    // acquire is only relevant if the decrement zeros the counter.
    if (ref_cnt.decrement(count, std::memory_order_release)) {
      std::atomic_thread_fence(std::memory_order_acquire);
      // If there are no live weak pointers, we can immediately destroy
      // everything. Otherwise, we have to defer the disposal of the
      // managed object since an atomic_weak_ptr might be about to
      // take a snapshot...
      if (weak_cnt.load(std::memory_order_relaxed) == 1) {
        // Immediately destroy the managed object and
        // collect the control data, since no more
        // live (strong or weak) references exist
        dispose();
        return EjectAction::destroy;
      }
      else {
        // At least one weak reference exists, so we have to
        // delay the destruction of the managed object
        return EjectAction::delay;
      }
    }
    return EjectAction::nothing;
  }

  bool add_weak_refs(uint64_t count) { return weak_cnt.increment(count, std::memory_order_relaxed); }

  // Release weak references to the object. If this causes the weak reference count
  // to hit zero, returns true, indicating that the caller should delete this object.
  bool release_weak_refs(uint64_t count) {
    return weak_cnt.decrement(count, std::memory_order_release);
  }
};

}
}

#endif  // CDRC_INTERNAL_COUNTED_OBJECT_H
