#pragma once
#include "SkrBase/config.h"
#ifdef __cplusplus
    #include "SkrBase/misc/hash.hpp"
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
}
inline skr_guid_t skr_guid_t::Create()
{
    skr_guid_t guid;
    skr_make_guid(&guid);
    return guid;
}
#endif
