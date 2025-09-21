#pragma once
#include <Foundation/NSArray.h>
#include <Metal/Metal.h>
#include <Metal/MTLArgument.h>

#include "SkrGraphics/backend/metal/cgpu_metal_types.h"
#include "SkrGraphics/backend/metal/metal_availability.h"
#include "metal_vma.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct CGPUAdapter_Metal CGPUAdapter_Metal;

typedef enum EMTLUtilEncoderType {
    MTLUtilEncoderTypeBlit = 1,
    MTLUtilEncoderTypeAS = 2
} EMTLUtilEncoderType;
typedef uint32_t MTLUtilEncoderTypes;

// Feature Select Helpers
void MetalUtil_EnumFormatSupports(struct CGPUAdapter_Metal* MAdapter);
void MetalUtil_RecordAdapterDetail(struct CGPUAdapter_Metal* MAdapter);

NSArray<id<MTLDevice>>* MetalUtil_GetAvailableMTLDeviceArray();
ECGPUResourceType MetalUtil_GetShaderResourceType(id<MTLBufferBinding> SRT, uint32_t set, const MTLStructMember* member, CGPUShaderResource* resource);
MemoryType MetalUtil_MemoryUsageToMemoryType(ECGPUMemoryUsage usage);
MTLResourceOptions MetalUtil_MemoryTypeToResourceOptions(MemoryType usage);
bool MetalUtil_DSHasBindAtIndex(const CGPUDescriptorSet_Metal* ds, uint32_t binding_index, uint32_t* out_index);
bool MetalUtil_DSBindResourceAtIndex(CGPUDescriptorSet_Metal* ds, uint32_t binding_index, __unsafe_unretained id<MTLResource> resource, MTLResourceUsage usage);
char8_t* MetalUtil_DuplicateString(const char8_t* src_string);
void MetalUtil_FlushUtilEncoders(CGPUCommandBuffer_Metal* CMD, MTLUtilEncoderTypes types);

id<MTLTexture> MetalUtil_CreateTextureView(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc);

// Blit utilities
id<MTLRenderPipelineState> MetalUtil_GetBlitPipeline(CGPUDevice_Metal* device, MTLPixelFormat format);
id<MTLSamplerState> MetalUtil_GetLinearSampler(CGPUDevice_Metal* device);

inline static MTLTextureType MetalUtil_TranslateTextureType(ECGPUTextureDimension dim, uint32_t array_size, bool is_cube)
{
    if (is_cube)
    {
        return array_size > 1 ? MTLTextureTypeCubeArray : MTLTextureTypeCube;
    }
    
    switch (dim)
    {
        case CGPU_TEXTURE_DIMENSION_1D:
            return array_size > 1 ? MTLTextureType1DArray : MTLTextureType1D;
        case CGPU_TEXTURE_DIMENSION_2D:
            return array_size > 1 ? MTLTextureType2DArray : MTLTextureType2D;
        case CGPU_TEXTURE_DIMENSION_3D:
            return MTLTextureType3D;
        default:
            cgpu_assert(false && "Invalid texture dimension");
            return MTLTextureType2D;
    }
}

#ifdef __cplusplus
}
#endif