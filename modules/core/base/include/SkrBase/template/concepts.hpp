#pragma once
#include <type_traits>

namespace skr::concepts
{
namespace detail
{
template <typename T>
std::remove_reference_t<T>& decl_lval(T&& t) {}
template <typename T>
std::remove_reference_t<T>&& decl_rval(T&& t) {}

template <class From, class To>
concept convertible_to =
#ifdef __clang__
    __is_convertible_to(From, To) &&
#else
    std::is_convertible_v<From, To> &&
#endif
    requires {
        static_cast<To>(std::declval<From>());
    };
} // namespace detail

template <typename T, typename... Args>
inline constexpr bool IsConstructible = requires(Args&&... args) {
    T{ std::forward<Args>(args)... };
};
template <typename T, typename... Args>
concept Constructible = IsConstructible<T, Args...>;

template <typename T>
inline constexpr bool IsDefaultConstructible = requires() {
    T{};
};
template <typename T>
concept DefaultConstructible = IsDefaultConstructible<T>;

template <typename T>
inline constexpr bool IsCopyConstructible = requires(T const& t) {
    T{t};
};
template <typename T>
concept CopyConstructible = IsCopyConstructible<T>;

template <typename T>
inline constexpr bool IsMoveConstructible = requires() {
    T{std::declval<T>()};
};
template <typename T>
concept MoveConstructible = IsMoveConstructible<T>;

template <typename T>
inline constexpr bool IsComparable = requires(T const& a, T const& b) {
    { a == b } -> detail::convertible_to<bool>;
    { a != b } -> detail::convertible_to<bool>;
    { b == a } -> detail::convertible_to<bool>;
    { b != a } -> detail::convertible_to<bool>;
};
template <typename T>
concept Comparable = IsComparable<T>;

template <typename T>
inline constexpr bool IsIterable = requires(T v) {
    v.begin();
    v.end();
};
template <typename T>
concept Iterable = IsIterable<T>;

template <typename T>
inline constexpr bool IsLinearIterable = requires(T v) {
    v.end() - v.begin();
    detail::decl_lval(v.begin()) += 1;
    detail::decl_lval(v.begin()) -= 1;
};

template <typename T>
concept LinearIterable = IsLinearIterable<T>;

template <typename T>
inline constexpr bool IsFunction = std::is_function_v<T>;

template <typename T>
concept Function = IsFunction<T>;

} // namespace skr::container::concepts