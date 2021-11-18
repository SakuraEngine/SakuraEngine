#define trans_case2(_format, _vkformat) \
    case PF_##_format:                  \
        return VK_FORMAT_##_vkformat
#define trans_case(_format) \
    case PF_##_format:      \
        return VK_FORMAT_##_format

inline static VkFormat pf_translate_to_vulkan(const ECGpuPixelFormat format)
{
    switch (format)
    {
        trans_case(R32G32B32A32_UINT);
        trans_case(R32G32B32A32_SINT);
        trans_case2(R32G32B32A32_FLOAT, R32G32B32A32_SFLOAT);
        trans_case(R32G32B32_UINT);
        trans_case(R32G32B32_SINT);
        trans_case2(R32G32B32_FLOAT, R32G32B32_SFLOAT);
        trans_case(R16G16B16A16_UNORM);
        trans_case(R16G16B16A16_SNORM);
        trans_case(R16G16B16A16_UINT);
        trans_case(R16G16B16A16_SINT);
        trans_case2(R16G16B16A16_FLOAT, R16G16B16A16_SFLOAT);
        trans_case(R32G32_UINT);
        trans_case(R32G32_SINT);
        trans_case2(R32G32_FLOAT, R32G32_SFLOAT);
        trans_case2(R10G10B10A2_UNORM, A2R10G10B10_UNORM_PACK32);
        trans_case2(R10G10B10A2_UINT, A2R10G10B10_UINT_PACK32);
        trans_case2(R9G9B9E5_UFLOAT, E5B9G9R9_UFLOAT_PACK32);
        trans_case(R8G8B8A8_UNORM);
        trans_case(R8G8B8A8_SNORM);
        trans_case(R8G8B8A8_UINT);
        trans_case(R8G8B8A8_SINT);
        trans_case2(R8G8B8A8_UNORM_SRGB, ETC2_R8G8B8A8_SRGB_BLOCK);
        trans_case(B8G8R8A8_UNORM);
        trans_case2(B8G8R8A8_UNORM_SRGB, B8G8R8A8_SRGB);
        trans_case2(R11G11B10_FLOAT, B10G11R11_UFLOAT_PACK32);
        trans_case(R16G16_UNORM);
        trans_case(R16G16_SNORM);
        trans_case(R16G16_UINT);
        trans_case(R16G16_SINT);
        trans_case2(R16G16_FLOAT, R16G16_SFLOAT);
        trans_case(R32_UINT);
        trans_case(R32_SINT);
        trans_case2(R32_FLOAT, R32_SFLOAT);
        trans_case2(B5G5R5A1_UNORM, B5G5R5A1_UNORM_PACK16);
        trans_case2(B5G6R5_UNORM, B5G6R5_UNORM_PACK16);
        trans_case(R8G8_UNORM);
        trans_case(R8G8_SNORM);
        trans_case(R8G8_UINT);
        trans_case(R8G8_SINT);
        trans_case(R16_UNORM);
        trans_case(R16_SNORM);
        trans_case(R16_UINT);
        trans_case(R16_SINT);
        trans_case2(R16_FLOAT, R16_SFLOAT);
        trans_case(R8_UNORM);
        trans_case(R8_SNORM);
        trans_case(R8_UINT);
        trans_case(R8_SINT);
        trans_case(D24_UNORM_S8_UINT);
        trans_case2(D32_FLOAT, D32_SFLOAT);
        trans_case(D16_UNORM);
        trans_case2(BC1_UNORM, BC1_RGB_UNORM_BLOCK);
        trans_case2(BC1_UNORM_SRGB, BC1_RGB_SRGB_BLOCK);
        trans_case2(BC2_UNORM, BC2_UNORM_BLOCK);
        trans_case2(BC2_UNORM_SRGB, BC2_SRGB_BLOCK);
        trans_case2(BC3_UNORM, BC3_UNORM_BLOCK);
        trans_case2(BC3_UNORM_SRGB, BC3_SRGB_BLOCK);
        trans_case2(BC4_UNORM, BC4_UNORM_BLOCK);
        trans_case2(BC4_SNORM, BC4_SNORM_BLOCK);
        trans_case2(BC5_UNORM, BC5_UNORM_BLOCK);
        trans_case2(BC5_SNORM, BC5_SNORM_BLOCK);
        trans_case2(BC6H_UF16, BC6H_UFLOAT_BLOCK);
        trans_case2(BC6H_SF16, BC6H_SFLOAT_BLOCK);
        trans_case2(BC7_UNORM, BC7_UNORM_BLOCK);
        trans_case2(BC7_UNORM_SRGB, BC7_SRGB_BLOCK);
        default:
            return VK_FORMAT_UNDEFINED;
    }
    return VK_FORMAT_UNDEFINED;
}

#undef trans_case
#undef trans_case2