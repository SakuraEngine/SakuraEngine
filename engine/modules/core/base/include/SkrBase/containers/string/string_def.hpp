#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/vector/vector_def.hpp"
#include "SkrBase/misc/integer_tools.hpp"
#include <type_traits>

namespace skr::container
{
template <typename T, typename TSize, bool kConst>
using StringDataRef = VectorDataRef<T, TSize, kConst>;

template <typename TStr>
struct StringPartitionResult
{
    TStr left  = {};
    TStr mid   = {};
    TStr right = {};
};

enum class EStringParseStatus
{
    Success,
    OutOfRange,
    Invalid,
};
template <typename T, typename TStringView>
struct StringParseResult
{
    using ViewType = TStringView;

    T                  value;
    EStringParseStatus parse_status;
    ViewType           parsed;
    ViewType           rest;

    inline bool is_success() const noexcept { return parse_status == EStringParseStatus::Success; }
    inline bool is_full_parsed() const noexcept { return is_success() && rest.is_empty(); }
    inline bool is_out_of_range() const noexcept { return parse_status == EStringParseStatus::OutOfRange; }
    inline bool is_invalid() const noexcept { return parse_status == EStringParseStatus::Invalid; }

    inline T value_or(T v) const noexcept
    {
        if (is_success())
        {
            return value;
        }
        else
        {
            return v;
        }
    }

    inline operator bool() const noexcept { return is_success(); }
};
} // namespace skr::container