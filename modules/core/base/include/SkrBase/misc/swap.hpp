#pragma once
#include <type_traits>

namespace skr
{
template <typename T>
struct Swap {
};

namespace concepts
{
template <typename T>
concept HasStdSwap = requires(T a, T b) {
    std::swap(a, b);
};
template <typename T>
concept NoStdSwap = !HasStdSwap<T>;
template <typename T>
concept NoStdSwapAndSwapble =
    NoStdSwap<T> &&
    std::is_constructible_v<T> &&
    requires(T a, T b) {
        a = std::move(b);
    };

template <typename T>
concept HasSwap = requires(T a, T b) {
    Swap<T>::call(a, b);
};

} // namespace concepts

template <concepts::NoStdSwapAndSwapble T>
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