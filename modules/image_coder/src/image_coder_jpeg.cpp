#include "SkrRT/platform/memory.h"
#include "SkrRT/misc/log.h"
#include "SkrRT/misc/defer.hpp"
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
	int SubSampling = 0;
	int ColorSpace = 0;
	if (tjDecompressHeader3(Decompressor, 
        reinterpret_cast<const uint8_t*>(encoded_view.data()), (unsigned long)encoded_view.size(),
        &ImageWidth, &ImageHeight, &SubSampling, &ColorSpace) != 0)
	{
		return false;
	}

	// set after call to base SetCompressed as it will reset members
	const auto color_format = (SubSampling == TJSAMP_GRAY) ? IMAGE_CODER_COLOR_FORMAT_Gray : IMAGE_CODER_COLOR_FORMAT_RGBA;
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