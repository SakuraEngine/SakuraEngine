#pragma once
#include <SkrBase/config.h>
#include <SkrBase/misc/hash.hpp>
#include <SkrBase/misc/debug.h>
#include <initializer_list>
#include <SkrBase/misc/data_encode.hpp>
#include <SkrBase/memory.hpp>
#include <SkrBase/containers/misc/optional.hpp>

// guid
namespace skr
{
struct GUID
{
    using OptGUID = container::Optional<GUID>;

    // style enum
    enum class EStyle
    {
        Base64,       // like: XXXXXXXXXXXXXXXXXXXXX
        Base64Padded, // like: XXXXXXXXXXXXXXXXXXXXX==
        Hex,          // like: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        HexSpec,      // like: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
        HexSpecBrace  // like: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    };

    // style length
    inline static constexpr uint64_t kBase64Length       = 22; // like: XXXXXXXXXXXXXXXXXXXXXXXX
    inline static constexpr uint64_t kBase64PaddedLength = 24; // like: XXXXXXXXXXXXXXXXXXXXXXXX
    inline static constexpr uint64_t kHexLength          = 32; // like: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    inline static constexpr uint64_t kHexSpecLength      = 36; // like: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    inline static constexpr uint64_t kHexSpecBraceLength = 38; // like: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    inline static constexpr uint64_t kMaxEncodedLength   = kHexSpecBraceLength;

    // size from style
    static constexpr uint64_t EncodeLen(EStyle style);

    // ctors
    constexpr GUID();
    constexpr GUID(uint32_t time_low, uint16_t time_mid, uint16_t time_high_and_version, const uint8_t clock_and_nodes[8]);
    constexpr GUID(uint32_t time_low, uint16_t time_mid, uint16_t time_high_and_version, std::initializer_list<uint8_t> clock_and_nodes);

    // factories
    static GUID           Create(); // create a new random GUID
    constexpr static GUID Zero();
    static OptGUID        Decode(const skr_char8* str, EStyle style);
    static OptGUID        Decode(const skr_char8* str, uint64_t len, EStyle style);
    static OptGUID        DecodeAuto(const skr_char8* str);
    static OptGUID        DecodeAuto(const skr_char8* str, uint64_t len);

    // copy & move
    constexpr GUID(const GUID&);
    constexpr GUID(GUID&&);

    // assign & move assign
    constexpr GUID& operator=(const GUID&);
    constexpr GUID& operator=(GUID&&);

    // validate
    constexpr bool is_zero() const;

    // RFC 4122, see https://www.rfc-editor.org/rfc/rfc4122
    // [time-low]-[time-mid]-[time-high-and-version]-[clock-seq-and-reserved & clock-seq-low]-[node]
    // time-low = 32位 unsigned integer
    // time-mid = 16位 unsigned integer
    // time-high-and-version = 16位 unsigned integer(时间戳高位部分与版本(Version)号混合)
    // clock-seq-and-reserved = 8位 unsigned integer(时钟序列高位部分与预定义变量(Variant)混合组成)
    // clock-seq-low = 8位 unsigned integer
    // node = 48位 unsigned integer
    constexpr uint32_t time_low() const;
    constexpr uint16_t time_mid() const;
    constexpr uint16_t time_high_and_version() const;
    constexpr uint8_t  clock_seq_and_reserved() const;
    constexpr uint8_t  clock_seq_low() const;
    constexpr uint8_t  node(uint8_t idx0_5) const;

    // compare
    constexpr bool operator==(const GUID& rhs) const;
    constexpr bool operator!=(const GUID& rhs) const;
    constexpr bool operator<(const GUID& rhs) const;
    constexpr bool operator>(const GUID& rhs) const;
    constexpr bool operator<=(const GUID& rhs) const;
    constexpr bool operator>=(const GUID& rhs) const;

    // hash
    constexpr skr_hash get_hash() const;

    // consteval parse
    static consteval GUID ParseConsteval(const skr_char8* str, size_t len);

