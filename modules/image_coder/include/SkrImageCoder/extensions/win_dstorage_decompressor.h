#pragma once
#include "SkrImageCoder/skr_image_coder.h"
#include "SkrRT/platform/win/dstorage_windows.h"

SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
HRESULT skr_image_coder_win_dstorage_decompressor(skr_win_dstorage_decompress_request_t* request, void* user_data);