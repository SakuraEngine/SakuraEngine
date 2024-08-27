#pragma once
#include <SkrBase/containers/string/string_config.hpp>
#include <array>

// unicode algo
namespace skr::container::unicode
{
// utf-16
namespace utf16
{
static constexpr uint64_t SEQUENCE_MAXIMUM_LENGTH = 2;

static constexpr char32_t LEADING_SURROGATE_HEADER   = 0xD800;
static constexpr char32_t TRAILING_SURROGATE_HEADER  = 0xDC00;
static constexpr char32_t LEADING_SURROGATE_MINIMUM  = 0xD800;
static constexpr char32_t TRAILING_SURROGATE_MINIMUM = 0xDC00;
static constexpr char32_t LEADING_SURROGATE_MAXIMUM  = 0xDBFF;
static constexpr char32_t TRAILING_SURROGATE_MAXIMUM = 0xDFFF;
static constexpr char32_t SURROGATE_MASK             = 0x03FF;
static constexpr char32_t SINGLE_UNIT_MAXIMUM_VALUE  = 0xFFFF;

constexpr bool is_leading_surrogate(const char16_t c) noexcept
{
    return c >= LEADING_SURROGATE_MINIMUM && c <= LEADING_SURROGATE_MAXIMUM;
}

constexpr bool is_trailing_surrogate(const char16_t c) noexcept
{
    return c >= TRAILING_SURROGATE_MINIMUM && c <= TRAILING_SURROGATE_MAXIMUM;
}

constexpr bool is_surrogate(const char16_t c) noexcept
{
    return c >= LEADING_SURROGATE_MINIMUM && c <= TRAILING_SURROGATE_MAXIMUM;
}

constexpr uint64_t parse_utf16_length(const char16_t c) noexcept
{
    return is_leading_surrogate(c) ? SEQUENCE_MAXIMUM_LENGTH : 1;
}
} // namespace utf16

static constexpr uint64_t UTF8_SEQUENCE_MAXIMUM_LENGTH = 4;

constexpr char32_t get_utf8_maximum_codepoint(const uint64_t codeunit_count)
{
    constexpr std::array<char32_t, UTF8_SEQUENCE_MAXIMUM_LENGTH> maximum_codepoints{
        0x7F,
        0x7FF,
        0xFFFF,
        0x1FFFFF,
    };
    return maximum_codepoints.at(codeunit_count - 1);
}

/**
 * @param length length of a utf8 sequence
 * @return mask of utf8 code unit
 */
constexpr skr_char8 get_utf8_mask(const uint64_t length) noexcept
{
    constexpr skr_char8 masks[] = {
        0b00111111_as_char,
        0b01111111_as_char,
        0b00011111_as_char,
        0b00001111_as_char,
        0b00000111_as_char,
    };
    return masks[length];
}

/**
 * @param c start of a utf8 sequence
 * @return length of utf8 sequence, return 0 if this is not start of a utf8 sequence
 */
constexpr uint8_t parse_utf8_length(const skr_char8 c) noexcept
{
    if (c == 0)
        return 1;
    constexpr skr_char8 mask = 0b10000000_as_char;
    uint8_t             size = 0;
    uint8_t             v    = c;
    while (v & mask)
    {
        ++size;
        v <<= 1;
    }
    // 1 byte when start with 0
    // 0 byte when start with 10
    // 2 bytes when start with 110
    // n bytes when start with 1..(n)..0
    return size > 1 ? size : 1 - size;
}

constexpr uint64_t parse_utf8_length(const char16_t utf16) noexcept
{
    if (utf16 <= get_utf8_maximum_codepoint(1))
        return 1;
    if (utf16 <= get_utf8_maximum_codepoint(2))
        return 2;
    if (utf16::is_surrogate(utf16))
        return 4;
    return 3;
}

constexpr uint64_t parse_utf8_length(const char32_t utf32) noexcept
{
    if (utf32 <= get_utf8_maximum_codepoint(1))
        return 1;
    if (utf32 <= get_utf8_maximum_codepoint(2))
        return 2;
    if (utf32 <= get_utf8_maximum_codepoint(3))
        return 3;
    return 4;
}

/**
 * @param utf8 input utf-8 code unit sequence
 * @param length length of utf-8 code unit sequence
 * @return codepoint of input utf8 code unit sequence
 */
constexpr char32_t utf8_to_utf32(skr_char8 const* const utf8, const uint64_t length) noexcept
{
    if (!utf8)
        return 0;
    const skr_char8     c0             = utf8[0];
    const skr_char8     lead_mask      = get_utf8_mask(length);
    char32_t            utf32          = c0 & lead_mask;
    constexpr skr_char8 following_mask = get_utf8_mask(0);
    for (uint64_t i = 1; i < length; ++i)
    {
        constexpr uint64_t bits_following = 6;
        const skr_char8    ci             = utf8[i];
        utf32 <<= bits_following;
        utf32 |= ci & following_mask;
    }
    return utf32;
}

/**
 * @param utf32 input utf-32 code unit
 * @return decoded utf-8 code unit sequence from utf-32 code unit
 */
constexpr std::array<skr_char8, UTF8_SEQUENCE_MAXIMUM_LENGTH> utf32_to_utf8(const char32_t utf32) noexcept
{
    if (utf32 <= get_utf8_maximum_codepoint(1)) return { static_cast<skr_char8>(utf32),
                                                         0, 0, 0 };
    if (utf32 <= get_utf8_maximum_codepoint(2)) return { static_cast<skr_char8>((utf32 >> 6) | 0xc0),
                                                         static_cast<skr_char8>((utf32 & 0x3f) | 0x80),
                                                         0, 0 };
    if (utf32 <= get_utf8_maximum_codepoint(3)) return { static_cast<skr_char8>((utf32 >> 12) | 0xe0),
                                                         static_cast<skr_char8>(((utf32 >> 6) & 0x3f) | 0x80),
                                                         static_cast<skr_char8>((utf32 & 0x3f) | 0x80),
                                                         0 };
    return { static_cast<skr_char8>((utf32 >> 18) | 0xf0),
             static_cast<skr_char8>(((utf32 >> 12) & 0x3f) | 0x80),
             static_cast<skr_char8>(((utf32 >> 6) & 0x3f) | 0x80),
             static_cast<skr_char8>((utf32 & 0x3f) | 0x80) };
}

constexpr char32_t utf16_to_utf32(char16_t const* const utf16, const uint64_t length) noexcept
{
    //                  1         0
    //         9876543210 9876543210
    //         |||||||||| ||||||||||
    // [110110]9876543210 |||||||||| high surrogate
    //            [110111]9876543210 low  surrogate
    return length == 1 ? utf16[0] : ((utf16[0] & utf16::SURROGATE_MASK) << 10) + (utf16[1] & utf16::SURROGATE_MASK);
}

constexpr std::array<char16_t, utf16::SEQUENCE_MAXIMUM_LENGTH> utf32_to_utf16(char32_t const utf32) noexcept
{
    return utf32 <= utf16::SINGLE_UNIT_MAXIMUM_VALUE ?
           std::array<char16_t, utf16::SEQUENCE_MAXIMUM_LENGTH>{ static_cast<char16_t>(utf32) } :
           std::array<char16_t, utf16::SEQUENCE_MAXIMUM_LENGTH>{
               static_cast<char16_t>((utf32 >> 10) + utf16::LEADING_SURROGATE_HEADER),
               static_cast<char16_t>((utf32 & utf16::SURROGATE_MASK) + utf16::TRAILING_SURROGATE_HEADER)
           };
}

constexpr std::array<char16_t, utf16::SEQUENCE_MAXIMUM_LENGTH> utf8_to_utf16(skr_char8 const* const utf8, const uint64_t length) noexcept
{
    return utf32_to_utf16(utf8_to_utf32(utf8, length));
}

constexpr std::array<skr_char8, UTF8_SEQUENCE_MAXIMUM_LENGTH> utf16_to_utf8(char16_t const* const utf16, const uint64_t length) noexcept
{
    return utf32_to_utf8(utf16_to_utf32(utf16, length));
}
} // namespace skr::container::unicode

