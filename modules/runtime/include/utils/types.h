#pragma once
#include "platform/configure.h"

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

typedef struct SKR_ALIGNAS(16) skr_float4x4_t {
    float M[4][4];
} skr_float4x4_t;

typedef struct skr_blob_t {
    uint8_t* bytes SKR_IF_CPP( = nullptr);
    uint64_t size SKR_IF_CPP( = 0u);
} skr_blob_t;