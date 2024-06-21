#pragma once
#include "./common.inc"

#ifdef __cplusplus
extern "C" {
#endif

typedef long _plat_atom32;

inline static _plat_atom32 _plat32_atomic_fetch_add_explicit(_Atomic(_plat_atom32)*p, _plat_atom32 add, _plat_memory_order mo) {
  (void)(mo);
  return (_plat_atom32)_InterlockedExchangeAdd((volatile _plat_atom32*)p, (_plat_atom32)add);
}

inline static _plat_atom32 _plat32_atomic_fetch_sub_explicit(_Atomic(_plat_atom32)*p, _plat_atom32 sub, _plat_memory_order mo) {
  (void)(mo);
  return (_plat_atom32)_InterlockedExchangeAdd((volatile _plat_atom32*)p, -((_plat_atom32)sub));
}

inline static _plat_atom32 _plat32_atomic_fetch_and_explicit(_Atomic(_plat_atom32)*p, _plat_atom32 x, _plat_memory_order mo) {
  (void)(mo);
  return (_plat_atom32)_InterlockedAnd((volatile _plat_atom32*)p, (_plat_atom32)x);
}

inline static _plat_atom32 _plat32_atomic_fetch_or_explicit(_Atomic(_plat_atom32)*p, _plat_atom32 x, _plat_memory_order mo) {
  (void)(mo);
  return (_plat_atom32)_InterlockedOr((volatile _plat_atom32*)p, (_plat_atom32)x);
}

inline static bool _plat32_atomic_compare_exchange_strong_explicit(_Atomic(_plat_atom32)*p, _plat_atom32* expected, _plat_atom32 desired, _plat_memory_order mo1, _plat_memory_order mo2) {
  (void)(mo1); (void)(mo2);
  _plat_atom32 read = (_plat_atom32)_InterlockedCompareExchange((volatile _plat_atom32*)p, (_plat_atom32)desired, (_plat_atom32)(*expected));
  if (read == *expected) {
    return true;
  }
  else {
    *expected = read;
    return false;
  }
}

inline static bool _plat32_atomic_compare_exchange_weak_explicit(_Atomic(_plat_atom32)*p, _plat_atom32* expected, _plat_atom32 desired, _plat_memory_order mo1, _plat_memory_order mo2) {
  return _plat32_atomic_compare_exchange_strong_explicit(p, expected, desired, mo1, mo2);
}

inline static _plat_atom32 _plat32_atomic_exchange_explicit(_Atomic(_plat_atom32)*p, _plat_atom32 exchange, _plat_memory_order mo) {
  (void)(mo);
  return (_plat_atom32)_InterlockedExchange((volatile _plat_atom32*)p, (_plat_atom32)exchange);
}

inline static void _plat32_atomic_thread_fence(_plat_memory_order mo) {
  (void)(mo);
  _Atomic(_plat_atom32) x = 0;
  _plat32_atomic_exchange_explicit(&x, 1, mo);
}

inline static _plat_atom32 _plat32_atomic_load_explicit(_Atomic(_plat_atom32) const* p, _plat_memory_order mo) {
  (void)(mo);
#if defined(_M_IX86) || defined(_M_X64)
  return *p;
#else
  _plat_atom32 x = *p;
  if (mo > _plat_memory_order_relaxed) {
    while (!_plat32_atomic_compare_exchange_weak_explicit((_Atomic(_plat_atom32)*)p, &x, x, mo, _plat_memory_order_relaxed)) { /* nothing */ };
  }
  return x;
#endif
}

inline static void _plat32_atomic_store_explicit(_Atomic(_plat_atom32)*p, _plat_atom32 x, _plat_memory_order mo) {
  (void)(mo);
#if defined(_M_IX86) || defined(_M_X64)
  *p = x;
#else
  _plat32_atomic_exchange_explicit(p, x, mo);
#endif
}

#ifdef __cplusplus
}
#endif