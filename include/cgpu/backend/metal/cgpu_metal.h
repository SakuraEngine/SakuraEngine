#pragma once
#include "cgpu/api.h"
#include "metal_availability.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNTIME_API const CGpuProcTable* CGPU_MetalProcTable();
RUNTIME_API const CGpuSurfacesProcTable* CGPU_MetalSurfacesProcTable();

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance_metal(CGpuInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_metal(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_metal(CGpuInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters_metal(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API const CGpuAdapterDetail* cgpu_query_adapter_detail_metal(const CGpuAdapterId adapter);
RUNTIME_API uint32_t cgpu_query_queue_count_metal(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device_metal(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_metal(CGpuDeviceId device);

// API Object APIs
RUNTIME_API CGpuFenceId cgpu_create_fence_metal(CGpuDeviceId device);
RUNTIME_API void cgpu_free_fence_metal(CGpuFenceId fence);

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue_metal(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue_metal(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle_metal(CGpuQueueId queue);
RUNTIME_API void cgpu_free_queue_metal(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool_metal(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc);
RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer_metal(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_free_command_buffer_metal(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool_metal(CGpuCommandPoolId pool);

#ifdef __cplusplus
}
#endif