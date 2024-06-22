#pragma once
#ifdef _WIN32
#include "plat/msvc32.inc"
#include "plat/msvc64.inc"

// C11 Standard Macros

#define atomic_store_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_store_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile uint32_t*: _plat32_atomic_store_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile int64_t*: _plat64_atomic_store_explicit((volatile _plat_atom64*) px, v, mo), \
  volatile uint64_t*: _plat64_atomic_store_explicit((volatile _plat_atom64*)px, v, mo))

#define atomic_load_explicit(px, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_load_explicit((volatile _plat_atom32*)px, mo), \
  volatile uint32_t*: _plat32_atomic_load_explicit((volatile _plat_atom32*)px, mo), \
  volatile int64_t*: _plat64_atomic_load_explicit((volatile _plat_atom64*)px, mo), \
  volatile uint64_t*: _plat64_atomic_load_explicit((volatile _plat_atom64*)px, mo))

#define atomic_exchange_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_exchange_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile uint32_t*: _plat32_atomic_exchange_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile int64_t*: _plat64_atomic_exchange_explicit((volatile _plat_atom64*)px, v, mo), \
  volatile uint64_t*: _plat64_atomic_exchange_explicit((volatile _plat_atom64*)px, v, mo))

#define atomic_fetch_add_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_fetch_add_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile uint32_t*: _plat32_atomic_fetch_add_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile int64_t*: _plat64_atomic_fetch_add_explicit((volatile _plat_atom64*)px, v, mo), \
  volatile uint64_t*: _plat64_atomic_fetch_add_explicit((volatile _plat_atom64*)px, v, mo))

#define atomic_fetch_sub_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_fetch_sub_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile uint32_t*: _plat32_atomic_fetch_sub_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile int64_t*: _plat64_atomic_fetch_sub_explicit((volatile _plat_atom64*)px, v, mo), \
  volatile uint64_t*: _plat64_atomic_fetch_sub_explicit((volatile _plat_atom64*)px, v, mo))

#define atomic_fetch_or_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_fetch_or_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile uint32_t*: _plat32_atomic_fetch_or_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile int64_t*: _plat64_atomic_fetch_or_explicit((volatile _plat_atom64*)px, v, mo), \
  volatile uint64_t*: _plat64_atomic_fetch_or_explicit((volatile _plat_atom64*)px, v, mo))

#define atomic_fetch_xor_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_fetch_xor_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile uint32_t*: _plat32_atomic_fetch_xor_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile int64_t*: _plat64_atomic_fetch_xor_explicit((volatile _plat_atom64*)px, v, mo), \
  volatile uint64_t*: _plat64_atomic_fetch_xor_explicit((volatile _plat_atom64*)px, v, mo))

#define atomic_fetch_and_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _plat32_atomic_fetch_and_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile uint32_t*: _plat32_atomic_fetch_and_explicit((volatile _plat_atom32*)px, v, mo), \
  volatile int64_t*: _plat64_atomic_fetch_and_explicit((volatile _plat_atom64*)px, v, mo), \
  volatile uint64_t*: _plat64_atomic_fetch_and_explicit((volatile _plat_atom64*)px, v, mo))

#define atomic_compare_exchange_weak_explicit(px, e, v, mo1, mo2) _Generic((px), \
  volatile int32_t*: _plat32_atomic_compare_exchange_weak_explicit((volatile _plat_atom32*)px, (_plat_atom32*)e, v, mo1, mo2), \
  volatile uint32_t*: _plat32_atomic_compare_exchange_weak_explicit((volatile _plat_atom32*)px, (_plat_atom32*)e, v, mo1, mo2), \
  volatile int64_t*: _plat64_atomic_compare_exchange_weak_explicit((volatile _plat_atom64*)px, (_plat_atom64*)e, v, mo1, mo2), \
  volatile uint64_t*: _plat64_atomic_compare_exchange_weak_explicit((volatile _plat_atom64*)px, (_plat_atom64*)e, v, mo1, mo2))

#define atomic_compare_exchange_strong_explicit(px, e, v, mo1, mo2) _Generic((px), \
  volatile int32_t*: _plat32_atomic_compare_exchange_strong_explicit((volatile _plat_atom32*)px, (_plat_atom32*)e, v, mo1, mo2), \
  volatile uint32_t*: _plat32_atomic_compare_exchange_strong_explicit((volatile _plat_atom32*)px, (_plat_atom32*)e, v, mo1, mo2), \
  volatile int64_t*: _plat64_atomic_compare_exchange_strong_explicit((volatile _plat_atom64*)px, (_plat_atom64*)e, v, mo1, mo2), \
  volatile uint64_t*: _plat64_atomic_compare_exchange_strong_explicit((volatile _plat_atom64*)px, (_plat_atom64*)e, v, mo1, mo2))

#define atomic_store(px, v) atomic_store_explicit(px, v, memory_order_seq_cst)
#define atomic_load(px, v) atomic_load_explicit(px, memory_order_seq_cst)
#define atomic_exchange(px, v) atomic_exchange_explicit(px, v, memory_order_seq_cst)

#define atomic_fetch_add(px, v) atomic_fetch_add_explicit(px, v, memory_order_seq_cst)
#define atomic_fetch_sub(px, v) atomic_fetch_sub_explicit(px, v, memory_order_seq_cst)
#define atomic_fetch_or(px, v) atomic_fetch_or_explicit(px, v, memory_order_seq_cst)
#define atomic_fetch_xor(px, v) atomic_fetch_xor_explicit(px, v, memory_order_seq_cst)
#define atomic_fetch_and(px, v) atomic_fetch_and_explicit(px, v, memory_order_seq_cst)

#define atomic_compare_exchange_weak(px, e, v) atomic_compare_exchange_weak_explicit(px, e, v, memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_strong(px, e, v) atomic_compare_exchange_strong_explicit(px, e, v, memory_order_seq_cst, memory_order_seq_cst)

#else

#include <stdatomic.h> // IWYU pragma: export

#endif