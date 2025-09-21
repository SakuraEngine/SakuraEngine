#pragma once
#include "SkrBase/config.h"

// select sha256 implementation
#ifdef __cplusplus
    #include "./impl/sha256.hpp"
using skr_sha256_t = skr::SHA256;
#else
typedef struct skr_sha256_t
{
    uint8_t digest[256 / 8];
} skr_sha256_t;
#endif
