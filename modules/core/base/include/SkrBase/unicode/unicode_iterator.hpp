#pragma once
#include "SkrBase/unicode/unicode_algo.hpp"

namespace skr
{
template <typename TS, bool kConst>
struct UTF8Cursor {
    using DataType = std::conditional_t<kConst, const skr_char8, skr_char8>;
    using SizeType = TS;

private:
    DataType* _data      = nullptr;
    SizeType  _size      = 0;
    SizeType  _seq_index = 0;
    SizeType  _seq_len   = 0;
};

template <typename TS, bool kConst>
struct UTF16Cursor {
    using DataType = std::conditional_t<kConst, const skr_char16, skr_char16>;
    using SizeType = TS;

private:
    DataType* _data      = nullptr;
    SizeType  _size      = 0;
    SizeType  _seq_index = 0;
    SizeType  _seq_len   = 0;
};

} // namespace skr