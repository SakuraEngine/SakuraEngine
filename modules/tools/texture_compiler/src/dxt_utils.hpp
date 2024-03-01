#pragma once
#include "SkrTextureCompiler/texture_compiler.hpp"
#include "SkrImageCoder/skr_image_coder.h"
#include "SkrRenderer/resources/texture_resource.h"
#include "ispc/ispc_texcomp.h"

#define TEX_COMPRESS_ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

inline SKR_CONSTEXPR uint64_t Util_DXBCCompressedSize(uint32_t width, uint32_t height, ECGPUFormat format, uint32_t block_width = 4, uint32_t block_height = 4)
{
    const auto alignedW = TEX_COMPRESS_ALIGN(width, 4);
    const auto alignedH = TEX_COMPRESS_ALIGN(height, 4);
    const auto blocksW = alignedW / block_width;
    const auto blocksH = alignedH / block_height;
    switch (format)
    {
    case CGPU_FORMAT_DXBC1_RGB_UNORM:
    case CGPU_FORMAT_DXBC1_RGB_SRGB:
    case CGPU_FORMAT_DXBC1_RGBA_UNORM:
    case CGPU_FORMAT_DXBC1_RGBA_SRGB:
        return (blocksW * blocksH) * 8;
    case CGPU_FORMAT_DXBC3_UNORM:
    case CGPU_FORMAT_DXBC3_SRGB:
        return (blocksW * blocksH) * 16;
    case CGPU_FORMAT_DXBC4_UNORM:
    case CGPU_FORMAT_DXBC4_SNORM:
        return (blocksW * blocksH) * 8;
    case CGPU_FORMAT_DXBC6H_UFLOAT:
    case CGPU_FORMAT_DXBC6H_SFLOAT:
        return (blocksW * blocksH) * 16;
    case CGPU_FORMAT_DXBC7_UNORM:
    case CGPU_FORMAT_DXBC7_SRGB:
        return (blocksW * blocksH) * 16;
    default:
        return 0;
    }
}

inline static skr::String Util_CompressedTypeString(ECGPUFormat format)
{
    switch (format)
    {
    case CGPU_FORMAT_DXBC1_RGB_UNORM:
    case CGPU_FORMAT_DXBC1_RGB_SRGB:
    case CGPU_FORMAT_DXBC1_RGBA_UNORM:
    case CGPU_FORMAT_DXBC1_RGBA_SRGB:
        return u8"bc1";
    case CGPU_FORMAT_DXBC3_UNORM:
    case CGPU_FORMAT_DXBC3_SRGB:
        return u8"bc3";
    case CGPU_FORMAT_DXBC4_UNORM:
    case CGPU_FORMAT_DXBC4_SNORM:
        return u8"bc4";
    case CGPU_FORMAT_DXBC6H_UFLOAT:
    case CGPU_FORMAT_DXBC6H_SFLOAT:
        return u8"bc6";
    case CGPU_FORMAT_DXBC7_UNORM:
    case CGPU_FORMAT_DXBC7_SRGB:
        return u8"bc7";
    default:
        return {};
    }
}

inline static skr::Vector<uint8_t> Util_DXTCompressWithImageCoder(skr::ImageDecoderId decoder, ECGPUFormat compressed_format)
{
    // fetch RGBA data
    const auto bit_depth = decoder->get_bit_depth();
    const auto encoded_format = decoder->get_color_format();
    const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
    bool sucess = decoder->decode(raw_format, bit_depth);
    uint8_t* rgba_data = rgba_data = decoder->get_data();
    uint64_t rgba_size = decoder->get_size(); (void)rgba_size;
    if (!sucess)
    {
        SKR_UNREACHABLE_CODE()
        return {};
    }
    // compress
    rgba_surface rgba_surface = {};
    const auto image_width = decoder->get_width();
    const auto image_height = decoder->get_height();
    switch (raw_format)
    {
    case IMAGE_CODER_COLOR_FORMAT_RGBA:
    case IMAGE_CODER_COLOR_FORMAT_BGRA:
        rgba_surface.stride = image_width * 4;
        break;
    case IMAGE_CODER_COLOR_FORMAT_Gray:
    case IMAGE_CODER_COLOR_FORMAT_GrayF:
        rgba_surface.stride = image_width;
        break;
    default:
        break;
    }
    rgba_surface.ptr = rgba_data;
    rgba_surface.width = image_width;
    rgba_surface.height = image_height;
    const auto compressed_size = Util_DXBCCompressedSize(image_width, image_height, compressed_format);
    skr::Vector<uint8_t> compressed_data(compressed_size);
    switch (compressed_format)
    {
        case CGPU_FORMAT_DXBC1_RGB_UNORM:
        case CGPU_FORMAT_DXBC1_RGB_SRGB:
        case CGPU_FORMAT_DXBC1_RGBA_UNORM:
        case CGPU_FORMAT_DXBC1_RGBA_SRGB:
            CompressBlocksBC1(&rgba_surface, compressed_data.data());
            break;
        case CGPU_FORMAT_DXBC3_UNORM:
        case CGPU_FORMAT_DXBC3_SRGB:
            CompressBlocksBC3(&rgba_surface, compressed_data.data());
            break;
        case CGPU_FORMAT_DXBC4_UNORM:
        case CGPU_FORMAT_DXBC4_SNORM:
            CompressBlocksBC4(&rgba_surface, compressed_data.data());
            break;
        case CGPU_FORMAT_DXBC5_UNORM:
        case CGPU_FORMAT_DXBC5_SNORM:
            CompressBlocksBC5(&rgba_surface, compressed_data.data());
            break;
        case CGPU_FORMAT_DXBC6H_UFLOAT:
        case CGPU_FORMAT_DXBC6H_SFLOAT:
            {
                bc6h_enc_settings bc6_settings = {};
                GetProfile_bc6h_basic(&bc6_settings);
                CompressBlocksBC6H(&rgba_surface, compressed_data.data(), &bc6_settings);
            }
            break;
        case CGPU_FORMAT_DXBC7_UNORM:
        case CGPU_FORMAT_DXBC7_SRGB:
            {
                bc7_enc_settings bc7_settings = {};
                GetProfile_basic(&bc7_settings);
                CompressBlocksBC7(&rgba_surface, compressed_data.data(), &bc7_settings);
            }
            break;
        case CGPU_FORMAT_DXBC2_UNORM:
        case CGPU_FORMAT_DXBC2_SRGB:
        default:
            SKR_UNREACHABLE_CODE()
            return {};
    }
    return compressed_data;
} 