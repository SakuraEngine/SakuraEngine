#pragma once
#include <type_traits>

namespace skr
{
namespace concepts
{
template <typename T>
concept HasStdSwap = requires(T a, T b) {
    std::swap(a, b);
};
template <typename T>
concept NoStdSwap = !HasStdSwap<T>;
} // namespace concepts

template <typename T>
struct Swap;

template <concepts::NoStdSwap T>
struct Swap<T> {
    inline static void call(T& a, T& b)
    {
        T tmp = std::move(a);
        a     = std::move(b);
        b     = std::move(tmp);
    }
};

template <concepts::HasStdSwap T>
struct Swap<T> {
    inline static void call(T& a, T& b)
    {
        std::swap(a, b);
    }
};
} // namespace skr