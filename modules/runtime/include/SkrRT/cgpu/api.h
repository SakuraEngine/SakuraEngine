#pragma once
#include "cgpu/cgpu_config.h"
#include "cgpu/flags.h"
#include "platform/dstorage.h"

#define CGPU_ARRAY_LEN(array) ((sizeof(array) / sizeof(array[0])))
#define CGPU_MAX_MRT_COUNT 8u
#define CGPU_MAX_VERTEX_ATTRIBS 15
#define CGPU_MAX_VERTEX_BINDINGS 15
#define CGPU_COLOR_MASK_RED 0x1
#define CGPU_COLOR_MASK_GREEN 0x2
#define CGPU_COLOR_MASK_BLUE 0x4
#define CGPU_COLOR_MASK_ALPHA 0x8
#define CGPU_COLOR_MASK_ALL CGPU_COLOR_MASK_RED | CGPU_COLOR_MASK_GREEN | CGPU_COLOR_MASK_BLUE | CGPU_COLOR_MASK_ALPHA
#define CGPU_COLOR_MASK_NONE 0

#if defined(SKR_PLATFORM_WA32)
#define DEFINE_CGPU_OBJECT(name) struct name##Descriptor; typedef const host_ptr_t name##Id;
#else
#define DEFINE_CGPU_OBJECT(name) struct name##Descriptor; typedef const struct name* name##Id;
#endif

typedef uint32_t CGPUQueueIndex;

DEFINE_CGPU_OBJECT(CGPUSurface)
DEFINE_CGPU_OBJECT(CGPUInstance)
DEFINE_CGPU_OBJECT(CGPUAdapter)
DEFINE_CGPU_OBJECT(CGPUDevice)
DEFINE_CGPU_OBJECT(CGPUQueue)
DEFINE_CGPU_OBJECT(CGPUSemaphore)
DEFINE_CGPU_OBJECT(CGPUFence)
DEFINE_CGPU_OBJECT(CGPUCommandPool)
DEFINE_CGPU_OBJECT(CGPUCommandBuffer)
DEFINE_CGPU_OBJECT(CGPUSwapChain)
DEFINE_CGPU_OBJECT(CGPUShaderLibrary)
DEFINE_CGPU_OBJECT(CGPURootSignature)
DEFINE_CGPU_OBJECT(CGPURootSignaturePool)
DEFINE_CGPU_OBJECT(CGPUDescriptorSet)
DEFINE_CGPU_OBJECT(CGPUMemoryPool)
DEFINE_CGPU_OBJECT(CGPUBuffer)
DEFINE_CGPU_OBJECT(CGPUTexture)
DEFINE_CGPU_OBJECT(CGPUSampler)
DEFINE_CGPU_OBJECT(CGPUTextureView)
DEFINE_CGPU_OBJECT(CGPUQueryPool)
DEFINE_CGPU_OBJECT(CGPURenderPassEncoder)
DEFINE_CGPU_OBJECT(CGPUComputePassEncoder)
DEFINE_CGPU_OBJECT(CGPURenderPipeline)
DEFINE_CGPU_OBJECT(CGPUComputePipeline)
DEFINE_CGPU_OBJECT(CGPUShaderReflection)
DEFINE_CGPU_OBJECT(CGPUPipelineReflection)

typedef struct SkrDStorageQueueDescriptor CGPUDStorageQueueDescriptor;
typedef struct SkrDStorageFileInfo CGPUDStorageFileInfo;

typedef SkrDStorageQueue CGPUDStorageQueue;
typedef const CGPUDStorageQueue* CGPUDStorageQueueId;

typedef SkrDStorageFileRange CGPUDStorageFileRange;
typedef SkrDStorageFile CGPUDStorageFile;
typedef const CGPUDStorageFile* CGPUDStorageFileId;
typedef SkrDStorageFileHandle CGPUDStorageFileHandle;

DEFINE_CGPU_OBJECT(CGPUCompiledShader)
DEFINE_CGPU_OBJECT(CGPULinkedShader)
DEFINE_CGPU_OBJECT(CGPUBinder)
DEFINE_CGPU_OBJECT(CGPUStateBuffer)
DEFINE_CGPU_OBJECT(CGPUShaderStateEncoder)
// compute dispatches never use these state
DEFINE_CGPU_OBJECT(CGPURasterStateEncoder)
DEFINE_CGPU_OBJECT(CGPUUserStateEncoder)

struct CGPUExportTextureDescriptor;
struct CGPUImportTextureDescriptor;

struct CGPUVertexLayout;
struct CGPUTiledTextureRegions;
struct CGPUTiledTexturePackedMips;
struct CGPUBufferToBufferTransfer;
struct CGPUBufferToTilesTransfer;
struct CGPUBufferToTextureTransfer;
struct CGPUTextureToTextureTransfer;
struct CGPUQueryDescriptor;
struct CGPUDescriptorData;
struct CGPUResourceBarrierDescriptor;
struct CGPUTextureAliasingBindDescriptor;
struct CGPURenderPassDescriptor;
struct CGPUComputePassDescriptor;
struct CGPUQueueSubmitDescriptor;
struct CGPUQueuePresentDescriptor;
struct CGPUAcquireNextDescriptor;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-const-variable"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

#ifdef __cplusplus
CGPU_EXTERN_C_BEGIN
#endif

SKR_UNUSED static const CGPUBufferId CGPU_BUFFER_OUT_OF_HOST_MEMORY = (CGPUBufferId)1;
SKR_UNUSED static const CGPUBufferId CGPU_BUFFER_OUT_OF_DEVICE_MEMORY = (CGPUBufferId)3;

