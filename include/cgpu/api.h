#pragma once
#include <stdint.h>
#include "cgpu_config.h"
#include "flags.h"

#define CGPU_ARRAY_LEN(array) ((sizeof(array) / sizeof(array[0])))
#define MAX_MRT_COUNT 8
#define MAX_VERTEX_ATTRIBS 15
#define MAX_VERTEX_BINDINGS 15
#define COLOR_MASK_RED 0x1
#define COLOR_MASK_GREEN 0x2
#define COLOR_MASK_BLUE 0x4
#define COLOR_MASK_ALPHA 0x8
#define COLOR_MASK_ALL COLOR_MASK_RED | COLOR_MASK_GREEN | COLOR_MASK_BLUE | COLOR_MASK_ALPHA
#define COLOR_MASK_NONE 0

#ifdef __cplusplus
extern "C" {
#endif

struct CGpuInstanceDescriptor;
struct CGpuAdapterDetail;
struct CGpuDeviceDescriptor;
struct CGpuCommandPoolDescriptor;
struct CGpuCommandBufferDescriptor;
struct CGpuShaderLibraryDescriptor;
struct CGpuPipelineShaderDescriptor;
struct CGpuResourceBarrierDescriptor;
struct CGpuComputePipelineDescriptor;
struct CGpuRenderPipelineDescriptor;
struct CGpuBufferDescriptor;
struct CGpuTextureDescriptor;
struct CGpuTextureViewDescriptor;
struct CGpuSamplerDescriptor;
struct CGpuSwapChainDescriptor;
struct CGpuAcquireNextDescriptor;
struct CGpuQueueSubmitDescriptor;
struct CGpuQueuePresentDescriptor;
struct CGpuBufferToBufferTransfer;
struct CGpuBufferToTextureTransfer;
struct CGpuTextureToTextureTransfer;
struct CGpuTextureToBufferTransfer;
struct CGpuRootSignatureDescriptor;
struct CGpuDescriptorSetDescriptor;
struct CGpuRenderPassDescriptor;
struct CGpuComputePassDescriptor;
struct CGpuDescriptorSet;
struct CGpuDescriptorData;
struct CGpuRenderPassEncoder;
struct CGpuComputePassEncoder;
struct CGpuGraphicsPipeline;
struct CGpuComputePipeline;
struct CGpuShaderReflection;
struct CGpuPipelineReflection;

typedef uint32_t CGpuQueueIndex;
#if defined(RUNTIME_PLATFORM_WA32)
typedef const host_ptr_t CGpuSurfaceId;
typedef const host_ptr_t CGpuInstanceId;
typedef const host_ptr_t CGpuAdapterId;
typedef const host_ptr_t CGpuDeviceId;
typedef const host_ptr_t CGpuQueueId;
typedef const host_ptr_t CGpuSemaphoreId;
typedef const host_ptr_t CGpuFenceId;
typedef const host_ptr_t CGpuCommandPoolId;
typedef const host_ptr_t CGpuCommandBufferId;
typedef const host_ptr_t CGpuSwapChainId;
typedef const host_ptr_t CGpuShaderLibraryId;
typedef const host_ptr_t CGpuRootSignatureId;
typedef const host_ptr_t CGpuDescriptorSetId;
typedef const host_ptr_t CGpuBufferId;
typedef const host_ptr_t CGpuTextureId;
typedef const host_ptr_t CGpuSamplerId;
typedef const host_ptr_t CGpuTextureViewId;
typedef const host_ptr_t CGpuRenderPassEncoderId;
typedef const host_ptr_t CGpuComputePassEncoderId;
typedef const host_ptr_t CGpuRenderPipelineId;
typedef const host_ptr_t CGpuComputePipelineId;
typedef const host_ptr_t CGpuShaderReflectionId;
typedef const host_ptr_t CGpuPipelineReflectionId;
#else
typedef const struct CGpuSurface_Dummy* CGpuSurfaceId;
typedef const struct CGpuInstance* CGpuInstanceId;
typedef const struct CGpuAdapter* CGpuAdapterId;
typedef const struct CGpuDevice* CGpuDeviceId;
typedef const struct CGpuQueue* CGpuQueueId;
typedef const struct CGpuSemaphore* CGpuSemaphoreId;
typedef const struct CGpuFence* CGpuFenceId;
typedef const struct CGpuCommandPool* CGpuCommandPoolId;
typedef const struct CGpuCommandBuffer* CGpuCommandBufferId;
typedef const struct CGpuSwapChain* CGpuSwapChainId;
typedef const struct CGpuShaderLibrary* CGpuShaderLibraryId;
typedef const struct CGpuRootSignature* CGpuRootSignatureId;
typedef const struct CGpuDescriptorSet* CGpuDescriptorSetId;
typedef const struct CGpuBuffer* CGpuBufferId;
typedef const struct CGpuTexture* CGpuTextureId;
typedef const struct CGpuSampler* CGpuSamplerId;
typedef const struct CGpuTextureView* CGpuTextureViewId;
typedef const struct CGpuRenderPassEncoder* CGpuRenderPassEncoderId;
typedef const struct CGpuComputePassEncoder* CGpuComputePassEncoderId;
typedef const struct CGpuRenderPipeline* CGpuRenderPipelineId;
typedef const struct CGpuComputePipeline* CGpuComputePipelineId;
typedef const struct CGpuShaderReflection* CGpuShaderReflectionId;
typedef const struct CGpuPipelineReflection* CGpuPipelineReflectionId;
#endif

typedef enum ECGpuBackend
{
    CGPU_BACKEND_VULKAN = 0,
    CGPU_BACKEND_D3D12 = 1,
    CGPU_BACKEND_XBOX_D3D12 = 2,
    CGPU_BACKEND_AGC = 3,
    CGPU_BACKEND_METAL = 4,
    CGPU_BACKEND_COUNT,
    CGPU_BACKEND_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuBackend;

static const char8_t* gCGpuBackendNames[CGPU_BACKEND_COUNT] = {
    "vulkan",
    "d3d12",
    "d3d12(xbox)",
    "agc",
    "metal"
};

typedef enum ECGpuQueueType
{
    QUEUE_TYPE_GRAPHICS = 0,
    QUEUE_TYPE_COMPUTE = 1,
    QUEUE_TYPE_TRANSFER = 2,
    QUEUE_TYPE_COUNT,
    QUEUE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuQueueType;

typedef struct CGpuFormatSupport {
    uint8_t shader_read : 1;
    uint8_t shader_write : 1;
    uint8_t render_target_write : 1;
} CGpuFormatSupport;

typedef struct CGpuInstanceFeatures {
    bool specialization_constant;

} CGpuInstanceFeatures;

typedef struct CGpuBufferRange {
    uint64_t offset;
    uint64_t size;
} CGpuBufferRange;

typedef struct CGpuConstantSpecialization {
    uint32_t constantID;
    union
    {
        uint32_t u;
        int32_t i;
        float f;
    };
} CGpuConstantSpecialization;

// Above APIs
RUNTIME_API ECGpuBackend cgpu_instance_get_backend(CGpuInstanceId instance);

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance(const struct CGpuInstanceDescriptor* desc);
typedef CGpuInstanceId (*CGPUProcCreateInstance)(const struct CGpuInstanceDescriptor* descriptor);
RUNTIME_API void cgpu_query_instance_features(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
typedef void (*CGPUProcQueryInstanceFeatures)(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance(CGpuInstanceId instance);
typedef void (*CGPUProcFreeInstance)(CGpuInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
typedef void (*CGPUProcEnumAdapters)(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);

RUNTIME_API const struct CGpuAdapterDetail* cgpu_query_adapter_detail(const CGpuAdapterId adapter);
typedef const struct CGpuAdapterDetail* (*CGPUProcQueryAdapterDetail)(const CGpuAdapterId adapter);
RUNTIME_API uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type);
typedef uint32_t (*CGPUProcQueryQueueCount)(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const struct CGpuDeviceDescriptor* desc);
typedef CGpuDeviceId (*CGPUProcCreateDevice)(CGpuAdapterId adapter, const struct CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device(CGpuDeviceId device);
typedef void (*CGPUProcFreeDevice)(CGpuDeviceId device);

// API Objects APIs
RUNTIME_API CGpuFenceId cgpu_create_fence(CGpuDeviceId device);
typedef CGpuFenceId (*CGPUProcCreateFence)(CGpuDeviceId device);
RUNTIME_API void cgpu_wait_fences(const CGpuFenceId* fences, uint32_t fence_count);
typedef void (*CGPUProcWaitFences)(const CGpuFenceId* fences, uint32_t fence_count);
RUNTIME_API ECGpuFenceStatus cgpu_query_fence_status(CGpuFenceId fence);
typedef ECGpuFenceStatus (*CGPUProcQueryFenceStatus)(CGpuFenceId fence);
RUNTIME_API void cgpu_free_fence(CGpuFenceId fence);
typedef void (*CGPUProcFreeFence)(CGpuFenceId fence);
RUNTIME_API CGpuSemaphoreId cgpu_create_semaphore(CGpuDeviceId device);
typedef CGpuSemaphoreId (*CGPUProcCreateSemaphore)(CGpuDeviceId device);
RUNTIME_API void cgpu_free_semaphore(CGpuSemaphoreId semaphore);
typedef void (*CGPUProcFreeSemaphore)(CGpuSemaphoreId semaphore);
RUNTIME_API CGpuRootSignatureId cgpu_create_root_signature(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc);
typedef CGpuRootSignatureId (*CGPUProcCreateRootSignature)(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature(CGpuRootSignatureId signature);
typedef void (*CGPUProcFreeRootSignature)(CGpuRootSignatureId signature);
RUNTIME_API CGpuDescriptorSetId cgpu_create_descriptor_set(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc);
typedef CGpuDescriptorSetId (*CGPUProcCreateDescriptorSet)(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc);
RUNTIME_API void cgpu_update_descriptor_set(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count);
typedef void (*CGPUProcUpdateDescriptorSet)(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count);
RUNTIME_API void cgpu_free_descriptor_set(CGpuDescriptorSetId set);
typedef void (*CGPUProcFreeDescriptorSet)(CGpuDescriptorSetId set);
RUNTIME_API CGpuComputePipelineId cgpu_create_compute_pipeline(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc);
typedef CGpuComputePipelineId (*CGPUProcCreateComputePipeline)(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc);
RUNTIME_API void cgpu_free_compute_pipeline(CGpuComputePipelineId pipeline);
typedef void (*CGPUProcFreeComputePipeline)(CGpuComputePipelineId pipeline);
RUNTIME_API CGpuRenderPipelineId cgpu_create_render_pipeline(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc);
typedef CGpuRenderPipelineId (*CGPUProcCreateRenderPipeline)(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc);
RUNTIME_API void cgpu_free_render_pipeline(CGpuRenderPipelineId pipeline);
typedef void (*CGPUProcFreeRenderPipeline)(CGpuRenderPipelineId pipeline);

// Queue APIs
// Warn: If you get a queue at an index with a specific type, you must hold the handle and reuses it.
RUNTIME_API CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
typedef CGpuQueueId (*CGPUProcGetQueue)(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc);
typedef void (*CGPUProcSubmitQueue)(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_queue_present(CGpuQueueId queue, const struct CGpuQueuePresentDescriptor* desc);
typedef void (*CGPUProcQueuePresent)(CGpuQueueId queue, const struct CGpuQueuePresentDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle(CGpuQueueId queue);
typedef void (*CGPUProcWaitQueueIdle)(CGpuQueueId queue);
RUNTIME_API void cgpu_free_queue(CGpuQueueId queue);
typedef void (*CGPUProcFreeQueue)(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool(CGpuQueueId queue, const struct CGpuCommandPoolDescriptor* desc);
typedef CGpuCommandPoolId (*CGPUProcCreateCommandPool)(CGpuQueueId queue, const struct CGpuCommandPoolDescriptor* desc);
RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc);
typedef CGpuCommandBufferId (*CGPUProcCreateCommandBuffer)(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_reset_command_pool(CGpuCommandPoolId pool);
typedef void (*CGPUProcResetCommandPool)(CGpuCommandPoolId pool);
RUNTIME_API void cgpu_free_command_buffer(CGpuCommandBufferId cmd);
typedef void (*CGPUProcFreeCommandBuffer)(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool(CGpuCommandPoolId pool);
typedef void (*CGPUProcFreeCommandPool)(CGpuCommandPoolId pool);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
typedef CGpuShaderLibraryId (*CGPUProcCreateShaderLibrary)(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library(CGpuShaderLibraryId library);
typedef void (*CGPUProcFreeShaderLibrary)(CGpuShaderLibraryId library);

// Buffer APIs
RUNTIME_API CGpuBufferId cgpu_create_buffer(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
typedef CGpuBufferId (*CGPUProcCreateBuffer)(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer(CGpuBufferId buffer, const struct CGpuBufferRange* range);
typedef void (*CGPUProcMapBuffer)(CGpuBufferId buffer, const struct CGpuBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer(CGpuBufferId buffer);
typedef void (*CGPUProcUnmapBuffer)(CGpuBufferId buffer);
RUNTIME_API void cgpu_free_buffer(CGpuBufferId buffer);
typedef void (*CGPUProcFreeBuffer)(CGpuBufferId buffer);

// Sampler APIs
RUNTIME_API CGpuSamplerId cgpu_create_sampler(CGpuDeviceId device, const struct CGpuSamplerDescriptor* desc);
typedef CGpuSamplerId (*CGPUProcCreateSampler)(CGpuDeviceId device, const struct CGpuSamplerDescriptor* desc);
RUNTIME_API void cgpu_free_sampler(CGpuSamplerId sampler);
typedef void (*CGPUProcFreeSampler)(CGpuSamplerId sampler);

// Texture/TextureView APIs
RUNTIME_API CGpuTextureId cgpu_create_texture(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc);
typedef CGpuTextureId (*CGPUProcCreateTexture)(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc);
RUNTIME_API void cgpu_free_texture(CGpuTextureId texture);
typedef void (*CGPUProcFreeTexture)(CGpuTextureId texture);
RUNTIME_API CGpuTextureViewId cgpu_create_texture_view(CGpuDeviceId device, const struct CGpuTextureViewDescriptor* desc);
typedef CGpuTextureViewId (*CGPUProcCreateTextureView)(CGpuDeviceId device, const struct CGpuTextureViewDescriptor* desc);
RUNTIME_API void cgpu_free_texture_view(CGpuTextureViewId render_target);
typedef void (*CGPUProcFreeTextureView)(CGpuTextureViewId render_target);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const struct CGpuSwapChainDescriptor* desc);
typedef CGpuSwapChainId (*CGPUProcCreateSwapChain)(CGpuDeviceId device, const struct CGpuSwapChainDescriptor* desc);
RUNTIME_API uint32_t cgpu_acquire_next_image(CGpuSwapChainId swapchain, const struct CGpuAcquireNextDescriptor* desc);
typedef uint32_t (*CGPUProcAcquireNext)(CGpuSwapChainId swapchain, const struct CGpuAcquireNextDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain(CGpuSwapChainId swapchain);
typedef void (*CGPUProcFreeSwapChain)(CGpuSwapChainId swapchain);

// CMDs
RUNTIME_API void cgpu_cmd_begin(CGpuCommandBufferId cmd);
typedef void (*CGPUProcCmdBegin)(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_buffer(CGpuCommandBufferId cmd, const struct CGpuBufferToBufferTransfer* desc);
typedef void (*CGPUProcCmdTransferBufferToBuffer)(CGpuCommandBufferId cmd, const struct CGpuBufferToBufferTransfer* desc);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_texture(CGpuCommandBufferId cmd, const struct CGpuBufferToTextureTransfer* desc);
typedef void (*CGPUProcCmdTransferBufferToTexture)(CGpuCommandBufferId cmd, const struct CGpuBufferToTextureTransfer* desc);
RUNTIME_API void cgpu_cmd_resource_barrier(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc);
typedef void (*CGPUProcCmdResourceBarrier)(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc);
RUNTIME_API void cgpu_cmd_end(CGpuCommandBufferId cmd);
typedef void (*CGPUProcCmdEnd)(CGpuCommandBufferId cmd);

// Compute Pass
RUNTIME_API CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc);
typedef CGpuComputePassEncoderId (*CGPUProcCmdBeginComputePass)(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc);
RUNTIME_API void cgpu_compute_encoder_bind_descriptor_set(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId set);
typedef void (*CGPUProcComputeEncoderBindDescriptorSet)(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId set);
RUNTIME_API void cgpu_compute_encoder_bind_pipeline(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline);
typedef void (*CGPUProcComputeEncoderBindPipeline)(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline);
RUNTIME_API void cgpu_compute_encoder_dispatch(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
typedef void (*CGPUProcComputeEncoderDispatch)(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
RUNTIME_API void cgpu_cmd_end_compute_pass(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder);
typedef void (*CGPUProcCmdEndComputePass)(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder);

// Render Pass
RUNTIME_API CGpuRenderPassEncoderId cgpu_cmd_begin_render_pass(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc);
typedef CGpuRenderPassEncoderId (*CGPUProcCmdBeginRenderPass)(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc);
RUNTIME_API void cgpu_render_encoder_bind_descriptor_set(CGpuRenderPassEncoderId encoder, CGpuDescriptorSetId set);
typedef void (*CGPUProcRenderEncoderBindDescriptorSet)(CGpuRenderPassEncoderId encoder, CGpuDescriptorSetId set);
RUNTIME_API void cgpu_render_encoder_set_viewport(CGpuRenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
typedef void (*CGPUProcRenderEncoderSetViewport)(CGpuRenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
RUNTIME_API void cgpu_render_encoder_set_scissor(CGpuRenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*CGPUProcRenderEncoderSetScissor)(CGpuRenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
RUNTIME_API void cgpu_render_encoder_bind_pipeline(CGpuRenderPassEncoderId encoder, CGpuRenderPipelineId pipeline);
typedef void (*CGPUProcRenderEncoderBindPipeline)(CGpuRenderPassEncoderId encoder, CGpuRenderPipelineId pipeline);
RUNTIME_API void cgpu_render_encoder_bind_vertex_buffers(CGpuRenderPassEncoderId encoder, uint32_t buffer_count,
    const CGpuBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
typedef void (*CGPUProcRendeEncoderBindVertexBuffers)(CGpuRenderPassEncoderId encoder, uint32_t buffer_count,
    const CGpuBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
RUNTIME_API void cgpu_render_encoder_bind_index_buffer(CGpuRenderPassEncoderId encoder, CGpuBufferId buffer,
    uint32_t index_stride, uint64_t offset);
typedef void (*CGPUProcRendeEncoderBindIndexBuffer)(CGpuRenderPassEncoderId encoder, CGpuBufferId buffer,
    uint32_t index_stride, uint64_t offset);
RUNTIME_API void cgpu_render_encoder_push_constants(CGpuRenderPassEncoderId encoder, CGpuRootSignatureId rs, const char8_t* name, const void* data);
typedef void (*CGPUProcRenderEncoderPushConstants)(CGpuRenderPassEncoderId encoder, CGpuRootSignatureId rs, const char8_t* name, const void* data);
RUNTIME_API void cgpu_render_encoder_draw(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
typedef void (*CGPUProcRenderEncoderDraw)(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_instanced(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
typedef void (*CGPUProcRenderEncoderDrawInstanced)(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
RUNTIME_API void cgpu_render_encoder_draw_indexed(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
typedef void (*CGPUProcRenderEncoderDrawIndexed)(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_indexed_instanced(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
typedef void (*CGPUProcRenderEncoderDrawIndexedInstanced)(CGpuRenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
RUNTIME_API void cgpu_cmd_end_render_pass(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder);
typedef void (*CGPUProcCmdEndRenderPass)(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder);

// Types
typedef struct CGpuProcTable {
    // Instance APIs
    const CGPUProcCreateInstance create_instance;
    const CGPUProcQueryInstanceFeatures query_instance_features;
    const CGPUProcFreeInstance free_instance;

    // Adapter APIs
    const CGPUProcEnumAdapters enum_adapters;
    const CGPUProcQueryAdapterDetail query_adapter_detail;
    const CGPUProcQueryQueueCount query_queue_count;

    // Device APIs
    const CGPUProcCreateDevice create_device;
    const CGPUProcFreeDevice free_device;

    // API Objects
    const CGPUProcCreateFence create_fence;
    const CGPUProcWaitFences wait_fences;
    const CGPUProcQueryFenceStatus query_fence_status;
    const CGPUProcFreeFence free_fence;
    const CGPUProcCreateSemaphore create_semaphore;
    const CGPUProcFreeSemaphore free_semaphore;
    const CGPUProcCreateRootSignature create_root_signature;
    const CGPUProcFreeRootSignature free_root_signature;
    const CGPUProcCreateDescriptorSet create_descriptor_set;
    const CGPUProcFreeDescriptorSet free_descriptor_set;
    const CGPUProcUpdateDescriptorSet update_descriptor_set;
    const CGPUProcCreateComputePipeline create_compute_pipeline;
    const CGPUProcFreeComputePipeline free_compute_pipeline;
    const CGPUProcCreateRenderPipeline create_render_pipeline;
    const CGPUProcFreeRenderPipeline free_render_pipeline;

    // Queue APIs
    const CGPUProcGetQueue get_queue;
    const CGPUProcSubmitQueue submit_queue;
    const CGPUProcWaitQueueIdle wait_queue_idle;
    const CGPUProcQueuePresent queue_present;
    const CGPUProcFreeQueue free_queue;

    // Command APIs
    const CGPUProcCreateCommandPool create_command_pool;
    const CGPUProcCreateCommandBuffer create_command_buffer;
    const CGPUProcResetCommandPool reset_command_pool;
    const CGPUProcFreeCommandBuffer free_command_buffer;
    const CGPUProcFreeCommandPool free_command_pool;

    // Shader APIs
    const CGPUProcCreateShaderLibrary create_shader_library;
    const CGPUProcFreeShaderLibrary free_shader_library;

    // Buffer APIs
    const CGPUProcCreateBuffer create_buffer;
    const CGPUProcMapBuffer map_buffer;
    const CGPUProcUnmapBuffer unmap_buffer;
    const CGPUProcFreeBuffer free_buffer;

    // Sampler APIs
    const CGPUProcCreateSampler create_sampler;
    const CGPUProcFreeSampler free_sampler;

    // Texture/TextureView APIs
    const CGPUProcCreateTexture create_texture;
    const CGPUProcFreeTexture free_texture;
    const CGPUProcCreateTextureView create_texture_view;
    const CGPUProcFreeTextureView free_texture_view;

    // Swapchain APIs
    const CGPUProcCreateSwapChain create_swapchain;
    const CGPUProcAcquireNext acquire_next_image;
    const CGPUProcFreeSwapChain free_swapchain;

    // CMDs
    const CGPUProcCmdBegin cmd_begin;
    const CGPUProcCmdTransferBufferToBuffer cmd_transfer_buffer_to_buffer;
    const CGPUProcCmdTransferBufferToTexture cmd_transfer_buffer_to_texture;
    const CGPUProcCmdResourceBarrier cmd_resource_barrier;
    const CGPUProcCmdEnd cmd_end;

    // Compute CMDs
    const CGPUProcCmdBeginComputePass cmd_begin_compute_pass;
    const CGPUProcComputeEncoderBindDescriptorSet compute_encoder_bind_descriptor_set;
    const CGPUProcComputeEncoderBindPipeline compute_encoder_bind_pipeline;
    const CGPUProcComputeEncoderDispatch compute_encoder_dispatch;
    const CGPUProcCmdEndComputePass cmd_end_compute_pass;

    // Render CMDs
    const CGPUProcCmdBeginRenderPass cmd_begin_render_pass;
    const CGPUProcRenderEncoderBindDescriptorSet render_encoder_bind_descriptor_set;
    const CGPUProcRenderEncoderBindPipeline render_encoder_bind_pipeline;
    const CGPUProcRendeEncoderBindVertexBuffers render_encoder_bind_vertex_buffers;
    const CGPUProcRendeEncoderBindIndexBuffer render_encoder_bind_index_buffer;
    const CGPUProcRenderEncoderPushConstants render_encoder_push_constants;
    const CGPUProcRenderEncoderSetViewport render_encoder_set_viewport;
    const CGPUProcRenderEncoderSetScissor render_encoder_set_scissor;
    const CGPUProcRenderEncoderDraw render_encoder_draw;
    const CGPUProcRenderEncoderDrawInstanced render_encoder_draw_instanced;
    const CGPUProcRenderEncoderDrawIndexed render_encoder_draw_indexed;
    const CGPUProcRenderEncoderDrawIndexedInstanced render_encoder_draw_indexed_instanced;
    const CGPUProcCmdEndRenderPass cmd_end_render_pass;
} CGpuProcTable;

// surfaces
RUNTIME_API void cgpu_free_surface(CGpuDeviceId device, CGpuSurfaceId surface);
typedef void (*CGPUSurfaceProc_Free)(CGpuDeviceId device, CGpuSurfaceId surface);

#if defined(_WIN32) || defined(_WIN64)
typedef struct HWND__* HWND;
RUNTIME_API CGpuSurfaceId cgpu_surface_from_hwnd(CGpuDeviceId device, HWND window);
typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromHWND)(CGpuDeviceId device, HWND window);
#endif
#ifdef __APPLE__
// RUNTIME_API CGpuSurfaceId cgpu_surface_from_ui_view(CGpuDeviceId device, UIView* window);
// typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromUIView)(CGpuDeviceId device, UIView* window);
typedef struct CGpuNSView CGpuNSView;
RUNTIME_API CGpuSurfaceId cgpu_surface_from_ns_view(CGpuDeviceId device, CGpuNSView* window);
typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromNSView)(CGpuDeviceId device, CGpuNSView* window);
#endif
typedef struct CGpuSurfacesProcTable {
#if defined(_WIN32) || defined(_WIN64)
    const CGPUSurfaceProc_CreateFromHWND from_hwnd;
#endif
#ifdef __APPLE__
    // const CGPUSurfaceProc_CreateFromUIView from_ui_view;
    const CGPUSurfaceProc_CreateFromNSView from_ns_view;
#endif
    const CGPUSurfaceProc_Free free_surface;
} CGpuSurfacesProcTable;

typedef struct GPUVendorPreset {
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t driver_version;
    char gpu_name[MAX_GPU_VENDOR_STRING_LENGTH]; // If GPU Name is missing then value will be empty string
} GPUVendorPreset;

typedef struct CGpuAdapterDetail {
    uint32_t uniform_buffer_alignment;
    uint32_t upload_buffer_texture_alignment;
    uint32_t upload_buffer_texture_row_alignment;
    uint32_t max_vertex_input_bindings;
    uint32_t wave_lane_count;
    bool multidraw_indirect : 1;
    bool support_geom_shader : 1;
    bool support_tessellation : 1;
    bool is_uma : 1;
    bool is_virtual : 1;
    bool is_cpu : 1;
    CGpuFormatSupport format_supports[PF_COUNT];
    GPUVendorPreset vendor_preset;
} CGpuAdapterDetail;

// Objects (Heap Safety)
typedef struct CGpuInstance {
    const CGpuProcTable* proc_table;
    const CGpuSurfacesProcTable* surfaces_table;
    // Some Cached Data
    struct CGpuRuntimeTable* runtime_table;
    ECGpuBackend backend;
    ECGpuNvAPI_Status nvapi_status;
    ECGpuAGSReturnCode ags_status;
    bool enable_set_name;
} CGpuInstance;

typedef struct CGpuAdapter {
    CGpuInstanceId instance;
    const CGpuProcTable* proc_table_cache;
} CGpuAdapter;

typedef struct CGpuDevice {
    const CGpuAdapterId adapter;
    const CGpuProcTable* proc_table_cache;
#ifdef __cplusplus
    CGpuDevice()
        : adapter(CGPU_NULLPTR)
    {
    }
#endif
} CGpuDevice;

typedef struct CGpuQueue {
    CGpuDeviceId device;
    ECGpuQueueType type;
    CGpuQueueIndex index;
} CGpuQueue;

typedef struct CGpuFence {
    CGpuDeviceId device;
} CGpuFence; // Empty struct so we dont need to def it

typedef struct CGpuSemaphore {
    CGpuDeviceId device;
} CGpuSemaphore; // Empty struct so we dont need to def it

typedef struct CGpuCommandPool {
    CGpuQueueId queue;
} CGpuCommandPool;

typedef struct CGpuCommandBuffer {
    CGpuDeviceId device;
    CGpuCommandPoolId pool;
    ECGpuPipelineType current_dispatch;
} CGpuCommandBuffer;

// Notice that we must keep this header same with CGpuCommandBuffer
// because Vulkan & D3D12 Backend simply use command buffer handle as encoder handle
typedef struct CGpuComputePassEncoder {
    CGpuDeviceId device;
} CGpuComputePassEncoder;

typedef struct CGpuRenderPassEncoder {
    CGpuDeviceId device;
} CGpuRenderPassEncoder;

// Shaders
typedef struct CGpuShaderResource {
    const char8_t* name;
#if SIZE_MAX == UINT64_MAX
    uint64_t name_hash;
#elif SIZE_MAX == UINT32_MAX
    uint32_t name_hash;
    uint32_t pad;
#else
    #error "unsupported hash size!"
#endif
    ECGpuResourceType type;
    ECGpuTextureDimension dim;
    uint32_t set;
    uint32_t binding;
    uint32_t size;
    uint32_t offset;
    CGpuShaderStages stages;
} CGpuShaderResource;

typedef struct CGpuVertexInput {
    const char8_t* name;
    ECGpuFormat format;
} CGpuVertexInput;

typedef struct CGpuShaderReflection {
    const char8_t* entry_name;
    ECGpuShaderStage stage;
    CGpuVertexInput* vertex_inputs;
    CGpuShaderResource* shader_resources;
    uint32_t vertex_inputs_count;
    uint32_t shader_resources_count;
    uint32_t thread_group_sizes[3];
} CGpuShaderReflection;

typedef struct CGpuShaderLibrary {
    CGpuDeviceId device;
    char8_t* name;
    CGpuShaderReflection* entry_reflections;
    uint32_t entrys_count;
} CGpuShaderLibrary;

typedef struct CGpuPipelineReflection {
    CGpuShaderReflection* stages[SHADER_STAGE_COUNT];
    // descriptor sets / root tables
    CGpuShaderResource* shader_resources;
    uint32_t shader_resources_count;
} CGpuPipelineReflection;

typedef struct CGpuDescriptorData {
    // Update Via Shader Reflection.
    const char8_t* name;
    // Update Via Binding Slot.
    uint32_t binding;
    ECGpuResourceType binding_type;
    union
    {
        struct
        {
            /// Offset to bind the buffer descriptor
            const uint64_t* offsets;
            const uint64_t* sizes;
        } buffers_params;
        // Descriptor set buffer extraction options
        struct
        {
            struct CGpuPipelineShaderDescriptor* shader;
            uint32_t buffer_index;
            ECGpuShaderStage shader_stage;
        } extraction_params;
        struct
        {
            uint32_t uav_mip_slice;
            bool blend_mip_chain;
        } uav_params;
        bool enable_stencil_resource;
    };
    union
    {
        /// Array of texture descriptors (srv and uav textures)
        CGpuTextureViewId* textures;
        /// Array of sampler descriptors
        CGpuSamplerId* samplers;
        /// Array of buffer descriptors (srv, uav and cbv buffers)
        CGpuBufferId* buffers;
        /// Array of pipeline descriptors
        CGpuRenderPipelineId* render_pipelines;
        /// Array of pipeline descriptors
        CGpuComputePipelineId* compute_pipelines;
        /// DescriptorSet buffer extraction
        CGpuDescriptorSetId* descriptor_sets;
        /// Custom binding (raytracing acceleration structure ...)
        // CGpuAccelerationStructureId* acceleration_structures;
    };
    uint32_t count;
} CGpuDescriptorData;

typedef struct CGpuBuffer {
    CGpuDeviceId device;
    /**
     * CPU address of the mapped buffer.
     * Applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
     */
    void* cpu_mapped_address;
    uint64_t size : 32;
    uint64_t descriptors : 20;
    uint64_t memory_usage : 3;
} CGpuBuffer;

typedef union CGpuClearValue
{
    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
    struct
    {
        float depth;
        uint32_t stencil;
    };
} CGpuClearValue;

static const CGpuClearValue fastclear_0000 = {
    { 0.f, 0.f, 0.f, 0.f }
};
static const CGpuClearValue fastclear_0001 = {
    { 0.f, 0.f, 0.f, 1.f }
};
static const CGpuClearValue fastclear_1110 = {
    { 1.f, 1.f, 1.f, 1.f }
};
static const CGpuClearValue fastclear_1111 = {
    { 1.f, 1.f, 1.f, 1.f }
};

typedef struct CGpuSwapChain {
    CGpuDeviceId device;
    const CGpuTextureId* back_buffers;
    uint32_t buffer_count;
} CGpuSwapChain;

// Descriptors (on Stack)
#pragma region DESCRIPTORS

#define CGPU_CHAINED_DESCRIPTOR_HEADER ECGpuBackend backend;

typedef struct CGpuChainedDescriptor {
    CGPU_CHAINED_DESCRIPTOR_HEADER
} CGpuChainedDescriptor;

// Device & Pipeline
typedef struct CGpuInstanceDescriptor {
    const CGpuChainedDescriptor* chained;
    ECGpuBackend backend;
    bool enable_debug_layer;
    bool enable_gpu_based_validation;
    bool enable_set_name;
} CGpuInstanceDescriptor;

typedef struct CGpuQueueGroupDescriptor {
    ECGpuQueueType queueType;
    uint32_t queueCount;
} CGpuQueueGroupDescriptor;

typedef struct CGpuQueueSubmitDescriptor {
    CGpuCommandBufferId* cmds;
    CGpuFenceId signal_fence;
    CGpuSemaphoreId* wait_semaphores;
    CGpuSemaphoreId* signal_semaphores;
    uint32_t cmds_count;
    uint32_t wait_semaphore_count;
    uint32_t signal_semaphore_count;
} CGpuQueueSubmitDescriptor;

typedef struct CGpuQueuePresentDescriptor {
    CGpuSwapChainId swapchain;
    const CGpuSemaphoreId* wait_semaphores;
    uint32_t wait_semaphore_count;
    uint8_t index;
} CGpuQueuePresentDescriptor;

typedef struct CGpuAcquireNextDescriptor {
    CGpuSemaphoreId signal_semaphore;
    CGpuFenceId fence;
} CGpuAcquireNextDescriptor;

typedef struct CGpuBufferToBufferTransfer {
    CGpuBufferId dst;
    uint64_t dst_offset;
    CGpuBufferId src;
    uint64_t src_offset;
    uint64_t size;
} CGpuBufferToBufferTransfer;

typedef struct CGpuBufferToTextureTransfer {
    CGpuTextureId dst;
    uint32_t dst_mip_level;
    uint32_t elems_per_row;
    uint32_t rows_per_image;
    uint32_t base_array_layer;
    uint32_t layer_count;
    CGpuBufferId src;
    uint64_t src_offset;
} CGpuBufferToTextureTransfer;

typedef struct CGpuBufferBarrier {
    CGpuBufferId buffer;
    ECGpuResourceState src_state;
    ECGpuResourceState dst_state;
    uint8_t queue_acquire : 1;
    uint8_t queue_release : 1;
    ECGpuQueueType queue_type : 5;
    struct {
        uint8_t begin_ony : 1;
        uint8_t end_only : 1;
    } d3d12;
} CGpuBufferBarrier;

typedef struct CGpuTextureBarrier {
    CGpuTextureId texture;
    ECGpuResourceState src_state;
    ECGpuResourceState dst_state;
    uint8_t queue_acquire : 1;
    uint8_t queue_release : 1;
    ECGpuQueueType queue_type : 5;
    /// Specifiy whether following barrier targets particular subresource
    uint8_t subresource_barrier : 1;
    /// Following values are ignored if subresource_barrier is false
    uint8_t mip_level : 7;
    uint16_t array_layer;
    struct {
        uint8_t begin_ony : 1;
        uint8_t end_only : 1;
    } d3d12;
} CGpuTextureBarrier;

typedef struct CGpuResourceBarrierDescriptor {
    const CGpuBufferBarrier* buffer_barriers;
    uint32_t buffer_barriers_count;
    const CGpuTextureBarrier* texture_barriers;
    uint32_t texture_barriers_count;
} CGpuResourceBarrierDescriptor;

typedef struct CGpuDeviceDescriptor {
    bool disable_pipeline_cache;
    CGpuQueueGroupDescriptor* queueGroups;
    uint32_t queueGroupCount;
} CGpuDeviceDescriptor;

typedef struct CGpuCommandPoolDescriptor {
    uint32_t ___nothing_and_useless__;
} CGpuCommandPoolDescriptor;

typedef struct CGpuCommandBufferDescriptor {
#if defined(PROSPERO) || defined(ORBIS)
    uint32_t max_size; // AGC CommandBuffer Size
#endif
    bool is_secondary : 1;
} CGpuCommandBufferDescriptor;

typedef struct CGpuPipelineShaderDescriptor {
    CGpuShaderLibraryId library;
    const char8_t* entry;
    ECGpuShaderStage stage;
    // ++ constant_specialization
    const CGpuConstantSpecialization* constants;
    uint32_t num_constants;
    // -- constant_specialization
} CGpuPipelineShaderDescriptor;

typedef struct CGpuSwapChainDescriptor {
    /// Present Queues
    CGpuQueueId* presentQueues;
    /// Present Queues Count
    uint32_t presentQueuesCount;
    /// Surface to Create SwapChain on
    CGpuSurfaceId surface;
    /// Number of backbuffers in this swapchain
    uint32_t imageCount;
    /// Width of the swapchain
    uint32_t width;
    /// Height of the swapchain
    uint32_t height;
    /// Set whether swap chain will be presented using vsync
    bool enableVsync;
    /// We can toggle to using FLIP model if app desires
    bool useFlipSwapEffect;
    /// Clear Value.
    float clearValue[4];
    /// format
    ECGpuFormat format;
} CGpuSwapChainDescriptor;

typedef struct CGpuComputePassDescriptor {
    const char8_t* name;
} CGpuComputePassDescriptor;

typedef struct CGpuColorAttachment {
    CGpuTextureViewId view;
    CGpuTextureViewId resolve_view;
    ECGpuLoadAction load_action;
    ECGpuStoreAction store_action;
    CGpuClearValue clear_color;
} CGpuColorAttachment;

typedef struct CGpuDepthStencilAttachment {
    CGpuTextureViewId view;
    ECGpuLoadAction depth_load_action;
    ECGpuStoreAction depth_store_action;
    float clear_depth;
    uint8_t write_depth;
    ECGpuLoadAction stencil_load_action;
    ECGpuStoreAction stencil_store_action;
    uint32_t clear_stencil;
    uint8_t write_stencil;
} CGpuDepthStencilAttachment;

typedef struct CGpuRenderPassDescriptor {
    const char8_t* name;
    // TODO: remove this
    ECGpuSampleCount sample_count;
    const CGpuColorAttachment* color_attachments;
    const CGpuDepthStencilAttachment* depth_stencil;
    uint32_t render_target_count;
} CGpuRenderPassDescriptor;

typedef struct CGpuRootSignatureDescriptor {
    struct CGpuPipelineShaderDescriptor* shaders;
    uint32_t shader_count;
    const CGpuSamplerId* static_samplers;
    const char8_t* const* static_sampler_names;
    uint32_t static_sampler_count;
    const char8_t* const* root_constant_names;
    uint32_t root_constant_count;
} CGpuRootSignatureDescriptor;

typedef struct CGpuDescriptorSetDescriptor {
    CGpuRootSignatureId root_signature;
    uint32_t set_index;
} CGpuDescriptorSetDescriptor;

typedef struct CGpuComputePipelineDescriptor {
    CGpuRootSignatureId root_signature;
    CGpuPipelineShaderDescriptor* compute_shader;
} CGpuComputePipelineDescriptor;

typedef struct CGpuBlendStateDescriptor {
    /// Source blend factor per render target.
    ECGpuBlendConstant src_factors[MAX_MRT_COUNT];
    /// Destination blend factor per render target.
    ECGpuBlendConstant dst_factors[MAX_MRT_COUNT];
    /// Source alpha blend factor per render target.
    ECGpuBlendConstant src_alpha_factors[MAX_MRT_COUNT];
    /// Destination alpha blend factor per render target.
    ECGpuBlendConstant dst_alpha_factors[MAX_MRT_COUNT];
    /// Blend mode per render target.
    ECGpuBlendMode blend_modes[MAX_MRT_COUNT];
    /// Alpha blend mode per render target.
    ECGpuBlendMode blend_alpha_modes[MAX_MRT_COUNT];
    /// Write mask per render target.
    int32_t masks[MAX_MRT_COUNT];
    /// Set whether alpha to coverage should be enabled.
    bool alpha_to_coverage;
    /// Set whether each render target has an unique blend function. When false the blend function in slot 0 will be used for all render targets.
    bool independent_blend;
} CGpuBlendStateDescriptor;

typedef struct CGpuDepthStateDesc {
    bool depth_test;
    bool depth_write;
    ECGpuCompareMode depth_func;
    bool stencil_test;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    ECGpuCompareMode stencil_front_func;
    ECGpuStencilOp stencil_front_fail;
    ECGpuStencilOp depth_front_fail;
    ECGpuStencilOp stencil_front_pass;
    ECGpuCompareMode stencil_back_func;
    ECGpuStencilOp stencil_back_fail;
    ECGpuStencilOp depth_back_fail;
    ECGpuStencilOp stencil_back_pass;
} CGpuDepthStateDescriptor;

typedef struct CGpuRasterizerStateDescriptor {
    ECGpuCullMode cull_mode;
    int32_t depth_bias;
    float slope_scaled_depth_bias;
    ECGpuFillMode fill_mode;
    ECGpuFrontFace front_face;
    bool enable_multi_sample;
    bool enable_scissor;
    bool enable_depth_clamp;
} CGpuRasterizerStateDescriptor;

typedef struct CGpuVertexAttribute {
    // TODO: handle this in a better way
    char8_t semantic_name[64];
    ECGpuFormat format;
    uint32_t binding;
    uint32_t location;
    uint32_t offset;
    ECGpuVertexInputRate rate;
} CGpuVertexAttribute;

typedef struct CGpuVertexLayout {
    uint32_t attribute_count;
    CGpuVertexAttribute attributes[MAX_VERTEX_ATTRIBS];
} CGpuVertexLayout;

typedef struct CGpuRenderPipelineDescriptor {
    CGpuRootSignatureId root_signature;
    const CGpuPipelineShaderDescriptor* vertex_shader;
    const CGpuPipelineShaderDescriptor* tesc_shader;
    const CGpuPipelineShaderDescriptor* tese_shader;
    const CGpuPipelineShaderDescriptor* geom_shader;
    const CGpuPipelineShaderDescriptor* fragment_shader;
    const CGpuVertexLayout* vertex_layout;
    const CGpuBlendStateDescriptor* blend_state;
    const CGpuDepthStateDescriptor* depth_state;
    const CGpuRasterizerStateDescriptor* rasterizer_state;
    const ECGpuFormat* color_formats;
    uint32_t render_target_count;
    ECGpuSampleCount sample_count;
    uint32_t sample_quality;
    ECGpuSlotMask color_resolve_disable_mask;
    ECGpuFormat depth_stencil_format;
    ECGpuPrimitiveTopology prim_topology;
    bool enable_indirect_command;
} CGpuRenderPipelineDescriptor;

typedef struct CGpuParameterTable {
    // This should be stored here because shader could be destoryed after RS creation
    CGpuShaderResource* resources;
    uint32_t resources_count;
    uint32_t set_index;
} CGpuParameterTable;

typedef struct CGpuRootSignature {
    CGpuDeviceId device;
    CGpuParameterTable* tables;
    uint32_t table_count;
    CGpuShaderResource* push_constants;
    uint32_t push_constant_count;
    ECGpuPipelineType pipeline_type;
} CGpuRootSignature;

typedef struct CGpuDescriptorSet {
    CGpuRootSignatureId root_signature;
    uint32_t index;
} CGpuDescriptorSet;

typedef struct CGpuComputePipeline {
    CGpuDeviceId device;
    CGpuRootSignatureId root_signature;
} CGpuComputePipeline;

typedef struct CGpuRenderPipeline {
    CGpuDeviceId device;
    CGpuRootSignatureId root_signature;
} CGpuRenderPipeline;

// Resources
typedef struct CGpuShaderLibraryDescriptor {
    const char8_t* name;
    uint32_t name_size;
    const uint32_t* code;
    uint32_t code_size;
    ECGpuShaderStage stage;
    ECGpuTextureDimension dimension;
} CGpuShaderLibraryDescriptor;

typedef struct CGpuBufferDescriptor {
    /// Size of the buffer (in bytes)
    uint64_t size;
    /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    struct Buffer* count_buffer;
    /// Debug name used in gpu profile
    const char8_t* name;
    /// Flags specifying the suitable usage of this buffer (Uniform buffer, Vertex Buffer, Index Buffer,...)
    CGpuResourceTypes descriptors;
    /// Decides which memory heap buffer will use (default, upload, readback)
    ECGpuMemoryUsage memory_usage;
    /// Image format
    ECGpuFormat format;
    /// Creation flags
    CGpuBufferCreationFlags flags;
    /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t first_element;
    /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t elemet_count;
    /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t element_stride;
    /// What state will the texture get created in
    ECGpuResourceState start_state;
} CGpuBufferDescriptor;

typedef struct CGpuTextureDescriptor {
    /// Debug name used in gpu profile
    const char8_t* name;
    /// Imported native image handle
    const void* native_handle;
    /// Texture creation flags (decides memory allocation strategy, sharing access,...)
    CGpuTextureCreationFlags flags;
    /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
    CGpuClearValue clear_value;
    /// Width
    uint32_t width;
    /// Height
    uint32_t height;
    /// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
    uint32_t depth;
    /// Texture array size (Should be 1 if texture is not a texture array or cubemap)
    uint32_t array_size;
    ///  image format
    ECGpuFormat format;
    /// Number of mip levels
    uint32_t mip_levels;
    /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support SAMPLE_COUNT_1)
    ECGpuSampleCount sample_count;
    /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
    uint32_t sample_quality;
    /// Owner queue of the resource at creation
    CGpuQueueId owner_queue;
    /// What state will the texture get created in
    ECGpuResourceState start_state;
    /// Descriptor creation
    CGpuResourceTypes descriptors;
} CGpuTextureDescriptor;

typedef struct CGpuTextureViewDescriptor {
    /// Debug name used in gpu profile
    const char8_t* name;
    CGpuTextureId texture;
    ECGpuFormat format;
    CGpuTexutreViewUsages usages : 8;
    CGpuTextureViewAspects aspects : 8;
    ECGpuTextureDimension dims : 8;
    uint32_t base_array_layer : 8;
    uint32_t array_layer_count : 8;
    uint32_t base_mip_level : 8;
    uint32_t mip_level_count : 8;
} CGpuTextureViewDescriptor;

typedef struct CGpuTexture {
    CGpuDeviceId device;
    ECGpuSampleCount sample_count;
    /// Current state of the buffer
    uint32_t width : 16;
    uint32_t height : 16;
    uint32_t depth : 16;
    uint32_t mip_levels : 5;
    uint32_t array_size_minus_one : 11;
    uint32_t format : 8;
    /// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the pVkImageView
    uint32_t aspect_mask : 4;
    uint32_t node_index : 4;
    uint32_t is_cube : 1;
    /// This value will be false if the underlying resource is not owned by the texture (swapchain textures,...)
    uint32_t owns_image : 1;
} CGpuTexture;

typedef struct CGpuTextureView {
    CGpuDeviceId device;
    CGpuTextureViewDescriptor info;
} CGpuTextureView;

typedef struct CGpuSamplerDescriptor {
    ECGpuFilterType min_filter;
    ECGpuFilterType mag_filter;
    ECGpuMipMapMode mipmap_mode;
    ECGpuAddressMode address_u;
    ECGpuAddressMode address_v;
    ECGpuAddressMode address_w;
    float mip_lod_bias;
    float max_anisotropy;
    ECGpuCompareMode compare_func;
} CGpuSamplerDescriptor;

typedef struct CGpuSampler {
    CGpuDeviceId device;
} CGpuSampler;

#pragma endregion DESCRIPTORS

#define SINGLE_GPU_NODE_COUNT 1
#define SINGLE_GPU_NODE_MASK 1
#define SINGLE_GPU_NODE_INDEX 0

#ifdef __cplusplus
} // end extern "C"
#endif
