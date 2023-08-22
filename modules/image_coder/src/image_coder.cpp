#include "SkrRT/misc/log.h"
#include "SkrRT/misc/defer.hpp"
#include "SkrRT/platform/memory.h"
#include "SkrRT/platform/debug.h"
#include "SkrRT/containers/sptr.hpp"
#include "SkrImageCoder/skr_image_coder.h"
#include "image_coder_png.hpp"
#include "image_coder_jpeg.hpp"

namespace skr
{

skr::SObjectPtr<IImageEncoder> IImageEncoder::Create(EImageCoderFormat format) SKR_NOEXCEPT
{
    switch (format)
    {
    case IMAGE_CODER_FORMAT_PNG:
        return SObjectPtr<skr::PNGImageEncoder>::Create();
    case IMAGE_CODER_FORMAT_JPEG:
        return SObjectPtr<skr::JPEGImageEncoder>::Create();
    case IMAGE_CODER_FORMAT_GrayScaleJPEG:
    case IMAGE_CODER_FORMAT_BMP:
    case IMAGE_CODER_FORMAT_ICO:
    case IMAGE_CODER_FORMAT_EXR:
    case IMAGE_CODER_FORMAT_ICNS:
    case IMAGE_CODER_FORMAT_TGA:
    case IMAGE_CODER_FORMAT_HDR:
    case IMAGE_CODER_FORMAT_TIFF:
    default:
        SKR_UNIMPLEMENTED_FUNCTION()
        return nullptr;
    }
}

skr::SObjectPtr<IImageDecoder> IImageDecoder::Create(EImageCoderFormat format) SKR_NOEXCEPT
{
    switch (format)
    {
    case IMAGE_CODER_FORMAT_PNG:
        return SObjectPtr<skr::PNGImageDecoder>::Create();
    case IMAGE_CODER_FORMAT_JPEG:
        return SObjectPtr<skr::JPEGImageDecoder>::Create();
    case IMAGE_CODER_FORMAT_GrayScaleJPEG:
    case IMAGE_CODER_FORMAT_BMP:
    case IMAGE_CODER_FORMAT_ICO:
    case IMAGE_CODER_FORMAT_EXR:
    case IMAGE_CODER_FORMAT_ICNS:
    case IMAGE_CODER_FORMAT_TGA:
    case IMAGE_CODER_FORMAT_HDR:
    case IMAGE_CODER_FORMAT_TIFF:
    default:
        SKR_UNIMPLEMENTED_FUNCTION()
        return nullptr;
    }
}

}

namespace
{
    static const uint8_t IMAGE_MAGIC_PNG[]  = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	static const uint8_t IMAGE_MAGIC_JPEG[] = {0xFF, 0xD8, 0xFF};
	static const uint8_t IMAGE_MAGIC_BMP[]  = {0x42, 0x4D};
	static const uint8_t IMAGE_MAGIC_ICO[]  = {0x00, 0x00, 0x01, 0x00};
	static const uint8_t IMAGE_MAGIC_EXR[]  = {0x76, 0x2F, 0x31, 0x01};
	static const uint8_t IMAGE_MAGIC_ICNS[] = {0x69, 0x63, 0x6E, 0x73};

    template <int32_t MagicCount> 
    bool StartsWith(const uint8_t* Content, int64_t ContentSize, const uint8_t (&Magic)[MagicCount])
	{
		if (ContentSize < MagicCount)
		{
			return false;
		}

		for (int32_t I = 0; I < MagicCount; ++I)
		{
			if (Content[I] != Magic[I])
			{
				return false;
			}
		}

		return true;
	}
}

EImageCoderFormat skr_image_coder_detect_format(const uint8_t* encoded_data, uint64_t size)
{
    EImageCoderFormat format = IMAGE_CODER_FORMAT_INVALID;
    if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_PNG))
    {
        format = IMAGE_CODER_FORMAT_PNG;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_JPEG))
    {
        format = IMAGE_CODER_FORMAT_JPEG; // @Todo: Should we detect grayscale vs non-grayscale?
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_BMP))
    {
        format = IMAGE_CODER_FORMAT_BMP;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_ICO))
    {
        format = IMAGE_CODER_FORMAT_ICO;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_EXR))
    {
        format = IMAGE_CODER_FORMAT_EXR;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_ICNS))
    {
        format = IMAGE_CODER_FORMAT_ICNS;
    }
    return format;
}

#ifdef _WIN32
#include "SkrImageCoder/extensions/win_dstorage_decompressor.h"
#include "SkrProfile/profile.h"

HRESULT skr_image_coder_win_dstorage_decompressor(skr_win_dstorage_decompress_request_t* request, void* user_data)
{
    SkrZoneScopedN("DirectStoragePNGDecompressor");
    EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)request->src_buffer, request->src_size);
    SKR_LOG_TRACE(u8"skr_image_coder_win_dstorage_decompressor: format=%d", format);
    auto decoder = skr::IImageDecoder::Create(format);
    const auto encoded_size = request->src_size;
    if (decoder->initialize((const uint8_t*)request->src_buffer, request->src_size))
    {
        SKR_DEFER({ SkrZoneScopedN("DirectStoragePNGDecompressorFree"); decoder.reset(); });

        const auto encoded_format = decoder->get_color_format();
        const auto color_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
        if (decoder->decode(color_format, decoder->get_bit_depth()))
        {
            SKR_LOG_TRACE(u8"image decoder: width = %d, height = %d, encoded_size = %d, raw_size = %d", 
                decoder->get_width(), decoder->get_height(), encoded_size, decoder->get_size()
            );
            if (auto data = decoder->get_data())
            {
                memcpy(request->dst_buffer, data, decoder->get_size());
                return 0L; // S_OK
            }
        }
    }
    return 1L; // S_FALSE
}
#endif