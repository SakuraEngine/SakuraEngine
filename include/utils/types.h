#pragma once
#include "platform/configure.h"

typedef struct skr_float3_t {
    struct
    {
        float x;
        float y;
        float z;
    };
} skr_float3_t;

typedef struct skr_float4_t {
    SKR_ALIGNAS(16) struct
    {
        float x;
        float y;
        float z;
        float w;
    };
} skr_float4_t;

typedef struct skr_blob_t {
    uint8_t* bytes;
    uint64_t size;
} skr_blob_t;