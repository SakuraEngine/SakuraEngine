#pragma once
#ifdef _WIN32

#include "plat/msvc32.inc"
#include "plat/msvc64.inc"

template <typename T>
static constexpr auto _SAtomic_Integral64 = std::is_integral_v<T> && sizeof(T) == sizeof(int64_t);

template <typename T>
static constexpr auto _SAtomic_Integral32 = std::is_integral_v<T> && sizeof(T) == sizeof(int32_t);

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A> requires(_SAtomic_Integral32<A>)
C skr_atomic_load_explicit(const _SAtomic(A)* obj, skr_memory_order order) {
  return _plat32_atomic_load_explicit((_SAtomic(_plat_atom32)*)obj, order);
}
template<typename A, typename B = A> requires(_SAtomic_Integral32<A>)
void skr_atomic_store_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  _plat32_atomic_store_explicit((_SAtomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
C skr_atomic_exchange_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat32_atomic_exchange_explicit((_SAtomic(_plat_atom32)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
C skr_atomic_fetch_add_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat32_atomic_fetch_add_explicit((_SAtomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
C skr_atomic_fetch_sub_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat32_atomic_fetch_sub_explicit((_SAtomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
C skr_atomic_fetch_or_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat32_atomic_fetch_or_explicit((_SAtomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
C skr_atomic_fetch_xor_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat32_atomic_fetch_xor_explicit((_SAtomic(_plat_atom32)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
C skr_atomic_fetch_and_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat32_atomic_fetch_and_explicit((_SAtomic(_plat_atom32)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
bool skr_atomic_compare_exchange_weak_explicit(_SAtomic(A)* obj, C* exp, B desr, skr_memory_order succ, skr_memory_order fail) {
  return _plat32_atomic_compare_exchange_weak_explicit((_SAtomic(_plat_atom32)*)obj, (_plat_atom32*)exp, desr, succ, fail);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral32<A>)
bool skr_atomic_compare_exchange_strong_explicit(_SAtomic(A)* obj, C* exp, B desr, skr_memory_order succ, skr_memory_order fail) {
  return _plat32_atomic_compare_exchange_strong_explicit((_SAtomic(_plat_atom32)*)obj, (_plat_atom32*)exp, desr, succ, fail);
}

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A> requires(_SAtomic_Integral64<A>)
C skr_atomic_load_explicit(const _SAtomic(A)* obj, skr_memory_order order) {
  return _plat64_atomic_load_explicit((_SAtomic(_plat_atom64)*)obj, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
void skr_atomic_store_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  _plat64_atomic_store_explicit((_SAtomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
C skr_atomic_exchange_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat64_atomic_exchange_explicit((_SAtomic(_plat_atom64)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
C skr_atomic_fetch_add_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat64_atomic_fetch_add_explicit((_SAtomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
C skr_atomic_fetch_sub_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat64_atomic_fetch_sub_explicit((_SAtomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
C skr_atomic_fetch_or_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat64_atomic_fetch_or_explicit((_SAtomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
C skr_atomic_fetch_xor_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat64_atomic_fetch_xor_explicit((_SAtomic(_plat_atom64)*)obj, desr, order);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
C skr_atomic_fetch_and_explicit(_SAtomic(A)* obj, B desr, skr_memory_order order) {
  return _plat64_atomic_fetch_and_explicit((_SAtomic(_plat_atom64)*)obj, desr, order);
}

template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
bool skr_atomic_compare_exchange_weak_explicit(_SAtomic(A)* obj, C* exp, B desr, skr_memory_order succ, skr_memory_order fail) {
  return _plat64_atomic_compare_exchange_weak_explicit((_SAtomic(_plat_atom64)*)obj, (_plat_atom64*)exp, desr, succ, fail);
}
template<typename A, typename B, typename C = A> requires(_SAtomic_Integral64<A>)
bool skr_atomic_compare_exchange_strong_explicit(_SAtomic(A)* obj, C* exp, B desr, skr_memory_order succ, skr_memory_order fail) {
  return _plat64_atomic_compare_exchange_strong_explicit((_SAtomic(_plat_atom64)*)obj, (_plat_atom64*)exp, desr, succ, fail);
}

//------------------------------------------------------------------------------------------------//

template<typename A, typename C = A>
C skr_atomic_load(const _SAtomic(A)* obj) {
  return skr_atomic_load_explicit(obj, skr_memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
void skr_atomic_store(_SAtomic(A)* obj, B desr) {
  skr_atomic_store_explicit(obj, desr, skr_memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C skr_atomic_exchange(_SAtomic(A)* obj, B desr) {
  return skr_atomic_exchange_explicit(obj, desr, skr_memory_order_seq_cst);
}

template<typename A, typename B, typename C = A>
C skr_atomic_fetch_add(_SAtomic(A)* obj, B desr) {
  return skr_atomic_fetch_add_explicit(obj, desr, skr_memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C skr_atomic_fetch_sub(_SAtomic(A)* obj, B desr) {
  return skr_atomic_fetch_sub_explicit(obj, desr, skr_memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C skr_atomic_fetch_or(_SAtomic(A)* obj, B desr) {
  return skr_atomic_fetch_or_explicit(obj, desr, skr_memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C skr_atomic_fetch_xor(_SAtomic(A)* obj, B desr) {
  return skr_atomic_fetch_xor_explicit(obj, desr, skr_memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
C skr_atomic_fetch_and(_SAtomic(A)* obj, B desr) {
  return skr_atomic_fetch_and_explicit(obj, desr, skr_memory_order_seq_cst);
}

template<typename A, typename B, typename C = A>
bool skr_atomic_compare_exchange_weak(_SAtomic(A)* obj, C* exp, B desr) {
  return skr_atomic_compare_exchange_weak_explicit(obj, exp, desr, skr_memory_order_seq_cst, skr_memory_order_seq_cst);
}
template<typename A, typename B, typename C = A>
bool skr_atomic_compare_exchange_strong(_SAtomic(A)* obj, C* exp, B desr) {
  return skr_atomic_compare_exchange_strong_explicit(obj, exp, desr, skr_memory_order_seq_cst, skr_memory_order_seq_cst);
}

#else // C++23 using interface from libc++ implementation

#include <atomic>
#include <version>

#ifdef _SAtomic
# undef _SAtomic
#endif

#define _SAtomic(_Tp) ::std::atomic<_Tp>

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