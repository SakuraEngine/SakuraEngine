#pragma once
#include "SkrBase/config.h"

namespace skr::container
{
// char types
using skr_char  = char;
using skr_char8 = char8_t;

// literals
inline namespace literal
{
constexpr skr_char8 operator""_as_char(const unsigned long long value) noexcept
{
    return static_cast<skr_char8>(value);
}
} // namespace literal

} // namespace skr::container