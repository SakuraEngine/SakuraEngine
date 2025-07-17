#pragma once
#include <SkrContainers/string.hpp>
#include "image_coder_base.hpp"

namespace skr
{
using turbo_jpeg_handle = void*;

struct SKR_IMAGE_CODER_API JPEGImageEncoder : public BaseImageEncoder {
public:
    JPEGImageEncoder() SKR_NOEXCEPT;
    ~JPEGImageEncoder() SKR_NOEXCEPT;

    EImageCoderFormat get_image_format() const SKR_NOEXCEPT final;

    bool encode() SKR_NOEXCEPT final;

    uint32_t          channels = 0;
    turbo_jpeg_handle Compressor;
};

struct SKR_IMAGE_CODER_API JPEGImageDecoder : public BaseImageDecoder {
public:
    JPEGImageDecoder() SKR_NOEXCEPT;
    ~JPEGImageDecoder() SKR_NOEXCEPT;

    EImageCoderFormat get_image_format() const SKR_NOEXCEPT final;
    bool              initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;
    bool              decode(EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT final;

    bool load_jpeg_header() SKR_NOEXCEPT;

    int ColorSpace = 0;
	int SubSampling = 0;
    
    uint32_t          channels = 0;
    turbo_jpeg_handle Decompressor;
};

} // namespace skr