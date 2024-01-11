#pragma once
#include "SkrRT/goap/config.hpp"
#include "SkrRT/goap/traits/base.hpp"

namespace skr::goap
{
template <typename T>
struct Compare;

template <typename T> requires(skr::concepts::IsComparable<T>)
struct Compare<T> {
    static bool Equal(const T& a, const T& b) SKR_NOEXCEPT { return a == b; }
    static bool NotEqual(const T& a, const T& b) SKR_NOEXCEPT { return a != b; }

    static bool Greater(const T& a, const T& b) SKR_NOEXCEPT { return a > b; }
    static bool GreaterEqual(const T& a, const T& b) SKR_NOEXCEPT { return a >= b; }
    static bool Less(const T& a, const T& b) SKR_NOEXCEPT { return a < b; }
    static bool LessEqual(const T& a, const T& b) SKR_NOEXCEPT { return a <= b; }
    static bool And(const T& a, const T& b) SKR_NOEXCEPT { return a & b; }
};

namespace concepts
{
template <typename T>
inline constexpr bool IsComparable = requires(const T& a, const T& b) {
    { Compare<T>::Equal(a, b) } -> std::convertible_to<bool>;
    { Compare<T>::NotEqual(a, b) } -> std::convertible_to<bool>;
};
} // namespace concepts
} // namespace skr::goap