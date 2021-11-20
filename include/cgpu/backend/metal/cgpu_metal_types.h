#pragma once
#include "cgpu/api.h"
#ifndef __OBJC__
    #include <assert.h>
static_assert(0, "This Header Should Only Be Included By OBJC SOURCES!!!!!");
#endif
#import <MetalKit/MetalKit.h>

// Types
typedef struct CGpuDevice_Metal {
    CGpuDevice super;
    id<MTLDevice> pDevice;
} CGpuDevice_Metal;

typedef struct CGpuAdapter_Metal {
    CGpuAdapter super;
    CGpuDevice_Metal device;
} CGpuAdapter_Metal;

typedef struct CGpuInstance_Metal {
    CGpuInstance super;
    CGpuAdapter_Metal adapter;
} CGpuInstance_Metal;

/* clang-format off */
#define trans_case(PF, MPF) case PF: return MPF;
inline static MTLPixelFormat pf_translate_to_metal(const ECGpuPixelFormat format)
{
    switch (format)
    {
        trans_case(PF_UNDEFINED, MTLPixelFormatInvalid)
        trans_case(PF_R32G32B32A32_UINT, MTLPixelFormatRGBA32Uint)
        trans_case(PF_R32G32B32A32_SINT, MTLPixelFormatRGBA32Sint)
        trans_case(PF_R32G32B32A32_FLOAT, MTLPixelFormatRGBA32Float)
        trans_case(PF_R32G32B32_UINT, MTLPixelFormatInvalid)
        trans_case(PF_R32G32B32_SINT, MTLPixelFormatInvalid)
        trans_case(PF_R32G32B32_FLOAT, MTLPixelFormatInvalid)
        trans_case(PF_R16G16B16A16_UNORM, MTLPixelFormatRGBA16Unorm)
        trans_case(PF_R16G16B16A16_SNORM, MTLPixelFormatRGBA16Snorm)
        trans_case(PF_R16G16B16A16_UINT, MTLPixelFormatRGBA16Uint)
        trans_case(PF_R16G16B16A16_SINT, MTLPixelFormatRGBA16Sint)
        trans_case(PF_R16G16B16A16_FLOAT, MTLPixelFormatRGBA16Float)
        trans_case(PF_R32G32_UINT, MTLPixelFormatRG32Uint)
        trans_case(PF_R32G32_SINT, MTLPixelFormatRG32Sint)
        trans_case(PF_R32G32_FLOAT, MTLPixelFormatRG32Float) 
        trans_case(PF_R10G10B10A2_UNORM, MTLPixelFormatRGB10A2Unorm) 
        trans_case(PF_R10G10B10A2_UINT, MTLPixelFormatRGB10A2Uint) 
        trans_case(PF_R9G9B9E5_UFLOAT, MTLPixelFormatRGB9E5Float) 
        trans_case(PF_R8G8B8A8_UNORM, MTLPixelFormatRGBA8Unorm) 
        trans_case(PF_R8G8B8A8_SNORM, MTLPixelFormatRGBA8Snorm) 
        trans_case(PF_R8G8B8A8_UINT, MTLPixelFormatRGBA8Uint) 
        trans_case(PF_R8G8B8A8_SINT, MTLPixelFormatRGBA8Sint) 
        trans_case(PF_R8G8B8A8_UNORM_SRGB, MTLPixelFormatRGBA8Unorm_sRGB) 
        trans_case(PF_B8G8R8A8_UNORM, MTLPixelFormatBGRA8Unorm) 
        trans_case(PF_B8G8R8A8_UNORM_SRGB, MTLPixelFormatBGRA8Unorm_sRGB) 
        trans_case(PF_R11G11B10_FLOAT, MTLPixelFormatRG11B10Float) 
        trans_case(PF_R16G16_UNORM, MTLPixelFormatRG16Unorm) 
        trans_case(PF_R16G16_SNORM, MTLPixelFormatRG16Snorm) 
        trans_case(PF_R16G16_UINT, MTLPixelFormatRG16Uint) 
        trans_case(PF_R16G16_SINT, MTLPixelFormatRG16Sint) 
        trans_case(PF_R16G16_FLOAT, MTLPixelFormatRG16Float) 
        trans_case(PF_R32_UINT, MTLPixelFormatR32Uint) 
        trans_case(PF_R32_SINT, MTLPixelFormatR32Sint) 
        trans_case(PF_R32_FLOAT, MTLPixelFormatR32Float) 
        trans_case(PF_B5G5R5A1_UNORM, MTLPixelFormatBGR5A1Unorm) 
        trans_case(PF_B5G6R5_UNORM, MTLPixelFormatB5G6R5Unorm) 
        trans_case(PF_R8G8_UNORM, MTLPixelFormatRG8Unorm) 
        trans_case(PF_R8G8_SNORM, MTLPixelFormatRG8Snorm) 
        trans_case(PF_R8G8_UINT, MTLPixelFormatRG8Uint) 
        trans_case(PF_R8G8_SINT, MTLPixelFormatRG8Sint) 
        trans_case(PF_R16_UNORM, MTLPixelFormatR16Unorm) 
        trans_case(PF_R16_SNORM, MTLPixelFormatR16Snorm) 
        trans_case(PF_R16_UINT, MTLPixelFormatR16Uint) 
        trans_case(PF_R16_SINT, MTLPixelFormatR16Sint) 
        trans_case(PF_R8_UNORM, MTLPixelFormatR8Unorm) 
        trans_case(PF_R8_SNORM, MTLPixelFormatR8Snorm) 
        trans_case(PF_R8_UINT, MTLPixelFormatR8Uint) 
        trans_case(PF_R8_SINT, MTLPixelFormatR8Sint) 
        trans_case(PF_D24_UNORM_S8_UINT, MTLPixelFormatDepth24Unorm_Stencil8) 
        trans_case(PF_D32_FLOAT, MTLPixelFormatDepth32Float) 
        trans_case(PF_D16_UNORM, MTLPixelFormatDepth16Unorm) 
        trans_case(PF_BC1_UNORM, MTLPixelFormatBC1_RGBA) 
        trans_case(PF_BC1_UNORM_SRGB, MTLPixelFormatBC1_RGBA_sRGB) 
        trans_case(PF_BC2_UNORM, MTLPixelFormatBC2_RGBA) 
        trans_case(PF_BC2_UNORM_SRGB, MTLPixelFormatBC2_RGBA_sRGB) 
        trans_case(PF_BC3_UNORM, MTLPixelFormatBC3_RGBA) 
        trans_case(PF_BC3_UNORM_SRGB, MTLPixelFormatBC3_RGBA_sRGB) 
        trans_case(PF_BC4_UNORM, MTLPixelFormatBC4_RUnorm) 
        trans_case(PF_BC4_SNORM, MTLPixelFormatBC4_RSnorm) 
        trans_case(PF_BC5_UNORM, MTLPixelFormatBC5_RGUnorm) 
        trans_case(PF_BC5_SNORM, MTLPixelFormatBC5_RGSnorm) 
        trans_case(PF_BC6H_UF16, MTLPixelFormatBC6H_RGBUfloat) 
        trans_case(PF_BC6H_SF16, MTLPixelFormatBC6H_RGBFloat) 
        trans_case(PF_BC7_UNORM, MTLPixelFormatBC7_RGBAUnorm) 
        trans_case(PF_BC7_UNORM_SRGB, MTLPixelFormatBC7_RGBAUnorm_sRGB) 
        default: 
        {
            assert(0 && "format not supported!");
            return MTLPixelFormatInvalid;
        }
    }
}
#undef trans_case