#pragma once
#include <Foundation/NSArray.h>
#include <Metal/Metal.h>
#include <Metal/MTLArgument.h>

#include "SkrGraphics/backend/metal/cgpu_metal_types.h"
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
void MetalUtil_GetShaderResourceType(uint32_t set, const MTLStructMember* member, CGPUShaderResource* resource);
MTLBindingAccess MetalUtil_ResourceTypeToAccess(ECGPUResourceType type);
MemoryType MetalUtil_MemoryUsageToMemoryType(ECGPUMemoryUsage usage);
MTLResourceOptions MetalUtil_MemoryTypeToResourceOptions(MemoryType usage);
bool MetalUtil_DSHasBindAtIndex(const CGPUDescriptorSet_Metal* ds, uint32_t binding_index, uint32_t* out_index);
bool MetalUtil_DSBindResourceAtIndex(CGPUDescriptorSet_Metal* ds, uint32_t binding_index, __unsafe_unretained id<MTLResource> resource, MTLResourceUsage usage);
char8_t* MetalUtil_DuplicateString(const char8_t* src_string);
void MetalUtil_FlushUtilEncoders(CGPUCommandBuffer_Metal* CMD, MTLUtilEncoderTypes types);

#ifdef __cplusplus
}
#endif