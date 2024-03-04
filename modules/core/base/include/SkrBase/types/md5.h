#pragma once
#include "SkrBase/config.h"

#define SKR_MD5_DIGEST_LENGTH 128 / 8
typedef struct skr_md5_t {
    uint8_t digest[SKR_MD5_DIGEST_LENGTH];
} skr_md5_t;

typedef struct skr_md5_u32x4_view_t {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} skr_md5_u32x4_view_t;

SKR_EXTERN_C bool skr_parse_md5(const char8_t* str32, skr_md5_t* out_md5);
SKR_EXTERN_C void skr_make_md5(const char8_t* str, uint32_t str_size, skr_md5_t* out_md5);
