#include "SkrCore/memory/memory.h"
#include "SkrCore/log.h"
#include "SkrBase/misc/defer.hpp"
#include "image_coder_png.hpp"
#include "libpng/png.h"
#include "libpng/pnginfo.h"

namespace skr
{
PNGImageDecoder::~PNGImageDecoder() SKR_NOEXCEPT
{

}

bool PNGImageDecoder::valid_data() const SKR_NOEXCEPT
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
        PNGImageDecoder* decoder = (PNGImageDecoder*)png_get_io_ptr(png_ptr);
        if (decoder->read_offset + (int64_t)length <= decoder->encoded_view.size())
        {
            memcpy(data, decoder->encoded_view.data() + decoder->read_offset, length);
            decoder->read_offset += length;
        }
        else
        {
            decoder->error = u8"Invalid read position for CompressedData.";
        }
    }

    inline static void user_write_compressed(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        PNGImageEncoder* encoder = (PNGImageEncoder*)png_get_io_ptr(png_ptr);
        skr::span<uint8_t> oldView = { encoder->encoded_data, encoder->encoded_size };
        auto newMemory = (uint8_t*)PNGImageDecoder::Allocate(oldView.size() + length, encoder->get_alignment());
        memcpy(newMemory, oldView.data(), oldView.size());
        PNGImageDecoder::Deallocate((uint8_t*)oldView.data(), encoder->get_alignment());

        auto offsetPtr = newMemory + oldView.size();
        memcpy(offsetPtr, data, length);
        
        encoder->encoded_data = newMemory;
        encoder->encoded_size = oldView.size() + length;
    }

    inline static void user_flush_data(png_structp png_ptr)
    {
    }

    inline static void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
    {
        PNGImageDecoder* decoder = (PNGImageDecoder*)png_get_error_ptr(png_ptr);
        decoder->error = (const char8_t*)error_msg;
        SKR_LOG_ERROR(u8"[libPNG] PNGImageDecoder: %s", error_msg);
    }

    inline static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
    {
        SKR_LOG_WARN(u8"[libPNG] PNGImageDecoder: %s", warning_msg);
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

bool PNGImageDecoder::load_png_header() SKR_NOEXCEPT
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

            png_color_type = info_ptr->color_type;
            channels = info_ptr->channels;

            const auto width = info_ptr->width;
            const auto height = info_ptr->height;
            const auto bit_depth = info_ptr->bit_depth;
            EImageCoderColorFormat color_format = IMAGE_CODER_COLOR_FORMAT_RGBA;
            if (info_ptr->valid & PNG_INFO_tRNS)
            {
                color_format = IMAGE_CODER_COLOR_FORMAT_RGBA;
            }
            else
            {
                const auto notGray = (png_color_type & PNG_COLOR_MASK_COLOR || png_color_type & PNG_COLOR_MASK_ALPHA);
                color_format = notGray ? IMAGE_CODER_COLOR_FORMAT_RGBA : IMAGE_CODER_COLOR_FORMAT_Gray;
            }

            if (color_format == IMAGE_CODER_COLOR_FORMAT_RGBA && bit_depth <= 8)
            {
                color_format = IMAGE_CODER_COLOR_FORMAT_BGRA;
            }
            setRawProps(width, height, color_format, bit_depth);
        }
        return true;
    }
    return false;
}

bool PNGImageDecoder::initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT
{
    if (BaseImageDecoder::initialize(data, size))
    {
        return load_png_header();
    }
    return false;
}

static int PNG_isLittleEndian(void)
{
    const union { uint32_t u; uint8_t c[4]; } one = { 1 };
    return one.c[0];
}

