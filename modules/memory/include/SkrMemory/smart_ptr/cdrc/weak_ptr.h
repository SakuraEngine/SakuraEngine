
#ifndef CDRC_WEAK_PTR_H_
#define CDRC_WEAK_PTR_H_

#include <cstddef>

#include "internal/counted_object.h"
#include "internal/fwd_decl.h"

#include "rc_ptr.h"

namespace cdrc {

template<typename T, typename memory_manager, typename pointer_policy>
class weak_ptr : public pointer_policy::template rc_ptr_policy<T> {

  using counted_object_t = internal::counted_object<T>;
  using counted_ptr_t = typename pointer_policy::template pointer_type<counted_object_t>;

  using atomic_ptr_t = atomic_rc_ptr<T, memory_manager, pointer_policy>;
  using rc_ptr_t = rc_ptr<T, memory_manager, pointer_policy>;
  using snapshot_ptr_t = snapshot_ptr<T, memory_manager, pointer_policy>;
  using atomic_weak_ptr_t = atomic_weak_ptr<T, memory_manager, pointer_policy>;
  using weak_snapshot_ptr_t = weak_snapshot_ptr<T, memory_manager, pointer_policy>;

  friend atomic_ptr_t;
  friend snapshot_ptr_t;
  friend atomic_weak_ptr_t;

  friend typename pointer_policy::template arc_ptr_policy<T>;
  friend typename pointer_policy::template rc_ptr_policy<T>;

 public:
  weak_ptr() noexcept: ptr(nullptr) {}

  /* implicit */ weak_ptr(std::nullptr_t) noexcept: ptr(nullptr) {}

  /* implicit */ weak_ptr(const weak_snapshot_ptr_t &other) noexcept: ptr(other.get_counted()) {
    if (ptr && !mm.increment_weak_cnt(ptr)) ptr = nullptr;
  }

  /* implicit */ weak_ptr(const rc_ptr_t& other) noexcept : ptr(other.ptr) { if (ptr) mm.increment_weak_cnt(ptr); }

  weak_ptr(const weak_ptr &other) noexcept: ptr(other.ptr) { if (ptr) mm.increment_weak_cnt(ptr); }

  weak_ptr(weak_ptr&& other) noexcept: ptr(other.release()) {}

  ~weak_ptr() { clear(); }

  void clear() {
    if (ptr) mm.decrement_weak_cnt(ptr);
    ptr = nullptr;
  }

  // copy assignment
  weak_ptr &operator=(const weak_ptr &other) {
    auto tmp = ptr;
    ptr = other.ptr;
    if (ptr) mm.increment_weak_cnt(ptr);
    if (tmp) mm.decrement_weak_cnt(tmp);
    return *this;
  }

  // move assignment
  weak_ptr &operator=(weak_ptr &&other) {
    auto tmp = ptr;
    ptr = other.release();
    if (tmp) mm.decrement_weak_cnt(tmp);
    return *this;
  }

  rc_ptr_t lock() const noexcept {
    if (ptr && mm.increment_ref_cnt(ptr)) return rc_ptr_t(ptr, rc_ptr_t::AddRef::no);
    else return nullptr;
  }

  size_t use_count() const noexcept { return (ptr == nullptr) ? 0 : ptr->ref_cnt.load(); }

  bool expired() const { return use_count() == 0; }

  void swap(weak_ptr &other) {
    std::swap(ptr, other.ptr);
  }

 protected:

  enum class AddRef {
    yes, no
  };

  weak_ptr(counted_ptr_t ptr_, AddRef add_ref) : ptr(ptr_) {
    if (ptr && add_ref == AddRef::yes && !mm.increment_weak_cnt(ptr)) {
      ptr = nullptr;
    }
  }

  [[nodiscard]] bool is_protected() const {
    return false;
  }

  counted_ptr_t release() {
    auto p = ptr;
    ptr = nullptr;
    return p;
  }

  counted_ptr_t get_counted() const {
    return ptr;
  }

  static inline memory_manager& mm = memory_manager::instance();

  counted_ptr_t ptr;
};


}  // namespace cdrc

#endif  // CDRC_WEAK_PTR_H_
