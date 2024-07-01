#pragma once
#include "SkrBase/config.h"
#ifdef __cplusplus
    #include "SkrBase/misc/hash.hpp"
    #include "SkrBase/misc/debug.h"
    #include <initializer_list>
#endif

typedef struct skr_guid_t {
#ifdef __cplusplus
    SKR_CONSTEXPR skr_guid_t() = default;
    SKR_CONSTEXPR skr_guid_t(uint32_t b0, uint16_t b1, uint16_t b2, const uint8_t b3s[8])
        : storage0(b0)
        , storage1((uint32_t)b1 << 16 | (uint32_t)b2)
        ,
    #if SKR_IS_LITTLE_ENDIAN
        storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24)
        , storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
    #else
        storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24)
        , storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
    #endif
    {
    }
    SKR_CONSTEXPR skr_guid_t(uint32_t b0, uint16_t b1, uint16_t b2, std::initializer_list<uint8_t> b3s_l)
        : storage0(b0)
        , storage1((uint32_t)b1 << 16 | (uint32_t)b2)
        ,
    #define b3s (b3s_l).begin()
    #if SKR_IS_LITTLE_ENDIAN
        storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24)
        , storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
    #else
        storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24)
        , storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
    #endif
    #undef b3s
    {
    }
    SKR_CONSTEXPR bool     is_zero() const { return !(storage0 && storage1 && storage2 && storage3); }
    SKR_CONSTEXPR uint32_t data1() const { return storage0; }
    SKR_CONSTEXPR uint16_t data2() const { return (uint16_t)(storage1 >> 16); }
    SKR_CONSTEXPR uint16_t data3() const { return (uint16_t)(storage1 & UINT16_MAX); }
    SKR_CONSTEXPR uint16_t data4(uint8_t idx0_7) const { return ((uint8_t*)&storage2)[idx0_7]; }

    static skr_guid_t Create();

    SKR_INLINE constexpr size_t get_hash() const
    {
        using namespace skr;
        constexpr Hash<uint64_t> hasher{};

        size_t result = hasher.operator()(static_cast<uint64_t>(storage0) << 32 | storage1);
        return hash_combine(result, hasher.operator()(static_cast<uint64_t>(storage2) << 32 | storage3));
    }

    // for skr::Hash
    SKR_INLINE static size_t _skr_hash(const skr_guid_t& guid)
    {
        return guid.get_hash();
    }

#endif
    uint32_t storage0 SKR_IF_CPP(= 0);
    uint32_t storage1 SKR_IF_CPP(= 0);
    uint32_t storage2 SKR_IF_CPP(= 0);
    uint32_t storage3 SKR_IF_CPP(= 0);
} skr_guid_t;

SKR_EXTERN_C void skr_make_guid(skr_guid_t* out_guid);

#ifdef __cplusplus
namespace skr
{
using GUID = skr_guid_t;
namespace literals {
namespace details {

constexpr const size_t short_guid_form_length = 36; // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
constexpr const size_t long_guid_form_length  = 38; // {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

constexpr int parse_hex_digit(const char8_t c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    else
        SKR_ASSERT(0 && "invalid character in GUID");
    return -1;
}

template <class T>
constexpr T parse_hex(const char8_t* ptr)
{
    constexpr size_t digits = sizeof(T) * 2;
    T                result{};
    for (size_t i = 0; i < digits; ++i)
        result |= parse_hex_digit(ptr[i]) << (4 * (digits - i - 1));
    return result;
}

constexpr skr_guid_t make_guid_helper(const char8_t* begin)
{
    auto Data1 = parse_hex<uint32_t>(begin);
    begin += 8 + 1;
    auto Data2 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    auto Data3 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    uint8_t Data4[8] = {};
    Data4[0]         = parse_hex<uint8_t>(begin);
    begin += 2;
    Data4[1] = parse_hex<uint8_t>(begin);
    begin += 2 + 1;
    for (size_t i = 0; i < 6; ++i)
        Data4[i + 2] = parse_hex<uint8_t>(begin + i * 2);
    return skr_guid_t(Data1, Data2, Data3, Data4);
}
} // namespace details

constexpr skr_guid_t operator""_guid(const char8_t* str, size_t N)
{
    using namespace details;
    if (!(N == long_guid_form_length || N == short_guid_form_length))
        SKR_ASSERT(0 && "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");
    if (N == long_guid_form_length && (str[0] != '{' || str[long_guid_form_length - 1] != '}'))
        SKR_ASSERT(0 && "Missing opening or closing brace");

    return make_guid_helper(str + (N == long_guid_form_length ? 1 : 0));
}
} // namespace literals
} // namespace skr

#define SKR_CONSTEXPR_GUID(__GUID) []() constexpr {  \
    using namespace skr::literals;             \
    constexpr skr_guid_t result = u8##__GUID##_guid; \
    return result;                                   \
}()

inline skr_guid_t skr_guid_t::Create()
{
    skr_guid_t guid;
    skr_make_guid(&guid);
    return guid;
}

inline SKR_CONSTEXPR bool operator==(skr_guid_t a, skr_guid_t b)
{
    int result = true;
    result &= (a.storage0 == b.storage0);
    result &= (a.storage0 == b.storage0);
    result &= (a.storage0 == b.storage0);
    result &= (a.storage0 == b.storage0);
    return result;
}

inline SKR_CONSTEXPR bool operator!=(skr_guid_t a, skr_guid_t b)
{
    return !(a == b);
}
#endif
