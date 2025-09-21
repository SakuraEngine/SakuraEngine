#pragma once
#include "./../attributes.hxx"
#include "./../type_traits.hxx"
#include "./../types/vec.hxx"

template<concepts::arithmetic_vec T>
[[callop("ASFLOAT")]] extern vec<float, vec_dim_v<T>> asfloat(T v);
template<concepts::arithmetic_scalar T>
[[callop("ASFLOAT")]] extern float asfloat(T v);

template<concepts::arithmetic_vec T>
[[callop("ASINT")]] extern vec<int, vec_dim_v<T>> asint(T v);
template<concepts::arithmetic_scalar T>
[[callop("ASINT")]] extern int asint(T v);

template<concepts::arithmetic_vec T>
[[callop("ASUINT")]] extern vec<uint, vec_dim_v<T>> asuint(T v);
template<concepts::arithmetic_scalar T>
[[callop("ASUINT")]] extern uint asuint(T v);

template<concepts::arithmetic T, concepts::arithmetic U>
	requires(sizeof(T) == sizeof(U))
extern T bit_cast(U v)
{
	if constexpr (is_float_family_v<T>)
		return asfloat(v);
	else if constexpr (is_sint_family_v<T>)
		return asint(v);
	else if constexpr (is_uint_family_v<T>)
		return asuint(v);
	else
		static_assert(cppsl::is_same_v<T, void>, "Unsupported bit_cast type!");
}

template<concepts::bool_family T>
[[callop("ALL")]] extern bool all(const T& x);

template<concepts::bool_family T>
[[callop("ANY")]] extern bool any(const T& x);

template<concepts::primitive T, concepts::bool_family B>
	requires(same_dim_v<T, B> || is_scalar_v<B>)
[[callop("SELECT")]] extern T select(const T& false_v, const T& true_v, const B& bool_v);

template<concepts::primitive T, concepts::bool_family B>
	requires(vec_dim_v<T> == vec_dim_v<B> || vec_dim_v<B> == 1)
inline T ite(B bool_v, T true_v, T false_v) {
	return select(false_v, true_v, bool_v);
}

template<concepts::arithmetic T, concepts::arithmetic A, concepts::arithmetic B>
	requires((same_dim_v<T, B> || is_scalar_v<B>) && (same_dim_v<T, A> || is_scalar_v<A>))
[[callop("CLAMP")]] extern T clamp(const T& v, const A& min_v, const B& max_v);

template<concepts::float_family T, concepts::float_family B>
	requires(same_dim_v<T, B> || is_scalar_v<B>)
[[callop("LERP")]] extern T lerp(const T& left_v, const T& right_v, const B& step);

template<concepts::float_family T, concepts::float_family B>
	requires(same_dim_v<T, B> || is_scalar_v<B>)
[[callop("SMOOTHSTEP")]] extern T smoothstep(const T& left_v, const B& right_v, const B& step);

template<concepts::float_family T, concepts::float_family B>
	requires(same_dim_v<T, B> || is_scalar_v<B>)
[[callop("STEP")]] extern T step(const T& left_v, const B& right_v);

template<concepts::float_family T>
[[callop("SATURATE")]] extern T saturate(const T& v);

template<concepts::signed_arithmetic T>
[[callop("ABS")]] extern T abs(const T& v);

template<concepts::arithmetic T, concepts::arithmetic U>
[[callop("MIN")]] extern T min(const T& a, U b)
	requires(vec_dim_v<T> == vec_dim_v<U>);

template<concepts::arithmetic T, concepts::arithmetic U>
[[callop("MAX")]] extern T max(const T& v, U b)
	requires(vec_dim_v<T> == vec_dim_v<U>);

template<concepts::uint_family T>
[[callop("CLZ")]] extern T clz(const T& v);

template<concepts::uint_family T>
[[callop("CTZ")]] extern T ctz(const T& v);

template<concepts::uint_family T>
[[callop("POPCOUNT")]] extern T popcount(const T& v);

template<concepts::float_vec_family T>
[[callop("ISINF")]] extern vec<bool, vec_dim_v<T>> is_inf(const T& v);

