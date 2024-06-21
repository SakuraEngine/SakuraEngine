#pragma once
#include <stdint.h>
#include <intrin0.h>
#ifndef __cplusplus
#include <stdbool.h>
#else
#include <type_traits>
extern "C" {
#endif

inline static bool atomic_is_lock_free() { return true; }

typedef enum _msc_memory_order {
  memory_order_relaxed,
  memory_order_consume,
  memory_order_acquire,
  memory_order_release,
  memory_order_acq_rel,
  memory_order_seq_cst
} _msc_memory_order;

#define _Atomic(tp)         volatile tp

#ifndef ATOMIC_VAR_INIT
#define ATOMIC_VAR_INIT(x)  x
#endif

typedef int64_t _msc_atom64;
typedef long _msc_atom32;

#pragma region Atomic32

inline static _msc_atom32 _msc32_atomic_fetch_add_explicit(_Atomic(_msc_atom32)*p, _msc_atom32 add, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom32)_InterlockedExchangeAdd((volatile _msc_atom32*)p, (_msc_atom32)add);
}

inline static _msc_atom32 _msc32_atomic_fetch_sub_explicit(_Atomic(_msc_atom32)*p, _msc_atom32 sub, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom32)_InterlockedExchangeAdd((volatile _msc_atom32*)p, -((_msc_atom32)sub));
}

inline static _msc_atom32 _msc32_atomic_fetch_and_explicit(_Atomic(_msc_atom32)*p, _msc_atom32 x, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom32)_InterlockedAnd((volatile _msc_atom32*)p, (_msc_atom32)x);
}

inline static _msc_atom32 _msc32_atomic_fetch_or_explicit(_Atomic(_msc_atom32)*p, _msc_atom32 x, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom32)_InterlockedOr((volatile _msc_atom32*)p, (_msc_atom32)x);
}

inline static bool _msc32_atomic_compare_exchange_strong_explicit(_Atomic(_msc_atom32)*p, _msc_atom32* expected, _msc_atom32 desired, _msc_memory_order mo1, _msc_memory_order mo2) {
  (void)(mo1); (void)(mo2);
  _msc_atom32 read = (_msc_atom32)_InterlockedCompareExchange((volatile _msc_atom32*)p, (_msc_atom32)desired, (_msc_atom32)(*expected));
  if (read == *expected) {
    return true;
  }
  else {
    *expected = read;
    return false;
  }
}

inline static bool _msc32_atomic_compare_exchange_weak_explicit(_Atomic(_msc_atom32)*p, _msc_atom32* expected, _msc_atom32 desired, _msc_memory_order mo1, _msc_memory_order mo2) {
  return _msc32_atomic_compare_exchange_strong_explicit(p, expected, desired, mo1, mo2);
}

inline static _msc_atom32 _msc32_atomic_exchange_explicit(_Atomic(_msc_atom32)*p, _msc_atom32 exchange, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom32)_InterlockedExchange((volatile _msc_atom32*)p, (_msc_atom32)exchange);
}

inline static void _msc32_atomic_thread_fence(_msc_memory_order mo) {
  (void)(mo);
  _Atomic(_msc_atom32) x = 0;
  _msc32_atomic_exchange_explicit(&x, 1, mo);
}

inline static _msc_atom32 _msc32_atomic_load_explicit(_Atomic(_msc_atom32) const* p, _msc_memory_order mo) {
  (void)(mo);
#if defined(_M_IX86) || defined(_M_X64)
  return *p;
#else
  _msc_atom32 x = *p;
  if (mo > _msc_memory_order_relaxed) {
    while (!_msc32_atomic_compare_exchange_weak_explicit((_Atomic(_msc_atom32)*)p, &x, x, mo, _msc_memory_order_relaxed)) { /* nothing */ };
  }
  return x;
#endif
}

inline static void _msc32_atomic_store_explicit(_Atomic(_msc_atom32)*p, _msc_atom32 x, _msc_memory_order mo) {
  (void)(mo);
#if defined(_M_IX86) || defined(_M_X64)
  *p = x;
#else
  _msc32_atomic_exchange_explicit(p, x, mo);
#endif
}

#pragma endregion

#pragma region Atomic64

inline static _msc_atom64 _msc64_atomic_fetch_add_explicit(_Atomic(_msc_atom64)*p, _msc_atom64 add, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom64)_InterlockedExchangeAdd64((volatile _msc_atom64*)p, (_msc_atom64)add);
}

