#pragma once
#include "array.hxx"
#include "../type_traits.hxx"

template<typename T, uint64 N>
struct [[builtin("vec")]] vec;

template<typename T>
struct alignas(8) [[builtin("vec")]] vec<T, 2> {
	using ThisType = vec<T, 2>;
	static constexpr uint32 dim = 2;
	vec() noexcept = default;

	template<typename... Args>
		requires(sum_dim<0ull, Args...>() == 2)
	explicit constexpr vec(Args&&... args)
		: _v()// active union member
	{
		set<0>(args...);
	}

	template<concepts::arithmetic_scalar U>
	constexpr vec(U v) : _v(v, v) {}

	explicit constexpr vec(Array<T, 2> array) : vec{array[0], array[1]} {}

	template<concepts::arithmetic_scalar U>
	[[nodiscard]] operator vec<U, 2>() const {
		return vec<U, 2>(static_cast<U>(x), static_cast<U>(y));
	}

	template<uint64 I>
	float get() const {
		if constexpr (I == 1) {
			return y;
		}
		return x;
	}

#include "ops/vec_ops.inl"

	// DONT EDIT THIS FIELD LAYOUT
	union {
		Array<T, 2> _v;
#include "ops/swizzle2.inl"
	};
};

template<typename T>
struct alignas(16) [[builtin("vec")]] vec<T, 3> {
	using ThisType = vec<T, 3>;
	static constexpr uint32 dim = 3;
	vec() noexcept = default;

	template<typename... Args>
		requires(sum_dim<0ull, Args...>() == 3)
	explicit constexpr vec(Args&&... args)
		: _v()// active union member
	{
		set<0>(args...);
	}

	template<concepts::arithmetic_scalar U>
	constexpr vec(U v)
		: _v(v, v, v) {}

	explicit constexpr vec(Array<T, 3> array) : vec{array[0], array[1], array[2]} {}

	template<concepts::arithmetic_scalar U>
	[[nodiscard]] operator vec<U, 3>() const {
		return vec<U, 3>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z));
	}

	template<uint64 I>
	float get() const {
		if constexpr (I == 1) {
			return y;
		} else if constexpr (I == 2) {
			return z;
		}
		return x;
	}

#include "ops/vec_ops.inl"

	// DONT EDIT THIS FIELD LAYOUT
	union {
		Array<T, 3> _v;
#include "ops/swizzle3.inl"
	};
};

template<typename T>
struct alignas(16) [[builtin("vec")]] vec<T, 4> {
	using ThisType = vec<T, 4>;
	static constexpr uint32 dim = 4;
	vec() noexcept = default;

	template<typename... Args>
		requires(sum_dim<0ull, Args...>() == 4)
	explicit constexpr vec(Args&&... args)
		: _v()// active union member
	{
		set<0>(args...);
	}

	template<concepts::arithmetic_scalar U>
	constexpr vec(U v)
		: _v(v, v, v, v) {}

	explicit constexpr vec(Array<T, 4> array) : vec{array[0], array[1], array[2], array[3]} {}

	template<concepts::arithmetic_scalar U>
	[[nodiscard]] operator vec<U, 4>() const {
		return vec<U, 4>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z), static_cast<U>(w));
	}

	template<uint64 I>
	float get() const {
		if constexpr (I == 1) {
			return y;
		} else if constexpr (I == 2) {
			return z;
		} else if constexpr (I == 3) {
			return w;
		}
		return x;
	}

#include "ops/vec_ops.inl"

	// DONT EDIT THIS FIELD LAYOUT
	union {
		Array<T, 4> _v;
#include "ops/swizzle4.inl"
	};
};

template<typename T, uint64 N>
[[binop("SUB")]] vec<T, N> operator-(T, vec<T, N>);

template<typename T, uint64 N>
[[binop("ADD")]] vec<T, N> operator+(T, vec<T, N>);

template<typename T, uint64 N>
[[binop("MUL")]] vec<T, N> operator*(T, vec<T, N>);

template<typename T, uint64 N>
[[binop("DIV")]] vec<T, N> operator/(T, vec<T, N>);

template<typename... T>
auto make_vector(const T&... ts) {
	return vec<typename element_of<T...>::type, sum_dim_v<T...>>(ts...);
}

// Type helper template for N-dimensional vectors
template<typename T, uint64 N>
struct vecNt { using type = vec<T, N>; };
template<typename T>
struct vecNt<T, 1> { using type = T; };

// Convenience aliases
using float2 = vec<float, 2>;
using float3 = vec<float, 3>;
using float4 = vec<float, 4>;
template<uint64 N>
using floatN = typename vecNt<float, N>::type;

// using double2 = vec<double, 2>;
// using double3 = vec<double, 3>;
// using double4 = vec<double, 4>;
// template<uint64 N>
// using doubleN = typename vecNt<double, N>::type;

using int2 = vec<int32, 2>;
using int3 = vec<int32, 3>;
using int4 = vec<int32, 4>;
template<uint64 N>
using intN = typename vecNt<int32, N>::type;

using uint2 = vec<uint32, 2>;
using uint3 = vec<uint32, 3>;
using uint4 = vec<uint32, 4>;
template<uint64 N>
using uintN = typename vecNt<uint32, N>::type;

using half2 = vec<half, 2>;
using half3 = vec<half, 3>;
using half4 = vec<half, 4>;
template<uint64 N>
using halfN = typename vecNt<half, N>::type;

using bool2 = vec<bool, 2>;
using bool3 = vec<bool, 3>;
using bool4 = vec<bool, 4>;
template<uint64 N>
using boolN = typename vecNt<bool, N>::type;