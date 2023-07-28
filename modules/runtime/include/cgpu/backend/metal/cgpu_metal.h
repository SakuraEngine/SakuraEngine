#pragma once
#include "cgpu/api.h"
#include "metal_availability.h"

#ifdef __cplusplus
extern "C" {
#endif

SKR_RUNTIME_API const CGPUProcTable* CGPU_MetalProcTable();
SKR_RUNTIME_API const CGPUSurfacesProcTable* CGPU_MetalSurfacesProcTable();

// Instance APIs
SKR_RUNTIME_API CGPUInstanceId cgpu_create_instance_metal(CGPUInstanceDescriptor const* descriptor);
SKR_RUNTIME_API void cgpu_query_instance_features_metal(CGPUInstanceId instance, struct CGPUInstanceFeatures* features);
SKR_RUNTIME_API void cgpu_free_instance_metal(CGPUInstanceId instance);

// Adapter APIs
SKR_RUNTIME_API void cgpu_enum_adapters_metal(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num);
SKR_RUNTIME_API const CGPUAdapterDetail* cgpu_query_adapter_detail_metal(const CGPUAdapterId adapter);
SKR_RUNTIME_API uint32_t cgpu_query_queue_count_metal(const CGPUAdapterId adapter, const ECGPUQueueType type);

// Device APIs
SKR_RUNTIME_API CGPUDeviceId cgpu_create_device_metal(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc);
SKR_RUNTIME_API void cgpu_free_device_metal(CGPUDeviceId device);

// API Object APIs
SKR_RUNTIME_API CGPUFenceId cgpu_create_fence_metal(CGPUDeviceId device);
SKR_RUNTIME_API void cgpu_free_fence_metal(CGPUFenceId fence);

// Queue APIs
SKR_RUNTIME_API CGPUQueueId cgpu_get_queue_metal(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);
SKR_RUNTIME_API void cgpu_submit_queue_metal(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc);
SKR_RUNTIME_API void cgpu_wait_queue_idle_metal(CGPUQueueId queue);
SKR_RUNTIME_API void cgpu_free_queue_metal(CGPUQueueId queue);

// Command APIs
SKR_RUNTIME_API CGPUCommandPoolId cgpu_create_command_pool_metal(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc);
SKR_RUNTIME_API CGPUCommandBufferId cgpu_create_command_buffer_metal(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc);
SKR_RUNTIME_API void cgpu_free_command_buffer_metal(CGPUCommandBufferId cmd);
SKR_RUNTIME_API void cgpu_free_command_pool_metal(CGPUCommandPoolId pool);

#ifdef __cplusplus
}
#endif