
#ifndef CDRC_WEAK_SNAPSHOT_PTR_H_
#define CDRC_WEAK_SNAPSHOT_PTR_H_

#include <cstddef>

#include <type_traits>

#include "internal/counted_object.h"
#include "internal/fwd_decl.h"

#include "rc_ptr.h"

namespace cdrc {

template<typename T, typename memory_manager, typename pointer_policy>
class weak_snapshot_ptr : public pointer_policy::template snapshot_ptr_policy<T> {

  using counted_object_t = internal::counted_object<T>;
  using counted_ptr_t = typename pointer_policy::template pointer_type<counted_object_t>;

  using atomic_ptr_t = atomic_rc_ptr<T, memory_manager, pointer_policy>;
  using rc_ptr_t = rc_ptr<T, memory_manager, pointer_policy>;
  using atomic_weak_ptr_t = atomic_weak_ptr<T, memory_manager, pointer_policy>;
  using weak_ptr_t = weak_ptr<T, memory_manager, pointer_policy>;

  friend atomic_ptr_t;
  friend rc_ptr_t;
  friend atomic_weak_ptr_t;
  friend weak_ptr_t;

  using acquired_pointer_t = typename memory_manager::template acquired_pointer<counted_ptr_t>;

  friend typename pointer_policy::template arc_ptr_policy<T>;
  friend typename pointer_policy::template snapshot_ptr_policy<T>;

 public:
  weak_snapshot_ptr() : acquired_ptr() {}

  /* implicit */ weak_snapshot_ptr(std::nullptr_t) : acquired_ptr() {}

  weak_snapshot_ptr(weak_snapshot_ptr &&other) noexcept: acquired_ptr(std::move(other.acquired_ptr)) {}

  weak_snapshot_ptr(const weak_snapshot_ptr&) = delete;
  weak_snapshot_ptr &operator=(const weak_snapshot_ptr&) = delete;

  weak_snapshot_ptr &operator=(weak_snapshot_ptr &&other) {
    clear();
    swap(other);
    return *this;
  }

  rc_ptr_t lock() const noexcept {
    auto ptr = acquired_ptr.get();
    if (ptr && mm.increment_ref_cnt(ptr)) return rc_ptr_t(ptr, rc_ptr_t::AddRef::no);
    else return nullptr;
  }

  typename std::add_lvalue_reference_t<T> operator*() { return *(acquired_ptr.get()->get()); }

  const typename std::add_lvalue_reference_t<T> operator*() const { return *(acquired_ptr.get()->get()); }

  T *get() { 
    counted_ptr_t ptr = acquired_ptr.get();
    return (ptr == nullptr) ? nullptr : ptr->get(); 
  }

  const T *get() const { 
    counted_ptr_t ptr = acquired_ptr.get();
    return (ptr == nullptr) ? nullptr : ptr->get(); 
  }

  T *operator->() { 
    counted_ptr_t ptr = acquired_ptr.get();
    return (ptr == nullptr) ? nullptr : ptr->get(); 
  }

  const T *operator->() const { 
    counted_ptr_t ptr = acquired_ptr.getValue();
    return (ptr == nullptr) ? nullptr : ptr->get(); 
  }

  explicit operator bool() const { return acquired_ptr.get() != nullptr; }

  bool operator==(const weak_snapshot_ptr<T> &other) const { return get() == other.get(); }

  bool operator!=(const weak_snapshot_ptr<T> &other) const { return get() != other.get(); }

  void swap(weak_snapshot_ptr &other) {
    acquired_ptr.swap(other.acquired_ptr);
  }

  ~weak_snapshot_ptr() { clear(); }

  void clear() {
    counted_ptr_t ptr = acquired_ptr.get();
    if (!acquired_ptr.is_protected() && ptr != nullptr) {
      mm.decrement_ref_cnt(ptr);
    }
    acquired_ptr.clear();
  }

 protected:

  explicit weak_snapshot_ptr(acquired_pointer_t&& acquired_ptr) :
      acquired_ptr(std::move(acquired_ptr)) {}

  counted_ptr_t get_counted() const {
    return acquired_ptr.get();
  }

  counted_ptr_t& get_counted() {
    return acquired_ptr.get();
  }

  // For converting a weak_snapshot_ptr into an rc_ptr
  // If the ref count has already been incremented,
  // the pointer can just be transferred, otherwise
  // it should be incremented here.
  counted_ptr_t release() {
    auto old_ptr = acquired_ptr.getValue();
    if (acquired_ptr.is_protected()) {
      if (mm.increment_ref_cnt(old_ptr)) {
        acquired_ptr.clear();
        return old_ptr;
      }
      else {
        acquired_ptr.clear();
        return nullptr;
      }
    }
    acquired_ptr.clear();
    return old_ptr;
  }

  [[nodiscard]] bool is_protected() const {
    return acquired_ptr.is_protected();
  }

  static inline memory_manager& mm = memory_manager::instance();

  acquired_pointer_t acquired_ptr;
};

}  // namespace cdrc

#endif  // CDRC_WEAK_SNAPSHOT_PTR_H_
