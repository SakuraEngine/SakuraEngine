#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
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
static constexpr char32_t kSMPBaseCodePoint             = 0x10000;

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
// return: 0 => invalid code unit, 1-4 => sequence length
constexpr uint64_t utf8_adjust_index_to_head(const skr_char8* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index);
// return: index in code point
constexpr uint64_t utf8_code_point_index(const skr_char8* seq, uint32_t size, uint64_t index);
// return: index in code unit
constexpr uint64_t utf8_code_unit_index(const skr_char8* seq, uint32_t size, uint64_t index);

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
// return: 0 => invalid code unit, 1-2 => sequence length
constexpr uint64_t utf16_adjust_index_to_head(const skr_char16* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index);
// return: index in code point
constexpr uint64_t utf16_code_point_index(const skr_char16* seq, uint32_t size, uint64_t index);
// return: index in code unit
constexpr uint64_t utf16_code_unit_index(const skr_char16* seq, uint32_t size, uint64_t index);

//==================> sequence <==================
struct UTF8Seq;
struct UTF16Seq;
struct UTF8Seq {
    // ctor
    constexpr UTF8Seq();
    constexpr UTF8Seq(const skr_char8* c_str, uint8_t len);
    constexpr UTF8Seq(skr_char8 ch_0);
    constexpr UTF8Seq(skr_char8 ch_0, skr_char8 ch_1);
    constexpr UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2);
    constexpr UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2, skr_char8 ch_3);

    // cast
    constexpr UTF8Seq(const UTF16Seq& seq);
    constexpr UTF8Seq(skr_char32 ch);
    constexpr operator skr_char32() const;

    // factory
    constexpr static UTF8Seq Bad(skr_char8 bad_ch);
    constexpr static UTF8Seq ParseUTF8(const skr_char8* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index);
    constexpr static UTF8Seq ParseUTF16(const skr_char16* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index);

    // compare
    constexpr bool operator==(const UTF8Seq& rhs) const;

    // validate
    constexpr bool is_valid() const;
    constexpr      operator bool() const;

    // visitor
    constexpr skr_char8& at(uint32_t index);
    constexpr skr_char8  at(uint32_t index) const;
    constexpr skr_char8& operator[](uint32_t index);
    constexpr skr_char8  operator[](uint32_t index) const;

public:
    alignas(4) skr_char8 data[4] = {};
    skr_char8 bad_data           = {};
    uint8_t   len                = 0;
};
struct UTF16Seq {
    // ctor
    constexpr UTF16Seq();
    constexpr UTF16Seq(const skr_char16* c_str, uint8_t len);
    constexpr UTF16Seq(skr_char16 ch_0);
    constexpr UTF16Seq(skr_char16 ch_0, skr_char16 ch_1);

    // cast
    constexpr UTF16Seq(const UTF8Seq& seq);
    constexpr UTF16Seq(skr_char32 ch);
    constexpr operator skr_char32() const;

    // factory
    constexpr static UTF16Seq Bad(skr_char16 bad_ch);
    constexpr static UTF16Seq ParseUTF8(const skr_char8* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index);
    constexpr static UTF16Seq ParseUTF16(const skr_char16* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index);

    // compare
    constexpr bool operator==(const UTF16Seq& rhs) const;

    // validate
    constexpr bool is_valid() const;
    constexpr      operator bool() const;

