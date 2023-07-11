#pragma once
#include "SkrRT/platform/configure.h"
#include "SkrRT/misc/types.h"
#include "SkrImageCoder/module.configure.h"

SKR_DECLARE_TYPE_ID_FWD(skr, IImageInterface, skr_image_interface)
SKR_DECLARE_TYPE_ID_FWD(skr, IImageEncoder, skr_image_encoder)
SKR_DECLARE_TYPE_ID_FWD(skr, IImageDecoder, skr_image_decoder)

typedef enum EImageCoderFormat {
    IMAGE_CODER_FORMAT_INVALID = -1,
    IMAGE_CODER_FORMAT_PNG = 0,
    IMAGE_CODER_FORMAT_JPEG = 1,
    IMAGE_CODER_FORMAT_GrayScaleJPEG = 2,
    IMAGE_CODER_FORMAT_BMP = 3,
    IMAGE_CODER_FORMAT_ICO = 4,
    IMAGE_CODER_FORMAT_EXR = 5,
    IMAGE_CODER_FORMAT_ICNS = 6,
    IMAGE_CODER_FORMAT_TGA = 7,
    IMAGE_CODER_FORMAT_HDR = 8,
    IMAGE_CODER_FORMAT_TIFF = 9,

    IMAGE_CODER_FORMAT_COUNT,
    IMAGE_CODER_FORMAT_MAX_ENUM_BIT = 0x7FFFFFFF
} EImageCoderFormat;

typedef enum EImageCoderColorFormat {
    IMAGE_CODER_COLOR_FORMAT_INVALID = -1,
    IMAGE_CODER_COLOR_FORMAT_RGBA = 0,
    IMAGE_CODER_COLOR_FORMAT_BGRA = 1,
    IMAGE_CODER_COLOR_FORMAT_Gray = 2,
    IMAGE_CODER_COLOR_FORMAT_RGBAF = 3,
    IMAGE_CODER_COLOR_FORMAT_BGRE = 4,
    IMAGE_CODER_COLOR_FORMAT_GrayF = 5,

    IMAGE_CODER_COLOR_FORMAT_COUNT,
    IMAGE_CODER_COLOR_FORMAT_MAX_ENUM_BIT = 0x7FFFFFFF
} EImageCoderColorFormat;

typedef enum EImageCoderCompression {
    IMAGE_CODER_COMPRESSION_MINIMUM_SIZE = 0,
    IMAGE_CODER_COMPRESSION_UNCOMPRESSED = 100
} EImageCoderCompression;
typedef uint32_t ImageCoderCompression;

#ifdef __cplusplus
#include <containers/span.hpp>

namespace skr
{
struct SKR_IMAGE_CODER_API IImageInterface : public skr::IBlob
{
    virtual ~IImageInterface() SKR_NOEXCEPT = default;
    virtual EImageCoderFormat get_image_format() const SKR_NOEXCEPT = 0;
    virtual EImageCoderColorFormat get_color_format() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_height() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_bit_depth() const SKR_NOEXCEPT = 0;
};

struct SKR_IMAGE_CODER_API IImageEncoder : public IImageInterface
{
    static skr::SObjectPtr<IImageEncoder> Create(EImageCoderFormat format) SKR_NOEXCEPT;
    
    virtual ~IImageEncoder() SKR_NOEXCEPT = default;

    virtual bool initialize(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, 
        EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT = 0;
    virtual bool encode() SKR_NOEXCEPT = 0;
};
using ImageEncoderId = skr::SObjectPtr<IImageEncoder>;

struct SKR_IMAGE_CODER_API IImageDecoder : public IImageInterface
{
    static skr::SObjectPtr<IImageDecoder> Create(EImageCoderFormat format) SKR_NOEXCEPT;

    virtual ~IImageDecoder() SKR_NOEXCEPT = default;
    virtual bool initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT = 0;
    virtual bool decode(EImageCoderColorFormat format, uint32_t bit_depth) SKR_NOEXCEPT = 0;
};
using ImageDecoderId = skr::SObjectPtr<IImageDecoder>;

}
#endif

SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
EImageCoderFormat skr_image_coder_detect_format(const uint8_t* encoded_data, uint64_t size);