template<concepts::float_vec_family T>
[[callop("ISNAN")]] extern vec<bool, vec_dim_v<T>> is_nan(const T& v);

[[callop("ISINF")]] extern bool is_inf(float v);

[[callop("ISNAN")]] extern bool is_nan(float v);

template<concepts::float_family T>
[[callop("ACOS")]] extern T acos(const T& v);

template<concepts::float_family T>
[[callop("ACOSH")]] extern T acosh(const T& v);

template<concepts::float_family T>
[[callop("ASIN")]] extern T asin(const T& v);

template<concepts::float_family T>
[[callop("ASINH")]] extern T asinh(const T& v);

template<concepts::float_family T>
[[callop("ATAN")]] extern T atan(const T& v);

template<concepts::float_family T, concepts::float_family U>
[[callop("ATAN2")]] extern T atan2(const T& a, const U& b)
	requires(vec_dim_v<T> == vec_dim_v<U>);

template<concepts::float_family T>
[[callop("ATANH")]] extern T atanh(const T& v);

template<concepts::float_family T>
[[callop("COS")]] extern T cos(const T& v);

template<concepts::float_family T>
[[callop("COSH")]] extern T cosh(const T& v);

template<concepts::float_family T>
[[callop("SIN")]] extern T sin(const T& v);

template<concepts::float_family T>
[[callop("SINH")]] extern T sinh(const T& v);

template<concepts::float_family T>
[[callop("TAN")]] extern T tan(const T& v);

template<concepts::float_family T>
[[callop("TANH")]] extern T tanh(const T& v);

template<concepts::float_family T>
[[callop("EXP")]] extern T exp(const T& v);

template<concepts::float_family T>
[[callop("EXP2")]] extern T exp2(const T& v);

template<concepts::float_family T>
[[callop("EXP10")]] extern T exp10(const T& v);

template<concepts::float_family T>
[[callop("LOG")]] extern T log(const T& v);

template<concepts::float_family T>
[[callop("LOG2")]] extern T log2(const T& v);

template<concepts::float_family T>
[[callop("LOG10")]] extern T log10(const T& v);

template<concepts::arithmetic T, concepts::arithmetic B>
	requires(same_dim_v<T, B> || is_scalar_v<B>)
[[callop("POW")]] extern T pow(const T& base, const B& rate);

template<concepts::float_family T>
[[callop("SQRT")]] extern T sqrt(const T& v);

template<concepts::float_family T>
constexpr T sqr(const T& v) {
	return v * v;
}

template<concepts::float_family T>
constexpr T pow2(const T& v) {
	return v * v;
}

template<concepts::float_family T>
constexpr T pow4(T x) {
	return sqr(sqr(x));
}

template<concepts::float_family T>
constexpr T pow5(T x) {
	return pow4(x) * x;
}

template<concepts::float_family T>
constexpr T pow6(T x) {
	T x2 = sqr(x);
	return sqr(x2) * x2;
}

template<concepts::float_family T>
[[callop("RSQRT")]] extern T rsqrt(const T& v);

template<concepts::float_family T>
[[callop("CEIL")]] extern T ceil(const T& v);

template<concepts::float_family T>
[[callop("FLOOR")]] extern T floor(const T& v);

template<concepts::float_family T>
[[callop("FRACT")]] extern T fract(const T& v);

template<concepts::float_family T>
[[callop("TRUNC")]] extern T trunc(const T& v);

template<concepts::float_family T>
[[callop("ROUND")]] extern T round(const T& v);

template<concepts::float_family T>
[[callop("FMA")]] extern T fma(const T& a, const T& b, const T& c);

template<concepts::float_family T>
[[callop("COPYSIGN")]] extern T copysign(const T& a, T b);

template<concepts::float_vec_family T>
[[callop("CROSS")]] extern T cross(const T& a, const T& b);

[[callop("FACEFORWARD")]] extern float3 faceforward(const float3& a, const float3& b, const float3& c);
[[callop("FACEFORWARD")]] extern half3 faceforward(const half3& a, const half3& b, const half3& c);

[[callop("REFLECT")]] extern float3 reflect(const float3& i, const float3& n);
[[callop("REFLECT")]] extern half3 reflect(const half3& i, const half3& n);

