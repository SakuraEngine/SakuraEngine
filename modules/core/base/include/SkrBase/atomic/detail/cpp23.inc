#pragma once
#ifdef _WIN32

#include "plat/msvc32.inc"
#include "plat/msvc64.inc"

template <typename T>
static constexpr auto _Atomic_Integral64 = std::is_integral_v<T> && sizeof(T) == sizeof(int64_t);

template <typename T>
static constexpr auto _Atomic_Integral32 = std::is_integral_v<T> && sizeof(T) == sizeof(int32_t);

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_load_explicit(const _Atomic(A)* obj, _plat_memory_order order) {
  return _plat32_atomic_load_explicit((_Atomic(_plat_atom32)*)obj, order);
}
template<typename A, typename B = A> requires(_Atomic_Integral32<A>)
void atomic_store_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  _plat32_atomic_store_explicit((_Atomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_exchange_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat32_atomic_exchange_explicit((_Atomic(_plat_atom32)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_add_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat32_atomic_fetch_add_explicit((_Atomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_sub_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat32_atomic_fetch_sub_explicit((_Atomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_or_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat32_atomic_fetch_or_explicit((_Atomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_xor_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat32_atomic_fetch_xor_explicit((_Atomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
C atomic_fetch_and_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat32_atomic_fetch_and_explicit((_Atomic(_plat_atom32)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
bool atomic_compare_exchange_weak_explicit(_Atomic(A)* obj, C* exp, B desr, _plat_memory_order succ, _plat_memory_order fail) {
  return _plat32_atomic_compare_exchange_weak_explicit((_Atomic(_plat_atom32)*)obj, (_plat_atom32*)exp, desr, succ, fail);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral32<A>)
bool atomic_compare_exchange_strong_explicit(_Atomic(A)* obj, C* exp, B desr, _plat_memory_order succ, _plat_memory_order fail) {
  return _plat32_atomic_compare_exchange_strong_explicit((_Atomic(_plat_atom32)*)obj, (_plat_atom32*)exp, desr, succ, fail);
}

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_load_explicit(const _Atomic(A)* obj, _plat_memory_order order) {
  return _plat64_atomic_load_explicit((_Atomic(_plat_atom64)*)obj, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
void atomic_store_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  _plat64_atomic_store_explicit((_Atomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_exchange_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat64_atomic_exchange_explicit((_Atomic(_plat_atom64)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_add_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat64_atomic_fetch_add_explicit((_Atomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_sub_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat64_atomic_fetch_sub_explicit((_Atomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_or_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat64_atomic_fetch_or_explicit((_Atomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_xor_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat64_atomic_fetch_xor_explicit((_Atomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
C atomic_fetch_and_explicit(_Atomic(A)* obj, B desr, _plat_memory_order order) {
  return _plat64_atomic_fetch_and_explicit((_Atomic(_plat_atom64)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
bool atomic_compare_exchange_weak_explicit(_Atomic(A)* obj, C* exp, B desr, _plat_memory_order succ, _plat_memory_order fail) {
  return _plat64_atomic_compare_exchange_weak_explicit((_Atomic(_plat_atom64)*)obj, (_plat_atom64*)exp, desr, succ, fail);
}
template<typename A, typename B, typename C = A> requires(_Atomic_Integral64<A>)
bool atomic_compare_exchange_strong_explicit(_Atomic(A)* obj, C* exp, B desr, _plat_memory_order succ, _plat_memory_order fail) {
  return _plat64_atomic_compare_exchange_strong_explicit((_Atomic(_plat_atom64)*)obj, (_plat_atom64*)exp, desr, succ, fail);
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

#else // C++23 using interface from libc++ implementation

#include <atomic>
#include <version>

#ifdef _Atomic
# undef _Atomic
#endif

#define _Atomic(_Tp) ::std::atomic<_Tp>

using std::memory_order;
using std::memory_order_relaxed;
using std::memory_order_consume;
using std::memory_order_acquire;
using std::memory_order_release;
using std::memory_order_acq_rel;
using std::memory_order_seq_cst;

using std::atomic_flag;

using std::atomic_bool;
using std::atomic_char;
using std::atomic_schar;
using std::atomic_uchar;
using std::atomic_short;
using std::atomic_ushort;
using std::atomic_int;
using std::atomic_uint;
using std::atomic_long;
using std::atomic_ulong;
using std::atomic_llong;
using std::atomic_ullong;
using std::atomic_char8_t;
using std::atomic_char16_t;
using std::atomic_char32_t;
using std::atomic_wchar_t;

using std::atomic_int8_t;
using std::atomic_uint8_t;
using std::atomic_int16_t;
using std::atomic_uint16_t;
using std::atomic_int32_t;
using std::atomic_uint32_t;
using std::atomic_int64_t;
using std::atomic_uint64_t;

using std::atomic_int_least8_t;
using std::atomic_uint_least8_t;
using std::atomic_int_least16_t;
using std::atomic_uint_least16_t;
using std::atomic_int_least32_t;
using std::atomic_uint_least32_t;
using std::atomic_int_least64_t;
using std::atomic_uint_least64_t;

using std::atomic_int_fast8_t;
using std::atomic_uint_fast8_t;
using std::atomic_int_fast16_t;
using std::atomic_uint_fast16_t;
using std::atomic_int_fast32_t;
using std::atomic_uint_fast32_t;
using std::atomic_int_fast64_t;
using std::atomic_uint_fast64_t;

using std::atomic_intptr_t;
using std::atomic_uintptr_t;
using std::atomic_size_t;
using std::atomic_ptrdiff_t;
using std::atomic_intmax_t;
using std::atomic_uintmax_t;

using std::atomic_compare_exchange_strong;
using std::atomic_compare_exchange_strong_explicit;
using std::atomic_compare_exchange_weak;
using std::atomic_compare_exchange_weak_explicit;
using std::atomic_exchange;
using std::atomic_exchange_explicit;
using std::atomic_fetch_add;
using std::atomic_fetch_add_explicit;
using std::atomic_fetch_and;
using std::atomic_fetch_and_explicit;
using std::atomic_fetch_or;
using std::atomic_fetch_or_explicit;
using std::atomic_fetch_sub;
using std::atomic_fetch_sub_explicit;
using std::atomic_flag_clear;
using std::atomic_flag_clear_explicit;
using std::atomic_flag_test_and_set;
using std::atomic_flag_test_and_set_explicit;
using std::atomic_is_lock_free;
using std::atomic_load;
using std::atomic_load_explicit;
using std::atomic_store;
using std::atomic_store_explicit;

using std::atomic_signal_fence;
using std::atomic_thread_fence;

#endif