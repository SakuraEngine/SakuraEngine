#include "metal_utils.h"
#import "cgpu/backend/metal/cgpu_metal_types.h"

uint32_t MetalUtilInner_GetGPUFamilyTier(struct CGpuAdapter_Metal* MAdapter);
void MetalUtilInner_EnumFormatShaderWriteSupports(struct CGpuAdapter_Metal* MAdapter, uint32_t familyTier);
void MetalUtilInner_EnumFormatShaderReadSupports(struct CGpuAdapter_Metal* MAdapter, uint32_t familyTier);
void MetalUtilInner_EnumFormatRenderTargetWriteSupports(struct CGpuAdapter_Metal* MAdapter, uint32_t familyTier);

#define CAN_SHADER_READ(x) MAdapter->adapter_detail.format_supports[PF_##x].shader_read = true;
#define SET_SHADER_READ(x, opt) MAdapter->adapter_detail.format_supports[PF_##x].shader_read = (opt);
#define CAN_SHADER_WRITE(x) MAdapter->adapter_detail.format_supports[PF_##x].shader_write = true;
#define SET_SHADER_WRITE(x, opt) MAdapter->adapter_detail.format_supports[PF_##x].shader_write = (opt);
#define CAN_RENDER_TARGET_WRITE(x) MAdapter->adapter_detail.format_supports[PF_##x].render_target_write = true;
#define SET_RENDER_TARGET_WRITE(x, opt) MAdapter->adapter_detail.format_supports[PF_##x].render_target_write = (opt);
void MetalUtil_EnumFormatSupports(struct CGpuAdapter_Metal* MAdapter)
{
    for (uint32_t i = 0; i < PF_Count; ++i)
    {
        MAdapter->adapter_detail.format_supports[i].shader_read = 0;
        MAdapter->adapter_detail.format_supports[i].shader_write = 0;
        MAdapter->adapter_detail.format_supports[i].render_target_write = 0;
    }
    uint32_t familyTier = MetalUtilInner_GetGPUFamilyTier(MAdapter);
    MetalUtilInner_EnumFormatShaderReadSupports(MAdapter, familyTier);
    MetalUtilInner_EnumFormatShaderWriteSupports(MAdapter, familyTier);
    MetalUtilInner_EnumFormatRenderTargetWriteSupports(MAdapter, familyTier);
    return;
}

// Inner Utils
uint32_t MetalUtilInner_GetGPUFamilyTier(struct CGpuAdapter_Metal* MAdapter)
{
    uint32_t familyTier = 0;
#ifdef TARGET_MACOS
    familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v1] ? 1 : familyTier;
    #if defined(ENABLE_GPU_FAMILY_1_V2)
    if (@available(macOS 10.12, *))
    {
        familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v2] ? 1 : familyTier;
    }
    #endif
    #if defined(ENABLE_GPU_FAMILY_1_V3)
    if (@available(macOS 10.13, *))
    {
        familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v3] ? 1 : familyTier;
    }
    #endif
    #if defined(ENABLE_GPU_FAMILY_1_V4)
    if (@available(macOS 10.14, *))
    {
        familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v4] ? 1 : familyTier;
    }
    #endif
#elif defined(TARGET_IOS)
    // Tier 1
    familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1] ? 1 : familyTier;

    // Tier 2
    familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1] ? 2 : familyTier;

    // Tier 3
    #if defined(ENABLE_GPU_FAMILY_3)
    if (@available(iOS 9.0, *))
    {
        familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1] ? 3 : familyTier;
    }
    #endif
    #if defined(ENABLE_GPU_FAMILY_4)
    // Tier 4
    if (@available(iOS 11.0, *))
    {
        familyTier = [MAdapter->device.pDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1] ? 4 : familyTier;
    }
    #endif
#endif
    return familyTier;
}

