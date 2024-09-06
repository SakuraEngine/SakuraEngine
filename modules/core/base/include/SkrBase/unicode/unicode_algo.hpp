#pragma once
#include "SkrBase/config.h"
#include <type_traits>
#include <array>

namespace skr
{
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

//==================> sequence <==================
struct UTF8Seq {
    // ctor
    constexpr UTF8Seq();
    constexpr UTF8Seq(const skr_char8* c_str, uint32_t len);
    constexpr UTF8Seq(skr_char8 ch_0);
    constexpr UTF8Seq(skr_char8 ch_0, skr_char8 ch_1);
    constexpr UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2);
    constexpr UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2, skr_char8 ch_3);

    // validate
    constexpr bool is_valid() const;
    constexpr      operator bool() const;

    // visitor
    constexpr skr_char8& at(uint32_t index);
    constexpr skr_char8  at(uint32_t index) const;
    constexpr skr_char8& operator[](uint32_t index);
    constexpr skr_char8  operator[](uint32_t index) const;

public:
    uint32_t  len     = 0;
    skr_char8 data[4] = {};
};
struct UTF16Seq {
    // ctor
    constexpr UTF16Seq();
    constexpr UTF16Seq(const skr_char16* c_str, uint32_t len);
    constexpr UTF16Seq(skr_char16 ch_0);
    constexpr UTF16Seq(skr_char16 ch_0, skr_char16 ch_1);

    // validate
    constexpr bool is_valid() const;
    constexpr      operator bool() const;

    // visitor
    constexpr skr_char16& at(uint32_t index);
    constexpr skr_char16  at(uint32_t index) const;
    constexpr skr_char16& operator[](uint32_t index);
    constexpr skr_char16  operator[](uint32_t index) const;

public:
    uint32_t   len     = 0;
    skr_char16 data[2] = {};
};

//==================> convert <==================
constexpr UTF16Seq   utf8_to_utf16(UTF8Seq seq);
constexpr skr_char32 utf8_to_utf32(UTF8Seq seq);
constexpr UTF8Seq    utf16_to_utf8(UTF16Seq seq);
constexpr skr_char32 utf16_to_utf32(UTF16Seq seq);
constexpr UTF8Seq    utf32_to_utf8(skr_char32 ch);
constexpr UTF16Seq   utf32_to_utf16(skr_char32 ch);

//==================> parse code seq <==================
constexpr UTF8Seq  utf8_parse_seq(const skr_char8* seq, uint64_t index, uint64_t size, uint64_t& seq_index);
constexpr UTF16Seq utf16_parse_seq(const skr_char16* seq, uint64_t index, uint64_t size, uint64_t& seq_index);
} // namespace skr

