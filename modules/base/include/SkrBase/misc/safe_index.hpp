#pragma once
#include "SkrBase/misc/integer_tools.hpp"
#include <concepts>

namespace skr
{
template <std::integral T>
struct SafeIndex {
    inline static constexpr T npos = npos_of<T>;

    // ctor & copy & move & assign & move_assign
    inline constexpr SafeIndex(T value) noexcept
        : _value(value)
    {
    }
    inline constexpr SafeIndex() noexcept                            = default;
    inline constexpr SafeIndex(const SafeIndex&) noexcept            = default;
    inline constexpr SafeIndex(SafeIndex&&) noexcept                 = default;
    inline constexpr SafeIndex& operator=(const SafeIndex&) noexcept = default;
    inline constexpr SafeIndex& operator=(SafeIndex&&) noexcept      = default;

    // set
    inline constexpr SafeIndex& operator=(T value) noexcept
    {
        _value = value;
        return *this;
    }
    inline constexpr void reset() noexcept { _value = npos; }

    // validate
    inline constexpr bool is_valid() const noexcept { return _value != npos; }

    // value
    inline constexpr T    value() const noexcept { return _value; }
    inline constexpr bool value_or(T fallback_value) const noexcept { return is_valid() ? _value : fallback_value; }

    // cast
    inline constexpr operator T() const noexcept { return _value; }

private:
    T _value = npos;
};
} // namespace skr