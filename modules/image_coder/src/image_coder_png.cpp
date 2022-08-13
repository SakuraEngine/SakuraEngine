#include "platform/memory.h"
#include "utils/log.h"
#include "image_coder_png.hpp"
#include "libpng/png.h"

namespace skr
{
PNGImageCoder::~PNGImageCoder() SKR_NOEXCEPT
{
    BaseImageCoder::~BaseImageCoder();
}

bool PNGImageCoder::valid_data() const SKR_NOEXCEPT
{
    const int32_t pngSignatureSize = sizeof(png_size_t);
    if (encoded_view.size() > pngSignatureSize)
    {
        png_size_t PNGSignature = *reinterpret_cast<const png_size_t*>(encoded_view.data());
		return (0 == png_sig_cmp(reinterpret_cast<png_bytep>(&PNGSignature), 0, pngSignatureSize));
    }
    return false;
}

EImageCoderFormat PNGImageCoder::get_image_format() const SKR_NOEXCEPT
{
    return EImageCoderFormat::IMAGE_CODER_FORMAT_PNG;
}

EImageCoderColorFormat PNGImageCoder::get_color_format() const SKR_NOEXCEPT
{
    return color_format;
}

uint32_t PNGImageCoder::get_width() const SKR_NOEXCEPT
{
    return width;
}

uint32_t PNGImageCoder::get_height() const SKR_NOEXCEPT
{
    return height;
}

uint32_t PNGImageCoder::get_bit_depth() const SKR_NOEXCEPT
{
    return bit_depth;
}
}