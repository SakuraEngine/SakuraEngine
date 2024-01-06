#pragma once
#include "SkrRT/goap/config.hpp"

namespace skr::goap
{
template <size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }
    char value[N];
};
}