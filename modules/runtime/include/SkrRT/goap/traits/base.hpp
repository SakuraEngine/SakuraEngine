#pragma once
#include "SkrRT/goap/config.hpp"

namespace skr::goap
{
template <size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char8_t (&str)[N]) { std::copy_n(str, N, __zzValue); }
    constexpr skr::StringView view() const { return skr::StringView(__zzValue); }

    char8_t __zzValue[N];
};
}