#include "SkrCore/memory/memory.h"
#include "SkrCore/log.h"
#include "image_coder_base.hpp"

namespace skr
{

BaseImageEncoder::~BaseImageEncoder() SKR_NOEXCEPT
{
    if (encoded_data)
    {
        BaseImageEncoder::Deallocate(encoded_data, get_alignment());
    }
}

void BaseImageEncoder::setRawProps(uint32_t width, uint32_t height, 
    EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT
{
    props.width = width;
    props.height = height;
    props.color_format = format;
    props.bit_depth = bit_depth;
}

bool BaseImageEncoder::initialize(const uint8_t* _data, uint64_t _size, uint32_t _width, uint32_t _height, 
    EImageCoderColorFormat _format, uint32_t _bit_depth) SKR_NOEXCEPT
{
    if (_data == nullptr || _size == 0 || _width == 0 || _height == 0 || _format == IMAGE_CODER_COLOR_FORMAT_INVALID || _bit_depth == 0)
    {
        SKR_LOG_ERROR(u8"BaseImageEncoder::initialize() - Invalid parameters.");
        return false;
    }
    decoded_view = { _data, _size };
    setRawProps(_width, _height, _format, _bit_depth);
    initialized = true;
    return initialized;
}

BaseImageDecoder::~BaseImageDecoder() SKR_NOEXCEPT
{
    if (decoded_data)
    {
        BaseImageDecoder::Deallocate(decoded_data, get_alignment());
    }
}

void BaseImageDecoder::setRawProps(uint32_t width, uint32_t height, 
    EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT
{
    props.width = width;
    props.height = height;
    props.color_format = format;
    props.bit_depth = bit_depth;
}

bool BaseImageDecoder::initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT
{
    encoded_view = { data, size };
    initialized = true;
    return initialized;
}

const char* kImageEncoderMemoryName = "ImageEncoder";
uint8_t* BaseImageEncoder::Allocate(uint64_t size, uint64_t alignment) SKR_NOEXCEPT
{
    return (uint8_t*)sakura_malloc_alignedN(size, alignment, kImageEncoderMemoryName);
}

void BaseImageEncoder::Deallocate(uint8_t* ptr, uint64_t alignment) SKR_NOEXCEPT
{
    if (ptr)
    {
        sakura_free_alignedN(ptr, alignment, kImageEncoderMemoryName);
    }
}

const char* kImageDecoderMemoryName = "ImageDecoder";
uint8_t* BaseImageDecoder::Allocate(uint64_t size, uint64_t alignment) SKR_NOEXCEPT
{
    return (uint8_t*)sakura_malloc_alignedN(size, alignment, kImageDecoderMemoryName);
}

void BaseImageDecoder::Deallocate(uint8_t* ptr, uint64_t alignment) SKR_NOEXCEPT
{
    if (ptr)
    {
        sakura_free_alignedN(ptr, alignment, kImageDecoderMemoryName);
    }
}

} // namespace skr