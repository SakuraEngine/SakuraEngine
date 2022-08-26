#pragma once
#include <EASTL/string.h>
#include "image_coder_base.hpp"

namespace skr
{
class SKR_IMAGE_CODER_API PNGImageCoder : public BaseImageCoder
{
    ~PNGImageCoder() SKR_NOEXCEPT;
public:
    bool set_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;
    bool move_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;
    bool view_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT final;

    EImageCoderFormat get_image_format() const SKR_NOEXCEPT final;

    bool valid_data() const SKR_NOEXCEPT;
    bool load_png_header() SKR_NOEXCEPT;

    bool decode(EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT final;
    bool encode() SKR_NOEXCEPT final;

    uint8_t channels = 0;
    uint8_t _padding = 0;

    uint64_t read_offset = 0;
    uint64_t png_color_type = 0;

    eastl::string error;
};
}