bool PNGImageDecoder::decode(EImageCoderColorFormat in_format, uint32_t in_bit_depth) SKR_NOEXCEPT
{
    SKR_ASSERT(initialized);
    const auto width = get_width();
    const auto height = get_height();
    const auto bit_depth = get_bit_depth();

    read_offset = 0;
    // create png read struct
    png_structp png_ptr	= png_create_read_struct_2(PNG_LIBPNG_VER_STRING, this, 
        PNGImageCoderHelper::user_error_fn, PNGImageCoderHelper::user_warning_fn, 
        NULL, PNGImageCoderHelper::user_malloc, PNGImageCoderHelper::user_free);
	png_infop info_ptr	= png_create_info_struct(png_ptr);
    png_bytep* row_pointers = (png_bytep*) png_malloc( png_ptr, height * sizeof(png_bytep) );
    SKR_DEFER({ png_free(png_ptr, row_pointers); png_destroy_read_struct(&png_ptr, &info_ptr, NULL); });
    if (setjmp(png_jmpbuf(png_ptr)) != 0)
    {
        return false;
    }
    if (png_color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
    }
    if ((png_color_type & PNG_COLOR_MASK_COLOR) == 0 && bit_depth < 8)
    {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    // insert alpha channel if there is no alpha channel
    if ((png_color_type & PNG_COLOR_MASK_ALPHA) == 0 && (in_format == IMAGE_CODER_COLOR_FORMAT_RGBA || in_format == IMAGE_CODER_COLOR_FORMAT_BGRA))
    {
        // png images don't set PNG_COLOR_MASK_ALPHA if they have alpha from a tRNS chunk, but png_set_add_alpha seems to be safe regardless
        if ((png_color_type & PNG_COLOR_MASK_COLOR) == 0)
        {
            png_set_tRNS_to_alpha(png_ptr);
        }
        else if (png_color_type == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_tRNS_to_alpha(png_ptr);
        }
        if (in_bit_depth == 8)
        {
            png_set_add_alpha(png_ptr, 0xff , PNG_FILLER_AFTER);
        }
        else if (in_bit_depth == 16)
        {
            png_set_add_alpha(png_ptr, 0xffff , PNG_FILLER_AFTER);
        }
    }
    // calculate pixel depth
    const uint64_t pixel_channels = (in_format == IMAGE_CODER_COLOR_FORMAT_Gray) ? 1 : 4;
    const uint64_t bytes_per_pixel = pixel_channels * in_bit_depth / 8;
    const uint64_t bytes_per_row = width * bytes_per_pixel;
    // reallocate raw data
    decoded_size = bytes_per_row * height;
    decoded_data = PNGImageDecoder::Allocate(decoded_size, get_alignment()); 
    // read png data
    png_set_read_fn(png_ptr, this, PNGImageCoderHelper::user_read_compressed);
    for (uint32_t i = 0; i < height; i++)
    {
        row_pointers[i] = &decoded_data[i * bytes_per_row];
    }
    png_set_rows(png_ptr, info_ptr, row_pointers);
    uint32_t transform = (in_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? PNG_TRANSFORM_BGR : PNG_TRANSFORM_IDENTITY;
    // We're little endian so we need to swap
    if (bit_depth == 16 && PNG_isLittleEndian())
    {
        transform |= PNG_TRANSFORM_SWAP_ENDIAN;
    }
    // convert grayscale png to RGB if requested
    if ((png_color_type & PNG_COLOR_MASK_COLOR) == 0 &&
        (in_format == IMAGE_CODER_COLOR_FORMAT_RGBA || in_format == IMAGE_CODER_COLOR_FORMAT_BGRA))
    {
        transform |= PNG_TRANSFORM_GRAY_TO_RGB;
    }
    // convert RGB png to grayscale if requested
    if ((png_color_type & PNG_COLOR_MASK_COLOR) != 0 && in_format == IMAGE_CODER_COLOR_FORMAT_Gray)
    {
        png_set_rgb_to_gray_fixed(png_ptr, 2 /* warn if image is in color */, -1, -1);
    }
    // strip alpha channel if requested output is grayscale
    if (in_format == IMAGE_CODER_COLOR_FORMAT_Gray)
    {
        // this is not necessarily the best option, instead perhaps:
        // png_color background = {0,0,0};
        // png_set_background(png_ptr, &background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
        transform |= PNG_TRANSFORM_STRIP_ALPHA;
    }
    // Reduce 16-bit to 8-bit if requested
    if (bit_depth == 16 && in_bit_depth == 8)
    {
#if PNG_LIBPNG_VER >= 10504
        SKR_ASSERT(0); // Needs testing
        transform |= PNG_TRANSFORM_SCALE_16;
#else
        transform |= PNG_TRANSFORM_STRIP_16;
#endif
    }
    // Increase 8-bit to 16-bit if requested
    if (bit_depth <= 8 && in_bit_depth == 16)
    {
#if PNG_LIBPNG_VER >= 10504
        SKR_ASSERT(0); // Needs testing
        transform |= PNG_TRANSFORM_EXPAND_16;
#else
        // Expanding 8-bit images to 16-bit via transform needs a libpng update
        SKR_ASSERT(0);
#endif
    }
    png_read_png(png_ptr, info_ptr, transform, NULL);
    return true;
}

EImageCoderFormat PNGImageDecoder::get_image_format() const SKR_NOEXCEPT
{
    return EImageCoderFormat::IMAGE_CODER_FORMAT_PNG;
}

#define PNGDefaultZlibLevel 3

PNGImageEncoder::~PNGImageEncoder() SKR_NOEXCEPT
{

}

bool PNGImageEncoder::encode() SKR_NOEXCEPT
{
    SKR_ASSERT(initialized);
    const auto width = get_width();
    const auto height = get_height();
    const auto color_format = get_color_format();
    const auto raw_bit_depth = get_bit_depth();

    //Preserve old single thread code on some platform in relation to a type incompatibility at compile time.
    SKR_ASSERT(width > 0);
    SKR_ASSERT(height > 0);

    // Reset to the beginning of file so we can use png_read_png(), which expects to start at the beginning.
    read_offset = 0;

    png_structp png_ptr	= png_create_write_struct(PNG_LIBPNG_VER_STRING, this, PNGImageCoderHelper::user_error_fn, PNGImageCoderHelper::user_warning_fn);
    SKR_ASSERT(png_ptr);

    png_infop info_ptr	= png_create_info_struct(png_ptr);
    SKR_ASSERT(info_ptr);

    png_bytep* row_pointers = (png_bytep*) png_malloc( png_ptr, height * sizeof(png_bytep) );
    SKR_DEFER({ png_free(png_ptr, row_pointers); png_destroy_write_struct(&png_ptr, &info_ptr); });

    // Store the current stack pointer in the jump buffer. setjmp will return non-zero in the case of a write error.
#if PLATFORM_ANDROID || PLATFORM_LUMIN || PLATFORM_LUMINGL4
    //Preserve old single thread code on some platform in relation to a type incompatibility at compile time.
    if (setjmp(SetjmpBuffer) != 0)
#else
    //Use libPNG jump buffer solution to allow concurrent compression\decompression on concurrent threads.
    if (setjmp(png_jmpbuf(png_ptr)) != 0)
#endif
    {
        return false;
    }

    // ---------------------------------------------------------------------------------------------------------
    // Anything allocated on the stack after this point will not be destructed correctly in the case of an error

    {
        png_set_compression_level(png_ptr, PNGDefaultZlibLevel);
        //png_set_IHDR(png_ptr, info_ptr, width, height, raw_bit_depth, 
        //    (color_format == EImageCoderColorFormat::IMAGE_CODER_COLOR_FORMAT_Gray) ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGBA,
        //    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_set_write_fn(png_ptr, this, PNGImageCoderHelper::user_write_compressed, PNGImageCoderHelper::user_flush_data);

        const uint64_t PixelChannels = (color_format == EImageCoderColorFormat::IMAGE_CODER_COLOR_FORMAT_Gray) ? 1 : 4;
        const uint64_t BytesPerPixel = (raw_bit_depth * PixelChannels) / 8;
        const uint64_t BytesPerRow = BytesPerPixel * width;

        for (int64_t i = 0; i < height; i++)
        {
            auto dv = decoded_view.data();
            row_pointers[i] = (png_bytep)&dv[i * BytesPerRow];
        }
        png_set_rows(png_ptr, info_ptr, row_pointers);

        uint32_t Transform = (color_format == EImageCoderColorFormat::IMAGE_CODER_COLOR_FORMAT_BGRA) ? PNG_TRANSFORM_BGR : PNG_TRANSFORM_IDENTITY;

        // PNG files store 16-bit pixels in network byte order (big-endian, ie. most significant bits first).
        // We're little endian so we need to swap
        if (raw_bit_depth == 16 && PNG_isLittleEndian())
        {
            Transform |= PNG_TRANSFORM_SWAP_ENDIAN;
        }

        png_write_png(png_ptr, info_ptr, Transform, NULL);
    }
    return true;
}

EImageCoderFormat PNGImageEncoder::get_image_format() const SKR_NOEXCEPT
{
    return EImageCoderFormat::IMAGE_CODER_FORMAT_PNG;
}
} // namespace skr