namespace skr
{
//==================> utf-8 <==================
inline constexpr skr_char32 utf8_maximum_code_point(uint64_t seq_len)
{
    constexpr std::array<char32_t, 4> maximum_codepoints{
        static_cast<char32_t>(0x0000'007F),
        static_cast<char32_t>(0x0000'07FF),
        static_cast<char32_t>(0x0000'FFFF),
        static_cast<char32_t>(0x001F'FFFF),
    };
    return maximum_codepoints.at(seq_len - 1);
}
inline constexpr skr_char8 utf8_mask(uint64_t seq_len)
{
    constexpr std::array<char8_t, 5> masks{
        static_cast<skr_char8>(0b0011'1111),
        static_cast<skr_char8>(0b0111'1111),
        static_cast<skr_char8>(0b0001'1111),
        static_cast<skr_char8>(0b0000'1111),
        static_cast<skr_char8>(0b0000'0111),
    };
    return masks.at(seq_len);
}
inline constexpr uint64_t utf8_seq_len(skr_char8 ch)
{
    if (ch == 0)
    {
        return 1;
    }
    else
    {
        constexpr skr_char8 mask = static_cast<skr_char8>(0b1000'0000);
        uint32_t            size = 0;
        uint8_t             v    = ch;
        while (v & mask)
        {
            ++size;
            v <<= 1;
        }

        // 10xx'xxxx => trailing
        // 0xxx'xxxx => 1-byte
        // 110x'xxxx => 2-byte
        // 1110'xxxx => 3-byte
        // 1111'0xxx => 4-byte
        return size > 1 ? size : 1 - size;
    }
}
inline constexpr uint64_t utf8_seq_len(skr_char16 ch)
{
    if (ch <= utf8_maximum_code_point(1))
    { // 1-byte BPM codepoint, U+0000-U+007F
        return 1;
    }
    if (ch <= utf8_maximum_code_point(2))
    { // 2-byte BPM codepoint, U+0080-U+07FF
        return 2;
    }
    if (utf16_is_surrogate(ch))
    { // surrogate, means 4-byte SPM codepoint, U+D800-U+DFFF
        return 4;
    }

    // 3-byte BPM codepoint, U+0800-U+FFFF
    return 3;
}
inline constexpr uint64_t utf8_seq_len(skr_char32 ch)
{
    if (ch <= utf8_maximum_code_point(1))
    {
        return 1;
    }
    if (ch <= utf8_maximum_code_point(2))
    {
        return 2;
    }
    if (ch <= utf8_maximum_code_point(3))
    {
        return 3;
    }
    return 4;
}

//==================> utf-16 <==================
inline constexpr bool utf16_is_leading_surrogate(skr_char16 ch)
{
    return ch >= kUtf16LeadingSurrogateMin && ch <= kUtf16LeadingSurrogateMax;
}
inline constexpr bool utf16_is_trailing_surrogate(skr_char16 ch)
{
    return ch >= kUtf16TrailingSurrogateMin && ch <= kUtf16TrailingSurrogateMax;
}
inline constexpr bool utf16_is_surrogate(skr_char16 ch)
{
    return utf16_is_leading_surrogate(ch) || utf16_is_trailing_surrogate(ch);
}
inline constexpr uint64_t utf16_seq_len(skr_char8 ch)
{
    // 0 => utf-8 trailing unit
    // 1-3 => BPM plane code point
    // 4 => SPM plane code point
    uint64_t utf8_len = utf8_seq_len(ch);
    return utf8_len == 0 ? 0 :
           utf8_len <= 3 ? 1 :
                           2;
}
inline constexpr uint64_t utf16_seq_len(skr_char16 ch)
{
    return utf16_is_leading_surrogate(ch)  ? 2 :
           utf16_is_trailing_surrogate(ch) ? 0 :
                                             1;
}
inline constexpr uint64_t utf16_seq_len(skr_char32 ch)
{
    return ch <= kBMPMaxCodePoint ? 1 : 2;
}

//==================> utf-8 sequence <==================
inline constexpr UTF8Seq::UTF8Seq() = default;
inline constexpr UTF8Seq::UTF8Seq(const skr_char8* c_str, uint32_t len)
    : len(len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        data[i] = c_str[i];
    }
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0)
    : len(1)
    , data{ ch_0, 0, 0, 0 }
{
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0, skr_char8 ch_1)
    : len(2)
    , data{ ch_0, ch_1, 0, 0 }
{
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2)
    : len(3)
    , data{ ch_0, ch_1, ch_2, 0 }
{
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2, skr_char8 ch_3)
    : len(4)
    , data{ ch_0, ch_1, ch_2, ch_3 }
{
}

// validate
inline constexpr bool UTF8Seq::is_valid() const
{
    return len > 0;
}
inline constexpr UTF8Seq::operator bool() const
{
    return is_valid();
}

// visitor
inline constexpr skr_char8& UTF8Seq::at(uint32_t index)
{
    return data[index];
}
inline constexpr skr_char8 UTF8Seq::at(uint32_t index) const
{
    return data[index];
}
inline constexpr skr_char8& UTF8Seq::operator[](uint32_t index)
{
    return data[index];
}
inline constexpr skr_char8 UTF8Seq::operator[](uint32_t index) const
{
    return data[index];
}

//==================> utf-16 sequence <==================
inline constexpr UTF16Seq::UTF16Seq() = default;
inline constexpr UTF16Seq::UTF16Seq(const skr_char16* c_str, uint32_t len)
    : len(len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        data[i] = c_str[i];
    }
}
inline constexpr UTF16Seq::UTF16Seq(skr_char16 ch_0)
    : len(1)
    , data{ ch_0, 0 }
{
}
inline constexpr UTF16Seq::UTF16Seq(skr_char16 ch_0, skr_char16 ch_1)
    : len(2)
    , data{ ch_0, ch_1 }
{
}

// validate
inline constexpr bool UTF16Seq::is_valid() const
{
    return len > 0;
}
inline constexpr UTF16Seq::operator bool() const
{
    return is_valid();
}

// visitor
inline constexpr skr_char16& UTF16Seq::at(uint32_t index)
{
    return data[index];
}
inline constexpr skr_char16 UTF16Seq::at(uint32_t index) const
{
    return data[index];
}
inline constexpr skr_char16& UTF16Seq::operator[](uint32_t index)
{
    return data[index];
}
inline constexpr skr_char16 UTF16Seq::operator[](uint32_t index) const
{
    return data[index];
}

//==================> convert <==================
inline constexpr UTF16Seq utf8_to_utf16(UTF8Seq seq)
{
    return utf32_to_utf16(utf8_to_utf32(seq));
}
inline constexpr skr_char32 utf8_to_utf32(UTF8Seq seq)
{
    if (seq)
    {
        const auto         lead_mask     = utf8_mask(seq.len);
        constexpr auto     trailing_mask = utf8_mask(0);
        constexpr uint64_t trailing_bits = 6;

        const auto ch_0   = seq[0];
        char32_t   result = static_cast<char32_t>(ch_0 & lead_mask);
        for (uint32_t i = 1; i < seq.len; ++i)
        {
            const auto ch_i = seq[i];
            result <<= trailing_bits;
            result |= ch_i & trailing_mask;
        }
        return result;
    }
    else
    {
        return 0;
    }
}
inline constexpr UTF8Seq utf16_to_utf8(UTF16Seq seq)
{
    return utf32_to_utf8(utf16_to_utf32(seq));
}
inline constexpr skr_char32 utf16_to_utf32(UTF16Seq seq)
{
    //                  1         0
    //         9876543210 9876543210
    //         |||||||||| ||||||||||
    // [110110]9876543210 |||||||||| high surrogate
    //            [110111]9876543210 low  surrogate
    return seq.len == 1 ? seq[0] :
                          ((seq[0] & kUtf16SurrogateMask) << 10) + (seq[1] & kUtf16SurrogateMask);
}
inline constexpr UTF8Seq utf32_to_utf8(skr_char32 ch)
{
    if (ch <= utf8_maximum_code_point(1))
    {
        return { static_cast<skr_char8>(ch) };
    }
    if (ch <= utf8_maximum_code_point(2))
    {
        return {
            static_cast<skr_char8>((ch >> 6) | 0xc0),
            static_cast<skr_char8>((ch & utf8_mask(0)) | 0x80),
        };
    }
    if (ch <= utf8_maximum_code_point(3))
    {
        return {
            static_cast<skr_char8>((ch >> 12) | 0xe0),
            static_cast<skr_char8>(((ch >> 6) & utf8_mask(0)) | 0x80),
            static_cast<skr_char8>((ch & utf8_mask(0)) | 0x80)
        };
    }
    return {
        static_cast<skr_char8>((ch >> 18) | 0xf0),
        static_cast<skr_char8>(((ch >> 12) & utf8_mask(0)) | 0x80),
        static_cast<skr_char8>(((ch >> 6) & utf8_mask(0)) | 0x80),
        static_cast<skr_char8>((ch & utf8_mask(0)) | 0x80)
    };
}
inline constexpr UTF16Seq utf32_to_utf16(skr_char32 ch)
{
    return ch <= kBMPMaxCodePoint ?
           UTF16Seq{ static_cast<skr_char16>(ch) } :
           UTF16Seq{
               static_cast<char16_t>((ch >> 10) + kUtf16LeadingSurrogateHeader),
               static_cast<char16_t>((ch & kUtf16SurrogateMask) + kUtf16TrailingSurrogateHeader),
           };
}

//==================> parse code seq <==================
inline constexpr UTF8Seq utf8_parse_seq(const skr_char8* seq, uint64_t index, uint64_t size, uint64_t& seq_index)
{
    if (!seq) return {};
    if (index >= size) return {};

    seq_index    = index;
    auto seq_len = utf8_seq_len(seq[seq_index]);
    while (!seq_len)
    {
        if (seq_index == 0) return {};
        --seq_index;
        seq_len = utf8_seq_len(seq[seq_index]);
    }

    return UTF8Seq{ seq + seq_index, static_cast<uint32_t>(seq_len) };
}
inline constexpr UTF16Seq utf16_parse_seq(const skr_char16* seq, uint64_t index, uint64_t size, uint64_t& seq_index)
{
    if (!seq) return {};
    if (index >= size) return {};

    seq_index    = index;
    auto seq_len = utf16_seq_len(seq[seq_index]);
    while (!seq_len)
    {
        if (seq_index == 0) return {};
        --seq_index;
        seq_len = utf16_seq_len(seq[seq_index]);
    }
    return UTF16Seq{ seq + seq_index, static_cast<uint32_t>(seq_len) };
}
} // namespace skr