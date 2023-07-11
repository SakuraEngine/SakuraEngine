#pragma once
#include <SkrRT/containers/string.hpp>
#include "image_coder_base.hpp"

namespace skr
{
struct SKR_IMAGE_CODER_API PNGImageEncoder : public BaseImageEncoder
{
    ~PNGImageEncoder() SKR_NOEXCEPT;

    EImageCoderFormat get_image_format() const SKR_NOEXCEPT final;

    bool encode() SKR_NOEXCEPT final;
    uint64_t read_offset = 0;
};

struct SKR_IMAGE_CODER_API PNGImageDecoder : public BaseImageDecoder
{
    ~PNGImageDecoder() SKR_NOEXCEPT;

    EImageCoderFormat get_image_format() const SKR_NOEXCEPT final;
    bool initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;
    bool decode(EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT final;

    bool valid_data() const SKR_NOEXCEPT;
    bool load_png_header() SKR_NOEXCEPT;

    uint8_t channels = 0;
    uint8_t _padding = 0;
    uint64_t read_offset = 0;
    uint64_t png_color_type = 0;
    skr::string error;
};

}