#pragma once
#include "SkrImageCoder/skr_image_coder.configure.h"

#ifdef __cplusplus
    #include "module/module_manager.hpp"
    #include <containers/span.hpp>

class SKR_IMAGE_CODER_API SkrImageCoderModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;
};

#endif

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
struct SKR_IMAGE_CODER_API skr_image_coder_t {
    virtual bool set_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT = 0;
    virtual bool move_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT = 0;
    virtual bool view_encoded(const uint8_t* data, uint64_t size) SKR_NOEXCEPT = 0;
    virtual bool set_raw(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, 
        EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw) SKR_NOEXCEPT = 0;
    virtual bool move_raw(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, 
        EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw) SKR_NOEXCEPT = 0;
    virtual bool view_raw(const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, 
        EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw) SKR_NOEXCEPT = 0;

    virtual bool get_raw_data(uint8_t* pData, uint64_t* pSize) const SKR_NOEXCEPT = 0;
    virtual skr::span<const uint8_t> get_raw_data_view() const SKR_NOEXCEPT = 0;
    virtual bool get_encoded_data(uint8_t* pData, uint64_t* pSize) const SKR_NOEXCEPT = 0;
    virtual skr::span<const uint8_t> get_encoded_data_view() const SKR_NOEXCEPT = 0;

    virtual EImageCoderFormat get_image_format() const SKR_NOEXCEPT = 0;
    virtual EImageCoderColorFormat get_color_format() const SKR_NOEXCEPT = 0;
    virtual uint64_t get_raw_size() const SKR_NOEXCEPT = 0;
    virtual uint64_t get_encoded_size() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_height() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_bit_depth() const SKR_NOEXCEPT = 0;
};
#endif
typedef struct skr_image_coder_t skr_image_coder_t;
typedef struct skr_image_coder_t* skr_image_coder_id;

SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
skr_image_coder_id skr_image_coder_create_image(EImageCoderFormat format);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
void skr_image_coder_free_image(skr_image_coder_id image);

SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_set_encoded(skr_image_coder_id image, const uint8_t* data, uint64_t size);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_move_encoded(skr_image_coder_id image, const uint8_t* data, uint64_t size);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_view_encoded(skr_image_coder_id image, const uint8_t* data, uint64_t size);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_set_raw(skr_image_coder_id image, const uint8_t* data, 
    uint64_t size, uint32_t width, uint32_t height, 
    EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_move_raw(skr_image_coder_id image, const uint8_t* data, 
    uint64_t size, uint32_t width, uint32_t height, 
    EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_view_raw(skr_image_coder_id image, const uint8_t* data, 
    uint64_t size, uint32_t width, uint32_t height, 
    EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw);

SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_get_raw_data(skr_image_coder_id image, uint8_t* pData, uint64_t* pSize);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_get_raw_data_view(skr_image_coder_id image, uint8_t** ppData, uint64_t* pSize);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_get_encoded_data(skr_image_coder_id image, uint8_t* pData, uint64_t* pSize);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
bool skr_image_coder_get_encoded_data_view(skr_image_coder_id image, uint8_t** ppData, uint64_t* pSize);

SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
EImageCoderFormat skr_image_coder_get_image_format(skr_image_coder_id image);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
EImageCoderColorFormat skr_image_coder_get_color_format(skr_image_coder_id image);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
uint64_t skr_image_coder_get_raw_size(skr_image_coder_id image);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
uint64_t skr_image_coder_get_encoded_size(skr_image_coder_id image);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
uint32_t skr_image_coder_get_width(skr_image_coder_id image);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
uint32_t skr_image_coder_get_height(skr_image_coder_id image);
SKR_IMAGE_CODER_EXTERN_C SKR_IMAGE_CODER_API
uint32_t skr_image_coder_get_bit_depth(skr_image_coder_id image);