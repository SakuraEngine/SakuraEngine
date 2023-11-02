/* clang-format off */
#pragma once
#include "SkrRT/platform/configure.h"

typedef volatile SKR_ALIGNAS(4) uint32_t SAtomicU32;
typedef volatile SKR_ALIGNAS(8) uint64_t SAtomicU64;
typedef volatile SKR_ALIGNAS(4) int32_t SAtomic32;
typedef volatile SKR_ALIGNAS(8) int64_t SAtomic64;
typedef volatile SKR_ALIGNAS(SKR_PTR_SIZE) uintptr_t SAtomicUPtr;
typedef volatile SKR_ALIGNAS(SKR_PTR_SIZE) intptr_t SAtomicPtr;

#if defined(__clang__) && defined(_MSC_VER)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(_MSC_VER) && !defined(NX64)

#include <intrin0.h>

SKR_FORCEINLINE static void skr_memorybarrier_acquire() { _ReadWriteBarrier(); }
SKR_FORCEINLINE static void skr_memorybarrier_release() { _ReadWriteBarrier(); }

SKR_FORCEINLINE static uint32_t skr_atomicu32_load_relaxed(const SAtomicU32* pVar) { return *(pVar); }
SKR_FORCEINLINE static uint32_t skr_atomicu32_store_relaxed(SAtomicU32* dst, uint32_t val) { return _InterlockedExchange( (volatile long*)(dst), val ); }
SKR_FORCEINLINE static uint32_t skr_atomicu32_add_relaxed(SAtomicU32* dst, uint32_t val) { return _InterlockedExchangeAdd( (volatile long*)(dst), val ); }
SKR_FORCEINLINE static uint32_t skr_atomicu32_cas_relaxed(SAtomicU32* dst, uint32_t cmp_val, uint32_t new_val) { return _InterlockedCompareExchange( (volatile long*)(dst), (new_val), (cmp_val) ); }

SKR_FORCEINLINE static uint64_t skr_atomicu64_load_relaxed(const SAtomicU64* pVar) { return *(pVar); }
SKR_FORCEINLINE static uint64_t skr_atomicu64_store_relaxed(SAtomicU64* dst, uint64_t val) { return _InterlockedExchange64( (volatile LONG64*)(dst), val ); }
SKR_FORCEINLINE static uint64_t skr_atomicu64_add_relaxed(SAtomicU64* dst, uint64_t val) { return _InterlockedExchangeAdd64( (volatile LONG64*)(dst), val ); }
SKR_FORCEINLINE static uint64_t skr_atomicu64_cas_relaxed(SAtomicU64* dst, uint64_t cmp_val, uint64_t new_val) { return _InterlockedCompareExchange64( (volatile LONG64*)(dst), (new_val), (cmp_val) ); }

SKR_FORCEINLINE static int32_t skr_atomic32_load_relaxed(const SAtomic32* pVar) { return *(pVar); }
SKR_FORCEINLINE static int32_t skr_atomic32_store_relaxed(SAtomic32* dst, int32_t val) { return _InterlockedExchange( (volatile long*)(dst), val ); }
SKR_FORCEINLINE static int32_t skr_atomic32_add_relaxed(SAtomic32* dst, int32_t val) { return _InterlockedExchangeAdd( (volatile long*)(dst), val ); }
SKR_FORCEINLINE static int32_t skr_atomic32_cas_relaxed(SAtomic32* dst, int32_t cmp_val, int32_t new_val) { return _InterlockedCompareExchange( (volatile long*)(dst), (new_val), (cmp_val) ); }

SKR_FORCEINLINE static int64_t skr_atomic64_load_relaxed(const SAtomic64* pVar) { return *(pVar); }
SKR_FORCEINLINE static int64_t skr_atomic64_store_relaxed(SAtomic64* dst, int64_t val) { return _InterlockedExchange64( (volatile LONG64*)(dst), val ); }
SKR_FORCEINLINE static int64_t skr_atomic64_add_relaxed(SAtomic64* dst, int64_t val) { return _InterlockedExchangeAdd64( (volatile LONG64*)(dst), val ); }
SKR_FORCEINLINE static int64_t skr_atomic64_cas_relaxed(SAtomic64* dst, int64_t cmp_val, int64_t new_val) { return _InterlockedCompareExchange64( (volatile LONG64*)(dst), (new_val), (cmp_val) ); }

#else

