#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/data_encode.hpp"
#include "SkrBase/misc/hash.hpp"
#include <SkrBase/memory.hpp>
#include <SkrBase/containers/misc/optional.hpp>

namespace skr
{
struct MD5
{
    using OptMD5 = container::Optional<MD5>;

    // MD5 size
    inline static constexpr uint64_t kBitSize  = 128;
    inline static constexpr uint64_t kByteSize = kBitSize / 8;

    // style enum
    enum class EStyle
    {
        Base64,       // like: XXXXXXXXXXXXXXXXXXXXX
        Base64Padded, // like: XXXXXXXXXXXXXXXXXXXXX==
        Hex,          // like: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    };

    // style length
    inline static constexpr uint64_t kBase64Length       = 22;
    inline static constexpr uint64_t kBase64PaddedLength = 24;
    inline static constexpr uint64_t kHexLength          = 32;
    inline static constexpr uint64_t kMaxEncodedLength   = kHexLength;

    // size from style
    static constexpr uint64_t EncodeLen(EStyle style);

    // ctor
    MD5();

    // copy & move
    MD5(const MD5& rhs);
    MD5(MD5&& rhs);

    // assign & move assign
    MD5& operator=(const MD5& rhs);
    MD5& operator=(MD5&& rhs);

    // factories
    static MD5    Zero();
    static MD5    Build(const void* data, uint64_t data_len);
    static OptMD5 Decode(const skr_char8* str, EStyle style);
    static OptMD5 Decode(const skr_char8* str, uint64_t len, EStyle style);
    static OptMD5 DecodeAuto(const skr_char8* str);
    static OptMD5 DecodeAuto(const skr_char8* str, uint64_t len);

    // compare
    bool operator==(const MD5& rhs) const;
    bool operator!=(const MD5& rhs) const;
    bool operator<(const MD5& rhs) const;
    bool operator>(const MD5& rhs) const;
    bool operator<=(const MD5& rhs) const;
    bool operator>=(const MD5& rhs) const;

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
            uint32_t _a, _b, _c, _d;
        };
    };
};

struct MD5Builder
{
    // build step
    void init();
    void update(const void* str, uint64_t str_size);
    MD5  finalize(bool reset = true);

private:
    uint32_t payload[152 / 4];
};
} // namespace skr

// MD5 impl
namespace skr
{
// size from style
inline constexpr uint64_t MD5::EncodeLen(EStyle style)
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
inline MD5::MD5()
    : _a(0)
    , _b(0)
    , _c(0)
    , _d(0)
{
}

// copy & move
inline MD5::MD5(const MD5& rhs)
    : _a(rhs._a)
    , _b(rhs._b)
    , _c(rhs._c)
    , _d(rhs._d)
{
}
inline MD5::MD5(MD5&& rhs)
    : _a(rhs._a)
    , _b(rhs._b)
    , _c(rhs._c)
    , _d(rhs._d)
{
}

// assign & move assign
inline MD5& MD5::operator=(const MD5& rhs)
{
    _a = rhs._a;
    _b = rhs._b;
    _c = rhs._c;
    _d = rhs._d;
    return *this;
}
inline MD5& MD5::operator=(MD5&& rhs)
{
    _a = rhs._a;
    _b = rhs._b;
    _c = rhs._c;
    _d = rhs._d;
    return *this;
}

// factories
inline MD5 MD5::Zero()
{
    return {};
}
inline MD5 MD5::Build(const void* data, uint64_t data_len)
{
    MD5Builder builder;
    builder.init();
    builder.update(data, data_len);
    return builder.finalize();
}
inline MD5::OptMD5 MD5::Decode(const skr_char8* str, EStyle style)
{
    return Decode(str, EncodeLen(style), style);
}
inline MD5::OptMD5 MD5::Decode(const skr_char8* str, uint64_t len, EStyle style)
{
    // check length
    if (len != EncodeLen(style)) [[unlikely]]
    {
        return {};
    }

    // decode
    MD5 result;
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
        bool success = skr::base16_decode(
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
inline MD5::OptMD5 MD5::DecodeAuto(const skr_char8* str)
{
    return DecodeAuto(str, std::char_traits<char8_t>::length(str));
}
inline MD5::OptMD5 MD5::DecodeAuto(const skr_char8* str, uint64_t len)
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
inline bool MD5::operator==(const MD5& rhs) const
{
    return _a == rhs._a &&
        _b == rhs._b &&
        _c == rhs._c &&
        _d == rhs._d;
}
inline bool MD5::operator!=(const MD5& rhs) const
{
    return !(*this == rhs);
}
inline bool MD5::operator<(const MD5& rhs) const
{
    if (_a != rhs._a) return _a < rhs._a;
    if (_b != rhs._b) return _b < rhs._b;
    if (_c != rhs._c) return _c < rhs._c;
    return _d < rhs._d;
}
inline bool MD5::operator>(const MD5& rhs) const
{
    if (_a != rhs._a) return _a > rhs._a;
    if (_b != rhs._b) return _b > rhs._b;
    if (_c != rhs._c) return _c > rhs._c;
    return _d > rhs._d;
}
inline bool MD5::operator<=(const MD5& rhs) const
{
    return !(*this > rhs);
}
inline bool MD5::operator>=(const MD5& rhs) const
{
    return !(*this < rhs);
}

// visitor
inline uint8_t MD5::at_byte(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize);
    return _bytes[idx];
}
inline uint8_t MD5::at_u8(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize);
    return _bytes[idx];
}
inline uint16_t MD5::at_u16(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize / 2);
    return _u16s[idx];
}
inline uint32_t MD5::at_u32(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize / 4);
    return _u32s[idx];
}
inline uint64_t MD5::at_u64(uint32_t idx) const
{
    SKR_ASSERT(idx < kByteSize / 8);
    return static_cast<uint64_t>(_u32s[idx * 2]) | (static_cast<uint64_t>(_u32s[idx * 2 + 1]) << 32);
}
inline const uint8_t* MD5::as_byte() const
{
    return _bytes;
}
inline const uint8_t* MD5::as_u8() const
{
    return _u8s;
}
inline const uint16_t* MD5::as_u16() const
{
    return _u16s;
}
inline const uint32_t* MD5::as_u32() const
{
    return _u32s;
}
inline uint8_t* MD5::as_byte()
{
    return _bytes;
}
inline uint8_t* MD5::as_u8()
{
    return _u8s;
}
inline uint16_t* MD5::as_u16()
{
    return _u16s;
}
inline uint32_t* MD5::as_u32()
{
    return _u32s;
}

// hash
inline skr_hash MD5::get_hash() const
{
    constexpr Hash<uint64_t> hasher{};

    skr_hash result = hasher.operator()(static_cast<uint64_t>(_a) << 32 | _b);
    return hash_combine(
        result,
        hasher.operator()(static_cast<uint64_t>(_c) << 32 | _d)
    );
}

// validation
inline bool MD5::is_zero() const
{
    return _a == 0 && _b == 0 && _c == 0 && _d == 0;
}

// encode
inline uint64_t MD5::encode_length(EStyle style) const
{
    return EncodeLen(style);
}
inline skr_char8* MD5::encode(skr_char8* out_str, EStyle style) const
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
inline skr_char8* MD5::encode_base64(
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
inline skr_char8* MD5::encode_hex(
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
struct Hash<MD5>
{
    inline skr_hash operator()(const MD5& md5) const
    {
        return md5.get_hash();
    }
};
} // namespace skr

// memory traits for fast memory ops
namespace skr::memory
{
template <>
struct MemoryTraits<MD5, MD5> : MemoryTraitsPOD
{
};
} // namespace skr::memory