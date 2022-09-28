#pragma once
#include <EASTL/string.h>
#include "image_coder_base.hpp"

namespace skr
{
using turbo_jpeg_handle = void*;

class SKR_IMAGE_CODER_API JPEGImageCoder : public BaseImageCoder
{
public:
    JPEGImageCoder() SKR_NOEXCEPT;
    ~JPEGImageCoder() SKR_NOEXCEPT;

    bool set_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;
    bool move_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;
    bool view_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;

    EImageCoderFormat get_image_format() const SKR_NOEXCEPT final;

    bool load_jpeg_header() SKR_NOEXCEPT;

    bool decode(EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT final;
    bool encode() SKR_NOEXCEPT final;

    uint32_t channels = 0;
    turbo_jpeg_handle Compressor;
	turbo_jpeg_handle Decompressor;
};
}