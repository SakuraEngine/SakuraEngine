#pragma once
#include <type_traits>

// is detected
namespace skr
{
namespace detail
{
template <typename, template <typename...> class Op, typename... T>
struct is_detected_impl : std::false_type {
};
template <template <typename...> class Op, typename... T>
struct is_detected_impl<std::void_t<Op<T...>>, Op, T...> : std::true_type {
};
} // namespace detail

template <template <typename...> class Op, typename... T>
using is_detected = detail::is_detected_impl<void, Op, T...>;
template <template <typename...> class Op, typename... T>
inline constexpr bool is_detected_v = is_detected<Op, T...>::value;
} // namespace skr