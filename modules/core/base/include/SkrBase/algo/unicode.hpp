#pragma once
#include "SkrBase/config.h"

namespace skr::algo
{
//==================> sequence <==================
// TODO. 从 c 串中读取一个 utf-8/utf-16 串单元
struct UTF8Seq {
    uint32_t  len;
    skr_char8 data[4];
};
struct UTF16Seq {
    uint32_t   len;
    skr_char16 data[2];
};

//==================> constants <==================
static constexpr char32_t kUtf16LeadingSurrogateHeader  = 0xD800;
static constexpr char32_t kUtf16TrailingSurrogateHeader = 0xDC00;
static constexpr char32_t kUtf16LeadingSurrogateMin     = 0xD800;
static constexpr char32_t kUtf16LeadingSurrogateMax     = 0xDBFF;
static constexpr char32_t kUtf16TrailingSurrogateMin    = 0xDC00;
static constexpr char32_t kUtf16TrailingSurrogateMax    = 0xDFFF;
static constexpr char32_t kUtf16SurrogateMask           = 0x03FF;
static constexpr char32_t kBMPMaxCodePoint              = 0xFFFF;

//==================> utf-8 <==================
// return maximum code point for given sequence length
constexpr skr_char32 utf8_maximum_code_point(uint64_t seq_len);
// seq_len: 0 => trailing mask, 1-4 => leading mask
constexpr skr_char8 utf8_mask(uint64_t seq_len);
// return: 0 => ch is trailing, 1-4 => sequence length (if ch is leading)
constexpr uint64_t utf8_seq_len(skr_char8 ch);
// return: 1-4 => sequence length
constexpr uint64_t utf8_seq_len(skr_char16 ch);
// return: 1-4 => sequence length
constexpr uint64_t utf8_seq_len(skr_char32 ch);

//==================> utf-16 <==================
// is ch a leading surrogate
constexpr bool utf16_is_leading_surrogate(skr_char16 ch);
// is ch a trailing surrogate
constexpr bool utf16_is_trailing_surrogate(skr_char16 ch);
// is ch a surrogate
constexpr bool utf16_is_surrogate(skr_char16 ch);
// return: 0 => ch is trailing, 1-2 => sequence length (if ch is leading)
constexpr uint64_t utf16_seq_len(skr_char8 ch);
// return: 1-2 => sequence length
constexpr uint64_t utf16_seq_len(skr_char16 ch);
// return: 1-2 => sequence length
constexpr uint64_t utf16_seq_len(skr_char32 ch);

//==================> convert <==================
constexpr UTF16Seq   utf8_to_utf16(UTF8Seq seq);
constexpr skr_char32 utf8_to_utf32(UTF8Seq seq);
constexpr UTF8Seq    utf16_to_utf8(UTF16Seq seq);
constexpr skr_char32 utf16_to_utf32(UTF16Seq seq);
constexpr UTF8Seq    utf32_to_utf8(skr_char32 ch);
constexpr UTF16Seq   utf32_to_utf16(skr_char32 ch);

} // namespace skr::algo