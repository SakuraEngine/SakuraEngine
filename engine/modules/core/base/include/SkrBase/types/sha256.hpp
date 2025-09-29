#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/data_encode.hpp"
#include "SkrBase/misc/hash.hpp"
#include <SkrBase/memory.hpp>
#include <SkrBase/containers/misc/optional.hpp>

// SHA256
namespace skr
{
struct SHA256
{
    using OptSHA256 = container::Optional<SHA256>;

    // SHA256 size
    inline static constexpr uint64_t kBitSize  = 256;
    inline static constexpr uint64_t kByteSize = kBitSize / 8;

    // style enum
    enum class EStyle
    {
        Base64,
        Base64Padded,
        Hex,
    };

    // style length
    inline static constexpr uint64_t kBase64Length       = 43;
    inline static constexpr uint64_t kBase64PaddedLength = 44;
    inline static constexpr uint64_t kHexLength          = 64;
    inline static constexpr uint64_t kMaxEncodedLength   = kHexLength;

    // size from style
    static constexpr uint64_t EncodeLen(EStyle style);

    // ctor
    SHA256();

    // copy & move
    SHA256(const SHA256& rhs);
    SHA256(SHA256&& rhs);

    // assign & move assign
    SHA256& operator=(const SHA256& rhs);
    SHA256& operator=(SHA256&& rhs);

    // factories
    static SHA256    Zero();
    static SHA256    Build(const void* data, uint64_t data_len);
    static OptSHA256 Decode(const skr_char8* str, EStyle style);
    static OptSHA256 Decode(const skr_char8* str, uint64_t len, EStyle style);
    static OptSHA256 DecodeAuto(const skr_char8* str);
    static OptSHA256 DecodeAuto(const skr_char8* str, uint64_t len);

    // compare
    bool operator==(const SHA256& rhs) const;
    bool operator!=(const SHA256& rhs) const;
    bool operator<(const SHA256& rhs) const;
    bool operator>(const SHA256& rhs) const;
    bool operator<=(const SHA256& rhs) const;
    bool operator>=(const SHA256& rhs) const;

    // visitor
    uint8_t         at_byte(uint32_t idx) const;
    uint8_t         at_u8(uint32_t idx) const;
    uint16_t        at_u16(uint32_t idx) const;
    uint32_t        at_u32(uint32_t idx) const;
    uint64_t        at_u64(uint32_t idx) const;
    const uint8_t*  as_byte() const;
    const uint8_t*  as_u8() const;
    const uint16_t* as_u16() const;
    const uint32_t* as_u32() const;
    uint8_t*        as_byte();
    uint8_t*        as_u8();
    uint16_t*       as_u16();
    uint32_t*       as_u32();

    // hash
    skr_hash get_hash() const;

    // validation
    bool is_zero() const;

    // encode
    uint64_t   encode_length(EStyle style) const;
    skr_char8* encode(skr_char8* out_str, EStyle style = EStyle::Hex) const;
    skr_char8* encode_base64(
        skr_char8* out_str,
        bool       with_padding = false
    ) const;
    skr_char8* encode_hex(
        skr_char8* out_str,
        bool       lower = true
    ) const;

private:
    // storage
    union
    {
        uint8_t  _bytes[kByteSize];
        uint8_t  _u8s[kByteSize];
        uint16_t _u16s[kByteSize / 2];
        uint32_t _u32s[kByteSize / 4];
        struct
        {
            uint32_t _a, _b, _c, _d,
                _e, _f, _g, _h;
        };
    };
};

struct SHA256Builder
{
    // build step
    void   init();
    void   update(const void* str, uint64_t str_size);
    SHA256 finalize(bool reset = true);

private:
    uint64_t payload[112 / 8];
};
} // namespace skr