inline static _msc_atom64 _msc64_atomic_fetch_sub_explicit(_Atomic(_msc_atom64)*p, _msc_atom64 sub, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom64)_InterlockedExchangeAdd64((volatile _msc_atom64*)p, -((_msc_atom64)sub));
}

inline static _msc_atom64 _msc64_atomic_fetch_and_explicit(_Atomic(_msc_atom64)*p, _msc_atom64 x, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom64)_InterlockedAnd64((volatile _msc_atom64*)p, (_msc_atom64)x);
}

inline static _msc_atom64 _msc64_atomic_fetch_or_explicit(_Atomic(_msc_atom64)*p, _msc_atom64 x, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom64)_InterlockedOr64((volatile _msc_atom64*)p, (_msc_atom64)x);
}

inline static bool _msc64_atomic_compare_exchange_strong_explicit(_Atomic(_msc_atom64)*p, _msc_atom64* expected, _msc_atom64 desired, _msc_memory_order mo1, _msc_memory_order mo2) {
  (void)(mo1); (void)(mo2);
  _msc_atom64 read = (_msc_atom64)_InterlockedCompareExchange64((volatile _msc_atom64*)p, (_msc_atom64)desired, (_msc_atom64)(*expected));
  if (read == *expected) {
    return true;
  }
  else {
    *expected = read;
    return false;
  }
}

inline static bool _msc64_atomic_compare_exchange_weak_explicit(_Atomic(_msc_atom64)*p, _msc_atom64* expected, _msc_atom64 desired, _msc_memory_order mo1, _msc_memory_order mo2) {
  return _msc64_atomic_compare_exchange_strong_explicit(p, expected, desired, mo1, mo2);
}

inline static _msc_atom64 _msc64_atomic_exchange_explicit(_Atomic(_msc_atom64)*p, _msc_atom64 exchange, _msc_memory_order mo) {
  (void)(mo);
  return (_msc_atom64)_InterlockedExchange64((volatile _msc_atom64*)p, (_msc_atom64)exchange);
}

inline static void _msc64_atomic_thread_fence(_msc_memory_order mo) {
  (void)(mo);
  _Atomic(_msc_atom64) x = 0;
  _msc64_atomic_exchange_explicit(&x, 1, mo);
}

inline static _msc_atom64 _msc64_atomic_load_explicit(_Atomic(_msc_atom64) const* p, _msc_memory_order mo) {
  (void)(mo);
#if defined(_M_IX86) || defined(_M_X64)
  return *p;
#else
  _msc_atom64 x = *p;
  if (mo > _msc_memory_order_relaxed) {
    while (!_msc64_atomic_compare_exchange_weak_explicit((_Atomic(_msc_atom64)*)p, &x, x, mo, _msc_memory_order_relaxed)) { /* nothing */ };
  }
  return x;
#endif
}

inline static void _msc64_atomic_store_explicit(_Atomic(_msc_atom64)*p, _msc_atom64 x, _msc_memory_order mo) {
  (void)(mo);
#if defined(_M_IX86) || defined(_M_X64)
  *p = x;
#else
  _msc64_atomic_exchange_explicit(p, x, mo);
#endif
}

inline static int64_t _msc64_atomic_loadi64_explicit(_Atomic(int64_t)*p, _msc_memory_order mo) {
  (void)(mo);
#if defined(_M_X64)
  return *p;
#else
  int64_t old = *p;
  int64_t x = old;
  while ((old = InterlockedCompareExchange64(p, x, old)) != x) {
    x = old;
  }
  return x;
#endif
}

inline static void _msc64_atomic_storei64_explicit(_Atomic(int64_t)*p, int64_t x, _msc_memory_order mo) {
  (void)(mo);
#if defined(x_M_IX86) || defined(_M_X64)
  *p = x;
#else
  InterlockedExchange64(p, x);
#endif
}

#pragma endregion

#ifndef __cplusplus // C11 Standard Macros

#define atomic_store_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_store_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile uint32_t*: _msc32_atomic_store_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile int64_t*: _msc64_atomic_store_explicit((volatile _msc_atom64*) px, v, mo), \
  volatile uint64_t*: _msc64_atomic_store_explicit((volatile _msc_atom64*)px, v, mo))

#define atomic_load_explicit(px, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_load_explicit((volatile _msc_atom32*)px, mo), \
  volatile uint32_t*: _msc32_atomic_load_explicit((volatile _msc_atom32*)px, mo), \
  volatile int64_t*: _msc64_atomic_load_explicit((volatile _msc_atom64*)px, mo), \
  volatile uint64_t*: _msc64_atomic_load_explicit((volatile _msc_atom64*)px, mo))

