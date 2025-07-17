#include "SkrCore/memory/memory.h"
#include "SkrCore/log.h"
#include "SkrBase/misc/defer.hpp"
#include "image_coder_jpeg.hpp"

#include "turbojpeg/turbojpeg.h"
#include <algorithm> // for std::swap

namespace skr
{
namespace
{
    typedef enum TJPF TJPF;
	TJPF ConvertTJpegPixelFormat(EImageCoderColorFormat InFormat)
	{
		switch (InFormat)
		{
			case EImageCoderColorFormat::IMAGE_CODER_COLOR_FORMAT_BGRA:	return TJPF_BGRA;
			case EImageCoderColorFormat::IMAGE_CODER_COLOR_FORMAT_Gray:	return TJPF_GRAY;
			case EImageCoderColorFormat::IMAGE_CODER_COLOR_FORMAT_RGBA:	return TJPF_RGBA;
			default: return TJPF_RGBA;
		}
	}

	// CMYK to RGBA conversion using TurboJPEG's algorithm
	// This is a quick & dirty conversion suitable for testing
	void ConvertCMYKToRGBA(const uint8_t* cmyk, uint8_t* rgba, uint32_t pixelCount)
	{
		// Based on libjpeg-turbo's cmyk.h algorithm
		// Note: This is fully reversible only for C/M/Y/K values generated with rgb_to_cmyk()
		for (uint32_t i = 0; i < pixelCount; ++i)
		{
			const uint8_t c = cmyk[i * 4 + 0];
			const uint8_t m = cmyk[i * 4 + 1];
			const uint8_t y = cmyk[i * 4 + 2];
			const uint8_t k = cmyk[i * 4 + 3];

			// Using the algorithm from libjpeg-turbo's cmyk.h
			rgba[i * 4 + 0] = static_cast<uint8_t>((double)c * (double)k / 255.0 + 0.5);
			rgba[i * 4 + 1] = static_cast<uint8_t>((double)m * (double)k / 255.0 + 0.5);
			rgba[i * 4 + 2] = static_cast<uint8_t>((double)y * (double)k / 255.0 + 0.5);
			rgba[i * 4 + 3] = 255; // Alpha channel
		}
	}
}

JPEGImageDecoder::JPEGImageDecoder() SKR_NOEXCEPT
    : BaseImageDecoder()
    , Decompressor(tjInitDecompress())
{

}

JPEGImageDecoder::~JPEGImageDecoder() SKR_NOEXCEPT
{
	if (Decompressor)
	{
		tjDestroy(Decompressor);
	}
}

EImageCoderFormat JPEGImageDecoder::get_image_format() const SKR_NOEXCEPT
{
    return EImageCoderFormat::IMAGE_CODER_FORMAT_JPEG;
}

bool JPEGImageDecoder::load_jpeg_header() SKR_NOEXCEPT
{
	int ImageWidth = 0;
	int ImageHeight = 0;
	if (tjDecompressHeader3(Decompressor, 
        reinterpret_cast<const uint8_t*>(encoded_view.data()), (unsigned long)encoded_view.size(),
        &ImageWidth, &ImageHeight, &SubSampling, &ColorSpace) != 0)
	{
		return false;
	}

	// Determine color format based on JPEG colorspace, not subsampling
	EImageCoderColorFormat color_format = IMAGE_CODER_COLOR_FORMAT_RGBA;
	switch (ColorSpace)
	{
		case TJCS_GRAY:
			color_format = IMAGE_CODER_COLOR_FORMAT_Gray;
			break;
		case TJCS_RGB:
		case TJCS_YCbCr:
			// Both RGB and YCbCr JPEG can be decoded to RGBA
			color_format = IMAGE_CODER_COLOR_FORMAT_RGBA;
			break;
		case TJCS_CMYK:
		case TJCS_YCCK:
			// CMYK images will be converted to RGBA during decode
			color_format = IMAGE_CODER_COLOR_FORMAT_RGBA;
			break;
		default:
			SKR_LOG_ERROR(u8"Unknown JPEG colorspace: %d", ColorSpace);
			return false;
	}
	
	const auto bit_depth = 8; // We don't support 16 bit jpegs
	setRawProps(ImageWidth, ImageHeight, color_format, bit_depth);

    return true;
}

bool JPEGImageDecoder::initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT
{
    if (BaseImageDecoder::initialize(data, size))
    {
        return load_jpeg_header();
    }
    return false;
}

bool JPEGImageDecoder::decode(EImageCoderColorFormat in_format, uint32_t in_bit_depth) SKR_NOEXCEPT
{
	SKR_ASSERT(initialized);
    int pixel_channels = 0;
	if ((in_format == IMAGE_CODER_COLOR_FORMAT_RGBA || in_format == IMAGE_CODER_COLOR_FORMAT_BGRA) && in_bit_depth == 8)
	{
		pixel_channels = 4;
	}
	else if (in_format == IMAGE_CODER_COLOR_FORMAT_Gray && in_bit_depth == 8)
	{
		pixel_channels = 1;
	}
	else
	{
		SKR_ASSERT(false);
	}

	// Check if this is a CMYK/YCCK image that needs special handling
	if (ColorSpace == TJCS_CMYK || ColorSpace == TJCS_YCCK)
	{
		// CMYK/YCCK images must be decoded to CMYK first, then converted to RGBA
		if (in_format == IMAGE_CODER_COLOR_FORMAT_RGBA || in_format == IMAGE_CODER_COLOR_FORMAT_BGRA)
		{
			const uint32_t pixelCount = get_width() * get_height();
			const uint64_t cmyk_size = pixelCount * 4;
			uint8_t* cmyk_data = JPEGImageDecoder::Allocate(cmyk_size, get_alignment());
			
			// Decode to CMYK
			const int Flags = TJFLAG_NOREALLOC | TJFLAG_FASTDCT;
			int result = tjDecompress2(Decompressor, 
				encoded_view.data(), (unsigned long)encoded_view.size(), 
				cmyk_data, get_width(), 0, get_height(), TJPF_CMYK, Flags);
			
			if (result != 0)
			{
				SKR_LOG_FATAL(u8"TurboJPEG Error %d: %s", result, tjGetErrorStr2(Decompressor));
				sakura_free(cmyk_data);
				return false;
			}

			// Allocate RGBA buffer
			decoded_size = pixelCount * 4;
			decoded_data = JPEGImageDecoder::Allocate(decoded_size, get_alignment());
			
			// Convert CMYK to RGBA
			ConvertCMYKToRGBA(cmyk_data, decoded_data, pixelCount);
			
			// Free temporary CMYK buffer
			sakura_free(cmyk_data);

			// If BGRA was requested, swap R and B channels
			if (in_format == IMAGE_CODER_COLOR_FORMAT_BGRA)
			{
				for (uint32_t i = 0; i < pixelCount; ++i)
				{
					std::swap(decoded_data[i * 4 + 0], decoded_data[i * 4 + 2]);
				}
			}
			
			return true;
		}
		else
		{
			SKR_LOG_ERROR(u8"CMYK/YCCK JPEG can only be decoded to RGBA or BGRA format");
			return false;
		}
	}
	else
	{
		// Normal decoding path for RGB/YCbCr/Grayscale
		decoded_size = get_width() * get_height() * pixel_channels;
		decoded_data = JPEGImageDecoder::Allocate(decoded_size, get_alignment());
		const int PixelFormat = ConvertTJpegPixelFormat(in_format);
		const int Flags = TJFLAG_NOREALLOC | TJFLAG_FASTDCT;

		int result = 0;
		if (result = tjDecompress2(Decompressor, 
			encoded_view.data(), (unsigned long)encoded_view.size(), 
			decoded_data, get_width(), 0, get_height(), PixelFormat, Flags); result == 0)
		{
			return true;
		}

		SKR_LOG_FATAL(u8"TurboJPEG Error %d: %s", result, tjGetErrorStr2(Decompressor));
		sakura_free(decoded_data);
		decoded_data = nullptr;
	}
    return false;
}

JPEGImageEncoder::JPEGImageEncoder() SKR_NOEXCEPT
    : BaseImageEncoder()
    , Compressor(tjInitCompress())
{

}

JPEGImageEncoder::~JPEGImageEncoder() SKR_NOEXCEPT
{

}

EImageCoderFormat JPEGImageEncoder::get_image_format() const SKR_NOEXCEPT
{
    return EImageCoderFormat::IMAGE_CODER_FORMAT_JPEG;
}

bool JPEGImageEncoder::encode() SKR_NOEXCEPT
{
	SKR_ASSERT(initialized);
    const int32_t Quality = 5;

	const auto color_format = get_color_format();
    const int PixelFormat = ConvertTJpegPixelFormat(color_format);
    const int Subsampling = TJSAMP_420;
    const int Flags = TJFLAG_NOREALLOC | TJFLAG_FASTDCT;

    unsigned long OutBufferSize = tjBufSize(get_width(), get_height(), Subsampling);
    encoded_data = JPEGImageEncoder::Allocate(OutBufferSize, get_alignment());
	encoded_size = OutBufferSize;

    const bool bSuccess = tjCompress2(Compressor, decoded_view.data(), 
        get_width(), bytes_per_row, get_height(), PixelFormat, &encoded_data, 
        &OutBufferSize, Subsampling, Quality, Flags) == 0;

    return bSuccess;
}
}