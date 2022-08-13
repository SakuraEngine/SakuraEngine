#pragma once
#include <EASTL/string.h>
#include "image_coder_base.hpp"

namespace skr
{
class SKR_IMAGE_CODER_API PNGImageCoder : public BaseImageCoder
{
    ~PNGImageCoder() SKR_NOEXCEPT;
public:
    EImageCoderFormat get_image_format() const SKR_NOEXCEPT final;
    EImageCoderColorFormat get_color_format() const SKR_NOEXCEPT final;

    uint32_t get_width() const SKR_NOEXCEPT final;
    uint32_t get_height() const SKR_NOEXCEPT final;
    uint32_t get_bit_depth() const SKR_NOEXCEPT final;

    bool valid_data() const SKR_NOEXCEPT;
    bool load_png_header() SKR_NOEXCEPT;

    uint8_t channels = 0;
    uint8_t _padding = 0;

    uint64_t read_offset = 0;
    uint64_t png_color_type = 0;

    eastl::string error;
};
}