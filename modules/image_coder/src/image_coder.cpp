#include "utils/log.h"
#include "platform/memory.h"
#include "skr_image_coder/skr_image_coder.h"
#include "image_coder_png.hpp"

void SkrImageCoderModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("image coder module loaded!");
}

void SkrImageCoderModule::on_unload()
{
    SKR_LOG_INFO("image coder module unloaded!");
}

IMPLEMENT_DYNAMIC_MODULE(SkrImageCoderModule, SkrImageCoder);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrImageCoder",
    "prettyname" : "SakuraImageCoder",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRT", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrImageCoder)

skr_image_coder_id skr_image_coder_create_image(EImageCoderFormat format)
{
    switch (format)
    {
    case IMAGE_CODER_FORMAT_PNG:
        return SkrNew<skr::PNGImageCoder>();
    case IMAGE_CODER_FORMAT_JPEG:
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

void skr_image_coder_free_image(skr_image_coder_id image)
{
    SkrDelete(image);
}

bool skr_image_coder_set_encoded(skr_image_coder_id image, const uint8_t* data, uint64_t size)
{
    return image->set_encoded(data, size);
}

bool skr_image_coder_move_encoded(skr_image_coder_id image, const uint8_t *data, uint64_t size)
{
    return image->move_encoded(data, size);
}

bool skr_image_coder_set_raw(skr_image_coder_id image, const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw)
{
    return image->set_raw(data, size, width, height, format, bit_depth, bytes_per_raw);
}

bool skr_image_coder_move_raw(skr_image_coder_id image, const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw)
{
    return image->move_raw(data, size, width, height, format, bit_depth, bytes_per_raw);
}

bool skr_image_coder_get_raw_data_view(skr_image_coder_id image, uint8_t** pData, uint64_t* pSize)
{
    auto _ = image->get_raw_data_view();
    *pData = (uint8_t*)_.data();
    *pSize = _.size();
    return _.size();
}

bool skr_image_coder_get_raw_data(skr_image_coder_id image, uint8_t* pData, uint64_t* pSize)
{
    return image->get_raw_data(pData, pSize);
}

bool skr_image_coder_get_encoded_data_view(skr_image_coder_id image, uint8_t** pData, uint64_t* pSize)
{
    auto _ = image->get_encoded_data_view();
    *pData = (uint8_t*)_.data();
    *pSize = _.size();
    return _.size();
}

bool skr_image_coder_get_encoded_data(skr_image_coder_id image, uint8_t* pData, uint64_t* pSize)
{
    return image->get_encoded_data(pData, pSize);
}

EImageCoderFormat skr_image_coder_get_image_format(skr_image_coder_id image)
{
    return image->get_image_format();
}

EImageCoderColorFormat skr_image_coder_get_color_format(skr_image_coder_id image)
{
    return image->get_color_format();
}

uint64_t skr_image_coder_get_raw_size(skr_image_coder_id image)
{
    return image->get_raw_size();
}

uint64_t skr_image_coder_get_encoded_size(skr_image_coder_id image)
{
    return image->get_encoded_size();
}

uint32_t skr_image_coder_get_width(skr_image_coder_id image)
{
    return image->get_width();
}

uint32_t skr_image_coder_get_height(skr_image_coder_id image)
{
    return image->get_height();
}

uint32_t skr_image_coder_get_bit_depth(skr_image_coder_id image)
{
    return image->get_bit_depth();
}