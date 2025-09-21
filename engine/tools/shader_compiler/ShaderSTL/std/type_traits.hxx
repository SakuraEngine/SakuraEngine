#pragma once
#include "attributes.hxx"
#include "type_traits/concepts.hxx"

namespace detail
{
template <typename T>
struct vec_dim { static constexpr uint64 value = 1; };

template <typename T, uint64 N>
struct vec_dim<vec<T, N>> { static constexpr uint64 value = N; };

template <uint64 N>
struct vec_dim<matrix<N>> { static constexpr uint64 value = N * N; };

} // namespace detail

template <typename T, typename OtherType>
struct copy_dim { using type = T; };

template <typename T, typename ElemType, uint64 N>
struct copy_dim<T, vec<ElemType, N>> { using type = vec<T, N>; };

template <typename T>
[[ignore]] constexpr uint64 vec_dim_v = detail::vec_dim<cppsl::remove_cvref_t<T>>::value;

template <typename T, typename U>
[[ignore]] constexpr bool same_dim_v = (vec_dim_v<T> == vec_dim_v<U>);

template <uint64 dim, typename T, typename... Ts>
[[ignore]] consteval uint64 sum_dim()
{
    constexpr auto new_dim = dim + vec_dim_v<T>;
    if constexpr (sizeof...(Ts) == 0)
    {
        return new_dim;
    }
    else
    {
        return sum_dim<new_dim, Ts...>();
    }
}

template <typename... T>
static constexpr auto sum_dim_v = sum_dim<0ull, T...>();

namespace detail
{
template <uint64 N, typename T, typename... Ts>
struct element_of
{
    using type = typename element_of<N - 1, Ts...>::type;
    static_assert(__is_same_as(type, scalar_type<T>), "!!!");
};

template <typename T, typename... Ts>
struct element_of<1, T, Ts...>
{
    using type = scalar_type<T>;
};
} // namespace detail

template <typename... Ts>
struct element_of
{
    using type = typename detail::element_of<sizeof...(Ts), Ts...>::type;
};
template <typename T>
inline constexpr T identity;

template <concepts::arithmetic_scalar T>
inline constexpr T identity<T> = 1;
template <concepts::arithmetic_scalar T, uint32 N>
inline constexpr vec<T, N> identity<vec<T, N>> = vec<T, N>(1);
template <uint32 N>
inline constexpr matrix<N> identity<matrix<N>> = matrix<N>::identity();