SKR_FORCEINLINE static void skr_memorybarrier_acquire() { __asm__ __volatile__("": : :"memory"); }
SKR_FORCEINLINE static void skr_memorybarrier_release() { __asm__ __volatile__("": : :"memory"); }

SKR_FORCEINLINE static uint32_t skr_atomicu32_load_relaxed(const SAtomicU32* pVar) { return *(pVar); }
SKR_FORCEINLINE static uint32_t skr_atomicu32_store_relaxed(SAtomicU32* dst, uint32_t val) { return __sync_lock_test_and_set( (volatile uint32_t*)(dst), val ); }
SKR_FORCEINLINE static uint32_t skr_atomicu32_add_relaxed(SAtomicU32* dst, uint32_t val) { return __sync_fetch_and_add( (volatile uint32_t*)(dst), val ); }
SKR_FORCEINLINE static uint32_t skr_atomicu32_cas_relaxed(SAtomicU32* dst, uint32_t cmp_val, uint32_t new_val) { return __sync_val_compare_and_swap( (volatile uint32_t*)(dst), (cmp_val), (new_val) ); }

SKR_FORCEINLINE static uint64_t skr_atomicu64_load_relaxed(const SAtomicU64* pVar) { return *(pVar); }
SKR_FORCEINLINE static uint64_t skr_atomicu64_store_relaxed(SAtomicU64* dst, uint64_t val) { return __sync_lock_test_and_set( (volatile uint64_t*)(dst), val ); }
SKR_FORCEINLINE static uint64_t skr_atomicu64_add_relaxed(SAtomicU64* dst, uint64_t val) { return __sync_fetch_and_add( (volatile uint64_t*)(dst), val ); }
SKR_FORCEINLINE static uint64_t skr_atomicu64_cas_relaxed(SAtomicU64* dst, uint64_t cmp_val, uint64_t new_val) { return __sync_val_compare_and_swap( (volatile uint64_t*)(dst), (cmp_val), (new_val) ); }

SKR_FORCEINLINE static int32_t skr_atomic32_load_relaxed(const SAtomic32* pVar) { return *(pVar); }
SKR_FORCEINLINE static int32_t skr_atomic32_store_relaxed(SAtomic32* dst, int32_t val) { return __sync_lock_test_and_set( (volatile int32_t*)(dst), val ); }
SKR_FORCEINLINE static int32_t skr_atomic32_add_relaxed(SAtomic32* dst, int32_t val) { return __sync_fetch_and_add( (volatile int32_t*)(dst), val ); }
SKR_FORCEINLINE static int32_t skr_atomic32_cas_relaxed(SAtomic32* dst, int32_t cmp_val, int32_t new_val) { return __sync_val_compare_and_swap( (volatile int32_t*)(dst), (cmp_val), (new_val) ); }

SKR_FORCEINLINE static int64_t skr_atomic64_load_relaxed(const SAtomic64* pVar) { return *(pVar); }
SKR_FORCEINLINE static int64_t skr_atomic64_store_relaxed(SAtomic64* dst, int64_t val) { return __sync_lock_test_and_set( (volatile int64_t*)(dst), val ); }
SKR_FORCEINLINE static int64_t skr_atomic64_add_relaxed(SAtomic64* dst, int64_t val) { return __sync_fetch_and_add( (volatile int64_t*)(dst), val ); }
SKR_FORCEINLINE static int64_t skr_atomic64_cas_relaxed(SAtomic64* dst, int64_t cmp_val, int64_t new_val) { return __sync_val_compare_and_swap( (volatile int64_t*)(dst), (cmp_val), (new_val) ); }

#endif

// Unsigned Atomic

