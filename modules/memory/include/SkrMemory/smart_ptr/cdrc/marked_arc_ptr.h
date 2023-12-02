
#ifndef CDRC_MARKED_ARC_PTR_H
#define CDRC_MARKED_ARC_PTR_H

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <type_traits>

#include "atomic_rc_ptr.h"
#include "atomic_weak_ptr.h"

#include "rc_ptr.h"
#include "weak_ptr.h"

#include "snapshot_ptr.h"
#include "weak_snapshot_ptr.h"

namespace cdrc {

template<typename T>
class marked_ptr {

  static constexpr uintptr_t ONE_BIT = 1;
  static constexpr uintptr_t TWO_BIT = 1 << 1;

 public:
  marked_ptr() : ptr(0) {}

  /* implicit */ marked_ptr(std::nullptr_t) : ptr(0) {}

  /* implicit */ marked_ptr(T *new_ptr) : ptr(reinterpret_cast<uintptr_t>(new_ptr)) {}

  /* implicit */ operator T* () const { return get_ptr(); }

  typename std::add_lvalue_reference_t<T> operator*() const { return *(get_ptr()); }

  T* operator->() { return get_ptr(); }

  const T *operator->() const { return get_ptr(); }

  bool operator==(const marked_ptr &other) const { return ptr == other.ptr; }

  bool operator!=(const marked_ptr &other) const { return ptr != other.ptr; }

  bool operator==(const T *other) const { return get_ptr() == other; }

  bool operator!=(const T *other) const { return get_ptr() != other; }

  T* get_ptr() const { return reinterpret_cast<T *>(ptr & ~(ONE_BIT | TWO_BIT)); }

  void set_ptr(T* new_ptr) { ptr = reinterpret_cast<uintptr_t>(new_ptr) | get_mark(); }

  [[nodiscard]] uintptr_t get_mark() const { return ptr & (ONE_BIT | TWO_BIT); }

  void clear_mark() { ptr = ptr & ~(ONE_BIT | TWO_BIT); }

  void set_mark(uintptr_t mark) {
    assert(mark < (1 << 2));  // Marks should only occupy the bottom two bits
    clear_mark();
    ptr |= mark;
  }

  void set_mark_bit(int bit) {
    assert(bit == 1 || bit == 2);
    ptr |= (1 << (bit - 1));
  }

  bool get_mark_bit(int bit) {
    assert(bit == 1 || bit == 2);
    return ptr & (1 << (bit - 1));
  }

 private:
  uintptr_t ptr;
};

namespace internal {

template<typename memory_manager>
class marked_ptr_policy;

}  // namespace internal

// Alias templates for marked pointers with the default memory manager

template<typename T, typename memory_manager = internal::default_memory_manager<T>>
using marked_arc_ptr = atomic_rc_ptr<T, memory_manager, internal::marked_ptr_policy<memory_manager>>;

template<typename T, typename memory_manager = internal::default_memory_manager<T>>
using marked_rc_ptr = rc_ptr<T, memory_manager, internal::marked_ptr_policy<memory_manager>>;

template<typename T, typename memory_manager = internal::default_memory_manager<T>>
using marked_snapshot_ptr = snapshot_ptr<T, memory_manager, internal::marked_ptr_policy<memory_manager>>;

template<typename T, typename memory_manager = internal::default_memory_manager<T>>
using marked_aw_ptr = atomic_weak_ptr<T, memory_manager, internal::marked_ptr_policy<memory_manager>>;

template<typename T, typename memory_manager = internal::default_memory_manager<T>>
using marked_weak_ptr = weak_ptr<T, memory_manager, internal::marked_ptr_policy<memory_manager>>;

template<typename T, typename memory_manager = internal::default_memory_manager<T>>
using marked_ws_ptr = weak_snapshot_ptr<T, memory_manager, internal::marked_ptr_policy<memory_manager>>;


// Alias templates for marked pointers with hazard pointers

template<typename T>
using marked_arc_ptr_hp = marked_arc_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using marked_rc_ptr_hp = marked_rc_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using marked_snapshot_ptr_hp = marked_snapshot_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using marked_aw_ptr_hp = marked_aw_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using marked_weak_ptr_hp = marked_weak_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using marked_ws_ptr_hp = marked_ws_ptr<T, internal::acquire_retire<T>>;


// Alias templates for marked pointers with EBR

template<typename T>
using marked_arc_ptr_ebr = marked_arc_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using marked_rc_ptr_ebr = marked_rc_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using marked_snapshot_ptr_ebr = marked_snapshot_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using marked_aw_ptr_ebr = marked_aw_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using marked_weak_ptr_ebr = marked_weak_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using marked_ws_ptr_ebr = marked_ws_ptr<T, internal::acquire_retire_ebr<T>>;


// Alias templates for marked pointers with IBR

template<typename T>
using marked_arc_ptr_ibr = marked_arc_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using marked_rc_ptr_ibr = marked_rc_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using marked_snapshot_ptr_ibr = marked_snapshot_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using marked_aw_ptr_ibr = marked_aw_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using marked_weak_ptr_ibr = marked_weak_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using marked_ws_ptr_ibr = marked_ws_ptr<T, internal::acquire_retire_ibr<T>>;


// Alias templates for marked pointers with Hyaline

template<typename T>
using marked_arc_ptr_hyaline = marked_arc_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using marked_rc_ptr_hyaline = marked_rc_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using marked_snapshot_ptr_hyaline = marked_snapshot_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using marked_aw_ptr_hyaline = marked_aw_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using marked_weak_ptr_hyaline = marked_weak_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using marked_ws_ptr_hyaline = marked_ws_ptr<T, internal::acquire_retire_hyaline<T>>;


namespace internal {

// Policy class for marked pointers.
//
// This policy injects the behaviour into atomic_rc_ptr, rc_ptr, and snapshot_ptr
// required to deal with marked pointers. Specifically, it adds the methods
//  - get_mark() const    : uintptr_t
//  - set_mark(uintptr_t) : void
// to all three classes, which allow one to get and set the marked bits of the
// pointer respectively. To atomic_rc_ptr, it also adds the method
//  - contains(const marked_rc_ptr<T>&) const         : bool
//  - contains(const marked_snapshot_ptr<T>&) const   : bool
//  - contains(const T*) const                        : bool
// which returns true if the atomic_rc_ptr currently contains an rc_ptr that
// manages the same object managed by the given pointers, or manages the object
// given itself, in the case of the third overload.
//
template<typename memory_manager>
class marked_ptr_policy {
 public:

  template<typename T>
  using pointer_type = marked_ptr<T>;

  // Adds the set_mark(uintptr_t) and get_mark() methods to atomic_rc_ptr
  template<typename T>
  class arc_ptr_policy {
   public:
    void set_mark(uintptr_t mark) {
      auto &parent = get_parent();
      auto cur_ptr = parent.atomic_ptr.load();
      auto new_ptr = cur_ptr;
      new_ptr.set_mark(mark);
      while (!parent.atomic_ptr.compare_exchange_weak(cur_ptr, new_ptr)) {
        new_ptr = cur_ptr;
        new_ptr.set_mark(mark);
      }
    }

    uintptr_t get_mark() const { return get_parent().atomic_ptr.load().get_mark(); }

    void set_mark_bit(int bit) {
      assert(bit == 1 || bit == 2);
      auto &parent = get_parent();
      auto cur_ptr = parent.atomic_ptr.load();
      auto new_ptr = cur_ptr;
      new_ptr.set_mark_bit(bit);
      while (!parent.atomic_ptr.compare_exchange_weak(cur_ptr, new_ptr)) {
        new_ptr = cur_ptr;
        new_ptr.set_mark_bit(bit);
      }
    }

    bool compare_and_set_mark(const auto& expected, int desired_mark) {
      auto &parent = get_parent();
      auto expected_ptr = expected.get_counted();
      auto desired_ptr = expected.get_counted();
      desired_ptr.set_mark(desired_mark);
      return parent.atomic_ptr.compare_exchange_strong(expected_ptr, desired_ptr);
    }

    bool get_mark_bit(int bit) {
      return get_parent().atomic_ptr.load().get_mark_bit(bit);
    }

    bool contains(const auto& other) const { return get_parent().atomic_ptr.load() == other.get_counted(); }

   private:
    T *get_raw_ptr() const { return get_parent().atomic_ptr.load().get_ptr()->get(); }

    using parent_type = atomic_rc_ptr<T, memory_manager, marked_ptr_policy<memory_manager>>;
    parent_type &get_parent() { return *static_cast<parent_type*>(this); }
    const parent_type &get_parent() const { return *static_cast<const parent_type*>(this); }
  };

  // Adds the set_mark(uintptr_t) and get_mark() methods to rc_ptr
  template<typename T>
  class rc_ptr_policy {
   public:
    void set_mark(uintptr_t mark) { get_parent().ptr.set_mark(mark); }

    uintptr_t get_mark() const { return get_parent().ptr.get_mark(); }

   private:
    using parent_type = rc_ptr<T, memory_manager, marked_ptr_policy<memory_manager>>;
    parent_type &get_parent() { return *static_cast<parent_type*>(this); }
    const parent_type &get_parent() const { return *static_cast<const parent_type*>(this); }
  };

  // Adds the set_mark(uintptr_t) and get_mark() methods to snapshot_ptr
  template<typename T>
  class snapshot_ptr_policy {
   public:
    void set_mark(uintptr_t mark) { get_parent().get_counted().set_mark(mark); }

    uintptr_t get_mark() const { return get_parent().get_counted().get_mark(); }

   private:
    using parent_type = snapshot_ptr<T, memory_manager, marked_ptr_policy<memory_manager>>;
    parent_type &get_parent() { return *static_cast<parent_type*>(this); }
    const parent_type &get_parent() const { return *static_cast<const parent_type*>(this); }
  };
};

}  // namespace internal

}  // namespace cdrc

#endif //CDRC_MARKED_ARC_PTR_H