#define atomic_exchange_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_exchange_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile uint32_t*: _msc32_atomic_exchange_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile int64_t*: _msc64_atomic_exchange_explicit((volatile _msc_atom64*)px, v, mo), \
  volatile uint64_t*: _msc64_atomic_exchange_explicit((volatile _msc_atom64*)px, v, mo))

#define atomic_fetch_add_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_fetch_add_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile uint32_t*: _msc32_atomic_fetch_add_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile int64_t*: _msc64_atomic_fetch_add_explicit((volatile _msc_atom64*)px, v, mo), \
  volatile uint64_t*: _msc64_atomic_fetch_add_explicit((volatile _msc_atom64*)px, v, mo))

#define atomic_fetch_sub_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_fetch_sub_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile uint32_t*: _msc32_atomic_fetch_sub_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile int64_t*: _msc64_atomic_fetch_sub_explicit((volatile _msc_atom64*)px, v, mo), \
  volatile uint64_t*: _msc64_atomic_fetch_sub_explicit((volatile _msc_atom64*)px, v, mo))

#define atomic_fetch_or_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_fetch_or_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile uint32_t*: _msc32_atomic_fetch_or_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile int64_t*: _msc64_atomic_fetch_or_explicit((volatile _msc_atom64*)px, v, mo), \
  volatile uint64_t*: _msc64_atomic_fetch_or_explicit((volatile _msc_atom64*)px, v, mo))

#define atomic_fetch_xor_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_fetch_xor_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile uint32_t*: _msc32_atomic_fetch_xor_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile int64_t*: _msc64_atomic_fetch_xor_explicit((volatile _msc_atom64*)px, v, mo), \
  volatile uint64_t*: _msc64_atomic_fetch_xor_explicit((volatile _msc_atom64*)px, v, mo))

#define atomic_fetch_and_explicit(px, v, mo) _Generic((px), \
  volatile int32_t*: _msc32_atomic_fetch_and_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile uint32_t*: _msc32_atomic_fetch_and_explicit((volatile _msc_atom32*)px, v, mo), \
  volatile int64_t*: _msc64_atomic_fetch_and_explicit((volatile _msc_atom64*)px, v, mo), \
  volatile uint64_t*: _msc64_atomic_fetch_and_explicit((volatile _msc_atom64*)px, v, mo))

#define atomic_compare_exchange_weak_explicit(px, e, v, mo1, mo2) _Generic((px), \
  volatile int32_t*: _msc32_atomic_compare_exchange_weak_explicit((volatile _msc_atom32*)px, (_msc_atom32*)e, v, mo1, mo2), \
  volatile uint32_t*: _msc32_atomic_compare_exchange_weak_explicit((volatile _msc_atom32*)px, (_msc_atom32*)e, v, mo1, mo2), \
  volatile int64_t*: _msc64_atomic_compare_exchange_weak_explicit((volatile _msc_atom64*)px, (_msc_atom64*)e, v, mo1, mo2), \
  volatile uint64_t*: _msc64_atomic_compare_exchange_weak_explicit((volatile _msc_atom64*)px, (_msc_atom64*)e, v, mo1, mo2))

#define atomic_compare_exchange_strong_explicit(px, e, v, mo1, mo2) _Generic((px), \
  volatile int32_t*: _msc32_atomic_compare_exchange_strong_explicit((volatile _msc_atom32*)px, (_msc_atom32*)e, v, mo1, mo2), \
  volatile uint32_t*: _msc32_atomic_compare_exchange_strong_explicit((volatile _msc_atom32*)px, (_msc_atom32*)e, v, mo1, mo2), \
  volatile int64_t*: _msc64_atomic_compare_exchange_strong_explicit((volatile _msc_atom64*)px, (_msc_atom64*)e, v, mo1, mo2), \
  volatile uint64_t*: _msc64_atomic_compare_exchange_strong_explicit((volatile _msc_atom64*)px, (_msc_atom64*)e, v, mo1, mo2))

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

#else // C++ Template Specialization as https://en.cppreference.com/w/c/atomic/atomic_load
} // end extern "C"

//------------------------------------------------------------------------------------------------//

template <typename T>
static constexpr auto _Atomic_Integral64 = std::is_integral_v<T> && sizeof(T) == sizeof(int64_t);

