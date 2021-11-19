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
        trans_case(PF_R32G32_FLOAT, MTLPixelFormatRG32Float) //14
        default: 
        {
            assert(0 && "format not supported!");
            return MTLPixelFormatInvalid;
        }
    }
}
#undef trans_case