void MetalUtilInner_EnumFormatShaderReadSupports(struct CGpuAdapter_Metal* MAdapter, uint32_t familyTier)
{
    for (uint32_t i = 0; i < PF_Count; ++i)
    {
        MTLPixelFormat mformat = MetalUtil_TranslatePixelFormat((ECGpuFormat)i);
        if (mformat != MTLPixelFormatInvalid)
        {
#ifndef TARGET_IOS
            MAdapter->adapter_detail.format_supports[i].shader_read = MetalFormatOkayOnMac(mformat);
#else
            MAdapter->adapter_detail.format_supports[i].shader_read = MetalFormatOkayOnIOS(mformat);
#endif
        }
    }
#ifndef TARGET_IOS
    bool depth24Stencil8Supported = [MAdapter->device.pDevice isDepth24Stencil8PixelFormatSupported];
    SET_SHADER_READ(D24_UNORM_S8_UINT, depth24Stencil8Supported)
    SET_SHADER_READ(D24_UNORM_S8_UINT, depth24Stencil8Supported)
    SET_SHADER_READ(X8_D24_UNORM, depth24Stencil8Supported)
    SET_SHADER_READ(D24_UNORM_S8_UINT, depth24Stencil8Supported)
    SET_SHADER_READ(X8_D24_UNORM, depth24Stencil8Supported)
    SET_SHADER_READ(D24_UNORM_S8_UINT, depth24Stencil8Supported)
    SET_SHADER_READ(X8_D24_UNORM, depth24Stencil8Supported)
#else
    if (familyTier == 1)
    {
        SET_SHADER_READ(ASTC_4x4_UNORM, false)
        // this is a tier 1 decide so no astc and XR
        SET_SHADER_READ(ASTC_4x4_SRGB, false)
        SET_SHADER_READ(ASTC_5x4_UNORM, false)
        SET_SHADER_READ(ASTC_5x4_SRGB, false)
        SET_SHADER_READ(ASTC_5x5_UNORM, false)
        SET_SHADER_READ(ASTC_5x5_SRGB, false)
        SET_SHADER_READ(ASTC_6x5_UNORM, false)
        SET_SHADER_READ(ASTC_6x5_SRGB, false)
        SET_SHADER_READ(ASTC_6x6_UNORM, false)
        SET_SHADER_READ(ASTC_6x6_SRGB, false)
        SET_SHADER_READ(ASTC_8x5_UNORM, false)
        SET_SHADER_READ(ASTC_8x5_SRGB, false)
        SET_SHADER_READ(ASTC_8x6_UNORM, false)
        SET_SHADER_READ(ASTC_8x6_SRGB, false)
        SET_SHADER_READ(ASTC_8x8_UNORM, false)
        SET_SHADER_READ(ASTC_8x8_SRGB, false)
        SET_SHADER_READ(ASTC_10x5_UNORM, false)
        SET_SHADER_READ(ASTC_10x5_SRGB, false)
        SET_SHADER_READ(ASTC_10x6_UNORM, false)
        SET_SHADER_READ(ASTC_10x6_SRGB, false)
        SET_SHADER_READ(ASTC_10x8_UNORM, false)
        SET_SHADER_READ(ASTC_10x8_SRGB, false)
        SET_SHADER_READ(ASTC_10x10_UNORM, false)
        SET_SHADER_READ(ASTC_10x10_SRGB, false)
        SET_SHADER_READ(ASTC_12x10_UNORM, false)
        SET_SHADER_READ(ASTC_12x10_SRGB, false)
        SET_SHADER_READ(ASTC_12x12_UNORM, false)
        SET_SHADER_READ(ASTC_12x12_SRGB, false)
        // TODO when TinyImageFormat supports XR formats exclude them here for tier 1
    }
#endif
}

