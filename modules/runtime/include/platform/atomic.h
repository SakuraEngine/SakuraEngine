/* clang-format off */
#pragma once
#include "platform/configure.h"

typedef volatile SKR_ALIGNAS(4) uint32_t SAtomicU32;
typedef volatile SKR_ALIGNAS(8) uint64_t SAtomicU64;
typedef volatile SKR_ALIGNAS(4) uint32_t SAtomic32;
typedef volatile SKR_ALIGNAS(8) uint64_t SAtomic64;
typedef volatile SKR_ALIGNAS(PTR_SIZE) uintptr_t SAtomicUPtr;
typedef volatile SKR_ALIGNAS(PTR_SIZE) intptr_t SAtomicPtr;

#if defined(__clang__) && defined(_MSC_VER)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(_MSC_VER) && !defined(NX64)

#include <intrin0.h>

	#define skr_memorybarrier_acquire() _ReadWriteBarrier()
	#define skr_memorybarrier_release() _ReadWriteBarrier()

	#define skr_atomicu32_load_relaxed(pVar) (*(pVar))
	#define skr_atomicu32_store_relaxed(dst, val) _InterlockedExchange( (volatile long*)(dst), val )
	#define skr_atomicu32_add_relaxed(dst, val) _InterlockedExchangeAdd( (volatile long*)(dst), (val) )
	#define skr_atomicu32_cas_relaxed(dst, cmp_val, new_val) _InterlockedCompareExchange( (volatile long*)(dst), (new_val), (cmp_val) )

	#define skr_atomicu64_load_relaxed(pVar) (*(pVar))
	#define skr_atomicu64_store_relaxed(dst, val) _InterlockedExchange64( (volatile LONG64*)(dst), val )
	#define skr_atomicu64_add_relaxed(dst, val) _InterlockedExchangeAdd64( (volatile LONG64*)(dst), (val) )
	#define skr_atomicu64_cas_relaxed(dst, cmp_val, new_val) _InterlockedCompareExchange64( (volatile LONG64*)(dst), (new_val), (cmp_val) )

	#define skr_atomic32_load_relaxed(pVar) (*(pVar))
	#define skr_atomic32_store_relaxed(dst, val) _InterlockedExchange( (volatile long*)(dst), val )
	#define skr_atomic32_add_relaxed(dst, val) _InterlockedExchangeAdd( (volatile long*)(dst), (val) )
	#define skr_atomic32_cas_relaxed(dst, cmp_val, new_val) _InterlockedCompareExchange( (volatile long*)(dst), (new_val), (cmp_val) )

	#define skr_atomic64_load_relaxed(pVar) (*(pVar))
	#define skr_atomic64_store_relaxed(dst, val) _InterlockedExchange64( (volatile LONG64*)(dst), val )
	#define skr_atomic64_add_relaxed(dst, val) _InterlockedExchangeAdd64( (volatile LONG64*)(dst), (val) )
	#define skr_atomic64_cas_relaxed(dst, cmp_val, new_val) _InterlockedCompareExchange64( (volatile LONG64*)(dst), (new_val), (cmp_val) )
#else
	#define skr_memorybarrier_acquire() __asm__ __volatile__("": : :"memory")
	#define skr_memorybarrier_release() __asm__ __volatile__("": : :"memory")

	#define skr_atomicu32_load_relaxed(pVar) (*(pVar))
	#define skr_atomicu32_store_relaxed(dst, val) __sync_lock_test_and_set ( (volatile int32_t*)(dst), val )
	#define skr_atomicu32_add_relaxed(dst, val) __sync_fetch_and_add( (volatile int32_t*)(dst), (val) )
	#define skr_atomicu32_cas_relaxed(dst, cmp_val, new_val) __sync_val_compare_and_swap( (volatile int32_t*)(dst), (cmp_val), (new_val) )

	#define skr_atomicu64_load_relaxed(pVar) (*(pVar))
	#define skr_atomicu64_store_relaxed(dst, val) __sync_lock_test_and_set ((volatile int64_t*) (dst), val )
	#define skr_atomicu64_add_relaxed(dst, val) __sync_fetch_and_add( (volatile int64_t*)(dst), (val) )
	#define skr_atomicu64_cas_relaxed(dst, cmp_val, new_val) __sync_val_compare_and_swap( (volatile int64_t*)(dst), (cmp_val), (new_val) )

	#define skr_atomic32_load_relaxed(pVar) (*(pVar))
	#define skr_atomic32_store_relaxed(dst, val) __sync_lock_test_and_set ( (volatile int32_t*)(dst), val )
	#define skr_atomic32_add_relaxed(dst, val) __sync_fetch_and_add( (volatile int32_t*)(dst), (val) )
	#define skr_atomic32_cas_relaxed(dst, cmp_val, new_val) __sync_val_compare_and_swap( (volatile int32_t*)(dst), (cmp_val), (new_val) )

	#define skr_atomic64_load_relaxed(pVar) (*(pVar))
	#define skr_atomic64_store_relaxed(dst, val) __sync_lock_test_and_set ((volatile int64_t*) (dst), val )
	#define skr_atomic64_add_relaxed(dst, val) __sync_fetch_and_add( (volatile int64_t*)(dst), (val) )
	#define skr_atomic64_cas_relaxed(dst, cmp_val, new_val) __sync_val_compare_and_swap( (volatile int64_t*)(dst), (cmp_val), (new_val) )