    // encode
    uint64_t   encode_length(EStyle style) const;
    skr_char8* encode(skr_char8* out_str, EStyle style = EStyle::HexSpec) const;
    skr_char8* encode_base64(
        skr_char8* out_str,
        bool       with_padding = false
    ) const;
    skr_char8* encode_hex(
        skr_char8* out_str,
        bool       lower = true
    ) const;
    skr_char8* encode_spec(
        skr_char8* out_str,
        skr_char8  spec  = '-',
        bool       lower = true
    ) const;
    skr_char8* encode_spec_brace(
        skr_char8* out_str,
        skr_char8  spec        = '-',
        skr_char8  left_brace  = u8'{',
        skr_char8  right_brace = u8'}',
        bool       lower       = true
    ) const;

private:
    // helpers
    static constexpr uint32_t _combine_16(uint16_t a, uint16_t b);
    static constexpr uint32_t _combine_8(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    static constexpr GUID     _decode_hex(const skr_char8* str, size_t len, bool with_spec = false);

private:
    uint32_t _storage0 = 0;
    uint32_t _storage1 = 0;
    uint32_t _storage2 = 0;
    uint32_t _storage3 = 0;
};
} // namespace skr

// guid impl
namespace skr
{
// helpers
inline constexpr uint32_t GUID::_combine_16(uint16_t a, uint16_t b)
{
    if constexpr (endian::native == endian::little)
    {
        return a | b << 16;
    }
    else
    {
        return a << 16 | b;
    }
}
inline constexpr uint32_t GUID::_combine_8(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    if constexpr (endian::native == endian::little)
    {
        return a | b << 8 | c << 16 | d << 24;
    }
    else
    {
        return a << 24 | b << 16 | c << 8 | d;
    }
}
inline constexpr GUID GUID::_decode_hex(const skr_char8* str, size_t len, bool with_spec)
{
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t  data4[8];
    bool     success;

    // 8 ch
    success = hex_decode<uint32_t>(str, 8, data1);
    if (!success) [[unlikely]]
    {
        return {};
    }
    str += 8;

    if (with_spec) str += 1; // spec

    // 4 ch
    success = hex_decode<uint16_t>(str, 4, data2);
    if (!success) [[unlikely]]
    {
        return {};
    }
    str += 4;

    if (with_spec) str += 1; // spec

    // 4 ch
    success = hex_decode<uint16_t>(str, 4, data3);
    if (!success) [[unlikely]]
    {
        return {};
    }
    str += 4;

    if (with_spec) str += 1; // spec

    // 4 ch
    success = hex_decode<uint8_t>(str, 2, data4[0]);
    if (!success) [[unlikely]]
    {
        return {};
    }
    str += 2;
    success = hex_decode<uint8_t>(str, 2, data4[1]);
    if (!success) [[unlikely]]
    {
        return {};
    }
    str += 2;

    if (with_spec) str += 1; // spec

    // 12 ch
    for (size_t i = 0; i < 6; ++i)
    {
        success = hex_decode<uint8_t>(str + i * 2, 2, data4[i + 2]);
        if (!success) [[unlikely]]
        {
            return {};
        }
    }
    return GUID(data1, data2, data3, data4);
}

// size from style
inline constexpr uint64_t GUID::EncodeLen(EStyle style)
{
    switch (style)
    {
    case EStyle::Base64:
        return kBase64Length;
    case EStyle::Base64Padded:
        return kBase64PaddedLength;
    case EStyle::Hex:
        return kHexLength;
    case EStyle::HexSpec:
        return kHexSpecLength;
    case EStyle::HexSpecBrace:
        return kHexSpecBraceLength;
    default:
        return 0;
    }
}

// ctors
inline constexpr GUID::GUID() = default;
inline constexpr GUID::GUID(uint32_t time_low, uint16_t time_mid, uint16_t time_high_and_version, const uint8_t clock_and_nodes[8])
    : _storage0(time_low)
    , _storage1(_combine_16(time_mid, time_high_and_version))
    , _storage2(_combine_8(clock_and_nodes[0], clock_and_nodes[1], clock_and_nodes[2], clock_and_nodes[3]))
    , _storage3(_combine_8(clock_and_nodes[4], clock_and_nodes[5], clock_and_nodes[6], clock_and_nodes[7]))
{
}
inline constexpr GUID::GUID(uint32_t time_low, uint16_t time_mid, uint16_t time_high_and_version, std::initializer_list<uint8_t> clock_and_nodes)
    : _storage0(time_low)
    , _storage1(_combine_16(time_mid, time_high_and_version))
    , _storage2(_combine_8(clock_and_nodes.begin()[0], clock_and_nodes.begin()[1], clock_and_nodes.begin()[2], clock_and_nodes.begin()[3]))
    , _storage3(_combine_8(clock_and_nodes.begin()[4], clock_and_nodes.begin()[5], clock_and_nodes.begin()[6], clock_and_nodes.begin()[7]))
{
}

// factories
inline constexpr GUID GUID::Zero()
{
    return {};
}
inline GUID::OptGUID GUID::Decode(const skr_char8* str, EStyle style)
{
    return Decode(str, std::char_traits<char8_t>::length(str), style);
}
inline GUID::OptGUID GUID::Decode(const skr_char8* str, uint64_t len, EStyle style)
{
    // check length
    if (len != EncodeLen(style)) [[unlikely]]
    {
        return {};
    }

    // decode
    GUID result;
    switch (style)
    {
    case EStyle::Base64:
    case EStyle::Base64Padded: {
        // decode
        bool success = base64_decode(
            &result,
            sizeof(result),
            str,
            len
        );
        if (!success) [[unlikely]]
        {
            return {};
        }

        // handle byte order
        result._storage0 = byteswap(result._storage0);
        uint16_t* p16    = (uint16_t*)&result._storage1;
        p16[0]           = byteswap(p16[0]);
        p16[1]           = byteswap(p16[1]);
    }
    break;
    case EStyle::Hex: {
        result = _decode_hex(str, len);
    }
    break;
    case EStyle::HexSpecBrace: {
        str += 1; // skip '{'
        len -= 2; // skip '{' and '}'
    }
    case EStyle::HexSpec: {
        result = _decode_hex(str, len, true);
    }
    break;
    }
    return result;
}
inline GUID::OptGUID GUID::DecodeAuto(const skr_char8* str)
{
    return DecodeAuto(str, std::char_traits<char8_t>::length(str));
}
inline GUID::OptGUID GUID::DecodeAuto(const skr_char8* str, uint64_t len)
{
    switch (len)
    {
    case kBase64Length:
        return Decode(str, len, EStyle::Base64);
    case kBase64PaddedLength:
        return Decode(str, len, EStyle::Base64Padded);
    case kHexLength:
        return Decode(str, len, EStyle::Hex);
    case kHexSpecLength:
        return Decode(str, len, EStyle::HexSpec);
    case kHexSpecBraceLength:
        return Decode(str, len, EStyle::HexSpecBrace);
    default:
        [[unlikely]] return {};
    }
}

// copy & move
inline constexpr GUID::GUID(const GUID&) = default;
inline constexpr GUID::GUID(GUID&&)      = default;

// assign & move assign
inline constexpr GUID& GUID::operator=(const GUID&) = default;
inline constexpr GUID& GUID::operator=(GUID&&)      = default;

// validate
inline constexpr bool GUID::is_zero() const { return !(_storage0 && _storage1 && _storage2 && _storage3); }

// RFC 4122
inline constexpr uint32_t GUID::time_low() const { return _storage0; }
inline constexpr uint16_t GUID::time_mid() const { return ((uint16_t*)&_storage1)[0]; }
inline constexpr uint16_t GUID::time_high_and_version() const { return ((uint16_t*)&_storage1)[1]; }
inline constexpr uint8_t  GUID::clock_seq_and_reserved() const { return ((uint8_t*)&_storage2)[0]; }
inline constexpr uint8_t  GUID::clock_seq_low() const { return ((uint8_t*)&_storage2)[1]; }
inline constexpr uint8_t  GUID::node(uint8_t idx0_5) const
{
    SKR_ASSERT(idx0_5 < 6 && "node index out of range");
    return ((uint8_t*)&_storage2)[2 + idx0_5];
}

// compare
inline constexpr bool GUID::operator==(const GUID& rhs) const
{
    return _storage0 == rhs._storage0 &&
        _storage1 == rhs._storage1 &&
        _storage2 == rhs._storage2 &&
        _storage3 == rhs._storage3;
}
inline constexpr bool GUID::operator!=(const GUID& rhs) const
{
    return !(*this == rhs);
}
inline constexpr bool GUID::operator<(const GUID& rhs) const
{
    if (_storage0 != rhs._storage0) return _storage0 < rhs._storage0;
    if (_storage1 != rhs._storage1) return _storage1 < rhs._storage1;
    if (_storage2 != rhs._storage2) return _storage2 < rhs._storage2;
    return _storage3 < rhs._storage3;
}
inline constexpr bool GUID::operator>(const GUID& rhs) const
{
    if (_storage0 != rhs._storage0) return _storage0 > rhs._storage0;
    if (_storage1 != rhs._storage1) return _storage1 > rhs._storage1;
    if (_storage2 != rhs._storage2) return _storage2 > rhs._storage2;
    return _storage3 > rhs._storage3;
}
inline constexpr bool GUID::operator<=(const GUID& rhs) const
{
    return !(*this > rhs);
}
inline constexpr bool GUID::operator>=(const GUID& rhs) const
{
    return !(*this < rhs);
}

// hash
inline constexpr skr_hash GUID::get_hash() const
{
    using namespace skr;
    constexpr Hash<uint64_t> hasher{};

    skr_hash result = hasher.operator()(static_cast<uint64_t>(_storage0) << 32 | _storage1);
    return hash_combine(result, hasher.operator()(static_cast<uint64_t>(_storage2) << 32 | _storage3));
}

// consteval parse
inline consteval GUID GUID::ParseConsteval(const skr_char8* str, size_t len)
{
    if (len == GUID::kHexLength)
    {
        return _decode_hex(str, len);
    }
    else if (len == GUID::kHexSpecBraceLength)
    {
        return _decode_hex(str + 1, len - 2, true);
    }
    else if (len == GUID::kHexSpecLength)
    {
        return _decode_hex(str, len, true);
    }
    else
    {
        // make some noise to break compilation
        SKR_UNREACHABLE_CODE();
        return GUID::Zero();
    }
}

// encode
inline uint64_t GUID::encode_length(EStyle style) const
{
    return EncodeLen(style);
}
inline skr_char8* GUID::encode(skr_char8* out_str, EStyle style) const
{
    switch (style)
    {
    case EStyle::Base64:
        return encode_base64(out_str, false);
    case EStyle::Base64Padded:
        return encode_base64(out_str, true);
    case EStyle::Hex:
        return encode_hex(out_str);
    case EStyle::HexSpec:
        return encode_spec(out_str);
    case EStyle::HexSpecBrace:
        return encode_spec_brace(out_str);
    default:
        return nullptr;
    }
}
inline skr_char8* GUID::encode_base64(
    skr_char8* out_str,
    bool       with_padding
) const
{

    auto encode_len = with_padding ? kBase64PaddedLength : kBase64Length;

    uint32_t data[4] = {
        byteswap(_storage0),
        _storage1,
        _storage2,
        _storage3
    };
    auto* p16 = (uint16_t*)&data[1];
    p16[0]    = byteswap(p16[0]);
    p16[1]    = byteswap(p16[1]);

    base64_encode(
        out_str,
        data,
        sizeof(*this),
        with_padding
    );

    return out_str + encode_len;
}
inline skr_char8* GUID::encode_hex(
    skr_char8* out_str,
    bool       lower
) const
{
    // 8 ch
    hex_encode<uint32_t>(time_low(), out_str, 8, lower);
    out_str += 8;

    // 4 ch
    hex_encode<uint16_t>(time_mid(), out_str, 4, lower);
    out_str += 4;

    // 4 ch
    hex_encode<uint16_t>(time_high_and_version(), out_str, 4, lower);
    out_str += 4;

    // 4 ch
    hex_encode<uint8_t>(clock_seq_and_reserved(), out_str, 2, lower);
    out_str += 2;
    hex_encode<uint8_t>(clock_seq_low(), out_str, 2, lower);
    out_str += 2;

    // 12 ch
    for (size_t i = 0; i < 6; ++i)
        hex_encode<uint8_t>(node(i), out_str + i * 2, 2, lower);
    out_str += 12;

    return out_str;
}
inline skr_char8* GUID::encode_spec(
    skr_char8* out_str,
    skr_char8  spec,
    bool       lower
) const
{
    // 8 ch
    hex_encode<uint32_t>(time_low(), out_str, 8, lower);
    out_str += 8;

    // spec
    *out_str = spec;
    out_str += 1;

    // 4 ch
    hex_encode<uint16_t>(time_mid(), out_str, 4, lower);
    out_str += 4;
    *out_str = spec;

    // spec
    *out_str = spec;
    out_str += 1;

    // 4 ch
    hex_encode<uint16_t>(time_high_and_version(), out_str, 4, lower);
    out_str += 4;

    // spec
    *out_str = spec;
    out_str += 1;

    // 4 ch
    hex_encode<uint8_t>(clock_seq_and_reserved(), out_str, 2, lower);
    out_str += 2;
    hex_encode<uint8_t>(clock_seq_low(), out_str, 2, lower);
    out_str += 2;

    // spec
    *out_str = spec;
    out_str += 1;

    // 12 ch
    for (size_t i = 0; i < 6; ++i)
        hex_encode<uint8_t>(node(i), out_str + i * 2, 2, lower);
    out_str += 12;

    return out_str;
}
inline skr_char8* GUID::encode_spec_brace(
    skr_char8* out_str,
    skr_char8  spec,
    skr_char8  left_brace,
    skr_char8  right_brace,
    bool       lower
) const
{
    // brace
    *out_str = left_brace;
    out_str += 1;

    // spec
    out_str = encode_spec(out_str, spec, lower);

    // brace
    *out_str = right_brace;
    out_str += 1;

    return out_str;
}
} // namespace skr

// hasher
namespace skr
{
template <>
struct Hash<GUID>
{
    inline skr_hash operator()(const GUID& guid) const
    {
        return guid.get_hash();
    }
};
} // namespace skr

// memory traits for fast memory ops
namespace skr::memory
{
template <>
struct MemoryTraits<GUID, GUID> : MemoryTraitsPOD
{
};
} // namespace skr::memory

// literals
namespace skr
{
inline namespace literals
{
consteval GUID operator""_guid(const skr_char8* str, size_t N)
{
    return GUID::ParseConsteval(str, N);
}
} // namespace literals
} // namespace skr