SKR_FORCEINLINE static uint32_t skr_atomicu32_load_acquire(const SAtomicU32* pVar)
{
	uint32_t value = skr_atomicu32_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

SKR_FORCEINLINE static uint32_t skr_atomicu32_store_release(SAtomicU32* pVar, uint32_t val)
{
	skr_memorybarrier_release();
	return skr_atomicu32_store_relaxed(pVar, val);
}

SKR_FORCEINLINE static uint32_t skr_atomicu32_max_relaxed(SAtomicU32* dst, uint32_t val)
{
    uint32_t prev_val = val;
    do { prev_val = skr_atomicu32_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

SKR_FORCEINLINE static uint64_t skr_atomicu64_load_acquire(const SAtomicU64* pVar)
{
	uint64_t value = skr_atomicu64_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

SKR_FORCEINLINE static uint64_t skr_atomicu64_store_release(SAtomicU64* pVar, uint64_t val)
{
	skr_memorybarrier_release();
	return skr_atomicu64_store_relaxed(pVar, val);
}

SKR_FORCEINLINE static uint64_t skr_atomicu64_max_relaxed(SAtomicU64* dst, uint64_t val)
{
    uint64_t prev_val = val;
    do { prev_val = skr_atomicu64_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

// Atomic

SKR_FORCEINLINE static int32_t skr_atomic32_load_acquire(const SAtomic32* pVar)
{
	int32_t value = skr_atomic32_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

SKR_FORCEINLINE static int32_t skr_atomic32_store_release(SAtomic32* pVar, int32_t val)
{
	skr_memorybarrier_release();
	return skr_atomic32_store_relaxed(pVar, val);
}

SKR_FORCEINLINE static int32_t skr_atomic32_max_relaxed(SAtomic32* dst, int32_t val)
{
    int32_t prev_val = val;
    do { prev_val = skr_atomic32_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

SKR_FORCEINLINE static int64_t skr_atomic64_load_acquire(const SAtomic64* pVar)
{
	int64_t value = skr_atomic64_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

SKR_FORCEINLINE static int64_t skr_atomic64_store_release(SAtomic64* pVar, int64_t val)
{
	skr_memorybarrier_release();
	return skr_atomic64_store_relaxed(pVar, val);
}

SKR_FORCEINLINE static int64_t skr_atomic64_max_relaxed(SAtomic64* dst, int64_t val)
{
    int64_t prev_val = val;
    do { prev_val = skr_atomic64_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

#if PTR_SIZE == 4

SKR_FORCEINLINE static uintptr_t skr_atomicuptr_load_relaxed(SAtomicUPtr* pval) { return (uintptr_t)skr_atomicu32_load_relaxed((SAtomicU32*)pval); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_load_acquire(SAtomicUPtr* pval) { return (uintptr_t)skr_atomicu32_load_acquire((SAtomicU32*)pval); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_store_relaxed(SAtomicUPtr* pVar, uintptr_t val) { return (uintptr_t)skr_atomicu32_store_relaxed((SAtomicU32*)pVar, (uint32_t)val); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_store_release(SAtomicU32* pVar, uint32_t val) { return (uintptr_t)skr_atomicu32_store_release((SAtomicU32*)pVar, (uint32_t)val); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_add_relaxed(SAtomicUPtr* pVar, uintptr_t val) { return (uintptr_t)skr_atomicu32_add_relaxed((SAtomicU32*)pVar, (uint32_t)val); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_cas_relaxed(SAtomicUPtr* pVar, uintptr_t oldval, uintptr_t newval) { return (uintptr_t)skr_atomicu32_cas_relaxed((SAtomicU32*)pVar, (uint32_t)oldval, (uint32_t)newval); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_max_relaxed(SAtomicUPtr* pVar, uintptr_t val) { return (uintptr_t)skr_atomicu32_max_relaxed((SAtomicU32*)pVar, (uint32_t)val); }

SKR_FORCEINLINE static intptr_t skr_atomicptr_load_relaxed(SAtomicPtr* pval) { return (intptr_t)skr_atomic32_load_relaxed((SAtomic32*)pval); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_load_acquire(SAtomicPtr* pval) { return (intptr_t)skr_atomic32_load_acquire((SAtomic32*)pval); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_store_relaxed(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic32_store_relaxed((SAtomic32*)pVar, (int32_t)val); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_store_release(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic32_store_release((SAtomic32*)pVar, (int32_t)val); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_add_relaxed(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic32_add_relaxed((SAtomic32*)pVar, (int32_t)val); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_cas_relaxed(SAtomicPtr* pVar, intptr_t oldval, intptr_t newval) { return (intptr_t)skr_atomic32_cas_relaxed((SAtomic32*)pVar, (int32_t)oldval, (int32_t)newval); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_max_relaxed(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic32_max_relaxed((SAtomic32*)pVar, (int32_t)val); }

#elif PTR_SIZE == 8

SKR_FORCEINLINE static uintptr_t skr_atomicuptr_load_relaxed(SAtomicUPtr* pval) { return (uintptr_t)skr_atomicu64_load_relaxed((SAtomicU64*)pval); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_load_acquire(SAtomicUPtr* pval) { return (uintptr_t)skr_atomicu64_load_acquire((SAtomicU64*)pval); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_store_relaxed(SAtomicUPtr* pVar, uintptr_t val) { return (uintptr_t)skr_atomicu64_store_relaxed((SAtomicU64*)pVar, (uint64_t)val); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_store_release(SAtomicUPtr* pVar, uint32_t val) { return (uintptr_t)skr_atomicu64_store_release((SAtomicU64*)pVar, (uint64_t)val); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_add_relaxed(SAtomicUPtr* pVar, uintptr_t val) { return (uintptr_t)skr_atomicu64_add_relaxed((SAtomicU64*)pVar, (uint64_t)val); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_cas_relaxed(SAtomicUPtr* pVar, uintptr_t oldval, uintptr_t newval) { return (uintptr_t)skr_atomicu64_cas_relaxed((SAtomicU64*)pVar, (uint64_t)oldval, (uint64_t)newval); }
SKR_FORCEINLINE static uintptr_t skr_atomicuptr_max_relaxed(SAtomicUPtr* pVar, uintptr_t val) { return (uintptr_t)skr_atomicu64_max_relaxed((SAtomicU64*)pVar, (uint64_t)val); }

SKR_FORCEINLINE static intptr_t skr_atomicptr_load_relaxed(SAtomicPtr* pval) { return (intptr_t)skr_atomic64_load_relaxed((SAtomic64*)pval); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_load_acquire(SAtomicPtr* pval) { return (intptr_t)skr_atomic64_load_acquire((SAtomic64*)pval); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_store_relaxed(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic64_store_relaxed((SAtomic64*)pVar, (int64_t)val); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_store_release(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic64_store_release((SAtomic64*)pVar, (int64_t)val); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_add_relaxed(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic64_add_relaxed((SAtomic64*)pVar, (int64_t)val); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_cas_relaxed(SAtomicPtr* pVar, intptr_t oldval, intptr_t newval) { return (intptr_t)skr_atomic64_cas_relaxed((SAtomic64*)pVar, (int64_t)oldval, (int64_t)newval); }
SKR_FORCEINLINE static intptr_t skr_atomicptr_max_relaxed(SAtomicPtr* pVar, intptr_t val) { return (intptr_t)skr_atomic64_max_relaxed((SAtomic64*)pVar, (int64_t)val); }

#endif

// Yield
#if defined(__SSE2__)
#include <emmintrin.h>
static inline void skr_atomic_yield(void) {
	_mm_pause();
}
#elif (defined(__GNUC__) || defined(__clang__)) && \
	(defined(__x86_64__) || defined(__i386__) || defined(__arm__) || defined(__armel__) || defined(__ARMEL__) || \
	defined(__aarch64__) || defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)) || defined(__POWERPC__)
#if defined(__x86_64__) || defined(__i386__)
static inline void skr_atomic_yield(void) {
	__asm__ volatile ("pause" ::: "memory");
}
#elif defined(__aarch64__)
static inline void skr_atomic_yield(void) {
	__asm__ volatile("wfe");
}
#elif (defined(__arm__) && __ARM_ARCH__ >= 7)
static inline void skr_atomic_yield(void) {
	__asm__ volatile("yield" ::: "memory");
}
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) || defined(__POWERPC__)
#ifdef __APPLE__
static inline void skr_atomic_yield(void) {
	__asm__ volatile ("or r27,r27,r27" ::: "memory");
}
#else
static inline void skr_atomic_yield(void) {
	__asm__ __volatile__ ("or 27,27,27" ::: "memory");
}
#endif
#elif defined(__armel__) || defined(__ARMEL__)
static inline void skr_atomic_yield(void) {
	__asm__ volatile ("nop" ::: "memory");
}
#endif
#elif defined(__sun)
// Fallback for other archs
#include <synch.h>
static inline void skr_atomic_yield(void) {
	smt_pause();
}
#elif defined(__wasi__)
#include <sched.h>
static inline void skr_atomic_yield(void) {
	sched_yield();
}
#elif defined(__cplusplus)
#include <thread>
static inline void skr_atomic_yield(void) {
	std::this_thread::yield();
}
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static inline void skr_atomic_yield(void) {
	YieldProcessor();
}
#else
#include <unistd.h>
static inline void skr_atomic_yield(void) {
	sleep(0);
}
#endif

#if defined(__clang__) && defined(_MSC_VER)
#pragma clang diagnostic pop
#endif