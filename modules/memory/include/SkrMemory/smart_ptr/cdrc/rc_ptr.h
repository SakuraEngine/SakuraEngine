
#ifndef CDRC_RC_PTR_H_
#define CDRC_RC_PTR_H_

#include <cstddef>

#include <type_traits>
#include <utility>

#include "internal/counted_object.h"
#include "internal/fwd_decl.h"

#include "weak_ptr.h"

namespace cdrc {

template<typename T, typename memory_manager, typename pointer_policy>
class rc_ptr : public pointer_policy::template rc_ptr_policy<T> {

  using counted_object_t = internal::counted_object<T>;
  using counted_ptr_t = typename pointer_policy::template pointer_type<counted_object_t>;

  using atomic_ptr_t = atomic_rc_ptr<T, memory_manager, pointer_policy>;
  using weak_ptr_t = weak_ptr<T, memory_manager, pointer_policy>;
  using snapshot_ptr_t = snapshot_ptr<T, memory_manager, pointer_policy>;
  using weak_snapshot_ptr_t = weak_snapshot_ptr<T, memory_manager, pointer_policy>;
  using atomic_weak_ptr_t = atomic_weak_ptr<T, memory_manager, pointer_policy>;

  friend atomic_ptr_t;
  friend weak_ptr_t;
  friend snapshot_ptr_t;
  friend weak_snapshot_ptr_t;
  friend atomic_weak_ptr_t;

  friend typename pointer_policy::template arc_ptr_policy<T>;
  friend typename pointer_policy::template rc_ptr_policy<T>;

 public:
  rc_ptr() noexcept: ptr(nullptr) {}

  /* implicit */ rc_ptr(std::nullptr_t) noexcept: ptr(nullptr) {}

  /* implicit */ rc_ptr(const snapshot_ptr_t &other) noexcept: ptr(other.get_counted()) {
    if (ptr) mm.increment_ref_cnt(ptr);
  }

  /* implicit */ rc_ptr(const weak_ptr_t& other) noexcept : rc_ptr(other.lock()) { }

  rc_ptr(const rc_ptr &other) noexcept: ptr(other.ptr) { if (ptr) mm.increment_ref_cnt(ptr); }

  rc_ptr(rc_ptr &&other) noexcept: ptr(other.release()) {}

  ~rc_ptr() { clear(); }

  void clear() {
    if (ptr) { mm.decrement_ref_cnt(ptr); }
    ptr = nullptr;
  }

  // Assign the managed pointer to nullptr, releasing the current reference
  rc_ptr& operator=(std::nullptr_t) {
    clear();
    return *this;
  }

  // copy assignment
  rc_ptr &operator=(const rc_ptr &other) {
    auto tmp = ptr;
    ptr = other.ptr;
    if (ptr) mm.increment_ref_cnt(ptr);
    if (tmp) mm.decrement_ref_cnt(tmp);
    return *this;
  }

  // move assignment
  rc_ptr &operator=(rc_ptr &&other) {
    auto tmp = ptr;
    ptr = other.release();
    if (tmp) mm.decrement_ref_cnt(tmp);
    return *this;
  }

  typename std::add_lvalue_reference_t<T> operator*() { return *(ptr->get()); }

  const typename std::add_lvalue_reference_t<T> operator*() const { return *(ptr->get()); }

  T *get() { return (ptr == nullptr) ? nullptr : ptr->get(); }

  const T *get() const { return (ptr == nullptr) ? nullptr : ptr->get(); }

  T *operator->() { return (ptr == nullptr) ? nullptr : ptr->get(); }

  const T *operator->() const { return (ptr == nullptr) ? nullptr : ptr->get(); }

  explicit operator bool() const { return ptr != nullptr; }

  bool operator==(const rc_ptr &other) const { return get() == other.get(); }

  bool operator!=(const rc_ptr &other) const { return get() != other.get(); }

  size_t use_count() const noexcept { return (ptr == nullptr) ? 0 : ptr->get_use_count(); }

  size_t weak_count() const noexcept { return (ptr == nullptr) ? 0 : ptr->get_weak_count() - 1; }

  void swap(rc_ptr &other) {
    std::swap(ptr, other.ptr);
  }

  // Create a new rc_ptr containing an object of type T constructed from (args...).
  template<typename... Args>
  static rc_ptr make_shared(Args &&... args) {
    auto ptr = mm.create_object(std::forward<Args>(args)...);
    return rc_ptr(ptr, AddRef::no);
  }

 protected:

  enum class AddRef {
    yes, no
  };

  explicit rc_ptr(counted_ptr_t ptr_, AddRef add_ref) : ptr(ptr_) {
    if (ptr && add_ref == AddRef::yes) mm.increment_ref_cnt(ptr);
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

// Create a new rc_ptr containing an object of type T constructed from (args...).
template<typename T, typename memory_manager = internal::default_memory_manager<T>,
  typename pointer_policy = internal::default_pointer_policy, typename... Args>
static rc_ptr<T, memory_manager, pointer_policy> make_shared(Args &&... args) {
  return rc_ptr<T, memory_manager, pointer_policy>::make_shared(std::forward<Args>(args)...);
}

// Create a new rc_ptr containing an object of type T constructed from (args...).
template<typename T, typename memory_manager = internal::default_memory_manager<T>,
  typename pointer_policy = internal::default_pointer_policy, typename... Args>
static rc_ptr<T, memory_manager, pointer_policy> make_rc(Args &&... args) {
  return rc_ptr<T, memory_manager, pointer_policy>::make_shared(std::forward<Args>(args)...);
}

}  // namespace cdrc

#endif  // CDRC_RC_PTR_H_
