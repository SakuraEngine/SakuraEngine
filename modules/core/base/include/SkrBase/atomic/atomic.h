#pragma once
#if defined(__cplusplus)
	#include "detail/cpp23.inc"
#else
	#include "detail/c11.inc"
#endif

typedef _Atomic(uint32_t) SAtomicU32;
typedef _Atomic(uint64_t) SAtomicU64;
typedef _Atomic(int32_t) SAtomic32;
typedef _Atomic(int64_t) SAtomic64;

#define atomic_fetch_add_relaxed(px, v) atomic_fetch_add_explicit(px, v, memory_order_relaxed)
#define atomic_load_relaxed(px) atomic_load_explicit(px, memory_order_relaxed)
#define atomic_store_relaxed(px, v) atomic_store_explicit(px, v, memory_order_relaxed)

#define atomic_fetch_add_acquire(px, v) atomic_fetch_add_explicit(px, v, memory_order_acquire)
#define atomic_load_acquire(px) atomic_load_explicit(px, memory_order_acquire)
#define atomic_store_acquire(px, v) atomic_store_explicit(px, v, memory_order_acquire)

#define atomic_fetch_add_release(px, v) atomic_fetch_add_explicit(px, v, memory_order_release)
#define atomic_load_release(px) atomic_load_explicit(px, memory_order_release)
#define atomic_store_release(px, v) atomic_store_explicit(px, v, memory_order_release)