// SHA256 impl
namespace skr
{
// size from style
inline constexpr uint64_t SHA256::EncodeLen(EStyle style)
{
    switch (style)
    {
    case EStyle::Base64:
        return kBase64Length;
    case EStyle::Base64Padded:
        return kBase64PaddedLength;
    case EStyle::Hex:
        return kHexLength;
    default:
        return 0;
    }
}

// ctor
inline SHA256::SHA256()
    : _a(0)
    , _b(0)
    , _c(0)
    , _d(0)
    , _e(0)
    , _f(0)
    , _g(0)
    , _h(0)
{
}

// copy & move
inline SHA256::SHA256(const SHA256& rhs)
    : _a(rhs._a)
    , _b(rhs._b)
    , _c(rhs._c)
    , _d(rhs._d)
    , _e(rhs._e)
    , _f(rhs._f)
    , _g(rhs._g)
    , _h(rhs._h)
{
}
inline SHA256::SHA256(SHA256&& rhs)
    : _a(rhs._a)
    , _b(rhs._b)
    , _c(rhs._c)
    , _d(rhs._d)
    , _e(rhs._e)
    , _f(rhs._f)
    , _g(rhs._g)
    , _h(rhs._h)
{
}

// assign & move assign
inline SHA256& SHA256::operator=(const SHA256& rhs)
{
    _a = rhs._a;
    _b = rhs._b;
    _c = rhs._c;
    _d = rhs._d;
    _e = rhs._e;
    _f = rhs._f;
    _g = rhs._g;
    _h = rhs._h;
    return *this;
}
inline SHA256& SHA256::operator=(SHA256&& rhs)
{
    _a = rhs._a;
    _b = rhs._b;
    _c = rhs._c;
    _d = rhs._d;
    _e = rhs._e;
    _f = rhs._f;
    _g = rhs._g;
    _h = rhs._h;
    return *this;
}

// factories
inline SHA256 SHA256::Zero()
{
    return {};
}
inline SHA256 SHA256::Build(const void* data, uint64_t data_len)
{
    SHA256Builder builder;
    builder.init();
    builder.update(data, data_len);
    return builder.finalize();
}
inline SHA256::OptSHA256 SHA256::Decode(const skr_char8* str, EStyle style)
{
    return Decode(str, std::char_traits<char8_t>::length(str), style);
}
inline SHA256::OptSHA256 SHA256::Decode(const skr_char8* str, uint64_t len, EStyle style)
{
    // check length
    if (len != EncodeLen(style)) [[unlikely]]
    {
        return {};
    }

    // decode
    SHA256 result;
    switch (style)
    {
    case EStyle::Base64:
    case EStyle::Base64Padded: {
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
    }
    break;
    case EStyle::Hex: {
        bool success = base16_decode(
            &result,
            sizeof(result),
            str,
            len
        );
        if (!success) [[unlikely]]
        {
            return {};
        }
    }
    break;
    }
    return result;
}
inline SHA256::OptSHA256 SHA256::DecodeAuto(const skr_char8* str)
{
    return DecodeAuto(str, std::char_traits<char8_t>::length(str));
}
inline SHA256::OptSHA256 SHA256::DecodeAuto(const skr_char8* str, uint64_t len)
{
    switch (len)
    {
    case kBase64Length:
        return Decode(str, len, EStyle::Base64);
    case kBase64PaddedLength:
        return Decode(str, len, EStyle::Base64Padded);
    case kHexLength:
        return Decode(str, len, EStyle::Hex);
    default:
        [[unlikely]] return {};
    }
}

// compare
inline bool SHA256::operator==(const SHA256& rhs) const
{
    return (_a == rhs._a) && (_b == rhs._b) && (_c == rhs._c) && (_d == rhs._d) &&
        (_e == rhs._e) && (_f == rhs._f) && (_g == rhs._g) && (_h == rhs._h);
}
inline bool SHA256::operator!=(const SHA256& rhs) const
{
    return !(*this == rhs);
}
inline bool SHA256::operator<(const SHA256& rhs) const
{
    if (_a != rhs._a) return _a < rhs._a;
    if (_b != rhs._b) return _b < rhs._b;
    if (_c != rhs._c) return _c < rhs._c;
    if (_d != rhs._d) return _d < rhs._d;
    if (_e != rhs._e) return _e < rhs._e;
    if (_f != rhs._f) return _f < rhs._f;
    if (_g != rhs._g) return _g < rhs._g;
    return _h < rhs._h;
}
inline bool SHA256::operator>(const SHA256& rhs) const
{
    if (_a != rhs._a) return _a > rhs._a;
    if (_b != rhs._b) return _b > rhs._b;
    if (_c != rhs._c) return _c > rhs._c;
    if (_d != rhs._d) return _d > rhs._d;
    if (_e != rhs._e) return _e > rhs._e;
    if (_f != rhs._f) return _f > rhs._f;
    if (_g != rhs._g) return _g > rhs._g;
    return _h > rhs._h;
}
inline bool SHA256::operator<=(const SHA256& rhs) const
{
    return !(*this > rhs);
}
inline bool SHA256::operator>=(const SHA256& rhs) const
{
    return !(*this < rhs);
}

// visitor
inline uint8_t SHA256::at_byte(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize);
    return _bytes[idx];
}
inline uint8_t SHA256::at_u8(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize);
    return _bytes[idx];
}
inline uint16_t SHA256::at_u16(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize / 2);
    return _u16s[idx];
}
inline uint32_t SHA256::at_u32(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize / 4);
    return _u32s[idx];
}
inline uint64_t SHA256::at_u64(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize / 8);
    return static_cast<uint64_t>(_u32s[idx * 2]) | (static_cast<uint64_t>(_u32s[idx * 2 + 1]) << 32);
}
inline const uint8_t* SHA256::as_byte() const
{
    return _bytes;
}
inline const uint8_t* SHA256::as_u8() const
{
    return _u8s;
}
inline const uint16_t* SHA256::as_u16() const
{
    return _u16s;
}
inline const uint32_t* SHA256::as_u32() const
{
    return _u32s;
}
inline uint8_t* SHA256::as_byte()
{
    return _bytes;
}
inline uint8_t* SHA256::as_u8()
{
    return _u8s;
}
inline uint16_t* SHA256::as_u16()
{
    return _u16s;
}
inline uint32_t* SHA256::as_u32()
{
    return _u32s;
}