// codepoint
namespace skr::container
{
struct codepoint {
    explicit constexpr codepoint(const skr_char8 v0 = 0, const skr_char8 v1 = 0, const skr_char8 v2 = 0, const skr_char8 v3 = 0) noexcept
        : sequence{ v0, v1, v2, v3 }
    {
    }

    explicit constexpr codepoint(const skr_char8* const v, const uint8_t length) noexcept
        : sequence{}
    {
        for (uint8_t i = 0; i < length; ++i)
            sequence[i] = v[i];
    }

    explicit constexpr codepoint(const skr_char8* v) noexcept
        : codepoint{ v, unicode::parse_utf8_length(v[0]) }
    {
    }

    explicit constexpr codepoint(const char16_t* sp) noexcept
        : sequence{ unicode::utf16_to_utf8(sp, unicode::utf16::parse_utf16_length(sp[0])) }
    {
    }

    explicit constexpr codepoint(const char32_t cp) noexcept
        : sequence{ unicode::utf32_to_utf8(cp) }
    {
    }

    struct const_iterator {
        constexpr const_iterator() noexcept = default;

        explicit constexpr const_iterator(const skr_char8* v) noexcept
            : value{ v }
        {
        }

        constexpr const skr_char8& operator*() const noexcept
        {
            return *this->value;
        }

