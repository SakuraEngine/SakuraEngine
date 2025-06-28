#pragma once
#include "SkrGraphics/api.h"
#include "SkrGraphics/raytracing.h"

/* Resouce Binding Schema:

template <typename T>
struct Buffer
{
    constant T* cgpu_buffer_data;
    size_t cgpu_buffer_size;
};

template <typename T>
struct RWBuffer
{
    device T* cgpu_buffer_data;
    size_t cgpu_buffer_size;
};

template <typename T, access a>
struct Texture2D
{
    texture2d<T, a> cgpu_texture;
};

SRT(DescriptorSet): 
    struct SRT { Buffer& [id(0)]; Texture& [id(1)]; Sampler& [id(2)]; }; 
    SRT& srt [[buffer(0)]];

Descriptors:
    ConstantBuffer:
        template <typename T> struct ConstantBuffer { T data; };
        SINGLE: ConstantBuffer<T> cb;

    Buffer: 
        template <typename T> struct Buffer   { constant T* data; }; 
        template <typename T> struct RWBuffer { device T* data; }; 
        SINGLE: Buffer<T> b;
        ARRAY:  Buffer<T> b[N];       (WITH SIZE)
                device RWBuffer<T>* b;       (BINDLESS)
                constant Buffer<T>* b;       (BINDLESS)
    Texture: 
        template <typename T, access a> struct Texture2D { texture2d<T, a> t; };
        SINGLE: Texture2D<T, a> t;
        ARRAY:  Texture2D<T, a> t[N]; (WITH SIZE)
                constant Texture2D<T, a>* t; (BINDLESS)
*/

#ifdef __cplusplus
extern "C" {
#endif

CGPU_API const struct CGPUProcTable* CGPU_MetalProcTable();
CGPU_API const struct CGPUSurfacesProcTable* CGPU_MetalSurfacesProcTable();
CGPU_API const struct CGPURayTracingProcTable* CGPU_MetalRayTracingProcTable();

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
CGPU_API void cgpu_query_video_memory_info_metal(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
CGPU_API void cgpu_query_shared_memory_info_metal(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
CGPU_API void cgpu_free_device_metal(CGPUDeviceId device);

// API Object APIs
CGPU_API CGPUFenceId cgpu_create_fence_metal(CGPUDeviceId device);
CGPU_API void cgpu_wait_fences_metal(const CGPUFenceId* fences, uint32_t fence_count);
CGPU_API ECGPUFenceStatus cgpu_query_fence_status_metal(CGPUFenceId fence);
CGPU_API void cgpu_free_fence_metal(CGPUFenceId fence);
CGPU_API CGPUSemaphoreId cgpu_create_semaphore_metal(CGPUDeviceId device);
CGPU_API void cgpu_free_semaphore_metal(CGPUSemaphoreId semaphore);
CGPU_API CGPURootSignaturePoolId cgpu_create_root_signature_pool_metal(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc);
CGPU_API void cgpu_free_root_signature_pool_metal(CGPURootSignaturePoolId pool);
CGPU_API CGPURootSignatureId cgpu_create_root_signature_metal(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc);
CGPU_API void cgpu_free_root_signature_metal(CGPURootSignatureId signature);
CGPU_API CGPUDescriptorSetId cgpu_create_descriptor_set_metal(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc);
CGPU_API void cgpu_update_descriptor_set_metal(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count);
CGPU_API void cgpu_free_descriptor_set_metal(CGPUDescriptorSetId set);
CGPU_API CGPUComputePipelineId cgpu_create_compute_pipeline_metal(CGPUDeviceId device, const CGPUComputePipelineDescriptor* desc);
CGPU_API void cgpu_free_compute_pipeline_metal(CGPUComputePipelineId pipeline);
CGPU_API CGPUQueryPoolId cgpu_create_query_pool_metal(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc);
CGPU_API void cgpu_free_query_pool_metal(CGPUQueryPoolId pool);

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

// Buffer APIs
CGPU_API CGPUBufferId cgpu_create_buffer_metal(CGPUDeviceId device, const CGPUBufferDescriptor* desc);
CGPU_API void cgpu_map_buffer_metal(CGPUBufferId buffer, const struct CGPUBufferRange* desc);
CGPU_API void cgpu_unmap_buffer_metal(CGPUBufferId buffer);
CGPU_API void cgpu_free_buffer_metal(CGPUBufferId buffer);

// CMDs
CGPU_API void cgpu_cmd_begin_metal(CGPUCommandBufferId cmd);
CGPU_API void cgpu_cmd_transfer_buffer_to_buffer_metal(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc);
CGPU_API void cgpu_cmd_resource_barrier_metal(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc);
CGPU_API void cgpu_cmd_begin_query_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
CGPU_API void cgpu_cmd_end_query_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
CGPU_API void cgpu_cmd_reset_query_pool_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, uint32_t start_query, uint32_t query_count);
CGPU_API void cgpu_cmd_resolve_query_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count);
CGPU_API void cgpu_cmd_end_metal(CGPUCommandBufferId cmd);

// Compute CMDs
CGPU_API CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_metal(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc);
CGPU_API void cgpu_compute_encoder_bind_descriptor_set_metal(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set);
CGPU_API void cgpu_compute_encoder_push_constants_metal(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
CGPU_API void cgpu_compute_encoder_bind_pipeline_metal(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline);
CGPU_API void cgpu_compute_encoder_dispatch_metal(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
CGPU_API void cgpu_cmd_end_compute_pass_metal(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder);

// RayTracing APIs
CGPU_API CGPUAccelerationStructureId cgpu_create_acceleration_structure_metal(CGPUDeviceId device, const struct CGPUAccelerationStructureDescriptor* desc);
CGPU_API void cgpu_free_acceleration_structure_metal(CGPUAccelerationStructureId as);
CGPU_API void cgpu_cmd_build_acceleration_structures_metal(CGPUCommandBufferId cmd, const struct CGPUAccelerationStructureBuildDescriptor* desc);

#ifdef __cplusplus
}
#endif