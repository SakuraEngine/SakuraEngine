#pragma once
#include "SkrRT/config.h"
#include "SkrRT/misc/types.h"

SKR_EXTERN_C void skr_crypt_make_md5(const char8_t* str, uint32_t str_size, skr_md5_t* out_md5);
