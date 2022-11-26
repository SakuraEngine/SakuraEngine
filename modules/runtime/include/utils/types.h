#pragma once
#include "platform/configure.h"
#ifdef __cplusplus
#include <initializer_list>
#endif

typedef struct skr_guid_t {
#ifdef __cplusplus
    SKR_CONSTEXPR skr_guid_t() = default;
    SKR_CONSTEXPR skr_guid_t(uint32_t b0, uint16_t b1, uint16_t b2, const uint8_t b3s[8])
        : Storage0(b0), Storage1((uint32_t)b1 << 16 | (uint32_t)b2), 
#if SKR_IS_LITTLE_ENDIAN
        Storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24), 
        Storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
#else
        Storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24), 
        Storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
#endif
    {

    }
    SKR_CONSTEXPR skr_guid_t(uint32_t b0, uint16_t b1, uint16_t b2, std::initializer_list<uint8_t> b3s_l)
        : Storage0(b0), Storage1((uint32_t)b1 << 16 | (uint32_t)b2), 
#define b3s (b3s_l).begin()
    #if SKR_IS_LITTLE_ENDIAN
            Storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24), 
            Storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
    #else
            Storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24), 
            Storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
    #endif
#undef b3s
    {

    }
    SKR_CONSTEXPR bool isZero() const { return !(Storage0 && Storage1 && Storage2 && Storage3); }
    SKR_CONSTEXPR uint32_t Data1() const { return Storage0; }
    SKR_CONSTEXPR uint16_t Data2() const { return (uint16_t)(Storage1 >> 16); }
    SKR_CONSTEXPR uint16_t Data3() const { return (uint16_t)(Storage1 & UINT16_MAX); }
    SKR_CONSTEXPR uint16_t Data4(uint8_t idx0_7) const { return ((uint8_t*)&Storage2)[idx0_7]; }
#endif
    uint32_t Storage0 SKR_IF_CPP( = 0);
    uint32_t Storage1 SKR_IF_CPP( = 0);
    uint32_t Storage2 SKR_IF_CPP( = 0);
    uint32_t Storage3 SKR_IF_CPP( = 0);
} skr_guid_t;

typedef struct skr_md5_t {
    uint32_t a SKR_IF_CPP( = 0);
    uint32_t b SKR_IF_CPP( = 0);
    uint32_t c SKR_IF_CPP( = 0);
    uint32_t d SKR_IF_CPP( = 0);
} skr_md5_t;

RUNTIME_EXTERN_C RUNTIME_API bool skr_parse_md5(const char* str32, skr_md5_t* out_md5);
RUNTIME_EXTERN_C RUNTIME_API void skr_make_md5(const char* str, uint32_t str_size, skr_md5_t* out_md5);

extern const skr_guid_t $guid;

typedef struct skr_float2_t {
    struct
    {
        float x SKR_IF_CPP( = 0.f);
        float y SKR_IF_CPP( = 0.f);
    };
} skr_float2_t;

typedef struct skr_float3_t {
    struct
    {
        float x SKR_IF_CPP( = 0.f);
        float y SKR_IF_CPP( = 0.f);
        float z SKR_IF_CPP( = 0.f);
    };
} skr_float3_t;

typedef struct skr_rotator_t {
    struct
    {
        float pitch SKR_IF_CPP( = 0.f);
        float yaw SKR_IF_CPP( = 0.f);
        float roll SKR_IF_CPP( = 0.f);
    };
} skr_rotator_t;

typedef struct SKR_ALIGNAS(16) skr_float4_t {
    struct 
    {
        float x SKR_IF_CPP( = 0.f);
        float y SKR_IF_CPP( = 0.f);
        float z SKR_IF_CPP( = 0.f);
        float w SKR_IF_CPP( = 0.f);
    };
} skr_float4_t;

typedef struct SKR_ALIGNAS(16) skr_quaternion_t {
    struct 
    {
        float x SKR_IF_CPP( = 0.f);
        float y SKR_IF_CPP( = 0.f);
        float z SKR_IF_CPP( = 0.f);
        float w SKR_IF_CPP( = 0.f);
    };
} skr_quaternion_t;

typedef struct SKR_ALIGNAS(16) skr_float4x4_t {
    float M[4][4];
} skr_float4x4_t;

typedef struct skr_blob_t {
    uint8_t* bytes SKR_IF_CPP( = nullptr);
    uint64_t size SKR_IF_CPP( = 0u);
} skr_blob_t;

#ifdef __cplusplus
inline static SKR_CONSTEXPR bool operator==(skr_float2_t l, skr_float2_t r) 
{
    return (l.x == r.x) && (l.y == r.y);
}
inline static SKR_CONSTEXPR bool operator!=(skr_float2_t l, skr_float2_t r) 
{
    return (l.x != r.x) || (l.y != r.y);
}
inline SKR_CONSTEXPR bool operator==(skr_md5_t a, skr_md5_t b)
{
    int result = true;
    result &= (a.a == b.a);
    result &= (a.b == b.b);
    result &= (a.c == b.c);
    result &= (a.d == b.d);
    return result;
}
#endif