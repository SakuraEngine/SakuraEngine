#include "platform/memory.h"
#include "utils/log.h"
#include "utils/defer.hpp"
#include "image_coder_jpeg.hpp"

#include "turbojpeg/turbojpeg.h"

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
}

JPEGImageCoder::JPEGImageCoder() SKR_NOEXCEPT
    : BaseImageCoder()
    , Compressor(tjInitCompress())
    , Decompressor(tjInitDecompress())
{

}

JPEGImageCoder::~JPEGImageCoder() SKR_NOEXCEPT
{
    if (Compressor)
	{
		tjDestroy(Compressor);
	}
	if (Decompressor)
	{
		tjDestroy(Decompressor);
	}
}

EImageCoderFormat JPEGImageCoder::get_image_format() const SKR_NOEXCEPT
{
    return EImageCoderFormat::IMAGE_CODER_FORMAT_JPEG;
}

bool JPEGImageCoder::load_jpeg_header() SKR_NOEXCEPT
{
	int ImageWidth = 0;
	int ImageHeight = 0;
	int SubSampling = 0;
	int ColorSpace = 0;
	if (tjDecompressHeader3(Decompressor, 
        reinterpret_cast<const uint8_t*>(encoded_view.data()), (unsigned long)encoded_view.size(),
        &ImageWidth, &ImageHeight, &SubSampling, &ColorSpace) != 0)
	{
		return false;
	}

	// set after call to base SetCompressed as it will reset members
	width = ImageWidth;
	height = ImageHeight;
	bit_depth = 8; // We don't support 16 bit jpegs
	color_format = (SubSampling == TJSAMP_GRAY) ? IMAGE_CODER_COLOR_FORMAT_Gray : IMAGE_CODER_COLOR_FORMAT_RGBA;

    return true;
}

bool JPEGImageCoder::set_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT
{
    if (BaseImageCoder::set_encoded(data, size))
    {
        return load_jpeg_header();
    }
    return false;
}

bool JPEGImageCoder::move_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT
{
    if (BaseImageCoder::move_encoded(data, size))
    {
        return load_jpeg_header();
    }
    return false;
}

bool JPEGImageCoder::view_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT
{
    if (BaseImageCoder::view_encoded(data, size))
    {
        return load_jpeg_header();
    }
    return false;
}

bool JPEGImageCoder::decode(EImageCoderColorFormat in_format, uint32_t in_bit_depth) SKR_NOEXCEPT
{
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

    const uint64_t bytes_per_pixel = pixel_channels * in_bit_depth / 8;
    const auto decoded_size = width * height * pixel_channels;
    auto newMemory = (uint8_t*)sakura_malloc(decoded_size);
	const int PixelFormat = ConvertTJpegPixelFormat(in_format);
	const int Flags = TJFLAG_NOREALLOC | TJFLAG_FASTDCT;

	int result = 0;
	if (result = tjDecompress2(Decompressor, 
        encoded_view.data(), (unsigned long)encoded_view.size(), 
        newMemory, width, 0, height, PixelFormat, Flags); result == 0)
	{
		return move_raw(newMemory, decoded_size, width, height, in_format, in_bit_depth, (uint32_t)bytes_per_pixel);
	}

	SKR_LOG_FATAL("TurboJPEG Error %d: %s", result, tjGetErrorStr2(Decompressor));
    sakura_free(newMemory);
    return false;
}

bool JPEGImageCoder::encode() SKR_NOEXCEPT
{
    const int32_t Quality = 5;

    const int PixelFormat = ConvertTJpegPixelFormat(color_format);
    const int Subsampling = TJSAMP_420;
    const int Flags = TJFLAG_NOREALLOC | TJFLAG_FASTDCT;

    unsigned long OutBufferSize = tjBufSize(width, height, Subsampling);
    auto newMemory = (uint8_t*)sakura_malloc(OutBufferSize);

    const bool bSuccess = tjCompress2(Compressor, raw_view.data(), 
        width, raw_bytes_per_row, height, PixelFormat, &newMemory, 
        &OutBufferSize, Subsampling, Quality, Flags) == 0;

    move_encoded(newMemory, OutBufferSize);
    return bSuccess;
}
}