#endif

// Unsigned Atomic

FORCEINLINE static uint32_t skr_atomicu32_load_acquire(const SAtomicU32* pVar)
{
	uint32_t value = skr_atomicu32_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

FORCEINLINE static uint32_t skr_atomicu32_store_release(SAtomicU32* pVar, uint32_t val)
{
	skr_memorybarrier_release();
	return skr_atomicu32_store_relaxed(pVar, val);
}

FORCEINLINE static uint32_t skr_atomicu32_max_relaxed(SAtomicU32* dst, uint32_t val)
{
    uint32_t prev_val = val;
    do { prev_val = skr_atomicu32_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

FORCEINLINE static uint64_t skr_atomicu64_load_acquire(const SAtomicU64* pVar)
{
	uint64_t value = skr_atomicu64_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

FORCEINLINE static uint64_t skr_atomicu64_store_release(SAtomicU64* pVar, uint64_t val)
{
	skr_memorybarrier_release();
	return skr_atomicu64_store_relaxed(pVar, val);
}

FORCEINLINE static uint64_t skr_atomicu64_max_relaxed(SAtomicU64* dst, uint64_t val)
{
    uint64_t prev_val = val;
    do { prev_val = skr_atomicu64_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

// Atomic

FORCEINLINE static int32_t skr_atomic32_load_acquire(const SAtomic32* pVar)
{
	int32_t value = skr_atomic32_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

FORCEINLINE static int32_t skr_atomic32_store_release(SAtomic32* pVar, int32_t val)
{
	skr_memorybarrier_release();
	return skr_atomic32_store_relaxed(pVar, val);
}

FORCEINLINE static int32_t skr_atomic32_max_relaxed(SAtomic32* dst, int32_t val)
{
    int32_t prev_val = val;
    do { prev_val = skr_atomic32_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

FORCEINLINE static int64_t skr_atomic64_load_acquire(const SAtomic64* pVar)
{
	int64_t value = skr_atomic64_load_relaxed(pVar);
	skr_memorybarrier_acquire();
	return value;
}

FORCEINLINE static int64_t skr_atomic64_store_release(SAtomic64* pVar, int64_t val)
{
	skr_memorybarrier_release();
	return skr_atomic64_store_relaxed(pVar, val);
}

FORCEINLINE static int64_t skr_atomic64_max_relaxed(SAtomic64* dst, int64_t val)
{
    int64_t prev_val = val;
    do { prev_val = skr_atomic64_cas_relaxed(dst, prev_val, val); } while (prev_val < val);
    return prev_val;
}

#if PTR_SIZE == 4
	#define skr_atomicuptr_load_relaxed skr_atomicu32_load_relaxed
	#define skr_atomicuptr_load_acquire skr_atomicu32_load_acquire
	#define skr_atomicuptr_store_relaxed skr_atomicu32_store_relaxed
	#define skr_atomicuptr_store_release skr_atomicu32_store_release
	#define skr_atomicuptr_add_relaxed skr_atomicu32_add_relaxed
	#define skr_atomicuptr_cas_relaxed skr_atomicu32_cas_relaxed
	#define skr_atomicuptr_max_relaxed skr_atomicu32_max_relaxed

	#define skr_atomicptr_load_relaxed skr_atomic32_load_relaxed
	#define skr_atomicptr_load_acquire skr_atomic32_load_acquire
	#define skr_atomicptr_store_relaxed skr_atomic32_store_relaxed
	#define skr_atomicptr_store_release skr_atomic32_store_release
	#define skr_atomicptr_add_relaxed skr_atomic32_add_relaxed
	#define skr_atomicptr_cas_relaxed skr_atomic32_cas_relaxed
	#define skr_atomicptr_max_relaxed skr_atomic32_max_relaxed
#elif PTR_SIZE == 8
	#define skr_atomicuptr_load_relaxed skr_atomicu64_load_relaxed
	#define skr_atomicuptr_load_acquire skr_atomicu64_load_acquire
	#define skr_atomicuptr_store_relaxed skr_atomicu64_store_relaxed
	#define skr_atomicuptr_store_release skr_atomicu64_store_release
	#define skr_atomicuptr_add_relaxed skr_atomicu64_add_relaxed
	#define skr_atomicuptr_cas_relaxed skr_atomicu64_cas_relaxed
	#define skr_atomicuptr_max_relaxed skr_atomicu64_max_relaxed

	#define skr_atomicptr_load_relaxed skr_atomic64_load_relaxed
	#define skr_atomicptr_load_acquire skr_atomic64_load_acquire
	#define skr_atomicptr_store_relaxed skr_atomic64_store_relaxed
	#define skr_atomicptr_store_release skr_atomic64_store_release
	#define skr_atomicptr_add_relaxed skr_atomic64_add_relaxed
	#define skr_atomicptr_cas_relaxed skr_atomic64_cas_relaxed
	#define skr_atomicptr_max_relaxed skr_atomic64_max_relaxed
#endif

#if defined(__clang__) && defined(_MSC_VER)
#pragma clang diagnostic pop
#endif