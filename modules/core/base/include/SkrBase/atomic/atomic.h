#pragma once
#if defined(__cplusplus)
	#include "detail/cpp23.inc"
#else
	#include "detail/c11.inc"
#endif

typedef _SAtomic(uint32_t) SAtomicU32;
typedef _SAtomic(uint64_t) SAtomicU64;
typedef _SAtomic(int32_t) SAtomic32;
typedef _SAtomic(int64_t) SAtomic64;

#define skr_atomic_fetch_add_relaxed(px, v) skr_atomic_fetch_add_explicit(px, v, skr_memory_order_relaxed)
#define skr_atomic_load_relaxed(px) skr_atomic_load_explicit(px, skr_memory_order_relaxed)
#define skr_atomic_store_relaxed(px, v) skr_atomic_store_explicit(px, v, skr_memory_order_relaxed)

#define skr_atomic_fetch_add_acquire(px, v) skr_atomic_fetch_add_explicit(px, v, skr_memory_order_acquire)
#define skr_atomic_load_acquire(px) skr_atomic_load_explicit(px, skr_memory_order_acquire)
#define skr_atomic_store_acquire(px, v) skr_atomic_store_explicit(px, v, skr_memory_order_acquire)

#define skr_atomic_fetch_add_release(px, v) skr_atomic_fetch_add_explicit(px, v, skr_memory_order_release)
#define skr_atomic_load_release(px) skr_atomic_load_explicit(px, skr_memory_order_release)
#define skr_atomic_store_release(px, v) skr_atomic_store_explicit(px, v, skr_memory_order_release)