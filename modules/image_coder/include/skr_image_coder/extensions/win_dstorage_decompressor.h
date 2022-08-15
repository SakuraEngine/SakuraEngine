#pragma once
#include "skr_image_coder/skr_image_coder.h"
#include "cgpu/extensions/dstorage_windows.h"

#define SKR_WIN_DSTORAGE_COMPRESSION_TYPE_IMAGE CGPU_DSTORAGE_COMPRESSION_CUSTOM + 1

SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
HRESULT skr_image_coder_win_dstorage_decompressor(skr_win_dstorage_decompress_request_t* request, void* user_data);