template <typename T>
static constexpr auto _Atomic_Integral32 = std::is_integral_v<T> && sizeof(T) == sizeof(int32_t);

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_load_explicit(const _Atomic(A)* obj, _msc_memory_order order) {
  return _msc32_atomic_load_explicit((_Atomic(_msc_atom32)*)obj, order);
}
template<typename A, typename B = A> requires(_Atomic_Integral32<A>)
void atomic_store_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  _msc32_atomic_store_explicit((_Atomic(_msc_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_exchange_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc32_atomic_exchange_explicit((_Atomic(_msc_atom32)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_add_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc32_atomic_fetch_add_explicit((_Atomic(_msc_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_sub_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc32_atomic_fetch_sub_explicit((_Atomic(_msc_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_or_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc32_atomic_fetch_or_explicit((_Atomic(_msc_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_xor_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc32_atomic_fetch_xor_explicit((_Atomic(_msc_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_and_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc32_atomic_fetch_and_explicit((_Atomic(_msc_atom32)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
bool atomic_compare_exchange_weak_explicit(_Atomic(A)* obj, C* exp, B desr, _msc_memory_order succ, _msc_memory_order fail) {
  return _msc32_atomic_compare_exchange_weak_explicit((_Atomic(_msc_atom32)*)obj, (_msc_atom32*)exp, desr, succ, fail);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
bool atomic_compare_exchange_strong_explicit(_Atomic(A)* obj, C* exp, B desr, _msc_memory_order succ, _msc_memory_order fail) {
  return _msc32_atomic_compare_exchange_strong_explicit((_Atomic(_msc_atom32)*)obj, (_msc_atom32*)exp, desr, succ, fail);
}

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_load_explicit(const _Atomic(A)* obj, _msc_memory_order order) {
  return _msc64_atomic_load_explicit((_Atomic(_msc_atom64)*)obj, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
void atomic_store_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  _msc64_atomic_store_explicit((_Atomic(_msc_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_exchange_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc64_atomic_exchange_explicit((_Atomic(_msc_atom64)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_add_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc64_atomic_fetch_add_explicit((_Atomic(_msc_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_sub_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc64_atomic_fetch_sub_explicit((_Atomic(_msc_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_or_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc64_atomic_fetch_or_explicit((_Atomic(_msc_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_xor_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc64_atomic_fetch_xor_explicit((_Atomic(_msc_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_and_explicit(_Atomic(A)* obj, B desr, _msc_memory_order order) {
  return _msc64_atomic_fetch_and_explicit((_Atomic(_msc_atom64)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
bool atomic_compare_exchange_weak_explicit(_Atomic(A)* obj, C* exp, B desr, _msc_memory_order succ, _msc_memory_order fail) {
  return _msc64_atomic_compare_exchange_weak_explicit((_Atomic(_msc_atom64)*)obj, (_msc_atom64*)exp, desr, succ, fail);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
bool atomic_compare_exchange_strong_explicit(_Atomic(A)* obj, C* exp, B desr, _msc_memory_order succ, _msc_memory_order fail) {
  return _msc64_atomic_compare_exchange_strong_explicit((_Atomic(_msc_atom64)*)obj, (_msc_atom64*)exp, desr, succ, fail);
}

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A>
C atomic_load(const _Atomic(A)* obj) {
  return atomic_load_explicit(obj, memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
void atomic_store(_Atomic(A)* obj, B desr) {
  atomic_store_explicit(obj, desr, memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C atomic_exchange(_Atomic(A)* obj, B desr) {
  return atomic_exchange_explicit(obj, desr, memory_order_seq_cst);
}

template<typename A, typename B, typename C = A>
C atomic_fetch_add(_Atomic(A)* obj, B desr) {
  return atomic_fetch_add_explicit(obj, desr, memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C atomic_fetch_sub(_Atomic(A)* obj, B desr) {
  return atomic_fetch_sub_explicit(obj, desr, memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C atomic_fetch_or(_Atomic(A)* obj, B desr) {
  return atomic_fetch_or_explicit(obj, desr, memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C atomic_fetch_xor(_Atomic(A)* obj, B desr) {
  return atomic_fetch_xor_explicit(obj, desr, memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C atomic_fetch_and(_Atomic(A)* obj, B desr) {
  return atomic_fetch_and_explicit(obj, desr, memory_order_seq_cst);
}

template<typename A, typename B, typename C = A>
bool atomic_compare_exchange_weak(_Atomic(A)* obj, C* exp, B desr) {
  return atomic_compare_exchange_weak_explicit(obj, exp, desr, memory_order_seq_cst, memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
bool atomic_compare_exchange_strong(_Atomic(A)* obj, C* exp, B desr) {
  return atomic_compare_exchange_strong_explicit(obj, exp, desr, memory_order_seq_cst, memory_order_seq_cst);
}

//------------------------------------------------------------------------------------------------//

#endif