void MetalUtilInner_EnumFormatShaderWriteSupports(struct CGpuAdapter_Metal* MAdapter, uint32_t familyTier)
{
#if defined(ENABLE_TEXTURE_READ_WRITE)
    if (@available(macOS 10.13, iOS 11.0, *))
    {
        // this call is supported on mac and ios
        // technically I think you can write but not read some texture, this is telling
        // you you can do both. TODO work out the semantics behind write vs read/write.
        MTLReadWriteTextureTier rwTextureTier = [MAdapter->device.pDevice readWriteTextureSupport];
        // intentional fall through on this switch
        switch (rwTextureTier)
        {
            default:
            case MTLReadWriteTextureTier2:
                CAN_SHADER_WRITE(R32G32B32A32_SFLOAT);
                CAN_SHADER_WRITE(R32G32B32A32_UINT);
                CAN_SHADER_WRITE(R32G32B32A32_SINT);
                CAN_SHADER_WRITE(R16G16B16A16_SFLOAT);
                CAN_SHADER_WRITE(R16G16B16A16_UINT);
                CAN_SHADER_WRITE(R16G16B16A16_SINT);
                CAN_SHADER_WRITE(R8G8B8A8_UNORM);
                CAN_SHADER_WRITE(R8G8B8A8_UINT);
                CAN_SHADER_WRITE(R8G8B8A8_SINT);
                CAN_SHADER_WRITE(R16_SFLOAT);
                CAN_SHADER_WRITE(R16_UINT);
                CAN_SHADER_WRITE(R16_SINT);
                CAN_SHADER_WRITE(R8_UNORM);
                CAN_SHADER_WRITE(R8_UINT);
                CAN_SHADER_WRITE(R8_SINT);
            case MTLReadWriteTextureTier1:
                CAN_SHADER_WRITE(R32_SFLOAT);
                CAN_SHADER_WRITE(R32_UINT);
                CAN_SHADER_WRITE(R32_SINT);
            case MTLReadWriteTextureTierNone:
                break;
        }
    }
#endif
}