    // visitor
    constexpr skr_char16& at(uint32_t index);
    constexpr skr_char16  at(uint32_t index) const;
    constexpr skr_char16& operator[](uint32_t index);
    constexpr skr_char16  operator[](uint32_t index) const;

public:
    alignas(4) skr_char16 data[2] = {};
    skr_char16 bad_data           = {};
    uint8_t    len                = 0;
};
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
inline constexpr uint64_t utf8_adjust_index_to_head(const skr_char8* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index)
{
    uint64_t seq_len = utf8_seq_len(seq[index]);

    if (seq_len == 0)
    {
        if (index == 0)
        { // bad head
            adjusted_index = index;
            return 0;
        }
        else
        { // adjust to head
            uint64_t find_idx        = index;
            uint64_t code_uint_count = 0;
            while (find_idx > 0)
            {
                --find_idx;
                ++code_uint_count;
                seq_len = utf8_seq_len(seq[find_idx]);
                if (seq_len)
                {
                    ++code_uint_count;
                    break;
                }
            }

            if (seq_len == 0)
            { // reach seq start, bad head
                adjusted_index = index;
                return 0;
            }
            else if (seq_len < code_uint_count)
            { // count miss match
                adjusted_index = index;
                return 0;
            }
            else
            { // good head
                if (find_idx + seq_len > size)
                { // seq len overflow
                    adjusted_index = index;
                    return 0;
                }
                else
                {
                    adjusted_index = find_idx;
                    return seq_len;
                }
            }
        }
    }
    else
    {
        adjusted_index = index;
        return index + seq_len <= size ? seq_len : 0; // avoid overflow
    }
}
inline constexpr uint64_t utf8_code_point_index(const skr_char8* seq, uint32_t size, uint64_t index)
{
    SKR_ASSERT(index < size);

    uint64_t cur_idx          = 0;
    uint64_t code_point_count = 0;

    do
    {
        const auto seq_len = utf8_seq_len(seq[cur_idx]);
        cur_idx += seq_len ?
                   (cur_idx + seq_len <= size) ? seq_len : 1 : // check bad ch in tail
                   1;                                          // invalid code unit
        ++code_point_count;
    } while (cur_idx <= index);

    return code_point_count - 1;
}
inline constexpr uint64_t utf8_code_unit_index(const skr_char8* seq, uint32_t size, uint64_t index)
{
    SKR_ASSERT(index < size);

    uint64_t cur_idx          = 0;
    uint64_t code_point_count = 0;
    while (cur_idx < size && code_point_count < index)
    {
        const auto seq_len = utf8_seq_len(seq[cur_idx]);
        cur_idx += seq_len ?
                   (cur_idx + seq_len <= size) ? seq_len : 1 : // check bad ch in tail
                   1;                                          // invalid code unit
        ++code_point_count;
    }
    SKR_ASSERT(code_point_count == index && "invalid code point index");
    return cur_idx;
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
inline constexpr uint64_t utf16_adjust_index_to_head(const skr_char16* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index)
{
    uint64_t seq_len = utf16_seq_len(seq[index]);

    if (seq_len == 0)
    {
        if (index == 0)
        { // bad head
            adjusted_index = index;
            return 0;
        }
        else
        { // adjust to head
            uint64_t find_idx = index - 1;
            seq_len           = utf16_seq_len(seq[find_idx]);
            if (seq_len == 2)
            { // good code seq
                adjusted_index = find_idx;
                return seq_len;
            }
            else
            { // invalid code seq
                adjusted_index = index;
                return 0;
            }
        }
    }
    else
    {
        adjusted_index = index;
        return index + seq_len <= size ? seq_len : 0; // avoid overflow
    }
}
inline constexpr uint64_t utf16_code_point_index(const skr_char16* seq, uint32_t size, uint64_t index)
{
    SKR_ASSERT(index < size);

    uint64_t cur_idx          = 0;
    uint64_t code_point_count = 0;
    do
    {
        const auto seq_len = utf16_seq_len(seq[cur_idx]);
        cur_idx += seq_len ?
                   (cur_idx + seq_len <= size) ? seq_len : 1 : // check bad ch in tail
                   1;                                          // invalid code unit
        ++code_point_count;
    } while (cur_idx <= index);
    return code_point_count - 1;
}
inline constexpr uint64_t utf16_code_unit_index(const skr_char16* seq, uint32_t size, uint64_t index)
{
    SKR_ASSERT(index < size);

    uint64_t cur_idx          = 0;
    uint64_t code_point_count = 0;
    while (cur_idx < size && code_point_count < index)
    {
        const auto seq_len = utf16_seq_len(seq[cur_idx]);
        cur_idx += seq_len ?
                   (cur_idx + seq_len <= size) ? seq_len : 1 : // check bad ch in tail
                   1;                                          // invalid code unit
        ++code_point_count;
    }
    SKR_ASSERT(code_point_count == index && "invalid code point index");
    return cur_idx;
}

//==================> utf-8 sequence <==================
inline constexpr UTF8Seq::UTF8Seq() = default;
inline constexpr UTF8Seq::UTF8Seq(const skr_char8* c_str, uint8_t len)
    : len(len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        data[i] = c_str[i];
    }
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0)
    : data{ ch_0, 0, 0, 0 }
    , len(1)
{
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0, skr_char8 ch_1)
    : data{ ch_0, ch_1, 0, 0 }
    , len(2)
{
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2)
    : data{ ch_0, ch_1, ch_2, 0 }
    , len(3)
{
}
inline constexpr UTF8Seq::UTF8Seq(skr_char8 ch_0, skr_char8 ch_1, skr_char8 ch_2, skr_char8 ch_3)
    : data{ ch_0, ch_1, ch_2, ch_3 }
    , len(4)
{
}
inline constexpr UTF8Seq::UTF8Seq(const UTF16Seq& seq)
{
    skr_char32 u32_ch = static_cast<skr_char32>(seq);
    *this             = u32_ch;
}
inline constexpr UTF8Seq::UTF8Seq(skr_char32 ch)
{
    if (ch <= utf8_maximum_code_point(1))
    {
        data[0] = static_cast<skr_char8>(ch);
        len     = 1;
    }
    else if (ch <= utf8_maximum_code_point(2))
    {
        data[0] = static_cast<skr_char8>((ch >> 6) | 0xc0);
        data[1] = static_cast<skr_char8>((ch & utf8_mask(0)) | 0x80);
        len     = 2;
    }
    else if (ch <= utf8_maximum_code_point(3))
    {
        data[0] = static_cast<skr_char8>((ch >> 12) | 0xe0);
        data[1] = static_cast<skr_char8>(((ch >> 6) & utf8_mask(0)) | 0x80);
        data[2] = static_cast<skr_char8>((ch & utf8_mask(0)) | 0x80);
        len     = 3;
    }
    else
    {
        data[0] = static_cast<skr_char8>((ch >> 18) | 0xf0);
        data[1] = static_cast<skr_char8>(((ch >> 12) & utf8_mask(0)) | 0x80);
        data[2] = static_cast<skr_char8>(((ch >> 6) & utf8_mask(0)) | 0x80);
        data[3] = static_cast<skr_char8>((ch & utf8_mask(0)) | 0x80);
        len     = 4;
    }
}
inline constexpr UTF8Seq::operator skr_char32() const
{
    if (is_valid())
    {
        const auto         lead_mask     = utf8_mask(len);
        constexpr auto     trailing_mask = utf8_mask(0);
        constexpr uint64_t trailing_bits = 6;

        const auto ch_0   = data[0];
        char32_t   result = static_cast<char32_t>(ch_0 & lead_mask);
        for (uint32_t i = 1; i < len; ++i)
        {
            const auto ch_i = data[i];
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

// factory
inline constexpr UTF8Seq UTF8Seq::Bad(skr_char8 bad_ch)
{
    UTF8Seq result{};
    result.bad_data = bad_ch;
    return result;
}
inline constexpr UTF8Seq UTF8Seq::ParseUTF8(const skr_char8* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index)
{
    // bad input
    if (!seq) return {};
    if (index >= size) return {};

    // adjust seq index
    auto seq_len = utf8_adjust_index_to_head(seq, size, index, adjusted_index);

    return seq_len ? UTF8Seq{ seq + adjusted_index, static_cast<uint8_t>(seq_len) } :
                     UTF8Seq::Bad(seq[adjusted_index]);
}
inline constexpr UTF8Seq UTF8Seq::ParseUTF16(const skr_char16* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index)
{
    return { UTF16Seq::ParseUTF16(seq, size, index, adjusted_index) };
}

// compare
inline constexpr bool UTF8Seq::operator==(const UTF8Seq& rhs) const
{
    if (len != rhs.len) return false;
    for (uint32_t i = 0; i < len; ++i)
    {
        if (data[i] != rhs.data[i]) return false;
    }
    return true;
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
inline constexpr UTF16Seq::UTF16Seq(const skr_char16* c_str, uint8_t len)
    : len(len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        data[i] = c_str[i];
    }
}
inline constexpr UTF16Seq::UTF16Seq(skr_char16 ch_0)
    : data{ ch_0, 0 }
    , len(1)
{
}
inline constexpr UTF16Seq::UTF16Seq(skr_char16 ch_0, skr_char16 ch_1)
    : data{ ch_0, ch_1 }
    , len(2)
{
}

// cast
inline constexpr UTF16Seq::UTF16Seq(const UTF8Seq& seq)
{
    skr_char32 u32_ch = static_cast<skr_char32>(seq);
    *this             = u32_ch;
}
inline constexpr UTF16Seq::UTF16Seq(skr_char32 ch)
{
    if (ch <= kBMPMaxCodePoint)
    {
        data[0] = static_cast<skr_char16>(ch);
        len     = 1;
    }
    else
    {
        data[0] = static_cast<char16_t>(((ch - kSMPBaseCodePoint) >> 10) + kUtf16LeadingSurrogateHeader);
        data[1] = static_cast<char16_t>((ch & kUtf16SurrogateMask) + kUtf16TrailingSurrogateHeader);
        len     = 2;
    }
}
inline constexpr UTF16Seq::operator skr_char32() const
{
    //                  1         0
    //         9876543210 9876543210
    //         |||||||||| ||||||||||
    // [110110]9876543210 |||||||||| high surrogate
    //            [110111]9876543210 low  surrogate
    return len == 1 ? data[0] :
                      ((data[0] & kUtf16SurrogateMask) << 10) + (data[1] & kUtf16SurrogateMask) + kSMPBaseCodePoint;
}

// factory
inline constexpr UTF16Seq UTF16Seq::Bad(skr_char16 bad_ch)
{
    UTF16Seq result{};
    result.bad_data = bad_ch;
    return result;
}
inline constexpr UTF16Seq UTF16Seq::ParseUTF8(const skr_char8* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index)
{
    return { UTF8Seq::ParseUTF8(seq, size, index, adjusted_index) };
}
inline constexpr UTF16Seq UTF16Seq::ParseUTF16(const skr_char16* seq, uint64_t size, uint64_t index, uint64_t& adjusted_index)
{
    // bad input
    if (!seq) return {};
    if (index >= size) return {};

    // adjust seq index
    auto seq_len = utf16_adjust_index_to_head(seq, size, index, adjusted_index);

    return seq_len ? UTF16Seq{ seq + adjusted_index, static_cast<uint8_t>(seq_len) } :
                     UTF16Seq::Bad(seq[adjusted_index]);
}

// compare
inline constexpr bool UTF16Seq::operator==(const UTF16Seq& rhs) const
{
    if (len != rhs.len) return false;
    for (uint32_t i = 0; i < len; ++i)
    {
        if (data[i] != rhs.data[i]) return false;
    }
    return true;
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
} // namespace skr