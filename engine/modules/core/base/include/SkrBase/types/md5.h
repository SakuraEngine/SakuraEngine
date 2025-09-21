#pragma once
#include "SkrBase/config.h"

// select MD5 implementation
#ifdef __cplusplus
    #include "./impl/md5.hpp"
using skr_md5_t = skr::MD5;
#else
typedef struct skr_md5_t
{
    uint8_t digest[128 / 8];
} skr_md5_t;
#endif

SKR_EXTERN_C bool skr_parse_md5(const char8_t* str32, skr_md5_t* out_md5);
SKR_EXTERN_C void skr_make_md5(const char8_t* str, uint32_t str_size, skr_md5_t* out_md5);