template<concepts::float_vec_family T>
[[callop("DOT")]] extern scalar_type<T> dot(const T& a, const T& b);

template<concepts::float_vec_family T>
[[callop("LENGTH")]] extern scalar_type<T> length(const T& v);

template<concepts::float_vec_family T>
scalar_type<T> distance(const T& a, const T& b) { return length(a - b); }

template<concepts::float_vec_family T>
[[callop("LENGTH_SQUARED")]] extern scalar_type<T> length_squared(const T& v);

template<concepts::float_vec_family T>
[[callop("NORMALIZE")]] extern T normalize(const T& v);

template<concepts::uint_family T>
[[callop("REVERSEBITS")]] extern T reversebits(const T& v);

template<concepts::arithmetic_vec T>
scalar_type<T> reduce_sum(const T& v)
{
	if constexpr (vec_dim_v<T> == 1)
		return v;
	else if constexpr (vec_dim_v<T> == 2)
		return v.x + v.y;
	else if constexpr (vec_dim_v<T> == 3)
		return v.x + v.y + v.z;
	else if constexpr (vec_dim_v<T> == 4)
		return v.x + v.y + v.z + v.w;
}

template<concepts::arithmetic_vec T>
scalar_type<T> reduce_product(const T& v)
{
	if constexpr (vec_dim_v<T> == 1)
		return v.x;
	else if constexpr (vec_dim_v<T> == 2)
		return v.x * v.y;
	else if constexpr (vec_dim_v<T> == 3)
		return v.x * v.y * v.z;
	else if constexpr (vec_dim_v<T> == 4)
		return v.x * v.y * v.z * v.w;
}

template<concepts::arithmetic_vec T>
scalar_type<T> reduce_min(const T& v)
{
	if constexpr (vec_dim_v<T> == 1)
		return v.x;
	else if constexpr (vec_dim_v<T> == 2)
		return min(v.x, v.y);
	else if constexpr (vec_dim_v<T> == 3)
		return min(min(v.x, v.y), v.z);
	else if constexpr (vec_dim_v<T> == 4)
		return min(min(min(v.x, v.y), v.z), v.w);
}

template<concepts::arithmetic_vec T>
scalar_type<T> reduce_max(const T& v)
{
	if constexpr (vec_dim_v<T> == 1)
		return v.x;
	else if constexpr (vec_dim_v<T> == 2)
		return max(v.x, v.y);
	else if constexpr (vec_dim_v<T> == 3)
		return max(max(v.x, v.y), v.z);
	else if constexpr (vec_dim_v<T> == 4)
		return max(max(max(v.x, v.y), v.z), v.w);
}

template<concepts::matrix T>
[[callop("DETERMINANT")]] extern T determinant(const T& v);

template<concepts::matrix T>
[[callop("TRANSPOSE")]] extern T transpose(const T& v);

template<concepts::matrix T>
[[callop("INVERSE")]] extern T inverse(const T& v);

template<concepts::float_family T>
[[callop("DDX")]] extern T ddx();

template<concepts::float_family T>
[[callop("DDY")]] extern T ddy();

template<concepts::float_family T>
auto sign(T v) {
	using Type = copy_dim<int, T>::type;
	using MaskType = copy_dim<uint, T>::type;
	auto low = select(Type(1), Type(0), (bit_cast<MaskType>(v) & MaskType(0x7fffffffu)) == MaskType(0));
	auto high = select(Type(2), Type(0), (bit_cast<MaskType>(v) & MaskType(0x80000000u)) == MaskType(0));
	return low - high;
}

template<concepts::float_family T>
constexpr T rcp(T t) {
	return T(1.f) / t;
}

template <concepts::float_family T>
[[callop("FMOD")]] constexpr T fmod(const T& x, const T& y)
{
    return x - y * trunc(x / y);
}
template <concepts::float_family T>
[[callop("MODF")]] constexpr T modf(const T& x, T& int_part)
{
    int_part = trunc(x);
    return x - int_part;
}

template<concepts::float_family T>
constexpr T inverse_smoothstep(T y) { return T(0.5f) - sin(asin(T(1.0f) - T(2.0f) * y) / T(3.0f)); }
