#pragma once
#include <stdint.h>
#include "platform/configure.h"
#include "cgpu_config.h"
#include "flags.h"
#define CGPU_ARRAY_LEN(array) ((sizeof(array) / sizeof(array[0])))
#define MAX_MRT_COUNT 8
#define MAX_VERTEX_ATTRIBS 15

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
struct CGpuSwapChainDescriptor;
struct CGpuAcquireNextDescriptor;
struct CGpuQueueSubmitDescriptor;
struct CGpuQueuePresentDescriptor;
struct CGpuBufferUpdateDescriptor;
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
typedef const struct CGpuSurface_Dummy* CGpuSurfaceId;
typedef const int CGpuVersion;
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
typedef const struct CGpuTextureView* CGpuTextureViewId;
typedef const struct CGpuRenderPassEncoder* CGpuRenderPassEncoderId;
typedef const struct CGpuComputePassEncoder* CGpuComputePassEncoderId;
typedef const struct CGpuRenderPipeline* CGpuRenderPipelineId;
typedef const struct CGpuComputePipeline* CGpuComputePipelineId;
typedef const struct CGpuShaderReflection* CGpuShaderReflectionId;
typedef const struct CGpuPipelineReflection* CGpuPipelineReflectionId;

typedef enum ECGpuBackend
{
    ECGpuBackend_VULKAN = 0,
    ECGpuBackend_D3D12 = 1,
    ECGpuBackend_XBOX_D3D12 = 2,
    ECGpuBackend_AGC = 3,
    ECGpuBackend_METAL = 4,
    ECGpuBackend_COUNT
} ECGpuBackend;

static const char8_t* gCGpuBackendNames[ECGpuBackend_COUNT] = {
    "vulkan",
    "d3d12",
    "d3d12(xbox)",
    "agc",
    "metal"
};