        constexpr const skr_char8* raw() const noexcept
        {
            return this->value;
        }

        constexpr int64_t operator-(const const_iterator& rhs) const noexcept
        {
            return this->value - rhs.value;
        }

        constexpr const_iterator& operator+=(const int64_t diff) noexcept
        {
            this->value += diff;
            return *this;
        }

        constexpr const_iterator& operator-=(const int64_t diff) noexcept
        {
            return this->operator+=(-diff);
        }

        constexpr const_iterator operator+(const int64_t diff) const noexcept
        {
            const_iterator tmp = *this;
            tmp += diff;
            return tmp;
        }

        constexpr const_iterator operator-(const int64_t diff) const noexcept
        {
            const_iterator tmp = *this;
            tmp -= diff;
            return tmp;
        }

        constexpr const_iterator& operator++() noexcept
        {
            ++this->value;
            return *this;
        }

        constexpr const_iterator operator++(int) noexcept
        {
            const const_iterator tmp = *this;
            ++*this;
            return tmp;
        }

        constexpr const_iterator& operator--() noexcept
        {
            --this->value;
            return *this;
        }

        constexpr const_iterator operator--(int) noexcept
        {
            const const_iterator tmp = *this;
            --*this;
            return tmp;
        }

        constexpr bool operator==(const const_iterator& rhs) const noexcept
        {
            return this->value == rhs.value;
        }

        constexpr bool operator!=(const const_iterator& rhs) const noexcept
        {
            return !(*this == rhs);
        }

        constexpr bool operator<(const const_iterator& rhs) const noexcept
        {
            return this->value < rhs.value;
        }

        constexpr bool operator>(const const_iterator& rhs) const noexcept
        {
            return rhs < *this;
        }

        constexpr bool operator<=(const const_iterator& rhs) const noexcept
        {
            return rhs >= *this;
        }

        constexpr bool operator>=(const const_iterator& rhs) const noexcept
        {
            return !(*this < rhs);
        }

        const skr_char8* value = nullptr;
    };

    constexpr const_iterator begin() const noexcept
    {
        return const_iterator{ this->sequence.data() };
    }

    constexpr const_iterator end() const noexcept
    {
        return const_iterator{ this->sequence.data() + this->size() };
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return this->begin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return this->end();
    }

    constexpr bool operator==(const codepoint& rhs) const noexcept
    {
        const uint64_t size_l = this->size();
        if (size_l != rhs.size())
            return false;
        for (uint64_t i = 0; i < size_l; ++i)
            if (this->sequence[i] != rhs.sequence[i])
                return false;
        return true;
    }

    constexpr bool operator==(const skr_char8* rhs) const noexcept
    {
        for (uint64_t i = 0; i < this->size(); ++i)
            if (this->sequence[i] != rhs[i])
                return false;
        return true;
    }

    constexpr bool operator==(const char32_t rhs) const noexcept
    {
        return this->get_codepoint() == rhs;
    }

    constexpr bool operator!=(const codepoint& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    constexpr const skr_char8* raw() const noexcept
    {
        return this->sequence.data();
    }

    constexpr uint64_t size() const noexcept
    {
        for (uint64_t i = 0; i < unicode::UTF8_SEQUENCE_MAXIMUM_LENGTH; ++i)
            if (this->sequence[i] == 0)
                return i;
        return unicode::UTF8_SEQUENCE_MAXIMUM_LENGTH;
    }

    constexpr char32_t get_codepoint() const noexcept
    {
        return unicode::utf8_to_utf32(this->sequence.data(), this->size());
    }

    explicit constexpr operator char32_t() const noexcept
    {
        return this->get_codepoint();
    }

    std::array<skr_char8, unicode::UTF8_SEQUENCE_MAXIMUM_LENGTH> sequence;
};

constexpr bool operator==(const skr_char8* lhs, const codepoint& rhs) noexcept
{
    return rhs == lhs;
}
} // namespace skr::container

// literals
inline namespace literal
{
constexpr skr::container::codepoint operator""_cp(const skr::container::skr_char8 c) noexcept
{
    return skr::container::codepoint{ c };
}

constexpr skr::container::codepoint operator""_cp(const char32_t c) noexcept
{
    return skr::container::codepoint{ c };
}
} // namespace literal
