#pragma once
#include "builtins.hxx"

struct BufferFlags
{
    static constexpr uint32 ReadOnly = 1;
    static constexpr uint32 ReadWrite = 2;
};

struct TextureFlags
{
    static constexpr uint32 ReadOnly = 1;
    static constexpr uint32 ReadWrite = 2;
};

template <typename T, uint64 N>
struct vec;

template <uint64 X, uint64_t Y>
struct matrix;

template <typename Type, uint32 size>
struct Array;

template <typename Type, uint32 cache_flags>
struct SBuffer;

template <typename Type, uint32 cache_flags>
struct TBuffer;

template <typename T, uint32 cache_flags>
struct BBuffer;

template <typename Type>
struct ConstantBuffer;

struct Ray;
struct RaytracingAccelerationStructure;
struct CommittedHit;
struct TriangleHit;
struct ProceduralHit;
struct IndirectBuffer;

namespace detail
{
template <class T>
struct vec_or_matrix : public cppsl::false_type
{
    using scalar_type = T;
    static constexpr bool is_vec = false;
    static constexpr bool is_matrix = false;
};

template <class T, uint64 N>
struct vec_or_matrix<vec<T, N>> : public cppsl::true_type
{
    using scalar_type = T;
    static constexpr bool is_vec = true;
    static constexpr bool is_matrix = false;
};

template <uint64 X, uint64 Y>
struct vec_or_matrix<matrix<X, Y>> : public cppsl::true_type
{
    using scalar_type = float;
    static constexpr bool is_vec = false;
    static constexpr bool is_matrix = true;
};
#ifdef DEBUG
template <typename T>
struct is_char
{
    static constexpr bool value = false;
};
template <size_t n>
struct is_char<const char (&)[n]>
{
    static constexpr bool value = true;
};
#endif
} // namespace detail

template <typename T>
using scalar_type = typename ::detail::vec_or_matrix<cppsl::remove_cvref_t<T>>::scalar_type;

template <typename T>
static constexpr bool is_scalar_v = !::detail::vec_or_matrix<cppsl::remove_cvref_t<T>>::is_vec && !::detail::vec_or_matrix<cppsl::remove_cvref_t<T>>::is_matrix;

template <typename T>
static constexpr bool is_vec_v = ::detail::vec_or_matrix<cppsl::remove_cvref_t<T>>::is_vec;

template <typename T>
static constexpr bool is_matrix_v = ::detail::vec_or_matrix<cppsl::remove_cvref_t<T>>::is_matrix;

template <typename T>
static constexpr bool is_vec_or_matrix_v = ::detail::vec_or_matrix<cppsl::remove_cvref_t<T>>::value;

template <typename T>
inline constexpr bool is_array_v = false;

template <typename U, uint32 N>
inline constexpr bool is_array_v<Array<U, N>> = true;

template <typename T>
inline constexpr bool is_tbuffer_v = false;
template <typename U, uint32 N>
inline constexpr bool is_tbuffer_v<TBuffer<U, N>> = true;

template <typename T>
inline constexpr bool is_sbuffer_v = false;
template <typename U, uint32 N>
inline constexpr bool is_sbuffer_v<SBuffer<U, N>> = true;

template <typename T>
inline constexpr bool is_bbuffer_v = false;
template <typename T, uint32 N>
inline constexpr bool is_bbuffer_v<BBuffer<T, N>> = true;

template <typename T>
inline constexpr bool is_cbuffer_v = false;
template <typename U>
inline constexpr bool is_cbuffer_v<ConstantBuffer<U>> = true;

template <typename T>
inline constexpr bool is_buffer_v = is_tbuffer_v<T> || is_sbuffer_v<T> || is_bbuffer_v<T> || is_cbuffer_v<T>;

template <typename T>
inline constexpr bool is_rwbuffer_v = is_buffer_v<T> && (T::flags == BufferFlags::ReadWrite);

template <typename T>
static constexpr bool is_float_family_v = cppsl::is_same_v<scalar_type<T>, float> | cppsl::is_same_v<scalar_type<T>, double> | cppsl::is_same_v<scalar_type<T>, half>;

template <typename T>
static constexpr bool is_sint_family_v = cppsl::is_same_v<scalar_type<T>, int16> | cppsl::is_same_v<scalar_type<T>, int32> | cppsl::is_same_v<scalar_type<T>, int64>;

