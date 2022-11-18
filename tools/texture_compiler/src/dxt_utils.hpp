#pragma once
#include "SkrTextureCompiler/texture_compiler.hpp"
#include "skr_image_coder/skr_image_coder.h"
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

inline static eastl::string Util_CompressedTypeString(ECGPUFormat format)
{
    switch (format)
    {
    case CGPU_FORMAT_DXBC1_RGB_UNORM:
    case CGPU_FORMAT_DXBC1_RGB_SRGB:
    case CGPU_FORMAT_DXBC1_RGBA_UNORM:
    case CGPU_FORMAT_DXBC1_RGBA_SRGB:
        return "bc1";
    case CGPU_FORMAT_DXBC3_UNORM:
    case CGPU_FORMAT_DXBC3_SRGB:
        return "bc3";
    case CGPU_FORMAT_DXBC4_UNORM:
    case CGPU_FORMAT_DXBC4_SNORM:
        return "bc4";
    case CGPU_FORMAT_DXBC6H_UFLOAT:
    case CGPU_FORMAT_DXBC6H_SFLOAT:
        return "bc6";
    case CGPU_FORMAT_DXBC7_UNORM:
    case CGPU_FORMAT_DXBC7_SRGB:
        return "bc7";
    default:
        return {};
    }
}

FORCEINLINE static eastl::vector<uint8_t> Util_DXTCompressWithImageCoder(skr_image_coder_id image_coder, ECGPUFormat compressed_format)
{
    // fetch RGBA data
    uint8_t* rgba_data = nullptr;
    uint64_t rgba_size = 0;
    const auto bit_depth = image_coder->get_bit_depth();
    const auto encoded_format = image_coder->get_color_format();
    const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
    bool sucess = skr_image_coder_get_raw_data_view(image_coder, &rgba_data, &rgba_size, raw_format, bit_depth);
    if (!sucess)
    {
        SKR_UNREACHABLE_CODE()
        return {};
    }
    // compress
    const auto image_width = skr_image_coder_get_width(image_coder);
    const auto image_height = skr_image_coder_get_height(image_coder);
    const auto compressed_size = Util_DXBCCompressedSize(image_width, image_height, compressed_format);
    eastl::vector<uint8_t> compressed_data(compressed_size);
    rgba_surface rgba_surface = {};
    rgba_surface.ptr = rgba_data;
    rgba_surface.width = image_width;
    rgba_surface.height = image_height;
    rgba_surface.stride = image_width * 4;
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