
#ifndef CDRC_INTERNAL_FWD_DECL_H
#define CDRC_INTERNAL_FWD_DECL_H

#include <type_traits>

#include "smr/acquire_retire.h"
#include "smr/acquire_retire_ebr.h"
#include "smr/acquire_retire_ibr.h"
#include "smr/acquire_retire_hyaline.h"

namespace cdrc {

namespace internal {

// Blank policy that adds nothing
class default_pointer_policy {
 public:
  template<typename T>
  using pointer_type = std::add_pointer_t<T>;

  template<typename T>
  class arc_ptr_policy { };

  template<typename T>
  class rc_ptr_policy { };

  template<typename T>
  class snapshot_ptr_policy { };
};

template<typename T>
using default_memory_manager = internal::acquire_retire<T>;

}  // namespace internal

// Definition of each pointer type with default memory manager backend

template<typename T, typename memory_manager = internal::default_memory_manager<T>, typename pointer_policy = internal::default_pointer_policy>
class atomic_rc_ptr;

template<typename T, typename memory_manager = internal::default_memory_manager<T>, typename pointer_policy = internal::default_pointer_policy>
class rc_ptr;

template<typename T, typename memory_manager = internal::default_memory_manager<T>, typename pointer_policy = internal::default_pointer_policy>
class snapshot_ptr;

template<typename T, typename memory_manager = internal::default_memory_manager<T>, typename pointer_policy = internal::default_pointer_policy>
class atomic_weak_ptr;

template<typename T, typename memory_manager = internal::default_memory_manager<T>, typename pointer_policy = internal::default_pointer_policy>
class weak_ptr;

template<typename T, typename memory_manager = internal::default_memory_manager<T>, typename pointer_policy = internal::default_pointer_policy>
class weak_snapshot_ptr;

// Explicit hazard-pointer version of each type

template<typename T>
using atomic_rc_ptr_hp = atomic_rc_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using rc_ptr_hp = rc_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using snapshot_ptr_hp = snapshot_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using atomic_weak_ptr_hp = atomic_weak_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using weak_ptr_hp = weak_ptr<T, internal::acquire_retire<T>>;

template<typename T>
using weak_snapshot_ptr_hp = weak_snapshot_ptr<T, internal::acquire_retire<T>>;

// Explicit EBR version of each type

template<typename T>
using atomic_rc_ptr_ebr = atomic_rc_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using rc_ptr_ebr = rc_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using snapshot_ptr_ebr = snapshot_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using atomic_weak_ptr_ebr = atomic_weak_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using weak_ptr_ebr = weak_ptr<T, internal::acquire_retire_ebr<T>>;

template<typename T>
using weak_snapshot_ptr_ebr = weak_snapshot_ptr<T, internal::acquire_retire_ebr<T>>;


// Explicit IBR version of each type

template<typename T>
using atomic_rc_ptr_ibr = atomic_rc_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using rc_ptr_ibr = rc_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using snapshot_ptr_ibr = snapshot_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using atomic_weak_ptr_ibr = atomic_weak_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using weak_ptr_ibr = weak_ptr<T, internal::acquire_retire_ibr<T>>;

template<typename T>
using weak_snapshot_ptr_ibr = weak_snapshot_ptr<T, internal::acquire_retire_ibr<T>>;


// Explicit Hyaline version of each type

template<typename T>
using atomic_rc_ptr_hyaline = atomic_rc_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using rc_ptr_hyaline = rc_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using snapshot_ptr_hyaline = snapshot_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using atomic_weak_ptr_hyaline = atomic_weak_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using weak_ptr_hyaline = weak_ptr<T, internal::acquire_retire_hyaline<T>>;

template<typename T>
using weak_snapshot_ptr_hyaline = weak_snapshot_ptr<T, internal::acquire_retire_hyaline<T>>;


// Memory management backend aliases

template<typename T>
using hp_backend = internal::acquire_retire<T>;

template<typename T>
using ebr_backend = internal::acquire_retire_ebr<T>;

template<typename T>
using ibr_backend = internal::acquire_retire_ibr<T>;

template<typename T>
using hyaline_backend = internal::acquire_retire_hyaline<T>;

}  // namespace cdrc

#endif //CDRC_INTERNAL_FWD_DECL_H