typedef enum ECGpuQueueType
{
    ECGpuQueueType_Graphics = 0,
    ECGpuQueueType_Compute = 1,
    ECGpuQueueType_Transfer = 2,
    ECGpuQueueType_Count
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
RUNTIME_API void cgpu_free_fence(CGpuFenceId fence);
typedef void (*CGPUProcFreeFence)(CGpuFenceId fence);
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
RUNTIME_API void cgpu_cmd_update_buffer(CGpuCommandBufferId cmd, const struct CGpuBufferUpdateDescriptor* desc);
typedef void (*CGPUProcCmdUpdateBuffer)(CGpuCommandBufferId cmd, const struct CGpuBufferUpdateDescriptor* desc);
RUNTIME_API void cgpu_cmd_resource_barrier(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc);
typedef void (*CGPUProcCmdResourceBarrier)(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc);
RUNTIME_API void cgpu_cmd_set_viewport(CGpuCommandBufferId cmd, float x, float y, float width, float height, float min_depth, float max_depth);
typedef void (*CGPUProcCmdSetViewport)(CGpuCommandBufferId cmd, float x, float y, float width, float height, float min_depth, float max_depth);
RUNTIME_API void cgpu_cmd_set_scissor(CGpuCommandBufferId cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*CGPUProcCmdSetScissor)(CGpuCommandBufferId cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
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
    const CGPUProcFreeFence free_fence;
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
    const CGPUProcCmdUpdateBuffer cmd_update_buffer;
    const CGPUProcCmdResourceBarrier cmd_resource_barrier;
    const CGPUProcCmdSetViewport cmd_set_viewport;
    const CGPUProcCmdSetScissor cmd_set_scissor;
    const CGPUProcCmdEnd cmd_end;

    // Compute CMDs
    const CGPUProcCmdBeginComputePass cmd_begin_compute_pass;
    const CGPUProcComputeEncoderBindDescriptorSet compute_encoder_bind_descriptor_set;
    const CGPUProcComputeEncoderBindPipeline compute_encoder_bind_pipeline;
    const CGPUProcComputeEncoderDispatch compute_encoder_dispatch;
    const CGPUProcCmdEndComputePass cmd_end_compute_pass;

    // Render CMDs
    const CGPUProcCmdBeginRenderPass cmd_begin_render_pass;
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
    CGpuFormatSupport format_supports[PF_Count];
    GPUVendorPreset vendor_preset;
} CGpuAdapterDetail;

// Objects (Heap Safety)
typedef struct CGpuInstance {
    const CGpuProcTable* proc_table;
    const CGpuSurfacesProcTable* surfaces_table;
    // Some Cached Data
    struct CGpuRuntimeTable* runtime_table;
    ECGpuNvAPI_Status nvapi_status;
    ECGpuAGSReturnCode ags_status;
    bool enable_set_name;
} CGpuInstance;

typedef struct CGpuAdapter {
    const CGpuInstanceId instance;
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

// Shaders
typedef struct CGpuShaderResource {
    const char8_t* name;
    size_t name_hash;
    ECGpuResourceType type;
    ECGpuTextureDimension dim;
    uint32_t set;
    uint32_t binding;
    uint32_t size;
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
    CGpuShaderReflection* stages[SS_COUNT];
    // descriptor sets / root tables
    CGpuShaderResource* shader_resources;
    uint32_t shader_resources_count;
} CGpuPipelineReflection;

typedef struct CGpuDescriptorData {
    // Update Via Shader Reflection.
    const char8_t* name;
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
        // CGpuTextureId* textures;
        /// Array of sampler descriptors
        // CGpuSamplerId* samplers;
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

typedef struct CGpuTexture {
    CGpuDeviceId device;
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
    uint32_t uav : 1;
    /// This value will be false if the underlying resource is not owned by the texture (swapchain textures,...)
    uint32_t owns_image : 1;
} CGpuTexture;

typedef struct CGpuTextureView {
    CGpuDeviceId device;
    CGpuTextureId texture;
    CGpuClearValue clear_value;
    uint32_t array_size : 16;
    uint32_t depth : 16;
    uint32_t width : 16;
    uint32_t height : 16;
    uint32_t descriptors : 20;
    uint32_t mip_levels : 10;
    uint32_t sample_quality : 5;
    ECGpuFormat format;
    ECGpuSampleCount sample_count;
} CGpuTextureView;

typedef struct CGpuSwapChain {
    CGpuDeviceId device;
    const CGpuTextureId* views;
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
    uint32_t wait_semaphores_count;
    uint32_t signal_semaphores_count;
} CGpuQueueSubmitDescriptor;

typedef struct CGpuQueuePresentDescriptor {
    CGpuSwapChainId swapchain;
    CGpuSemaphoreId* wait_semaphores;
    uint32_t wait_semaphore_count;
    uint8_t index;
} CGpuQueuePresentDescriptor;

typedef struct CGpuAcquireNextDescriptor {
    CGpuSemaphoreId signal_semaphore;
    CGpuFenceId fence;
} CGpuAcquireNextDescriptor;

typedef struct CGpuBufferUpdateDescriptor {
    CGpuBufferId dst;
    uint64_t dst_offset;
    CGpuBufferId src;
    uint64_t src_offset;
    uint64_t size;
} CGpuBufferUpdateDescriptor;

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
    size_t num_constants;
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
    ECGpuLoadAction load_action;
    ECGpuStoreAction store_action;
    CGpuClearValue clear_color;
} CGpuColorAttachment;

typedef struct CGpuDepthStencilAttachment {
    CGpuTextureViewId view;
    ECGpuLoadAction depth_load_action;
    ECGpuStoreAction depth_store_action;
    float clear_depth;
    bool write_depth;
    ECGpuLoadAction stencil_load_action;
    ECGpuStoreAction stencil_store_action;
    uint32_t clear_stencil;
    bool write_stencil;
} CGpuDepthStencilAttachment;

typedef struct CGpuRenderPassDescriptor {
    const char8_t* name;
    ECGpuSampleCount sample_count;
    const CGpuColorAttachment* color_attachments;
    const CGpuDepthStencilAttachment* depth_stencil;
    uint32_t render_target_count;
} CGpuRenderPassDescriptor;

typedef struct CGpuRootSignatureDescriptor {
    struct CGpuPipelineShaderDescriptor* shaders;
    uint32_t shaders_count;
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
    char8_t semantic_name[64];
    ECGpuFormat format;
    uint32_t binding;
    uint32_t location;
    uint32_t offset;
    ECGpuVertexAttribRate rate;
} CGpuVertexAttribute;

typedef struct CGpuVertexLayout {
    uint32_t mAttribCount;
    CGpuVertexAttribute mAttribs[MAX_VERTEX_ATTRIBS];
} CGpuVertexLayout;

typedef struct CGpuRenderPipelineDescriptor {
    CGpuRootSignatureId root_signature;
    CGpuPipelineShaderDescriptor* vertex_shader;
    CGpuPipelineShaderDescriptor* fragment_shader;
    CGpuVertexLayout* vertex_layout;
    CGpuBlendStateDescriptor* blend_state;
    CGpuDepthStateDescriptor* depth_state;
    CGpuRasterizerStateDescriptor* rasterizer_state;
    ECGpuFormat* color_formats;
    uint32_t render_target_count;
    ECGpuSampleCount sample_count;
    uint32_t sample_quality;
    ECGpuFormat depth_stencil_format;
    ECGpuPrimitiveTopology prim_topology;
    bool enable_indirect_command;
} CGpuRenderPipelineDescriptor;

typedef struct CGpuParameterTable {
    // This should be stored here because shader could be destoryed after RS creation
    CGpuShaderResource* resources;
    uint32_t resources_count;
} CGpuParameterTable;

typedef struct CGpuRootSignature {
    CGpuDeviceId device;
    CGpuParameterTable* tables;
    uint32_t table_count;
} CGpuRootSignature;

typedef struct CGpuDescriptorSet {
    CGpuRootSignatureId root_signature;
    uint32_t index;
} CGpuDescriptorSet;

typedef struct CGpuComputePipeline {
    CGpuDeviceId device;
} CGpuComputePipeline;

typedef struct CGpuRenderPipeline {
    CGpuDeviceId device;
} CGpuRenderPipeline;

// Resources
typedef struct CGpuShaderLibraryDescriptor {
    const char8_t* name;
    uint32_t name_size;
    const uint32_t* code;
    size_t code_size;
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

typedef struct CGpuTextureViewDescriptor {
    /// Debug name used in gpu profile
    const char8_t* name;
    CGpuTextureId texture;
    ECGpuFormat format;
    ECGpuTextureDimension dims;
    uint32_t base_mip_level;
    uint32_t mip_level_count;
    uint32_t base_array_layer;
    uint32_t array_layer_count;
} CGpuTextureViewDescriptor;

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
    /// What state will the texture get created in
    ECGpuResourceState start_state;
    /// Descriptor creation
    CGpuResourceTypes descriptors;
} CGpuTextureDescriptor;

#pragma endregion DESCRIPTORS

#define SINGLE_GPU_NODE_COUNT 1
#define SINGLE_GPU_NODE_MASK 1
#define SINGLE_GPU_NODE_INDEX 0

#ifdef __cplusplus
} // end extern "C"
#endif
