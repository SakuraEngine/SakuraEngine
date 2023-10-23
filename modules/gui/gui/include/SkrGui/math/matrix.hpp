#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
// row major matrix
struct SKR_ALIGNAS(16) Matrix4 {
    // factory

    // info

    // compare

    // arithmetic ops

    // transform

    // calculations

    // visitor
    template <bool bConst>
    struct _RowVisitor {
        using Pointer   = ::std::conditional_t<bConst, const float*, float*>;
        using Reference = ::std::conditional_t<bConst, float, float&>;

        Pointer _row;
        inline constexpr _RowVisitor(Pointer row) SKR_NOEXCEPT
            : _row(row)
        {
        }
        inline constexpr Reference operator[](size_t n) SKR_NOEXCEPT
        {
            SKR_ASSERT(n > 0 && n < 4 && "Matrix4 row index out of range");
            return _row[n];
        }
    };
    inline constexpr _RowVisitor<false> operator[](size_t n) SKR_NOEXCEPT
    {
        SKR_ASSERT(n > 0 && n < 4 && "Matrix4 row index out of range");
        return _RowVisitor<false>(_m + n * 4);
    }
    inline constexpr const _RowVisitor<true> operator[](size_t n) const SKR_NOEXCEPT
    {
        SKR_ASSERT(n > 0 && n < 4 && "Matrix4 row index out of range");
        return _RowVisitor<true>(_m + n * 4);
    }

    float _m[16];
};
} // namespace skr::gui