// hash
inline skr_hash SHA256::get_hash() const
{
    constexpr Hash<uint64_t> hasher{};

    skr_hash result = hasher.operator()(static_cast<uint64_t>(_a) << 32 | _b);
    result          = hash_combine(
        result,
        hasher.operator()(static_cast<uint64_t>(_c) << 32 | _d)
    );
    result = hash_combine(
        result,
        hasher.operator()(static_cast<uint64_t>(_e) << 32 | _f)
    );
    return hash_combine(
        result,
        hasher.operator()(static_cast<uint64_t>(_g) << 32 | _h)
    );
}

// validation
inline bool SHA256::is_zero() const
{
    return _a == 0 && _b == 0 && _c == 0 && _d == 0 &&
        _e == 0 && _f == 0 && _g == 0 && _h == 0;
}

// encode
inline uint64_t SHA256::encode_length(EStyle style) const
{
    return EncodeLen(style);
}
inline skr_char8* SHA256::encode(skr_char8* out_str, EStyle style) const
{
    switch (style)
    {
    case EStyle::Base64:
        return encode_base64(out_str, false);
    case EStyle::Base64Padded:
        return encode_base64(out_str, true);
    case EStyle::Hex:
        return encode_hex(out_str, true);
    default:
        return nullptr;
    }
}
inline skr_char8* SHA256::encode_base64(
    skr_char8* out_str,
    bool       with_padding
) const
{
    auto encode_len = with_padding ? kBase64PaddedLength : kBase64Length;

    base64_encode(
        out_str,
        this,
        sizeof(*this),
        with_padding
    );

    return out_str + encode_len;
}
inline skr_char8* SHA256::encode_hex(
    skr_char8* out_str,
    bool       lower
) const
{
    base16_encode(
        out_str,
        this,
        sizeof(*this),
        lower
    );
    return out_str + kHexLength;
}

} // namespace skr

// Hasher
namespace skr
{
template <>
struct Hash<SHA256>
{
    inline skr_hash operator()(const SHA256& md5) const
    {
        return md5.get_hash();
    }
};
} // namespace skr

// memory traits for fast memory ops
namespace skr::memory
{
template <>
struct MemoryTraits<skr::SHA256, skr::SHA256> : MemoryTraitsPOD
{
};
} // namespace skr::memory