typedef enum ECGPUBackend
{
    CGPU_BACKEND_VULKAN = 0,
    CGPU_BACKEND_D3D12 = 1,
    CGPU_BACKEND_XBOX_D3D12 = 2,
    CGPU_BACKEND_AGC = 3,
    CGPU_BACKEND_METAL = 4,
    CGPU_BACKEND_COUNT,
    CGPU_BACKEND_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGPUBackend;

static const char8_t* gCGPUBackendNames[CGPU_BACKEND_COUNT] = {
    SKR_UTF8("vulkan"),
    SKR_UTF8("d3d12"),
    SKR_UTF8("d3d12(xbox)"),
    SKR_UTF8("agc"),
    SKR_UTF8("metal")
};

typedef enum ECGPUQueueType
{
    CGPU_QUEUE_TYPE_GRAPHICS = 0,
    CGPU_QUEUE_TYPE_COMPUTE = 1,
    CGPU_QUEUE_TYPE_TRANSFER = 2,
    CGPU_QUEUE_TYPE_TILE_MAPPING = 3,
    CGPU_QUEUE_TYPE_COUNT,
    CGPU_QUEUE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGPUQueueType;

typedef struct CGPUFormatSupport {
    uint8_t shader_read : 1;
    uint8_t shader_write : 1;
    uint8_t render_target_write : 1;
} CGPUFormatSupport;

typedef struct CGPUInstanceFeatures {
    bool specialization_constant;
} CGPUInstanceFeatures;

typedef struct CGPUBufferRange {
    uint64_t offset;
    uint64_t size;
} CGPUBufferRange;

typedef struct CGPUConstantSpecialization {
    uint32_t constantID;
    union
    {
        uint64_t u;
        int64_t i;
        double f;
    };
} CGPUConstantSpecialization;

// Above APIs
CGPU_API ECGPUBackend cgpu_instance_get_backend(CGPUInstanceId instance);

// Instance APIs
CGPU_API CGPUInstanceId cgpu_create_instance(const struct CGPUInstanceDescriptor* desc);
typedef CGPUInstanceId (*CGPUProcCreateInstance)(const struct CGPUInstanceDescriptor* descriptor);
CGPU_API void cgpu_query_instance_features(CGPUInstanceId instance, struct CGPUInstanceFeatures* features);
typedef void (*CGPUProcQueryInstanceFeatures)(CGPUInstanceId instance, struct CGPUInstanceFeatures* features);
CGPU_API void cgpu_free_instance(CGPUInstanceId instance);
typedef void (*CGPUProcFreeInstance)(CGPUInstanceId instance);

// Adapter APIs
CGPU_API void cgpu_enum_adapters(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num);
typedef void (*CGPUProcEnumAdapters)(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num);

CGPU_API const struct CGPUAdapterDetail* cgpu_query_adapter_detail(const CGPUAdapterId adapter);
typedef const struct CGPUAdapterDetail* (*CGPUProcQueryAdapterDetail)(const CGPUAdapterId adapter);
CGPU_API uint32_t cgpu_query_queue_count(const CGPUAdapterId adapter, const ECGPUQueueType type);
typedef uint32_t (*CGPUProcQueryQueueCount)(const CGPUAdapterId adapter, const ECGPUQueueType type);

// Device APIs
CGPU_API CGPUDeviceId cgpu_create_device(CGPUAdapterId adapter, const struct CGPUDeviceDescriptor* desc);
typedef CGPUDeviceId (*CGPUProcCreateDevice)(CGPUAdapterId adapter, const struct CGPUDeviceDescriptor* desc);
CGPU_API void cgpu_query_video_memory_info(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
typedef void (*CGPUProcQueryVideoMemoryInfo)(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
CGPU_API void cgpu_query_shared_memory_info(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
typedef void (*CGPUProcQuerySharedMemoryInfo)(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes);
CGPU_API void cgpu_free_device(CGPUDeviceId device);
typedef void (*CGPUProcFreeDevice)(CGPUDeviceId device);

// API Objects APIs
CGPU_API CGPUFenceId cgpu_create_fence(CGPUDeviceId device);
typedef CGPUFenceId (*CGPUProcCreateFence)(CGPUDeviceId device);
CGPU_API void cgpu_wait_fences(const CGPUFenceId* fences, uint32_t fence_count);
typedef void (*CGPUProcWaitFences)(const CGPUFenceId* fences, uint32_t fence_count);
CGPU_API ECGPUFenceStatus cgpu_query_fence_status(CGPUFenceId fence);
typedef ECGPUFenceStatus (*CGPUProcQueryFenceStatus)(CGPUFenceId fence);
CGPU_API void cgpu_free_fence(CGPUFenceId fence);
typedef void (*CGPUProcFreeFence)(CGPUFenceId fence);
CGPU_API CGPUSemaphoreId cgpu_create_semaphore(CGPUDeviceId device);
typedef CGPUSemaphoreId (*CGPUProcCreateSemaphore)(CGPUDeviceId device);
CGPU_API void cgpu_free_semaphore(CGPUSemaphoreId semaphore);
typedef void (*CGPUProcFreeSemaphore)(CGPUSemaphoreId semaphore);
CGPU_API CGPURootSignaturePoolId cgpu_create_root_signature_pool(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc);
typedef CGPURootSignaturePoolId (*CGPUProcCreateRootSignaturePool)(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc);
CGPU_API void cgpu_free_root_signature_pool(CGPURootSignaturePoolId pool);
typedef void (*CGPUProcFreeRootSignaturePool)(CGPURootSignaturePoolId pool);
CGPU_API CGPURootSignatureId cgpu_create_root_signature(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc);
typedef CGPURootSignatureId (*CGPUProcCreateRootSignature)(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc);
CGPU_API void cgpu_free_root_signature(CGPURootSignatureId signature);
typedef void (*CGPUProcFreeRootSignature)(CGPURootSignatureId signature);
CGPU_API CGPUDescriptorSetId cgpu_create_descriptor_set(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc);
typedef CGPUDescriptorSetId (*CGPUProcCreateDescriptorSet)(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc);
CGPU_API void cgpu_update_descriptor_set(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count);
typedef void (*CGPUProcUpdateDescriptorSet)(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count);
CGPU_API void cgpu_free_descriptor_set(CGPUDescriptorSetId set);
typedef void (*CGPUProcFreeDescriptorSet)(CGPUDescriptorSetId set);
CGPU_API CGPUComputePipelineId cgpu_create_compute_pipeline(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc);
typedef CGPUComputePipelineId (*CGPUProcCreateComputePipeline)(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc);
CGPU_API void cgpu_free_compute_pipeline(CGPUComputePipelineId pipeline);
typedef void (*CGPUProcFreeComputePipeline)(CGPUComputePipelineId pipeline);
CGPU_API CGPURenderPipelineId cgpu_create_render_pipeline(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc);
typedef CGPURenderPipelineId (*CGPUProcCreateRenderPipeline)(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc);
CGPU_API void cgpu_free_render_pipeline(CGPURenderPipelineId pipeline);
typedef void (*CGPUProcFreeRenderPipeline)(CGPURenderPipelineId pipeline);
CGPU_API CGPUQueryPoolId cgpu_create_query_pool(CGPUDeviceId, const struct CGPUQueryPoolDescriptor* desc);
typedef CGPUQueryPoolId (*CGPUProcCreateQueryPool)(CGPUDeviceId, const struct CGPUQueryPoolDescriptor* desc);
CGPU_API void cgpu_free_query_pool(CGPUQueryPoolId);
typedef void (*CGPUProcFreeQueryPool)(CGPUQueryPoolId);
CGPU_API CGPUMemoryPoolId cgpu_create_memory_pool(CGPUDeviceId, const struct CGPUMemoryPoolDescriptor* desc);
typedef CGPUMemoryPoolId (*CGPUProcCreateMemoryPool)(CGPUDeviceId, const struct CGPUMemoryPoolDescriptor* desc);
CGPU_API void cgpu_free_memory_pool(CGPUMemoryPoolId pool);
typedef void (*CGPUProcFreeMemoryPool)(CGPUMemoryPoolId pool);

// Queue APIs
// Warn: If you get a queue at an index with a specific type, you must hold the handle and reuses it.
CGPU_API CGPUQueueId cgpu_get_queue(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);
typedef CGPUQueueId (*CGPUProcGetQueue)(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);
CGPU_API void cgpu_submit_queue(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc);
typedef void (*CGPUProcSubmitQueue)(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc);
CGPU_API void cgpu_queue_present(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc);
typedef void (*CGPUProcQueuePresent)(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc);
CGPU_API void cgpu_wait_queue_idle(CGPUQueueId queue);
typedef void (*CGPUProcWaitQueueIdle)(CGPUQueueId queue);
CGPU_API float cgpu_queue_get_timestamp_period_ns(CGPUQueueId queue);
typedef float (*CGPUProcQueueGetTimestampPeriodNS)(CGPUQueueId queue);
CGPU_API void cgpu_queue_map_tiled_texture(CGPUQueueId queue, const struct CGPUTiledTextureRegions* desc);
typedef void (*CGPUProcQueueMapTiledTexture)(CGPUQueueId queue, const struct CGPUTiledTextureRegions* desc);
CGPU_API void cgpu_queue_unmap_tiled_texture(CGPUQueueId queue, const struct CGPUTiledTextureRegions* desc);
typedef void (*CGPUProcQueueUnmapTiledTexture)(CGPUQueueId queue, const struct CGPUTiledTextureRegions* desc);
CGPU_API void cgpu_queue_map_packed_mips(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions);
typedef void (*CGPUProcQueueMapPackedMips)(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions);
CGPU_API void cgpu_queue_unmap_packed_mips(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions);
typedef void (*CGPUProcQueueUnmapPackedMips)(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions);
CGPU_API void cgpu_free_queue(CGPUQueueId queue);
typedef void (*CGPUProcFreeQueue)(CGPUQueueId queue);

// Command APIs
CGPU_API CGPUCommandPoolId cgpu_create_command_pool(CGPUQueueId queue, const struct CGPUCommandPoolDescriptor* desc);
typedef CGPUCommandPoolId (*CGPUProcCreateCommandPool)(CGPUQueueId queue, const struct CGPUCommandPoolDescriptor* desc);
CGPU_API CGPUCommandBufferId cgpu_create_command_buffer(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc);
typedef CGPUCommandBufferId (*CGPUProcCreateCommandBuffer)(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc);
CGPU_API void cgpu_reset_command_pool(CGPUCommandPoolId pool);
typedef void (*CGPUProcResetCommandPool)(CGPUCommandPoolId pool);
CGPU_API void cgpu_free_command_buffer(CGPUCommandBufferId cmd);
typedef void (*CGPUProcFreeCommandBuffer)(CGPUCommandBufferId cmd);
CGPU_API void cgpu_free_command_pool(CGPUCommandPoolId pool);
typedef void (*CGPUProcFreeCommandPool)(CGPUCommandPoolId pool);

// Shader APIs
CGPU_API CGPUShaderLibraryId cgpu_create_shader_library(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc);
typedef CGPUShaderLibraryId (*CGPUProcCreateShaderLibrary)(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc);
CGPU_API void cgpu_free_shader_library(CGPUShaderLibraryId library);
typedef void (*CGPUProcFreeShaderLibrary)(CGPUShaderLibraryId library);

// Buffer APIs
CGPU_API CGPUBufferId cgpu_create_buffer(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc);
typedef CGPUBufferId (*CGPUProcCreateBuffer)(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc);
CGPU_API void cgpu_map_buffer(CGPUBufferId buffer, const struct CGPUBufferRange* range);
typedef void (*CGPUProcMapBuffer)(CGPUBufferId buffer, const struct CGPUBufferRange* range);
CGPU_API void cgpu_unmap_buffer(CGPUBufferId buffer);
typedef void (*CGPUProcUnmapBuffer)(CGPUBufferId buffer);
CGPU_API void cgpu_free_buffer(CGPUBufferId buffer);
typedef void (*CGPUProcFreeBuffer)(CGPUBufferId buffer);

// Sampler APIs
CGPU_API CGPUSamplerId cgpu_create_sampler(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc);
typedef CGPUSamplerId (*CGPUProcCreateSampler)(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc);
CGPU_API void cgpu_free_sampler(CGPUSamplerId sampler);
typedef void (*CGPUProcFreeSampler)(CGPUSamplerId sampler);

// Texture/TextureView APIs
CGPU_API CGPUTextureId cgpu_create_texture(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc);
typedef CGPUTextureId (*CGPUProcCreateTexture)(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc);
CGPU_API void cgpu_free_texture(CGPUTextureId texture);
typedef void (*CGPUProcFreeTexture)(CGPUTextureId texture);
CGPU_API CGPUTextureViewId cgpu_create_texture_view(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc);
typedef CGPUTextureViewId (*CGPUProcCreateTextureView)(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc);
CGPU_API void cgpu_free_texture_view(CGPUTextureViewId render_target);
typedef void (*CGPUProcFreeTextureView)(CGPUTextureViewId render_target);
CGPU_API bool cgpu_try_bind_aliasing_texture(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc);
typedef bool (*CGPUProcTryBindAliasingTexture)(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc);

// Shared Resource APIs
CGPU_API uint64_t cgpu_export_shared_texture_handle(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc);
typedef uint64_t (*CGPUProcExportSharedTextureHandle)(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc);
CGPU_API CGPUTextureId cgpu_import_shared_texture_handle(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc);
typedef CGPUTextureId (*CGPUProcImportSharedTextureHandle)(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc);

// Swapchain APIs
CGPU_API CGPUSwapChainId cgpu_create_swapchain(CGPUDeviceId device, const struct CGPUSwapChainDescriptor* desc);
typedef CGPUSwapChainId (*CGPUProcCreateSwapChain)(CGPUDeviceId device, const struct CGPUSwapChainDescriptor* desc);
CGPU_API uint32_t cgpu_acquire_next_image(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc);
typedef uint32_t (*CGPUProcAcquireNext)(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc);
CGPU_API void cgpu_free_swapchain(CGPUSwapChainId swapchain);
typedef void (*CGPUProcFreeSwapChain)(CGPUSwapChainId swapchain);

// CMDs
CGPU_API void cgpu_cmd_begin(CGPUCommandBufferId cmd);
typedef void (*CGPUProcCmdBegin)(CGPUCommandBufferId cmd);
CGPU_API void cgpu_cmd_transfer_buffer_to_buffer(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc);
typedef void (*CGPUProcCmdTransferBufferToBuffer)(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc);
CGPU_API void cgpu_cmd_transfer_texture_to_texture(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc);
typedef void (*CGPUProcCmdTransferTextureToTexture)(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc);
CGPU_API void cgpu_cmd_transfer_buffer_to_texture(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc);
typedef void (*CGPUProcCmdTransferBufferToTexture)(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc);
CGPU_API void cgpu_cmd_transfer_buffer_to_tiles(CGPUCommandBufferId cmd, const struct CGPUBufferToTilesTransfer* desc);
typedef void (*CGPUProcCmdTransferBufferToTiles)(CGPUCommandBufferId cmd, const struct CGPUBufferToTilesTransfer* desc);
CGPU_API void cgpu_cmd_resource_barrier(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc);
typedef void (*CGPUProcCmdResourceBarrier)(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc);
CGPU_API void cgpu_cmd_begin_query(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
typedef void (*CGPUProcCmdBeginQuery)(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
CGPU_API void cgpu_cmd_end_query(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
typedef void (*CGPUProcCmdEndQuery)(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
CGPU_API void cgpu_cmd_reset_query_pool(CGPUCommandBufferId cmd, CGPUQueryPoolId, uint32_t start_query, uint32_t query_count);
typedef void (*CGPUProcCmdResetQueryPool)(CGPUCommandBufferId cmd, CGPUQueryPoolId, uint32_t start_query, uint32_t query_count);
CGPU_API void cgpu_cmd_resolve_query(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count);
typedef void (*CGPUProcCmdResolveQuery)(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count);
CGPU_API void cgpu_cmd_end(CGPUCommandBufferId cmd);
typedef void (*CGPUProcCmdEnd)(CGPUCommandBufferId cmd);

// Compute Pass
CGPU_API CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc);
typedef CGPUComputePassEncoderId (*CGPUProcCmdBeginComputePass)(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc);
CGPU_API void cgpu_compute_encoder_bind_descriptor_set(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set);
typedef void (*CGPUProcComputeEncoderBindDescriptorSet)(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set);
CGPU_API void cgpu_compute_encoder_bind_pipeline(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline);
typedef void (*CGPUProcComputeEncoderBindPipeline)(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline);
CGPU_API void cgpu_compute_encoder_dispatch(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
typedef void (*CGPUProcComputeEncoderDispatch)(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
CGPU_API void cgpu_cmd_end_compute_pass(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder);
typedef void (*CGPUProcCmdEndComputePass)(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder);

// Render Pass
CGPU_API CGPURenderPassEncoderId cgpu_cmd_begin_render_pass(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc);
typedef CGPURenderPassEncoderId (*CGPUProcCmdBeginRenderPass)(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc);
CGPU_API void cgpu_render_encoder_set_shading_rate(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate);
typedef void (*CGPUProcRenderEncoderSetShadingRate)(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate);
CGPU_API void cgpu_render_encoder_bind_descriptor_set(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set);
typedef void (*CGPUProcRenderEncoderBindDescriptorSet)(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set);
CGPU_API void cgpu_render_encoder_set_viewport(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
typedef void (*CGPUProcRenderEncoderSetViewport)(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
CGPU_API void cgpu_render_encoder_set_scissor(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*CGPUProcRenderEncoderSetScissor)(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
CGPU_API void cgpu_render_encoder_bind_pipeline(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline);
typedef void (*CGPUProcRenderEncoderBindPipeline)(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline);
CGPU_API void cgpu_render_encoder_bind_vertex_buffers(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
typedef void (*CGPUProcRendeEncoderBindVertexBuffers)(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
CGPU_API void cgpu_render_encoder_bind_index_buffer(CGPURenderPassEncoderId encoder, CGPUBufferId buffer,
uint32_t index_stride, uint64_t offset);
typedef void (*CGPUProcRendeEncoderBindIndexBuffer)(CGPURenderPassEncoderId encoder, CGPUBufferId buffer,
uint32_t index_stride, uint64_t offset);
CGPU_API void cgpu_render_encoder_push_constants(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
typedef void (*CGPUProcRenderEncoderPushConstants)(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
CGPU_API void cgpu_compute_encoder_push_constants(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
typedef void (*CGPUProcComputeEncoderPushConstants)(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
CGPU_API void cgpu_render_encoder_draw(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
typedef void (*CGPUProcRenderEncoderDraw)(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
CGPU_API void cgpu_render_encoder_draw_instanced(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
typedef void (*CGPUProcRenderEncoderDrawInstanced)(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
CGPU_API void cgpu_render_encoder_draw_indexed(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
typedef void (*CGPUProcRenderEncoderDrawIndexed)(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
CGPU_API void cgpu_render_encoder_draw_indexed_instanced(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
typedef void (*CGPUProcRenderEncoderDrawIndexedInstanced)(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
CGPU_API void cgpu_cmd_end_render_pass(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder);
typedef void (*CGPUProcCmdEndRenderPass)(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder);

// Event & Markers
typedef struct CGPUEventInfo {
    const char8_t* name;
    float color[4];
} CGPUEventInfo;
typedef struct CGPUMarkerInfo {
    const char8_t* name;
    float color[4];
} CGPUMarkerInfo;
CGPU_API void cgpu_cmd_begin_event(CGPUCommandBufferId cmd, const CGPUEventInfo* event);
typedef void (*CGPUProcCmdBeginEvent)(CGPUCommandBufferId cmd, const CGPUEventInfo* event);
CGPU_API void cgpu_cmd_set_marker(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker);
typedef void (*CGPUProcCmdSetMarker)(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker);
CGPU_API void cgpu_cmd_end_event(CGPUCommandBufferId cmd);
typedef void (*CGPUProcCmdEndEvent)(CGPUCommandBufferId cmd);

// dstorage
typedef struct CGPUDStorageBufferIODescriptor {
    CGPUDStorageCompression compression;
    ECGPUDStorageSource source_type;
    SkrDStorageMemoryRange source_memory;
    CGPUDStorageFileRange source_file;
    CGPUBufferId buffer;
    uint64_t offset;
    uint64_t uncompressed_size;
    CGPUFenceId fence;
    const char8_t* name;
} CGPUDStorageBufferIODescriptor;

typedef struct CGPUDStorageTextureIODescriptor {
    CGPUDStorageCompression compression;
    ECGPUDStorageSource source_type;
    SkrDStorageMemoryRange source_memory;
    CGPUDStorageFileRange source_file;
    CGPUTextureId texture;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint64_t uncompressed_size;
    CGPUFenceId fence;
    const char8_t* name;
} CGPUDStorageTextureIODescriptor;

CGPU_API ECGPUDStorageAvailability cgpu_query_dstorage_availability(CGPUDeviceId device);
typedef ECGPUDStorageAvailability (*CGPUProcQueryDStorageAvailability)(CGPUDeviceId device);
CGPU_API CGPUDStorageQueueId cgpu_create_dstorage_queue(CGPUDeviceId device, const CGPUDStorageQueueDescriptor* desc);
typedef CGPUDStorageQueueId (*CGPUProcCreateDStorageQueue)(CGPUDeviceId device, const CGPUDStorageQueueDescriptor* desc);
CGPU_API CGPUDStorageFileHandle cgpu_dstorage_open_file(CGPUDStorageQueueId queue, const char* abs_path);
typedef CGPUDStorageFileHandle (*CGPUProcDStorageOpenFile)(CGPUDStorageQueueId queue, const char* abs_path);
CGPU_API void cgpu_dstorage_query_file_info(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file, CGPUDStorageFileInfo* info);
typedef void (*CGPUProcDStorageQueryFileInfo)(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file, CGPUDStorageFileInfo* info);
CGPU_API void cgpu_dstorage_enqueue_buffer_request(CGPUDStorageQueueId queue, const CGPUDStorageBufferIODescriptor* desc);
typedef void (*CGPUProcDStorageEnqueueBufferRequest)(CGPUDStorageQueueId queue, const CGPUDStorageBufferIODescriptor* desc);
CGPU_API void cgpu_dstorage_enqueue_texture_request(CGPUDStorageQueueId queue, const CGPUDStorageTextureIODescriptor* desc);
typedef void (*CGPUProcDStorageEnqueueTextureRequest)(CGPUDStorageQueueId queue, const CGPUDStorageTextureIODescriptor* desc);
CGPU_API void cgpu_dstorage_queue_submit(CGPUDStorageQueueId queue, CGPUFenceId fence);
typedef void (*CGPUProcDStorageQueueSubmit)(CGPUDStorageQueueId queue, CGPUFenceId fence);
CGPU_API void cgpu_dstorage_close_file(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file);
typedef void (*CGPUProcDStorageCloseFile)(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file);
CGPU_API void cgpu_free_dstorage_queue(CGPUDStorageQueueId queue);
typedef void (*CGPUProcFreeDStorageQueue)(CGPUDStorageQueueId queue);

// EXPERIMENTAL Compiled/Linked ISA APIs
CGPU_API CGPULinkedShaderId cgpu_compile_and_link_shaders(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count);
typedef CGPULinkedShaderId (*CGPUProcCompileAndLinkShaders)(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* desc, uint32_t count);
CGPU_API void cgpu_compile_shaders(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count, CGPUCompiledShaderId* out_isas);
typedef void (*CGPUProcCompileShaders)(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* desc, uint32_t count, CGPUCompiledShaderId* out_isas);
CGPU_API void cgpu_free_compiled_shader(CGPUCompiledShaderId shader);
typedef void (*CGPUProcFreeCompiledShader)(CGPUCompiledShaderId shader);
CGPU_API void cgpu_free_linked_shader(CGPULinkedShaderId shader);
typedef void (*CGPUProcFreeLinkedShader)(CGPULinkedShaderId shader);

// EXPERIMENTAL StateBuffer APIs
CGPU_API CGPUStateBufferId cgpu_create_state_buffer(CGPUCommandBufferId cmd, const struct CGPUStateBufferDescriptor* desc);
typedef CGPUStateBufferId (*CGPUProcCreateStateBuffer)(CGPUCommandBufferId cmd, const struct CGPUStateBufferDescriptor* desc);
CGPU_API void cgpu_render_encoder_bind_state_buffer(CGPURenderPassEncoderId encoder, CGPUStateBufferId stream);
typedef void (*CGPUProcRenderEncoderBindStateBuffer)(CGPURenderPassEncoderId encoder, CGPUStateBufferId stream);
CGPU_API void cgpu_compute_encoder_bind_state_buffer(CGPUComputePassEncoderId encoder, CGPUStateBufferId stream);
typedef void (*CGPUProcComputeEncoderBindStateBuffer)(CGPUComputePassEncoderId encoder, CGPUStateBufferId stream);
CGPU_API void cgpu_free_state_buffer(CGPUStateBufferId stream);
typedef void (*CGPUProcFreeStateBuffer)(CGPUStateBufferId stream);

// raster state encoder APIs
CGPU_API CGPURasterStateEncoderId cgpu_open_raster_state_encoder(CGPUStateBufferId stream, CGPURenderPassEncoderId encoder);
typedef CGPURasterStateEncoderId (*CGPUProcOpenRasterStateEncoder)(CGPUStateBufferId stream, CGPURenderPassEncoderId encoder);
CGPU_API void cgpu_raster_state_encoder_set_viewport(CGPURasterStateEncoderId, float x, float y, float width, float height, float min_depth, float max_depth);
typedef void (*CGPUProcRasterStateEncoderSetViewport)(CGPURasterStateEncoderId, float x, float y, float width, float height, float min_depth, float max_depth);
CGPU_API void cgpu_raster_state_encoder_set_scissor(CGPURasterStateEncoderId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*CGPUProcRasterStateEncoderSetScissor)(CGPURasterStateEncoderId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
CGPU_API void cgpu_raster_state_encoder_set_cull_mode(CGPURasterStateEncoderId, ECGPUCullMode cull_mode);
typedef void (*CGPUProcRasterStateEncoderSetCullMode)(CGPURasterStateEncoderId, ECGPUCullMode cull_mode);
CGPU_API void cgpu_raster_state_encoder_set_front_face(CGPURasterStateEncoderId, ECGPUFrontFace front_face);
typedef void (*CGPUProcRasterStateEncoderSetFrontFace)(CGPURasterStateEncoderId, ECGPUFrontFace front_face);
CGPU_API void cgpu_raster_state_encoder_set_primitive_topology(CGPURasterStateEncoderId, ECGPUPrimitiveTopology topology);
typedef void (*CGPUProcRasterStateEncoderSetPrimitiveTopology)(CGPURasterStateEncoderId, ECGPUPrimitiveTopology topology);
CGPU_API void cgpu_raster_state_encoder_set_depth_test_enabled(CGPURasterStateEncoderId, bool enabled);
typedef void (*CGPUProcRasterStateEncoderSetDepthTestEnabled)(CGPURasterStateEncoderId, bool enabled);
CGPU_API void cgpu_raster_state_encoder_set_depth_write_enabled(CGPURasterStateEncoderId, bool enabled);
typedef void (*CGPUProcRasterStateEncoderSetDepthWriteEnabled)(CGPURasterStateEncoderId, bool enabled);
CGPU_API void cgpu_raster_state_encoder_set_depth_compare_op(CGPURasterStateEncoderId, ECGPUCompareMode compare_op);
typedef void (*CGPUProcRasterStateEncoderSetDepthCompareOp)(CGPURasterStateEncoderId, ECGPUCompareMode compare_op);
CGPU_API void cgpu_raster_state_encoder_set_stencil_test_enabled(CGPURasterStateEncoderId, bool enabled);
typedef void (*CGPUProcRasterStateEncoderSetStencilTestEnabled)(CGPURasterStateEncoderId, bool enabled);
CGPU_API void cgpu_raster_state_encoder_set_stencil_compare_op(CGPURasterStateEncoderId, CGPUStencilFaces faces, ECGPUStencilOp failOp, ECGPUStencilOp passOp, ECGPUStencilOp depthFailOp, ECGPUCompareMode compareOp);
typedef void (*CGPUProcRasterStateEncoderSetStencilCompareOp)(CGPURasterStateEncoderId, CGPUStencilFaces faces, ECGPUStencilOp failOp, ECGPUStencilOp passOp, ECGPUStencilOp depthFailOp, ECGPUCompareMode compareOp);
CGPU_API void cgpu_raster_state_encoder_set_fill_mode(CGPURasterStateEncoderId, ECGPUFillMode fill_mode);
typedef void (*CGPUProcRasterStateEncoderSetFillMode)(CGPURasterStateEncoderId, ECGPUFillMode fill_mode);
CGPU_API void cgpu_raster_state_encoder_set_sample_count(CGPURasterStateEncoderId, ECGPUSampleCount sample_count);
typedef void (*CGPUProcRasterStateEncoderSetSampleCount)(CGPURasterStateEncoderId, ECGPUSampleCount sample_count);
CGPU_API void cgpu_close_raster_state_encoder(CGPURasterStateEncoderId encoder);
typedef void (*CGPUProcCloseRasterStateEncoder)(CGPURasterStateEncoderId encoder);

// shader state encoder APIs
CGPU_API CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_r(CGPUStateBufferId stream, CGPURenderPassEncoderId encoder);
typedef CGPUShaderStateEncoderId (*CGPUProcOpenShaderStateEncoderR)(CGPUStateBufferId stream, CGPURenderPassEncoderId encoder);
CGPU_API CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_c(CGPUStateBufferId stream, CGPUComputePassEncoderId encoder);
typedef CGPUShaderStateEncoderId (*CGPUProcOpenShaderStateEncoderC)(CGPUStateBufferId stream, CGPUComputePassEncoderId encoder);
CGPU_API void cgpu_shader_state_encoder_bind_shaders(CGPUShaderStateEncoderId, uint32_t stage_count, const ECGPUShaderStage* stages, const CGPUCompiledShaderId* shaders);
typedef void (*CGPUProcShaderStateEncoderBindShaders)(CGPUShaderStateEncoderId, uint32_t stage_count, const ECGPUShaderStage* stages, const CGPUCompiledShaderId* shaders);
CGPU_API void cgpu_shader_state_encoder_bind_linked_shader(CGPUShaderStateEncoderId, CGPULinkedShaderId linked);
typedef void (*CGPUProcShaderStateEncoderBindLinkedShader)(CGPUShaderStateEncoderId, CGPULinkedShaderId linked);
CGPU_API void cgpu_close_shader_state_encoder(CGPUShaderStateEncoderId encoder);
typedef void (*CGPUProcCloseShaderStateEncoder)(CGPUShaderStateEncoderId encoder);

// user state encoder APIs
CGPU_API CGPUUserStateEncoderId cgpu_open_user_state_encoder(CGPUStateBufferId stream, CGPURenderPassEncoderId encoder);
typedef CGPUUserStateEncoderId (*CGPUProcOpenUserStateEncoder)(CGPUStateBufferId stream, CGPURenderPassEncoderId encoder);
CGPU_API void cgpu_close_user_state_encoder(CGPUUserStateEncoderId encoder);
typedef void (*CGPUProcCloseUserStateEncoder)(CGPUUserStateEncoderId encoder);

// EXPERIMENTAL binder APIs
CGPU_API CGPUBinderId cgpu_create_binder(CGPUCommandBufferId cmd);
typedef CGPUBinderId (*CGPUProcCreateBinder)(CGPUCommandBufferId cmd);
CGPU_API void cgpu_binder_bind_vertex_layout(CGPUBinderId, const struct CGPUVertexLayout* layout);
typedef void (*CGPUProcBinderBindVertexLayout)(CGPUBinderId, const struct CGPUVertexLayout* layout);
CGPU_API void cgpu_binder_bind_vertex_buffer(CGPUBinderId, uint32_t first_binding, uint32_t binding_count, const CGPUBufferId* buffers, const uint64_t* offsets, const uint64_t* sizes, const uint64_t* strides);
typedef void (*CGPUProcBinderBindVertexBuffer)(CGPUBinderId, uint32_t first_binding, uint32_t binding_count, const CGPUBufferId* buffers, const uint64_t* offsets, const uint64_t* sizes, const uint64_t* strides);
CGPU_API void cgpu_free_binder(CGPUBinderId binder);
typedef void (*CGPUProcFreeBinder)(CGPUBinderId binder);

// cgpux
CGPU_API CGPUBufferId cgpux_create_mapped_constant_buffer(CGPUDeviceId device,
    uint64_t size, const char8_t* name, bool device_local_preferred);
CGPU_API CGPUBufferId cgpux_create_mapped_upload_buffer(CGPUDeviceId device,
    uint64_t size, const char8_t* name);
CGPU_API bool cgpux_adapter_is_nvidia(CGPUAdapterId adapter);
CGPU_API bool cgpux_adapter_is_amd(CGPUAdapterId adapter);
CGPU_API bool cgpux_adapter_is_intel(CGPUAdapterId adapter);

// Types
typedef struct CGPUProcTable {
    // Instance APIs
    const CGPUProcCreateInstance create_instance;
    const CGPUProcQueryInstanceFeatures query_instance_features;
    const CGPUProcFreeInstance free_instance;

    // Adapter APIs
    const CGPUProcEnumAdapters enum_adapters;
    const CGPUProcQueryAdapterDetail query_adapter_detail;
    const CGPUProcQueryVideoMemoryInfo query_video_memory_info;
    const CGPUProcQuerySharedMemoryInfo query_shared_memory_info;
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
    const CGPUProcCreateRootSignaturePool create_root_signature_pool;
    const CGPUProcFreeRootSignaturePool free_root_signature_pool;
    const CGPUProcCreateRootSignature create_root_signature;
    const CGPUProcFreeRootSignature free_root_signature;
    const CGPUProcCreateDescriptorSet create_descriptor_set;
    const CGPUProcFreeDescriptorSet free_descriptor_set;
    const CGPUProcUpdateDescriptorSet update_descriptor_set;
    const CGPUProcCreateComputePipeline create_compute_pipeline;
    const CGPUProcFreeComputePipeline free_compute_pipeline;
    const CGPUProcCreateRenderPipeline create_render_pipeline;
    const CGPUProcFreeRenderPipeline free_render_pipeline;
    const CGPUProcCreateMemoryPool create_memory_pool;
    const CGPUProcFreeMemoryPool free_memory_pool;
    const CGPUProcCreateQueryPool create_query_pool;
    const CGPUProcFreeQueryPool free_query_pool;

    // Queue APIs
    const CGPUProcGetQueue get_queue;
    const CGPUProcSubmitQueue submit_queue;
    const CGPUProcWaitQueueIdle wait_queue_idle;
    const CGPUProcQueuePresent queue_present;
    const CGPUProcQueueGetTimestampPeriodNS queue_get_timestamp_period;
    const CGPUProcQueueMapTiledTexture queue_map_tiled_texture;
    const CGPUProcQueueUnmapTiledTexture queue_unmap_tiled_texture;
    const CGPUProcQueueMapPackedMips queue_map_packed_mips;
    const CGPUProcQueueUnmapPackedMips queue_unmap_packed_mips;
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
    const CGPUProcTryBindAliasingTexture try_bind_aliasing_texture;

    // Shared Resource APIs
    const CGPUProcExportSharedTextureHandle export_shared_texture_handle;
    const CGPUProcImportSharedTextureHandle import_shared_texture_handle;

    // Swapchain APIs
    const CGPUProcCreateSwapChain create_swapchain;
    const CGPUProcAcquireNext acquire_next_image;
    const CGPUProcFreeSwapChain free_swapchain;

    // CMDs
    const CGPUProcCmdBegin cmd_begin;
    const CGPUProcCmdTransferBufferToBuffer cmd_transfer_buffer_to_buffer;
    const CGPUProcCmdTransferBufferToTexture cmd_transfer_buffer_to_texture;
    const CGPUProcCmdTransferBufferToTiles cmd_transfer_buffer_to_tiles;
    const CGPUProcCmdTransferTextureToTexture cmd_transfer_texture_to_texture;
    const CGPUProcCmdResourceBarrier cmd_resource_barrier;
    const CGPUProcCmdBeginQuery cmd_begin_query;
    const CGPUProcCmdEndQuery cmd_end_query;
    const CGPUProcCmdResetQueryPool cmd_reset_query_pool;
    const CGPUProcCmdResolveQuery cmd_resolve_query;
    const CGPUProcCmdEnd cmd_end;

    // Compute CMDs
    const CGPUProcCmdBeginComputePass cmd_begin_compute_pass;
    const CGPUProcComputeEncoderBindDescriptorSet compute_encoder_bind_descriptor_set;
    const CGPUProcComputeEncoderPushConstants compute_encoder_push_constants;
    const CGPUProcComputeEncoderBindPipeline compute_encoder_bind_pipeline;
    const CGPUProcComputeEncoderDispatch compute_encoder_dispatch;
    const CGPUProcCmdEndComputePass cmd_end_compute_pass;

    // Render CMDs
    const CGPUProcCmdBeginRenderPass cmd_begin_render_pass;
    const CGPUProcRenderEncoderSetShadingRate render_encoder_set_shading_rate;
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

    // Events & Markers
    const CGPUProcCmdBeginEvent cmd_begin_event;
    const CGPUProcCmdSetMarker cmd_set_marker;
    const CGPUProcCmdEndEvent cmd_end_event;

    // DStorage
    const CGPUProcQueryDStorageAvailability query_dstorage_availability;
    const CGPUProcCreateDStorageQueue create_dstorage_queue;
    const CGPUProcDStorageOpenFile dstorage_open_file;
    const CGPUProcDStorageQueryFileInfo dstorage_query_file_info;
    const CGPUProcDStorageEnqueueBufferRequest dstorage_enqueue_buffer_request;
    const CGPUProcDStorageEnqueueTextureRequest dstorage_enqueue_texture_request;
    const CGPUProcDStorageQueueSubmit dstorage_queue_submit;
    const CGPUProcDStorageCloseFile dstorage_close_file;
    const CGPUProcFreeDStorageQueue free_dstorage_queue;

    // Compiled/Linked ISA APIs
    const CGPUProcCompileAndLinkShaders compile_and_link_shaders;
    const CGPUProcCompileShaders compile_shaders;
    const CGPUProcFreeCompiledShader free_compiled_shader;
    const CGPUProcFreeLinkedShader free_linked_shader;

    // StateBuffer APIs
    const CGPUProcCreateStateBuffer create_state_buffer;
    const CGPUProcRenderEncoderBindStateBuffer render_encoder_bind_state_buffer;
    const CGPUProcComputeEncoderBindStateBuffer compute_encoder_bind_state_buffer;
    const CGPUProcFreeStateBuffer free_state_buffer;

    // raster state encoder APIs
    const CGPUProcOpenRasterStateEncoder open_raster_state_encoder;
    const CGPUProcRasterStateEncoderSetViewport raster_state_encoder_set_viewport;
    const CGPUProcRasterStateEncoderSetScissor raster_state_encoder_set_scissor;
    const CGPUProcRasterStateEncoderSetCullMode raster_state_encoder_set_cull_mode;
    const CGPUProcRasterStateEncoderSetFrontFace raster_state_encoder_set_front_face;
    const CGPUProcRasterStateEncoderSetPrimitiveTopology raster_state_encoder_set_primitive_topology;
    const CGPUProcRasterStateEncoderSetDepthTestEnabled raster_state_encoder_set_depth_test_enabled;
    const CGPUProcRasterStateEncoderSetDepthWriteEnabled raster_state_encoder_set_depth_write_enabled;
    const CGPUProcRasterStateEncoderSetDepthCompareOp raster_state_encoder_set_depth_compare_op;
    const CGPUProcRasterStateEncoderSetStencilTestEnabled raster_state_encoder_set_stencil_test_enabled;
    const CGPUProcRasterStateEncoderSetStencilCompareOp raster_state_encoder_set_stencil_compare_op;
    const CGPUProcRasterStateEncoderSetFillMode raster_state_encoder_set_fill_mode;
    const CGPUProcRasterStateEncoderSetSampleCount raster_state_encoder_set_sample_count;
    const CGPUProcCloseRasterStateEncoder close_raster_state_encoder;

    // shader state encoder APIs
    const CGPUProcOpenShaderStateEncoderR open_shader_state_encoder_r;
    const CGPUProcOpenShaderStateEncoderC open_shader_state_encoder_c;
    const CGPUProcShaderStateEncoderBindShaders shader_state_encoder_bind_shaders;
    const CGPUProcShaderStateEncoderBindLinkedShader shader_state_encoder_bind_linked_shader;
    const CGPUProcCloseShaderStateEncoder close_shader_state_encoder;

    // user state encoder APIs
    const CGPUProcOpenUserStateEncoder open_user_state_encoder;
    const CGPUProcCloseUserStateEncoder close_user_state_encoder;

    // binder APIs
    const CGPUProcCreateBinder create_binder;
    const CGPUProcBinderBindVertexLayout binder_bind_vertex_layout;
    const CGPUProcBinderBindVertexBuffer binder_bind_vertex_buffer;
    const CGPUProcFreeBinder free_binder;
} CGPUProcTable;

// surfaces
CGPU_API CGPUSurfaceId cgpu_surface_from_native_view(CGPUDeviceId device, void* view);
#if defined(_WIN32) || defined(_WIN64)
typedef struct HWND__* HWND;
CGPU_API CGPUSurfaceId cgpu_surface_from_hwnd(CGPUDeviceId device, HWND window);
typedef CGPUSurfaceId (*CGPUSurfaceProc_CreateFromHWND)(CGPUDeviceId device, HWND window);
#endif
#ifdef __APPLE__
// CGPU_API CGPUSurfaceId cgpu_surface_from_ui_view(CGPUDeviceId device, UIView* window);
// typedef CGPUSurfaceId (*CGPUSurfaceProc_CreateFromUIView)(CGPUDeviceId device, UIView* window);
typedef struct CGPUNSView CGPUNSView;
CGPU_API CGPUSurfaceId cgpu_surface_from_ns_view(CGPUDeviceId device, CGPUNSView* window);
typedef CGPUSurfaceId (*CGPUSurfaceProc_CreateFromNSView)(CGPUDeviceId device, CGPUNSView* window);
#endif
CGPU_API void cgpu_free_surface(CGPUDeviceId device, CGPUSurfaceId surface);
typedef void (*CGPUSurfaceProc_Free)(CGPUDeviceId device, CGPUSurfaceId surface);
typedef struct CGPUSurfacesProcTable {
#if defined(_WIN32) || defined(_WIN64)
    const CGPUSurfaceProc_CreateFromHWND from_hwnd;
#endif
#ifdef __APPLE__
    // const CGPUSurfaceProc_CreateFromUIView from_ui_view;
    const CGPUSurfaceProc_CreateFromNSView from_ns_view;
#endif
    const CGPUSurfaceProc_Free free_surface;
} CGPUSurfacesProcTable;

typedef struct CGPUVendorPreset {
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t driver_version;
    char gpu_name[MAX_GPU_VENDOR_STRING_LENGTH]; // If GPU Name is missing then value will be empty string
} CGPUVendorPreset;

static const uint64_t CGPU_DYNAMIC_STATE_CullMode = 1ull << 0;
static const uint64_t CGPU_DYNAMIC_STATE_FrontFace = 1ull << 1;
static const uint64_t CGPU_DYNAMIC_STATE_PrimitiveTopology = 1ull << 2;
static const uint64_t CGPU_DYNAMIC_STATE_DepthTest = 1ull << 3;
static const uint64_t CGPU_DYNAMIC_STATE_DepthWrite = 1ull << 4;
static const uint64_t CGPU_DYNAMIC_STATE_DepthCompare = 1ull << 5;
static const uint64_t CGPU_DYNAMIC_STATE_DepthBoundsTest = 1ull << 6;
static const uint64_t CGPU_DYNAMIC_STATE_StencilTest = 1ull << 7;
static const uint64_t CGPU_DYNAMIC_STATE_StencilOp = 1ull << 8;
static const uint64_t CGPU_DYNAMIC_STATE_Tier1 = (1ull << 9) - 1;

static const uint64_t CGPU_DYNAMIC_STATE_RasterDiscard = 1ull << 9;
static const uint64_t CGPU_DYNAMIC_STATE_DepthBias = 1ull << 10;
static const uint64_t CGPU_DYNAMIC_STATE_PrimitiveRestart = 1ull << 11;
static const uint64_t CGPU_DYNAMIC_STATE_LogicOp = 1ull << 12;
static const uint64_t CGPU_DYNAMIC_STATE_PatchControlPoints = 1ull << 13;
static const uint64_t CGPU_DYNAMIC_STATE_Tier2 = (1ull << 14) - 1;

static const uint64_t CGPU_DYNAMIC_STATE_TessellationDomainOrigin = 1ull << 14;
static const uint64_t CGPU_DYNAMIC_STATE_DepthClampEnable = 1ull << 15;
static const uint64_t CGPU_DYNAMIC_STATE_PolygonMode = 1ull << 16;
static const uint64_t CGPU_DYNAMIC_STATE_SampleCount = 1ull << 17;
static const uint64_t CGPU_DYNAMIC_STATE_SampleMask = 1ull << 18;
static const uint64_t CGPU_DYNAMIC_STATE_AlphaToCoverageEnable = 1ull << 19;
static const uint64_t CGPU_DYNAMIC_STATE_AlphaToOneEnable = 1ull << 20;
static const uint64_t CGPU_DYNAMIC_STATE_LogicOpEnable = 1ull << 21;
static const uint64_t CGPU_DYNAMIC_STATE_ColorBlendEnable = 1ull << 22;
static const uint64_t CGPU_DYNAMIC_STATE_ColorBlendEquation = 1ull << 23;
static const uint64_t CGPU_DYNAMIC_STATE_ColorWriteMask = 1ull << 24;
static const uint64_t CGPU_DYNAMIC_STATE_RasterStream = 1ull << 25;
static const uint64_t CGPU_DYNAMIC_STATE_ConservativeRasterMode = 1ull << 26;
static const uint64_t CGPU_DYNAMIC_STATE_ExtraPrimitiveOverestimationSize = 1ull << 27;
static const uint64_t CGPU_DYNAMIC_STATE_DepthClipEnable = 1ull << 28;
static const uint64_t CGPU_DYNAMIC_STATE_SampleLocationsEnable = 1ull << 29;
static const uint64_t CGPU_DYNAMIC_STATE_ColorBlendAdvanced = 1ull << 30;
static const uint64_t CGPU_DYNAMIC_STATE_ProvokingVertexMode = 1ull << 31;
static const uint64_t CGPU_DYNAMIC_STATE_LineRasterizationMode = 1ull << 32;
static const uint64_t CGPU_DYNAMIC_STATE_LineStippleEnable = 1ull << 33;
static const uint64_t CGPU_DYNAMIC_STATE_DepthClipNegativeOneToOne = 1ull << 34;
static const uint64_t CGPU_DYNAMIC_STATE_ViewportWScalingEnable = 1ull << 35;
static const uint64_t CGPU_DYNAMIC_STATE_ViewportSwizzle = 1ull << 36;
static const uint64_t CGPU_DYNAMIC_STATE_CoverageToColorEnable = 1ull << 37;
static const uint64_t CGPU_DYNAMIC_STATE_CoverageToColorLocation = 1ull << 38;
static const uint64_t CGPU_DYNAMIC_STATE_CoverageModulationMode = 1ull << 39;
static const uint64_t CGPU_DYNAMIC_STATE_CoverageModulationTableEnable = 1ull << 40;
static const uint64_t CGPU_DYNAMIC_STATE_CoverageModulationTable = 1ull << 41;
static const uint64_t CGPU_DYNAMIC_STATE_CoverageReductionMode = 1ull << 42;
static const uint64_t CGPU_DYNAMIC_STATE_RepresentativeFragmentTestEnable = 1ull << 43;
static const uint64_t CGPU_DYNAMIC_STATE_ShadingRateImageEnable = 1ull << 44;
static const uint64_t CGPU_DYNAMIC_STATE_Tier3 = (1ull << 45) - 1;

typedef uint64_t CGPUDynamicStateFeatures;

typedef struct CGPUAdapterDetail {
    uint32_t uniform_buffer_alignment;
    uint32_t upload_buffer_texture_alignment;
    uint32_t upload_buffer_texture_row_alignment;
    uint32_t max_vertex_input_bindings;
    uint32_t wave_lane_count;
    uint64_t host_visible_vram_budget;
    CGPUDynamicStateFeatures dynamic_state_features;
    bool support_host_visible_vram : 1;
    bool multidraw_indirect : 1;
    bool support_geom_shader : 1;
    bool support_tessellation : 1;
    bool is_uma : 1;
    bool is_virtual : 1;
    bool is_cpu : 1;
    bool support_tiled_buffer : 1;
    bool support_tiled_texture : 1;
    bool support_tiled_volume : 1;
    // RDNA2 
    bool support_shading_rate : 1;
    bool support_shading_rate_mask : 1;
    bool support_shading_rate_sv : 1;
    CGPUFormatSupport format_supports[CGPU_FORMAT_COUNT];
    CGPUVendorPreset vendor_preset;
} CGPUAdapterDetail;

// Objects (Heap Safety)
typedef struct CGPUInstance {
    const CGPUProcTable* proc_table;
    const CGPUSurfacesProcTable* surfaces_table;
    // Some Cached Data
    struct CGPURuntimeTable* runtime_table;
    ECGPUBackend backend;
    ECGPUNvAPI_Status nvapi_status;
    ECGPUAGSReturnCode ags_status;
    bool enable_set_name;
} CGPUInstance;

typedef struct CGPUAdapter {
    CGPUInstanceId instance;
    const CGPUProcTable* proc_table_cache;
} CGPUAdapter;

typedef struct CGPUDevice {
    const CGPUAdapterId adapter;
    const CGPUProcTable* proc_table_cache;
#ifdef __cplusplus
    CGPUDevice()
        : adapter(CGPU_NULLPTR)
    {
    }
#endif
    uint64_t next_texture_id;
    bool is_lost SKR_IF_CPP(= false);
} CGPUDevice;

typedef struct CGPUQueue {
    CGPUDeviceId device;
    ECGPUQueueType type;
    CGPUQueueIndex index;
} CGPUQueue;

typedef struct CGPUFence {
    CGPUDeviceId device;
} CGPUFence; // Empty struct so we dont need to def it

typedef struct CGPUSemaphore {
    CGPUDeviceId device;
} CGPUSemaphore; // Empty struct so we dont need to def it

typedef struct CGPUCommandPool {
    CGPUQueueId queue;
} CGPUCommandPool;

typedef struct CGPUCommandBuffer {
    CGPUDeviceId device;
    CGPUCommandPoolId pool;
    ECGPUPipelineType current_dispatch;
#ifdef __cplusplus
    inline void begin() const { cgpu_cmd_begin(this); }
    inline void end() const { cgpu_cmd_end(this); }
#endif
} CGPUCommandBuffer;

typedef struct CGPUQueryPool {
    CGPUDeviceId device;
    uint32_t count;
} CGPUQueryPool;

// Notice that we must keep this header same with CGPUCommandBuffer
// because Vulkan & D3D12 Backend simply use command buffer handle as encoder handle
typedef struct CGPUComputePassEncoder {
    CGPUDeviceId device;
} CGPUComputePassEncoder;

typedef struct CGPURenderPassEncoder {
    CGPUDeviceId device;
} CGPURenderPassEncoder;

// Shaders
typedef struct CGPUShaderResource {
    const char8_t* name;
    uint64_t name_hash;
    ECGPUResourceType type;
    ECGPUTextureDimension dim;
    uint32_t set;
    uint32_t binding;
    uint32_t size;
    uint32_t offset;
    CGPUShaderStages stages;
} CGPUShaderResource;

typedef struct CGPUVertexInput {
    const char8_t* name;
    const char8_t* semantics;
    ECGPUFormat format;
} CGPUVertexInput;

typedef struct CGPUShaderReflection {
    const char8_t* entry_name;
    ECGPUShaderStage stage;
    CGPUVertexInput* vertex_inputs;
    CGPUShaderResource* shader_resources;
    uint32_t vertex_inputs_count;
    uint32_t shader_resources_count;
    uint32_t thread_group_sizes[3];
} CGPUShaderReflection;

typedef struct CGPUShaderLibrary {
    CGPUDeviceId device;
    char8_t* name;
    CGPUShaderReflection* entry_reflections;
    uint32_t entrys_count;
} CGPUShaderLibrary;

typedef struct CGPUPipelineReflection {
    CGPUShaderReflection* stages[CGPU_SHADER_STAGE_COUNT];
    // descriptor sets / root tables
    CGPUShaderResource* shader_resources;
    uint32_t shader_resources_count;
} CGPUPipelineReflection;

typedef struct CGPUDescriptorData {
    // Update Via Shader Reflection.
    const char8_t* name;
    // Update Via Binding Slot.
    uint32_t binding;
    ECGPUResourceType binding_type;
    union
    {
        struct
        {
            /// Offset to bind the buffer descriptor
            const uint64_t* offsets;
            const uint64_t* sizes;
        } buffers_params;
        // Descriptor set buffer extraction options
        // TODO: Support descriptor buffer extraction
        //struct
        //{
        //    struct CGPUShaderEntryDescriptor* shader;
        //    uint32_t buffer_index;
        //    ECGPUShaderStage shader_stage;
        //} extraction_params;
        struct
        {
            uint32_t uav_mip_slice;
            bool blend_mip_chain;
        } uav_params;
        bool enable_stencil_resource;
    };
    union
    {
        const void** ptrs;
        /// Array of texture descriptors (srv and uav textures)
        CGPUTextureViewId* textures;
        /// Array of sampler descriptors
        CGPUSamplerId* samplers;
        /// Array of buffer descriptors (srv, uav and cbv buffers)
        CGPUBufferId* buffers;
        /// Array of pipeline descriptors
        CGPURenderPipelineId* render_pipelines;
        /// Array of pipeline descriptors
        CGPUComputePipelineId* compute_pipelines;
        /// DescriptorSet buffer extraction
        CGPUDescriptorSetId* descriptor_sets;
        /// Custom binding (raytracing acceleration structure ...)
        // CGPUAccelerationStructureId* acceleration_structures;
    };
    uint32_t count;
} CGPUDescriptorData;

typedef union CGPUClearValue
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
} CGPUClearValue;

static const CGPUClearValue fastclear_0000 = {
    { 0.f, 0.f, 0.f, 0.f }
};
static const CGPUClearValue fastclear_0001 = {
    { 0.f, 0.f, 0.f, 1.f }
};
static const CGPUClearValue fastclear_1110 = {
    { 1.f, 1.f, 1.f, 1.f }
};
static const CGPUClearValue fastclear_1111 = {
    { 1.f, 1.f, 1.f, 1.f }
};

typedef struct CGPUSwapChain {
    CGPUDeviceId device;
    const CGPUTextureId* back_buffers;
    uint32_t buffer_count;
} CGPUSwapChain;

// Descriptors (on Stack)
#pragma region DESCRIPTORS

#define CGPU_CHAINED_DESCRIPTOR_HEADER ECGPUBackend backend;

typedef struct CGPUChainedDescriptor {
    CGPU_CHAINED_DESCRIPTOR_HEADER
} CGPUChainedDescriptor;

// Device & Pipeline
typedef struct CGPUInstanceDescriptor {
    const CGPUChainedDescriptor* chained;
    ECGPUBackend backend;
    bool enable_debug_layer;
    bool enable_gpu_based_validation;
    bool enable_set_name;
} CGPUInstanceDescriptor;

typedef struct CGPUQueueGroupDescriptor {
    ECGPUQueueType queue_type;
    uint32_t queue_count;
} CGPUQueueGroupDescriptor;

typedef struct CGPUQueueSubmitDescriptor {
    CGPUCommandBufferId* cmds;
    CGPUFenceId signal_fence;
    CGPUSemaphoreId* wait_semaphores;
    CGPUSemaphoreId* signal_semaphores;
    uint32_t cmds_count;
    uint32_t wait_semaphore_count;
    uint32_t signal_semaphore_count;
} CGPUQueueSubmitDescriptor;

typedef struct CGPUQueuePresentDescriptor {
    CGPUSwapChainId swapchain;
    const CGPUSemaphoreId* wait_semaphores;
    uint32_t wait_semaphore_count;
    uint8_t index;
} CGPUQueuePresentDescriptor;

typedef struct CGPUQueryPoolDescriptor {
    ECGPUQueryType type;
    uint32_t query_count;
} CGPUQueryPoolDescriptor;

typedef struct CGPUQueryDescriptor {
    uint32_t index;
    ECGPUShaderStage stage;
} CGPUQueryDescriptor;

typedef struct CGPUAcquireNextDescriptor {
    CGPUSemaphoreId signal_semaphore;
    CGPUFenceId fence;
} CGPUAcquireNextDescriptor;

typedef struct CGPUTextureSubresource {
    CGPUTextureViewAspects aspects;
    uint32_t mip_level;
    uint32_t base_array_layer;
    uint32_t layer_count;
} CGPUTextureSubresource;

typedef struct CGPUCoordinate {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} CGPUCoordinate;

typedef struct CGPUCoordinateRegion {
    CGPUCoordinate start;
    CGPUCoordinate end;
} CGPUCoordinateRegion;

typedef struct CGPUTextureCoordinateRegion {
    CGPUCoordinate start;
    CGPUCoordinate end;
    uint32_t mip_level;
    uint32_t layer;
} CGPUTextureCoordinateRegion;

typedef struct CGPUTiledTextureRegions {
    CGPUTextureId texture;
    struct CGPUTextureCoordinateRegion* regions;
    uint32_t region_count;
} CGPUTiledTextureRegions;

typedef struct CGPUTiledTexturePackedMip {
    CGPUTextureId texture;
    uint32_t layer;
} CGPUTiledTexturePackedMip;

typedef struct CGPUTiledTexturePackedMips {
    struct CGPUTiledTexturePackedMip* packed_mips;
    uint32_t packed_mip_count;
} CGPUTiledTexturePackedMips;

typedef struct CGPUBufferToBufferTransfer {
    CGPUBufferId dst;
    uint64_t dst_offset;
    CGPUBufferId src;
    uint64_t src_offset;
    uint64_t size;
} CGPUBufferToBufferTransfer;

typedef struct CGPUBufferToTilesTransfer {
    CGPUBufferId src;
    uint64_t src_offset;
    CGPUTextureId dst;
    CGPUTextureCoordinateRegion region;
} CGPUBufferToTilesTransfer;

typedef struct CGPUTextureToTextureTransfer {
    CGPUTextureId src;
    CGPUTextureSubresource src_subresource;
    CGPUTextureId dst;
    CGPUTextureSubresource dst_subresource;
} CGPUTextureToTextureTransfer;

typedef struct CGPUBufferToTextureTransfer {
    CGPUTextureId dst;
    CGPUTextureSubresource dst_subresource;
    CGPUBufferId src;
    uint64_t src_offset;
} CGPUBufferToTextureTransfer;

typedef struct CGPUBufferBarrier {
    CGPUBufferId buffer;
    ECGPUResourceState src_state;
    ECGPUResourceState dst_state;
    uint8_t queue_acquire;
    uint8_t queue_release;
    ECGPUQueueType queue_type;
    uint8_t d3d12_begin_only;
    uint8_t d3d12_end_only;
} CGPUBufferBarrier;

typedef struct CGPUTextureBarrier {
    CGPUTextureId texture;
    ECGPUResourceState src_state;
    ECGPUResourceState dst_state;
    uint8_t queue_acquire;
    uint8_t queue_release;
    ECGPUQueueType queue_type;
    /// Specifiy whether following barrier targets particular subresource
    uint8_t subresource_barrier;
    /// Following values are ignored if subresource_barrier is false
    uint8_t mip_level;
    uint16_t array_layer;
    uint8_t d3d12_begin_only;
    uint8_t d3d12_end_only;
} CGPUTextureBarrier;

typedef struct CGPUResourceBarrierDescriptor {
    const CGPUBufferBarrier* buffer_barriers;
    uint32_t buffer_barriers_count;
    const CGPUTextureBarrier* texture_barriers;
    uint32_t texture_barriers_count;
} CGPUResourceBarrierDescriptor;

typedef struct CGPUDeviceDescriptor {
    bool disable_pipeline_cache;
    CGPUQueueGroupDescriptor* queue_groups;
    uint32_t queue_group_count;
} CGPUDeviceDescriptor;

typedef struct CGPUCommandPoolDescriptor {
    const char8_t* name;
} CGPUCommandPoolDescriptor;

typedef struct CGPUCommandBufferDescriptor {
#if defined(PROSPERO) || defined(ORBIS)
    uint32_t max_size; // AGC CommandBuffer Size
#endif
    bool is_secondary : 1;
} CGPUCommandBufferDescriptor;

typedef struct CGPUShaderEntryDescriptor {
    CGPUShaderLibraryId library;
    const char8_t* entry;
    ECGPUShaderStage stage;
    // ++ constant_specialization
    const CGPUConstantSpecialization* constants;
    uint32_t num_constants;
    // -- constant_specialization
} CGPUShaderEntryDescriptor;

typedef struct CGPUSwapChainDescriptor {
    /// Present Queues
    CGPUQueueId* present_queues;
    /// Present Queues Count
    uint32_t present_queues_count;
    /// Surface to Create SwapChain on
    CGPUSurfaceId surface;
    /// Number of backbuffers in this swapchain
    uint32_t image_count;
    /// Width of the swapchain
    uint32_t width;
    /// Height of the swapchain
    uint32_t height;
    /// Set whether swap chain will be presented using vsync
    bool enable_vsync;
    /// We can toggle to using FLIP model if app desires
    bool use_flip_swap_effect;
    /// Clear Value.
    float clear_value[4];
    /// format
    ECGPUFormat format;
} CGPUSwapChainDescriptor;

typedef struct CGPUComputePassDescriptor {
    const char8_t* name;
} CGPUComputePassDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct CGPUColorAttachment {
    CGPUTextureViewId view;
    CGPUTextureViewId resolve_view;
    ECGPULoadAction load_action;
    ECGPUStoreAction store_action;
    CGPUClearValue clear_color;
} CGPUColorAttachment;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct CGPUDepthStencilAttachment {
    CGPUTextureViewId view;
    ECGPULoadAction depth_load_action;
    ECGPUStoreAction depth_store_action;
    float clear_depth;
    uint8_t write_depth;
    ECGPULoadAction stencil_load_action;
    ECGPUStoreAction stencil_store_action;
    uint32_t clear_stencil;
    uint8_t write_stencil;
} CGPUDepthStencilAttachment;

typedef struct CGPURenderPassDescriptor {
    const char8_t* name;
    // TODO: support multi-target & remove this
    ECGPUSampleCount sample_count;
    const CGPUColorAttachment* color_attachments;
    const CGPUDepthStencilAttachment* depth_stencil;
    uint32_t render_target_count;
} CGPURenderPassDescriptor;

typedef struct CGPURootSignaturePoolDescriptor {
    const char8_t* name;
} CGPURootSignaturePoolDescriptor;

typedef struct CGPURootSignatureDescriptor {
    struct CGPUShaderEntryDescriptor* shaders;
    uint32_t shader_count;
    const CGPUSamplerId* static_samplers;
    const char8_t* const* static_sampler_names;
    uint32_t static_sampler_count;
    const char8_t* const* push_constant_names;
    uint32_t push_constant_count;
    CGPURootSignaturePoolId pool;
} CGPURootSignatureDescriptor;

typedef struct CGPUCompiledShaderDescriptor {
    CGPUShaderEntryDescriptor entry;
    void* shader_code;
    uint64_t code_size;
} CGPUCompiledShaderDescriptor;

typedef struct CGPUDescriptorSetDescriptor {
    CGPURootSignatureId root_signature;
    uint32_t set_index;
} CGPUDescriptorSetDescriptor;

typedef struct CGPUComputePipelineDescriptor {
    CGPURootSignatureId root_signature;
    CGPUShaderEntryDescriptor* compute_shader;
} CGPUComputePipelineDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct CGPUBlendStateDescriptor {
    /// Source blend factor per render target.
    ECGPUBlendConstant src_factors[CGPU_MAX_MRT_COUNT];
    /// Destination blend factor per render target.
    ECGPUBlendConstant dst_factors[CGPU_MAX_MRT_COUNT];
    /// Source alpha blend factor per render target.
    ECGPUBlendConstant src_alpha_factors[CGPU_MAX_MRT_COUNT];
    /// Destination alpha blend factor per render target.
    ECGPUBlendConstant dst_alpha_factors[CGPU_MAX_MRT_COUNT];
    /// Blend mode per render target.
    ECGPUBlendMode blend_modes[CGPU_MAX_MRT_COUNT];
    /// Alpha blend mode per render target.
    ECGPUBlendMode blend_alpha_modes[CGPU_MAX_MRT_COUNT];
    /// Write mask per render target.
    int32_t masks[CGPU_MAX_MRT_COUNT];
    /// Set whether alpha to coverage should be enabled.
    bool alpha_to_coverage;
    /// Set whether each render target has an unique blend function. When false the blend function in slot 0 will be used for all render targets.
    bool independent_blend;
} CGPUBlendStateDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct CGPUDepthStateDesc {
    bool depth_test;
    bool depth_write;
    ECGPUCompareMode depth_func;
    bool stencil_test;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    ECGPUCompareMode stencil_front_func;
    ECGPUStencilOp stencil_front_fail;
    ECGPUStencilOp depth_front_fail;
    ECGPUStencilOp stencil_front_pass;
    ECGPUCompareMode stencil_back_func;
    ECGPUStencilOp stencil_back_fail;
    ECGPUStencilOp depth_back_fail;
    ECGPUStencilOp stencil_back_pass;
} CGPUDepthStateDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct CGPURasterizerStateDescriptor {
    ECGPUCullMode cull_mode;
    int32_t depth_bias;
    float slope_scaled_depth_bias;
    ECGPUFillMode fill_mode;
    ECGPUFrontFace front_face;
    bool enable_multi_sample;
    bool enable_scissor;
    bool enable_depth_clamp;
} CGPURasterizerStateDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct CGPUVertexAttribute {
    // TODO: handle this in a better way
    char8_t semantic_name[64];
    uint32_t array_size;
    ECGPUFormat format;
    uint32_t binding;
    uint32_t offset;
    uint32_t elem_stride;
    ECGPUVertexInputRate rate;
} CGPUVertexAttribute;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct CGPUVertexLayout {
    uint32_t attribute_count;
    CGPUVertexAttribute attributes[CGPU_MAX_VERTEX_ATTRIBS];
} CGPUVertexLayout;

typedef struct CGPURenderPipelineDescriptor {
    CGPURootSignatureId root_signature;
    const CGPUShaderEntryDescriptor* vertex_shader;
    const CGPUShaderEntryDescriptor* tesc_shader;
    const CGPUShaderEntryDescriptor* tese_shader;
    const CGPUShaderEntryDescriptor* geom_shader;
    const CGPUShaderEntryDescriptor* fragment_shader;
    const CGPUVertexLayout* vertex_layout;
    const CGPUBlendStateDescriptor* blend_state;
    const CGPUDepthStateDescriptor* depth_state;
    const CGPURasterizerStateDescriptor* rasterizer_state;

    // caution: if any of these platten parameters have been changed, the hasher in cgpux.hpp must be updated

    const ECGPUFormat* color_formats;
    uint32_t render_target_count;
    ECGPUSampleCount sample_count;
    uint32_t sample_quality;
    ECGPUSlotMask color_resolve_disable_mask;
    ECGPUFormat depth_stencil_format;
    ECGPUPrimitiveTopology prim_topology;
    bool enable_indirect_command;
} CGPURenderPipelineDescriptor;

typedef struct CGPUMemoryPoolDescriptor {
    ECGPUMemoryPoolType type;
    ECGPUMemoryUsage memory_usage;
    uint64_t block_size;
    uint32_t min_block_count;
    uint32_t max_block_count;
    uint64_t min_alloc_alignment;
} CGPUMemoryPoolDescriptor;

typedef struct CGPUMemoryPool {
    CGPUDeviceId device;
    ECGPUMemoryPoolType type;
} CGPUMemoryPool;

typedef struct CGPUParameterTable {
    // This should be stored here because shader could be destoryed after RS creation
    CGPUShaderResource* resources;
    uint32_t resources_count;
    uint32_t set_index;
} CGPUParameterTable;

typedef struct CGPURootSignaturePool {
    CGPUDeviceId device;
    ECGPUPipelineType pipeline_type;
} CGPURootSignaturePool; 

typedef struct CGPURootSignature {
    CGPUDeviceId device;
    CGPUParameterTable* tables;
    uint32_t table_count;
    CGPUShaderResource* push_constants;
    uint32_t push_constant_count;
    CGPUShaderResource* static_samplers;
    uint32_t static_sampler_count;
    ECGPUPipelineType pipeline_type;
    CGPURootSignaturePoolId pool;
    CGPURootSignatureId pool_sig;
} CGPURootSignature;

typedef struct CGPUDescriptorSet {
    CGPURootSignatureId root_signature;
    uint32_t index;
} CGPUDescriptorSet;

typedef struct CGPUComputePipeline {
    CGPUDeviceId device;
    CGPURootSignatureId root_signature;
} CGPUComputePipeline;

typedef struct CGPURenderPipeline {
    CGPUDeviceId device;
    CGPURootSignatureId root_signature;
} CGPURenderPipeline;

typedef struct CGPUCompiledShader {
    CGPUDeviceId device;
    CGPURootSignatureId root_signature;
} CGPUCompiledShader;

typedef struct CGPULinkedShader {
    CGPUDeviceId device;
    CGPURootSignatureId root_signature;
} CGPULinkedShader;

typedef struct CGPUStateBuffer {
    CGPUDeviceId device;
    CGPUCommandBufferId cmd;
} CGPUStateBuffer;

typedef struct CGPURasterStateEncoder {
    CGPUDeviceId device;
} CGPURasterStateEncoder;

typedef struct CGPUShaderStateEncoder {
    CGPUDeviceId device;
} CGPUShaderStateEncoder;

typedef struct CGPUUserStateEncoder {
    CGPUDeviceId device;
} CGPUUserStateEncoder;

typedef struct CGPUBinder {
    CGPUDeviceId device;
    CGPUCommandBufferId cmd;
} CGPUBinder;

// Resources
typedef struct CGPUShaderLibraryDescriptor {
    const char8_t* name;
    const uint32_t* code;
    uint32_t code_size;
    ECGPUShaderStage stage;
    bool reflection_only;
} CGPUShaderLibraryDescriptor;

typedef struct CGPUBufferDescriptor {
    /// Size of the buffer (in bytes)
    uint64_t size;
    /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    struct Buffer* count_buffer;
    /// Debug name used in gpu profile
    const char8_t* name;
    /// Flags specifying the suitable usage of this buffer (Uniform buffer, Vertex Buffer, Index Buffer,...)
    CGPUResourceTypes descriptors;
    /// Memory usage
    /// Decides which memory heap buffer will use (default, upload, readback)
    ECGPUMemoryUsage memory_usage;
    /// Image format
    ECGPUFormat format;
    /// Creation flags
    CGPUBufferCreationFlags flags;
    /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t first_element;
    /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t elemet_count;
    /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t element_stride;
    /// Owner queue of the resource at creation
    CGPUQueueId owner_queue;
    /// What state will the buffer get created in
    ECGPUResourceState start_state;
    /// Preferred actual location
    /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
    bool prefer_on_device;
    /// Preferred actual location
    /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
    bool prefer_on_host;
} CGPUBufferDescriptor;

typedef struct CGPUBufferInfo {
    uint64_t size;
    void* cpu_mapped_address;
    uint32_t descriptors;
    uint32_t memory_usage;
} CGPUBufferInfo;

typedef struct CGPUBuffer {
    CGPUDeviceId device;
    const struct CGPUBufferInfo* info;
} CGPUBuffer;

typedef struct CGPUTextureDescriptor {
    /// Debug name used in gpu profile
    const char8_t* name;
    const void* native_handle;
    /// Texture creation flags (decides memory allocation strategy, sharing access,...)
    CGPUTextureCreationFlags flags;
    /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
    CGPUClearValue clear_value;
    /// Width
    uint64_t width;
    /// Height
    uint64_t height;
    /// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
    uint64_t depth;
    /// Texture array size (Should be 1 if texture is not a texture array or cubemap)
    uint32_t array_size;
    ///  image format
    ECGPUFormat format;
    /// Number of mip levels
    uint32_t mip_levels;
    /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support CGPU_SAMPLE_COUNT_1)
    ECGPUSampleCount sample_count;
    /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
    uint32_t sample_quality;
    /// Owner queue of the resource at creation
    CGPUQueueId owner_queue;
    /// What state will the texture get created in
    ECGPUResourceState start_state;
    /// Descriptor creation
    CGPUResourceTypes descriptors;
    /// Memory Aliasing
    uint32_t is_restrict_dedicated;
} CGPUTextureDescriptor;

typedef struct CGPUExportTextureDescriptor {
    CGPUTextureId texture;
} CGPUExportTextureDescriptor;

typedef struct CGPUImportTextureDescriptor {
    ECGPUBackend backend;
    uint64_t shared_handle;
    uint64_t width;
    uint64_t height;
    uint64_t depth;
    uint64_t size_in_bytes;
    ECGPUFormat format;
    uint32_t mip_levels;
} CGPUImportTextureDescriptor;

typedef struct CGPUTextureViewDescriptor {
    /// Debug name used in gpu profile
    const char8_t* name;
    CGPUTextureId texture;
    ECGPUFormat format;
    CGPUTexutreViewUsages usages : 8;
    CGPUTextureViewAspects aspects : 8;
    ECGPUTextureDimension dims : 8;
    uint32_t base_array_layer : 8;
    uint32_t array_layer_count : 8;
    uint32_t base_mip_level : 8;
    uint32_t mip_level_count : 8;
} CGPUTextureViewDescriptor;

typedef struct CGPUTextureAliasingBindDescriptor {
    CGPUTextureId aliased;
    CGPUTextureId aliasing;
} CGPUTextureAliasingBindDescriptor;

typedef struct CGPUTextureInfo {
    uint64_t width;
    uint64_t height;
    uint64_t depth;
    uint32_t mip_levels;
    uint32_t array_size_minus_one;
    uint64_t size_in_bytes;
    ECGPUFormat format;
    ECGPUSampleCount sample_count;
    uint64_t unique_id;
    uint32_t aspect_mask;
    uint32_t node_index;
    uint8_t owns_image;
    uint8_t is_cube;
    uint8_t is_allocation_dedicated;
    uint8_t is_restrict_dedicated;
    uint8_t is_aliasing;
    uint8_t is_tiled;
    uint8_t is_imported;
    uint8_t can_alias;
    uint8_t can_export;
} CGPUTextureInfo;

typedef struct CGPUTiledSubresourceInfo {
    uint16_t layer;
    uint16_t mip_level;
    uint32_t width_in_tiles;
    uint16_t height_in_tiles;
    uint16_t depth_in_tiles;
} CGPUTiledSubresourceInfo;

typedef struct CGPUTiledTextureInfo {
    uint64_t tile_size;
    uint64_t total_tiles_count;
    volatile uint64_t alive_tiles_count;

    uint32_t tile_width_in_texels;
    uint32_t tile_height_in_texels;
    uint32_t tile_depth_in_texels;
    const CGPUTiledSubresourceInfo* subresources;

    uint32_t packed_mip_start;
    uint32_t packed_mip_count;
    volatile uint64_t alive_pack_count;

    bool pack_unaligned;
} CGPUTiledTextureInfo;

typedef struct CGPUTexture {
    CGPUDeviceId device;
    const CGPUTextureInfo* info;
    const CGPUTiledTextureInfo* tiled_resource;
} CGPUTexture;

typedef struct CGPUTextureView {
    CGPUDeviceId device;
    CGPUTextureViewDescriptor info;
} CGPUTextureView;

typedef struct CGPUSamplerDescriptor {
    ECGPUFilterType min_filter;
    ECGPUFilterType mag_filter;
    ECGPUMipMapMode mipmap_mode;
    ECGPUAddressMode address_u;
    ECGPUAddressMode address_v;
    ECGPUAddressMode address_w;
    float mip_lod_bias;
    float max_anisotropy;
    ECGPUCompareMode compare_func;
} CGPUSamplerDescriptor;

typedef struct CGPUSampler {
    CGPUDeviceId device;
} CGPUSampler;

#pragma endregion DESCRIPTORS

#define CGPU_SINGLE_GPU_NODE_COUNT 1
#define CGPU_SINGLE_GPU_NODE_MASK 1
#define CGPU_SINGLE_GPU_NODE_INDEX 0

#ifdef __cplusplus
CGPU_EXTERN_C_END
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif