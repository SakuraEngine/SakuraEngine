#pragma once
#include <type_traits>
#include <concepts>

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