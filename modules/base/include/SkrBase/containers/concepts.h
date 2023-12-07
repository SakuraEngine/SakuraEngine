#pragma once
#include <type_traits>
namespace skr::container::concepts
{
template <typename T, typename... Args>
concept Constructible = requires(Args&&... args) {
    T{ std::forward<Args>(args)... };
};

template <typename T>
concept Iterable = requires(T v) {
    v.begin();
    v.end();
};

template <typename T>
concept Function = std::is_function_v<T>;
} // namespace skr::container::concepts