template <typename T>
static constexpr bool is_uint_family_v = cppsl::is_same_v<scalar_type<T>, uint16> | cppsl::is_same_v<scalar_type<T>, uint32> | cppsl::is_same_v<scalar_type<T>, uint64>;

template <typename T>
static constexpr bool is_int_family_v = is_sint_family_v<T> || is_uint_family_v<T>;

template <typename T>
static constexpr bool is_bool_family_v = cppsl::is_same_v<scalar_type<T>, bool>;

template <typename T>
static constexpr bool is_arithmetic_v = is_float_family_v<T> || is_bool_family_v<T> || is_int_family_v<T>;

template <typename T>
static constexpr bool is_signed_arithmetic_v = is_float_family_v<T> || is_sint_family_v<T>;

template <typename T>
static constexpr bool is_arithmetic_scalar_v = is_arithmetic_v<T> && !is_vec_v<T>;

template <typename T>
static constexpr bool is_arithmetic_vec_v = is_arithmetic_v<T> && is_vec_v<T>;

namespace concepts
{

template <typename T>
concept vec = is_vec_v<T>;

template <typename T>
concept non_vec = !is_vec_v<T>;

template <typename T>
concept matrix = is_matrix_v<T>;

template <typename T>
concept array = is_array_v<T>;

template <typename T>
concept tbuffer = is_tbuffer_v<T>;

template <typename T>
concept sbuffer = is_sbuffer_v<T>;

template <typename T>
concept bbuffer = is_bbuffer_v<T>;

template <typename T>
concept cbuffer = is_cbuffer_v<T>;

template <typename T>
concept buffer = is_buffer_v<T>;

template <typename T>
concept rw_buffer = is_buffer_v<T> && (T::flags == BufferFlags::ReadWrite);

template <typename T>
concept float_family = is_float_family_v<T>;

template <typename T>
concept float_vec_family = is_float_family_v<T> && is_vec_v<T>;

template <typename T>
concept bool_family = is_bool_family_v<T>;

template <typename T>
concept bool_vec_family = is_bool_family_v<T> && is_vec_v<T>;

template <typename T>
concept sint_family = is_sint_family_v<T>;

template <typename T>
concept sint_vec_family = is_sint_family_v<T> && is_vec_v<T>;

template <typename T>
concept uint_family = is_uint_family_v<T>;

template <typename T>
concept uint_vec_family = is_uint_family_v<T> && is_vec_v<T>;

template <typename T>
concept int_family = is_int_family_v<T>;

template <typename T>
concept int_vec_family = is_int_family_v<T> && is_vec_v<T>;

template <typename T>
concept arithmetic = is_arithmetic_v<T>;

template <typename T>
concept arithmetic_vec = is_arithmetic_v<T> && is_vec_v<T>;

template <typename T>
concept signed_arithmetic = is_signed_arithmetic_v<T>;

template <typename T>
concept arithmetic_scalar = is_arithmetic_scalar_v<T>;

template <typename T>
concept arithmetic_scalar_or_vec = is_arithmetic_scalar_v<T> || is_arithmetic_vec_v<T>;

template <typename T>
concept struct_type = __is_class(T) && !__is_union(T) && !__is_enum(T) && !is_vec_or_matrix_v<T> && !is_arithmetic_v<T>;

template <typename T>
concept primitive = is_arithmetic_v<T> || is_vec_or_matrix_v<T>;
#ifdef DEBUG
template <typename T>
concept string_literal = detail::is_char<T>::value;
#endif
} // namespace concepts

template <concepts::arithmetic_scalar T, uint32 cache_flag>
struct Image;

template <concepts::arithmetic_scalar T, uint32 cache_flag>
struct Volume;

template <typename T>
inline constexpr bool is_texture2d_v = false;
template <typename U, uint32 N>
inline constexpr bool is_texture2d_v<Image<U, N>> = true;

template <typename T>
inline constexpr bool is_texture3d_v = false;
template <typename U, uint32 N>
inline constexpr bool is_texture3d_v<Volume<U, N>> = true;

template <typename T>
inline constexpr bool is_texture_v = is_texture2d_v<T> || is_texture3d_v<T>;

namespace concepts
{

template <typename T>
concept texture2d = is_texture2d_v<T>;

template <typename T>
concept texture3d = is_texture3d_v<T>;

template <typename T>
concept texture = is_texture_v<T>;

} // namespace concepts