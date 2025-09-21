#include <SkrBase/misc/data_encode.hpp>

// base16
namespace skr
{
// clang-format off
static char8_t kBase16Table[16] = {
    // number parts
    '0', '1', '2', '3', '4', '5', '6', '7','8', '9', 
    // alphabet parts
    'A', 'B', 'C', 'D', 'E', 'F',
};
static char8_t kBase16TableLower[16] = {
    // number parts
    '0', '1', '2', '3', '4', '5', '6', '7','8', '9', 
    // alphabet parts
    'a', 'b', 'c', 'd', 'e', 'f',
};
static uint8_t kBase16DecodeTable[256] = {
    // 0x00-0x0F: Control characters
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x10-0x1F: Control characters
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x20-0x2F: Space and symbols
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x30-0x3F: Numbers (0-9) and symbols
       0,    1,    2,    3,    4,    5,    6,    7,    8,    9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x40-0x4F: @ and uppercase letters (A-F)
    0xFF,   10,   11,   12,   13,   14,   15, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x50-0x5F: Uppercase letters (P-Z) and symbols
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x60-0x6F: ` and lowercase letters (a-f)
    0xFF,   10,   11,   12,   13,   14,   15, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x70-0x7F: Lowercase letters (p-z) and DEL
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x80-0x8F: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x90-0x9F: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xA0-0xAF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xB0-0xBF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xC0-0xCF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xD0-0xDF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xE0-0xEF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xF0-0xFF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
// clang-format on

bool base16_is_valid_char(char8_t c)
{
    return kBase16DecodeTable[static_cast<uint8_t>(c)] != 0xFF;
}
bool base16_is_valid_string(const char8_t* str, uint64_t len)
{
    for (uint64_t i = 0; i < len; ++i)
    {
        if (!base16_is_valid_char(str[i])) [[unlikely]]
            return false;
    }
    return true;
}

// length calculation
uint64_t base16_encode_length(uint64_t byte_length)
{
    return byte_length * 2;
}
uint64_t base16_decode_length(uint64_t char_length)
{ // ceil for safe decode
    return (char_length + 1) / 2;
}

// group for encode/decode
// 2 char => 1 byte, no incomplete group issue
struct Base16Group
{
    inline static constexpr uint8_t kSegmentMask = 0b0000'1111;

    // getter & setter
    inline char8_t chars(uint64_t index) const { return _chars[index]; }
    inline void    set_chars(uint64_t index, char8_t c) { _chars[index] = c; }
    inline uint8_t data() const { return _data; }
    inline void    set_data(uint8_t d) { _data = d; }

    // encode group, data -> chars
    inline void encode(bool lower)
    { //! Note: encode sequence is big-endian
        auto table = lower ? kBase16TableLower : kBase16Table;
        _chars[0]  = table[_data >> 4]; // need not mask
        _chars[1]  = table[_data >> 0 & kSegmentMask];
    }

    // decode group, chars -> data
    inline bool decode()
    {
        auto high = kBase16DecodeTable[static_cast<uint8_t>(_chars[0])];
        auto low  = kBase16DecodeTable[static_cast<uint8_t>(_chars[1])];

        // handle invalid char
        if (high == 0xFF || low == 0xFF) [[unlikely]]
        {
            return false;
        }
        else
        {
            _data = (high << 4) | (low << 0);
            return true;
        }
    }

private:
    char8_t _chars[2] = {};
    uint8_t _data     = {};
};

// encode
void base16_encode(
    skr_char8*  out_str,
    const void* in_data,
    uint64_t    in_data_length,
    bool        lower
)
{
    // bad arguments
    if (!out_str || !in_data || in_data_length == 0) return;
    auto* in_data_u8 = reinterpret_cast<const uint8_t*>(in_data);

    // do encode
    for (uint64_t i = 0; i < in_data_length; ++i)
    {
        // do encode
        Base16Group group;
        group.set_data(in_data_u8[i]);
        group.encode(lower);

        // write output
        out_str[i * 2 + 0] = group.chars(0);
        out_str[i * 2 + 1] = group.chars(1);
    }
}

// decode
bool base16_decode(
    void*            out_data,
    uint64_t         out_data_length,
    const skr_char8* in_str,
    uint64_t         in_str_length
)
{
    // bad arguments
    if (!out_data || !in_str) [[unlikely]]
        return false;
    if (in_str_length == 0) [[unlikely]]
        return true;
    auto* out_data_u8 = reinterpret_cast<uint8_t*>(out_data);

    // do decode
    for (uint64_t i = 0; i < in_str_length; i += 2)
    {
        // check output buffer overflow
        if ((i / 2) >= out_data_length) [[unlikely]]
            return true;

        // do decode
        Base16Group group;
        group.set_chars(0, in_str[i]);
        group.set_chars(1, (i + 1 < in_str_length) ? in_str[i + 1] : '0');
        if (group.decode()) [[likely]]
        {
            out_data_u8[i / 2] = group.data();
        }
        else
        {
            return false; // invalid char
        }
    }
    return true;
}
} // namespace skr

// base32
namespace skr
{
// clang-format off
static char8_t kBase32Table[32] = {
    // alphabet parts (A-Z)
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    // number parts (2-7)
    '2', '3', '4', '5', '6', '7'
};
static char8_t kBase32TableLower[32] = {
    // alphabet parts (a-z)
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    // number parts (2-7)
    '2', '3', '4', '5', '6', '7'
};
static uint8_t kBase32DecodeTable[256] = {
    // 0x00-0x0F: Control characters
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x10-0x1F: Control characters
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x20-0x2F: Space and symbols (includes '2'-'7')
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x30-0x3F: Numbers ('0'-'9') and symbols ('2'-'7' are valid)
    0xFF, 0xFF,   26,   27,   28,   29,   30,   31, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x40-0x4F: @ and uppercase letters ('A'-'O')
    0xFF,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    // 0x50-0x5F: Uppercase letters ('P'-'Z') and symbols
      15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x60-0x6F: ` and lowercase letters ('a'-'o')
    0xFF,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    // 0x70-0x7F: Lowercase letters ('p'-'z') and DEL
      15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x80-0x8F: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x90-0x9F: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xA0-0xAF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xB0-0xBF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xC0-0xCF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xD0-0xDF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xE0-0xEF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xF0-0xFF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
// clang-format on

// validation
bool base32_is_valid_char(char8_t c)
{
    return kBase32DecodeTable[static_cast<uint8_t>(c)] != 0xFF || c == '=';
}
bool base32_is_valid_string(const char8_t* str, uint64_t len)
{
    for (uint64_t i = 0; i < len; ++i)
    {
        if (!base32_is_valid_char(str[i])) [[unlikely]]
            return false;
    }
    return true;
}

// length calculation
//! Note, decode length is approximate, actual length may be less due to padding and incomplete groups
uint64_t base32_encode_length(uint64_t byte_length, bool with_padding)
{
    return with_padding ?
        ((byte_length + 4) / 5) * 8 : // with padding
        ((byte_length * 8 + 4) / 5);  // no padding
}
uint64_t base32_decode_length(uint64_t char_length)
{
    return (char_length * 5 + 7) / 8;
}

// padding check
bool base32_is_strictly_padded(uint64_t char_length)
{
    return (char_length % 8) == 0;
}

// group for encode/decode
// 8 char => 5 byte, padding with '='
struct Base32Group
{
    inline static constexpr uint64_t kSegmentMask = 0b0001'1111; // 5 bits mask

    // getter & setter
    inline char8_t chars(uint64_t index) const { return _chars[index]; }
    inline void    set_chars(uint64_t index, char8_t c) { _chars[index] = c; }
    inline uint8_t data(uint64_t index) const
    {
        return ((uint8_t*)(&_data))[7 - index];
    }
    inline void set_data(uint64_t index, uint8_t value)
    {
        ((uint8_t*)(&_data))[7 - index] = value;
    }
    inline uint64_t data_valid_bits() const
    {
        return 64 - skr::countr_zero(_data);
    }

    // encode group, data -> chars
    inline uint64_t encode(uint64_t data_count, bool lower = false)
    {
        auto     table      = lower ? kBase32TableLower : kBase32Table;
        uint64_t char_count = (data_count * 8 + 4) / 5; // ceiled char count
        uint64_t move_count = 64;                       // read from high bit
        for (uint64_t ch_idx = 0; ch_idx < char_count; ++ch_idx) [[likely]]
        {
            move_count -= 5;
            uint64_t segment = (_data >> move_count) & kSegmentMask;
            _chars[ch_idx]   = table[segment];
        }
        return char_count;
    }

    // decode group, chars -> data
    inline bool decode(uint64_t char_count, uint64_t& out_data_count)
    {
        // clean up data
        _data = 0;

        uint64_t data_count = (char_count * 5 + 7) / 8; // ceiled byte count
        uint64_t move_count = 64;                       // write from high bit
        for (uint64_t i = 0; i < char_count; ++i)
        {
            auto segment = (uint64_t)kBase32DecodeTable[static_cast<uint8_t>(_chars[i])];
            if (segment != 0xFF) [[likely]]
            {
                move_count -= 5;
                _data |= segment << move_count;
            }
            else
            {
                return false; // invalid char
            }
        }

        out_data_count = data_count;
        return true;
    }

private:
    char8_t  _chars[8] = {};
    uint64_t _data     = {}; // 5 * 8 bits
};

// minimal decode length, will parse last group and discard 0 bits at the end
uint64_t base32_decode_length_minimal(const skr_char8* str, uint64_t char_length)
{
    // bad arguments
    if (!str || char_length == 0) return 0;

    // skip padding chars
    while (char_length > 0 && str[char_length - 1] == '=')
    {
        --char_length;
    }

    if (char_length == 0) return 0;

    uint64_t last_group_char_count = char_length % 8;
    uint64_t last_group_start      = char_length - last_group_char_count;

    // prepare group
    Base32Group group;
    for (uint64_t i = 0; i < last_group_char_count; ++i)
    {
        group.set_chars(i, str[last_group_start + i]);
    }

    // decode
    uint64_t valid_bit_count = 0;
    {
        uint64_t data_count = 0;
        if (group.decode(last_group_char_count, data_count)) [[likely]]
        {
            // scan uint64 for trailing zero bits
            valid_bit_count = group.data_valid_bits();
        }
        else
        {
            valid_bit_count = 40;
        }
    }

    // calculate total bit count
    uint64_t total_bits = (char_length / 8) * 40 + valid_bit_count;
    return (total_bits + 7) / 8;
}

// encode
void base32_encode(
    skr_char8*  out_str,
    const void* in_data,
    uint64_t    in_data_length,
    bool        with_padding,
    bool        lower
)
{
    // bad arguments
    if (!out_str || !in_data || in_data_length == 0) return;
    auto* in_data_u8 = reinterpret_cast<const uint8_t*>(in_data);

    // do encode
    uint64_t write_index = 0;
    for (uint64_t g = 0; g < in_data_length; g += 5)
    {
        auto slack = in_data_length - g;

        // prepare group
        Base32Group group;
        uint64_t    data_count = slack >= 5 ? 5 : slack;
        for (uint64_t i = 0; i < data_count; ++i)
        {
            group.set_data(i, in_data_u8[g + i]);
        }

        // do encode
        uint64_t char_count = group.encode(data_count, lower);

        // write chars
        for (uint64_t i = 0; i < char_count; ++i)
        {
            out_str[write_index] = group.chars(i);
            ++write_index;
        }

        // write padding
        if (with_padding && char_count < 8)
        {
            for (uint64_t i = char_count; i < 8; ++i)
            {
                out_str[write_index++] = '=';
            }
        }
    }
}

// decode
bool base32_decode(
    void*            out_data,
    uint64_t         out_data_length,
    const skr_char8* in_str,
    uint64_t         in_str_length
)
{
    // bad arguments
    if (!out_data || !in_str) [[unlikely]]
        return false;
    if (in_str_length == 0) [[unlikely]]
        return true;
    auto* out_data_u8 = reinterpret_cast<uint8_t*>(out_data);

    // skip padding chars
    while (in_str_length > 0 && in_str[in_str_length - 1] == '=')
    {
        --in_str_length;
    }

    if (in_str_length == 0) [[unlikely]]
        return true;

    // do decode
    uint64_t write_index = 0;
    for (uint64_t g = 0; g < in_str_length; g += 8)
    {
        auto slack = in_str_length - g;

        // prepare group
        Base32Group group;
        uint64_t    char_count = slack >= 8 ? 8 : slack;
        for (uint64_t i = 0; i < char_count; ++i)
        {
            group.set_chars(i, in_str[g + i]);
        }

        // do decode
        uint64_t data_count = 0;
        if (group.decode(char_count, data_count)) [[likely]]
        {
            // write data
            for (uint64_t i = 0; i < data_count; ++i)
            {
                // check output buffer overflow
                if ((write_index) >= out_data_length) [[unlikely]]
                    return true;

                out_data_u8[write_index] = group.data(i);
                ++write_index;
            }
        }
        else
        {
            return false; // invalid char
        }
    }
    return true;
}
} // namespace skr

// base64
namespace skr
{
// clang-format off
static char8_t kBase64Table[64] = {
    // uppercase alphabet (A-Z)
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    // lowercase alphabet (a-z)
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    // numbers (0-9)
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    // special characters
    '+', '/'
};
static uint8_t kBase64DecodeTable[256] = {
    // 0x00-0x0F: Control characters
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x10-0x1F: Control characters
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x20-0x2F: Space and symbols (includes '+' at 0x2B, '/' at 0x2F)
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   62, 0xFF, 0xFF, 0xFF,   63,
    // 0x30-0x3F: Numbers (0-9) and symbols
      52,   53,   54,   55,   56,   57,   58,   59,   60,   61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x40-0x4F: @ and uppercase letters (A-O)
    0xFF,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    // 0x50-0x5F: Uppercase letters (P-Z) and symbols
      15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x60-0x6F: ` and lowercase letters (a-o)
    0xFF,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
    // 0x70-0x7F: Lowercase letters (p-z) and DEL
      41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x80-0x8F: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0x90-0x9F: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xA0-0xAF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xB0-0xBF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xC0-0xCF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xD0-0xDF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xE0-0xEF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // 0xF0-0xFF: Extended ASCII
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
// clang-format on

// validation
bool base64_is_valid_char(char8_t c)
{
    return kBase64DecodeTable[static_cast<uint8_t>(c)] != 0xFF || c == '=';
}
bool base64_is_valid_string(const char8_t* str, uint64_t len)
{
    for (uint64_t i = 0; i < len; ++i)
    {
        if (!base64_is_valid_char(str[i])) [[unlikely]]
            return false;
    }
    return true;
}

// length calculation
//! Note, decode length is approximate, actual length may be less due to padding and incomplete groups
uint64_t base64_encode_length(uint64_t byte_length, bool with_padding)
{
    return with_padding ?
        ((byte_length + 2) / 3) * 4 : // with padding
        ((byte_length * 4 + 2) / 3);  // no padding
}
uint64_t base64_decode_length(uint64_t char_length)
{
    return (char_length * 3 + 3) / 4;
}

// padding check
bool base64_is_strictly_padded(uint64_t char_length)
{
    return (char_length % 4) == 0;
}

// group for encode/decode
// 4 char => 3 byte, padding with '='
struct Base64Group
{
    inline static constexpr uint64_t kSegmentMask = 0b0011'1111; // 6 bits mask

    // getter & setter
    inline char8_t chars(uint64_t index) const { return _chars[index]; }
    inline void    set_chars(uint64_t index, char8_t c) { _chars[index] = c; }
    inline uint8_t data(uint64_t index) const
    {
        return ((uint8_t*)(&_data))[3 - index];
    }
    inline void set_data(uint64_t index, uint8_t value)
    {
        ((uint8_t*)(&_data))[3 - index] = value;
    }
    inline uint64_t data_valid_bits() const
    {
        return 32 - skr::countr_zero(_data);
    }

    // encode group, data -> chars
    inline uint64_t encode(uint64_t data_count)
    {
        uint64_t char_count = (data_count * 4 + 2) / 3; // ceiled char count
        uint64_t move_count = 32;                       // read from high bit
        for (uint64_t ch_idx = 0; ch_idx < char_count; ++ch_idx)
        {
            move_count -= 6;
            uint64_t segment = (_data >> move_count) & kSegmentMask;
            _chars[ch_idx]   = kBase64Table[segment];
        }
        return char_count;
    }

    // decode group, chars -> data
    inline bool decode(uint64_t char_count, uint64_t& out_data_count)
    {
        // clean up data
        _data = 0;

        uint64_t data_count = (char_count * 3 + 3) / 4; // ceiled byte count
        uint64_t move_count = 32;                       // write from high bit
        for (uint64_t i = 0; i < char_count; ++i)
        {
            auto segment = (uint64_t)kBase64DecodeTable[static_cast<uint8_t>(_chars[i])];
            if (segment != 0xFF) [[likely]]
            {
                move_count -= 6;
                _data |= segment << move_count;
            }
            else
            {
                return false; // invalid char
            }
        }

        out_data_count = data_count;
        return true;
    }

private:
    char8_t  _chars[4] = {};
    uint32_t _data     = {}; // 3 * 8 bits
};

// minimal decode length, will parse last group and discard 0 bits at the end
uint64_t base64_decode_length_minimal(const skr_char8* str, uint64_t char_length)
{
    // bad arguments
    if (!str || char_length == 0) return 0;

    // skip padding chars
    while (char_length > 0 && str[char_length - 1] == '=')
    {
        --char_length;
    }

    if (char_length == 0) return 0;

    uint64_t last_group_char_count = char_length % 4;
    uint64_t last_group_start      = char_length - last_group_char_count;

    // prepare group
    Base64Group group;
    for (uint64_t i = 0; i < last_group_char_count; ++i)
    {
        group.set_chars(i, str[last_group_start + i]);
    }

    // decode
    uint64_t valid_bit_count = 0;
    {
        uint64_t data_count = 0;
        if (group.decode(last_group_char_count, data_count)) [[likely]]
        {
            // scan uint32 for trailing zero bits
            valid_bit_count = group.data_valid_bits();
        }
        else
        {
            valid_bit_count = 24;
        }
    }

    // calculate total bit count
    uint64_t total_bits = (char_length / 4) * 24 + valid_bit_count;
    return (total_bits + 7) / 8;
}

// encode
void base64_encode(
    skr_char8*  out_str,
    const void* in_data,
    uint64_t    in_data_length,
    bool        with_padding
)
{
    // bad arguments
    if (!out_str || !in_data || in_data_length == 0) return;
    auto* in_data_u8 = reinterpret_cast<const uint8_t*>(in_data);

    // do encode
    uint64_t write_index = 0;
    for (uint64_t g = 0; g < in_data_length; g += 3)
    {
        auto slack = in_data_length - g;

        // prepare group
        Base64Group group;
        uint64_t    data_count = slack >= 3 ? 3 : slack;
        for (uint64_t i = 0; i < data_count; ++i)
        {
            group.set_data(i, in_data_u8[g + i]);
        }

        // do encode
        uint64_t char_count = group.encode(data_count);

        // write chars
        for (uint64_t i = 0; i < char_count; ++i)
        {
            out_str[write_index] = group.chars(i);
            ++write_index;
        }

        // write padding
        if (with_padding && char_count < 4)
        {
            for (uint64_t i = char_count; i < 4; ++i)
            {
                out_str[write_index++] = '=';
            }
        }
    }
}

// decode
bool base64_decode(
    void*            out_data,
    uint64_t         out_data_length,
    const skr_char8* in_str,
    uint64_t         in_str_length
)
{
    // bad argument
    if (!out_data || !in_str) [[unlikely]]
        return false;
    if (in_str_length == 0) [[unlikely]]
        return true;
    auto* out_data_u8 = reinterpret_cast<uint8_t*>(out_data);

    // skip padding chars
    while (in_str_length > 0 && in_str[in_str_length - 1] == '=')
    {
        --in_str_length;
    }

    if (in_str_length == 0) [[unlikely]]
        return true;

    // do decode
    uint64_t write_index = 0;
    for (uint64_t g = 0; g < in_str_length; g += 4)
    {
        auto slack = in_str_length - g;

        // prepare group
        Base64Group group;
        uint64_t    char_count = slack >= 4 ? 4 : slack;
        for (uint64_t i = 0; i < char_count; ++i)
        {
            group.set_chars(i, in_str[g + i]);
        }

        // do decode
        uint64_t data_count = 0;
        if (group.decode(char_count, data_count)) [[likely]]
        {
            // write data
            for (uint64_t i = 0; i < data_count; ++i)
            {
                // check output buffer overflow
                if ((write_index) >= out_data_length) [[unlikely]]
                    return true;

                out_data_u8[write_index] = group.data(i);
                ++write_index;
            }
        }
        else
        {
            return false; // invalid char
        }
    }
    return true;
}
} // namespace skr