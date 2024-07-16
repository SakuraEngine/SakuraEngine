#pragma once
#ifdef _WIN32
  #include "plat/msvc32.inc"
  #include "plat/msvc64.inc"
#else
  #include "plat/gcc32.inc"
  #include "plat/gcc64.inc"
#endif

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