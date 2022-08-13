#include "platform/memory.h"
#include "utils/log.h"
#include "utils/defer.hpp"
#include "image_coder_png.hpp"
#include "libpng/png.h"
#include "libpng/pnginfo.h"

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

namespace
{
struct PNGImageCoderHelper 
{
    inline static void user_read_compressed(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        PNGImageCoder* coder = (PNGImageCoder*)png_get_io_ptr(png_ptr);
        if (coder->read_offset + (int64_t)length <= coder->get_encoded_size())
        {
            memcpy(data, coder->get_encoded_data_view().data() + coder->read_offset, length);
            coder->read_offset += length;
        }
        else
        {
            coder->error = "Invalid read position for CompressedData.";
        }
    }

    inline static void user_write_compressed(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        PNGImageCoder* coder = (PNGImageCoder*)png_get_io_ptr(png_ptr);
        auto oldView = coder->get_encoded_data_view();
        auto newMemory = (uint8_t*)sakura_malloc(oldView.size() + length);
        memcpy(newMemory, oldView.data(), oldView.size());
        auto offsetPtr = newMemory + oldView.size();
        memcpy(offsetPtr, data, length);
    }

    inline static void user_flush_data(png_structp png_ptr)
    {
    }

    inline static void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
    {
        PNGImageCoder* coder = (PNGImageCoder*)png_get_error_ptr(png_ptr);
        coder->error = error_msg;
        SKR_LOG_ERROR("[libPNG] PNGImageCoder: %s", error_msg);
    }

    inline static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
    {
        SKR_LOG_WARN("[libPNG] PNGImageCoder: %s", warning_msg);
    }

    inline static void* user_malloc(png_structp png_ptr, png_size_t size)
    {
        SKR_UNREF_PARAM(png_ptr);
        return sakura_malloc(size);
    }

    inline static void user_free(png_structp png_ptr, png_voidp struct_ptr)
    {
        SKR_UNREF_PARAM(png_ptr);
        sakura_free(struct_ptr);
    }
};
} // namespace

bool PNGImageCoder::load_png_header() SKR_NOEXCEPT
{
    if (valid_data())
    {
        png_structp png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, this,
        PNGImageCoderHelper::user_error_fn, PNGImageCoderHelper::user_warning_fn, NULL,
        PNGImageCoderHelper::user_malloc, PNGImageCoderHelper::user_free);

        png_infop info_ptr = png_create_info_struct(png_ptr);
        SKR_DEFER({ png_destroy_read_struct(&png_ptr, &info_ptr, NULL); });
        {
            png_set_read_fn(png_ptr, this, PNGImageCoderHelper::user_read_compressed);

            png_read_info(png_ptr, info_ptr);

            width = info_ptr->width;
            height = info_ptr->height;
            png_color_type = info_ptr->color_type;
            bit_depth = info_ptr->bit_depth;
            channels = info_ptr->channels;
            if (info_ptr->valid & PNG_INFO_tRNS)
            {
                color_format = IMAGE_CODER_COLOR_FORMAT_RGBA;
            }
            else
            {
                color_format = (png_color_type & PNG_COLOR_MASK_COLOR || png_color_type & PNG_COLOR_MASK_ALPHA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : IMAGE_CODER_COLOR_FORMAT_Gray;
            }

            if (color_format == IMAGE_CODER_COLOR_FORMAT_RGBA && bit_depth <= 8)
            {
                color_format = IMAGE_CODER_COLOR_FORMAT_BGRA;
            }
        }
        return true;
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
} // namespace skr