#pragma once
#include <type_traits>
namespace skr::container::concepts
{
namespace detail
{
template <typename T>
std::remove_reference_t<T>& decl_lval(T&& t) {}
template <typename T>
std::remove_reference_t<T>&& decl_rval(T&& t) {}
} // namespace detail
template <typename T, typename... Args>
concept Constructible = requires(Args&&... args) {
    T{ std::forward<Args>(args)... };
};

template <typename T>
concept DefaultConstructible = requires() {
    T{};
};

template <typename T>
concept CopyConstructible = requires(T const& t) {
    T{t};
};

template <typename T>
concept MoveConstructible = requires() {
    T{std::declval<T>()};
};

template <typename T>
concept Comparable = requires(T const& a) {
    a.operator==(a);
    a.operator!=(a);
};

template <typename T>
concept Iterable = requires(T v) {
    v.begin();
    v.end();
};

template <typename T>
concept LinearIterable = requires(T v) {
    v.end() - v.begin();
    detail::decl_lval(v.begin()) += 1;
    detail::decl_lval(v.begin()) -= 1;
};

template <typename T>
concept Function = std::is_function_v<T>;
} // namespace skr::container::concepts