void MetalUtilInner_EnumFormatRenderTargetWriteSupports(struct CGpuAdapter_Metal* MAdapter, uint32_t familyTier)
{
#ifndef TARGET_IOS
    if (familyTier >= 1)
    {
        CAN_RENDER_TARGET_WRITE(R8_UNORM); // this has a subscript 8 which makes no sense
        CAN_RENDER_TARGET_WRITE(R8_SNORM);
        CAN_RENDER_TARGET_WRITE(R8_UINT);
        CAN_RENDER_TARGET_WRITE(R8_SINT);

        CAN_RENDER_TARGET_WRITE(R16_UNORM);
        CAN_RENDER_TARGET_WRITE(R16_SNORM);
        CAN_RENDER_TARGET_WRITE(R16_SFLOAT);
        CAN_RENDER_TARGET_WRITE(R16_UINT);
        CAN_RENDER_TARGET_WRITE(R16_SINT);

        CAN_RENDER_TARGET_WRITE(R8G8_UNORM);
        CAN_RENDER_TARGET_WRITE(R8G8_SNORM);
        CAN_RENDER_TARGET_WRITE(R8G8_UINT);
        CAN_RENDER_TARGET_WRITE(R8G8_SINT);

        CAN_RENDER_TARGET_WRITE(R8G8B8A8_UNORM);
        CAN_RENDER_TARGET_WRITE(R8G8B8A8_SNORM);
        CAN_RENDER_TARGET_WRITE(R8G8B8A8_SRGB);
        CAN_RENDER_TARGET_WRITE(B8G8R8A8_UNORM);
        CAN_RENDER_TARGET_WRITE(B8G8R8A8_SRGB);
        CAN_RENDER_TARGET_WRITE(R16G16_UNORM);
        CAN_RENDER_TARGET_WRITE(R16G16_SNORM);
        CAN_RENDER_TARGET_WRITE(R16G16_SFLOAT);
        CAN_RENDER_TARGET_WRITE(R32_SFLOAT);
        CAN_RENDER_TARGET_WRITE(A2R10G10B10_UNORM);
        CAN_RENDER_TARGET_WRITE(A2B10G10R10_UNORM);
        CAN_RENDER_TARGET_WRITE(B10G11R11_UFLOAT);

        CAN_RENDER_TARGET_WRITE(R16G16B16A16_UNORM);
        CAN_RENDER_TARGET_WRITE(R16G16B16A16_SNORM);
        CAN_RENDER_TARGET_WRITE(R16G16B16A16_SFLOAT);
        CAN_RENDER_TARGET_WRITE(R32G32_SFLOAT);

        CAN_RENDER_TARGET_WRITE(R32G32B32A32_SFLOAT);

        CAN_RENDER_TARGET_WRITE(D16_UNORM);
        CAN_RENDER_TARGET_WRITE(D32_SFLOAT);
        CAN_RENDER_TARGET_WRITE(S8_UINT);
        CAN_RENDER_TARGET_WRITE(D32_SFLOAT_S8_UINT);
    };
#else
    if (familyTier >= 1)
    {
        CAN_RENDER_TARGET_WRITE(R8_UNORM); // this has a subscript 8 which makes no sense
        CAN_RENDER_TARGET_WRITE(R8_SNORM);
        CAN_RENDER_TARGET_WRITE(R8_UINT);
        CAN_RENDER_TARGET_WRITE(R8_SINT);
        CAN_RENDER_TARGET_WRITE(R8_SRGB);

        CAN_RENDER_TARGET_WRITE(R16_UNORM);
        CAN_RENDER_TARGET_WRITE(R16_SNORM);
        CAN_RENDER_TARGET_WRITE(R16_SFLOAT);
        CAN_RENDER_TARGET_WRITE(R16_UINT);
        CAN_RENDER_TARGET_WRITE(R16_SINT);

        CAN_RENDER_TARGET_WRITE(R8G8_UNORM);
        CAN_RENDER_TARGET_WRITE(R8G8_SNORM);
        CAN_RENDER_TARGET_WRITE(R8G8_SRGB);
        CAN_RENDER_TARGET_WRITE(R8G8_UINT);
        CAN_RENDER_TARGET_WRITE(R8G8_SINT);

        CAN_RENDER_TARGET_WRITE(B5G6R5_UNORM);
        CAN_RENDER_TARGET_WRITE(A1R5G5B5_UNORM);
        CAN_RENDER_TARGET_WRITE(A4B4G4R4_UNORM);
        CAN_RENDER_TARGET_WRITE(R5G6B5_UNORM);
        CAN_RENDER_TARGET_WRITE(B5G5R5A1_UNORM);

        CAN_RENDER_TARGET_WRITE(R8G8B8A8_UNORM);
        CAN_RENDER_TARGET_WRITE(R8G8B8A8_SNORM);
        CAN_RENDER_TARGET_WRITE(R8G8B8A8_SRGB);
        CAN_RENDER_TARGET_WRITE(B8G8R8A8_UNORM);
        CAN_RENDER_TARGET_WRITE(B8G8R8A8_SRGB);
        CAN_RENDER_TARGET_WRITE(R16G16_UNORM);
        CAN_RENDER_TARGET_WRITE(R16G16_SNORM);
        CAN_RENDER_TARGET_WRITE(R16G16_SFLOAT);
        CAN_RENDER_TARGET_WRITE(R32_SFLOAT);
        CAN_RENDER_TARGET_WRITE(A2R10G10B10_UNORM);
        CAN_RENDER_TARGET_WRITE(A2B10G10R10_UNORM);
        CAN_RENDER_TARGET_WRITE(B10G11R11_UFLOAT);

        CAN_RENDER_TARGET_WRITE(R16G16B16A16_UNORM);
        CAN_RENDER_TARGET_WRITE(R16G16B16A16_SNORM);
        CAN_RENDER_TARGET_WRITE(R16G16B16A16_SFLOAT);
        CAN_RENDER_TARGET_WRITE(R32G32_SFLOAT);

        CAN_RENDER_TARGET_WRITE(R32G32B32A32_SFLOAT);

        CAN_RENDER_TARGET_WRITE(D32_SFLOAT);
        CAN_RENDER_TARGET_WRITE(S8_UINT);
        CAN_RENDER_TARGET_WRITE(D32_SFLOAT_S8_UINT);
        CAN_RENDER_TARGET_WRITE(D32_SFLOAT_S8_UINT);
    }

    if (@available(iOS 13, *))
    {
        SET_SHADER_WRITE(D16_UNORM, true)
        SET_RENDER_TARGET_WRITE(D16_UNORM, true)
    }
#endif
}