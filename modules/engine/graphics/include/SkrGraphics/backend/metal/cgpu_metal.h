#pragma once
#include "SkrGraphics/api.h"
#include "metal_availability.h"

#ifdef __cplusplus
extern "C" {
#endif

CGPU_API const CGPUProcTable* CGPU_MetalProcTable();
CGPU_API const CGPUSurfacesProcTable* CGPU_MetalSurfacesProcTable();

// Instance APIs
CGPU_API CGPUInstanceId cgpu_create_instance_metal(CGPUInstanceDescriptor const* descriptor);
CGPU_API void cgpu_query_instance_features_metal(CGPUInstanceId instance, struct CGPUInstanceFeatures* features);
CGPU_API void cgpu_free_instance_metal(CGPUInstanceId instance);

// Adapter APIs
CGPU_API void cgpu_enum_adapters_metal(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num);
CGPU_API const CGPUAdapterDetail* cgpu_query_adapter_detail_metal(const CGPUAdapterId adapter);
CGPU_API uint32_t cgpu_query_queue_count_metal(const CGPUAdapterId adapter, const ECGPUQueueType type);

// Device APIs
CGPU_API CGPUDeviceId cgpu_create_device_metal(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc);
CGPU_API void cgpu_free_device_metal(CGPUDeviceId device);

// API Object APIs
CGPU_API CGPUFenceId cgpu_create_fence_metal(CGPUDeviceId device);
CGPU_API void cgpu_free_fence_metal(CGPUFenceId fence);
CGPU_API CGPURootSignatureId cgpu_create_root_signature_metal(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc);
CGPU_API void cgpu_free_root_signature_metal(CGPURootSignatureId signature);

// Queue APIs
CGPU_API CGPUQueueId cgpu_get_queue_metal(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);
CGPU_API void cgpu_submit_queue_metal(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc);
CGPU_API void cgpu_wait_queue_idle_metal(CGPUQueueId queue);
CGPU_API void cgpu_free_queue_metal(CGPUQueueId queue);

// Command APIs
CGPU_API CGPUCommandPoolId cgpu_create_command_pool_metal(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc);
CGPU_API CGPUCommandBufferId cgpu_create_command_buffer_metal(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc);
CGPU_API void cgpu_free_command_buffer_metal(CGPUCommandBufferId cmd);
CGPU_API void cgpu_free_command_pool_metal(CGPUCommandPoolId pool);

// Shader APIs
CGPU_API CGPUShaderLibraryId cgpu_create_shader_library_metal(CGPUDeviceId device, const CGPUShaderLibraryDescriptor* desc);
CGPU_API void cgpu_free_shader_library_metal(CGPUShaderLibraryId library);

#ifdef __cplusplus
}
#endif