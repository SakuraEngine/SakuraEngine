#pragma once
#include <type_traits>
#include <concepts>

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
    T{ t };
};

template <typename T>
concept MoveConstructible = requires() {
    T{ std::declval<T>() };
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

// transparent concept
namespace skr
{
template <typename U, typename T, typename Hasher>
concept TransparentTo = requires(U&& u, T&& t, Hasher hasher) {
    hasher(std::forward<T>(u));
    {
        std::forward<T>(u) == std::forward<T>(t)
    } -> std::convertible_to<bool>;
};
template <typename U, typename T>
concept DecaySameAs = std::same_as<std::decay_t<U>, std::decay_t<T>>;
template <typename U, typename T, typename Hasher>
concept TransparentToOrSameAs = TransparentTo<U, T, Hasher> || DecaySameAs<U, T>;
} // namespace skr

// TODO. linear memory traits，从某个对象中提取如下信息：
//  1. 是否是连续内存
//  2. data()
//  3. size()
//  4. 元素类型

// TODO. iterator traits，从某个对象中提取如下信息：
//  1. 是否可迭代（range）
//  2. 元素类型
