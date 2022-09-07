#pragma once
#include "cgpu/api.h"
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#ifdef __cplusplus
extern "C" {
#endif

struct DMA_Allocator;
struct DMA_Allocation;
struct D3D12Util_DescriptorHandle;
struct D3D12Util_DescriptorHeap;

RUNTIME_API const CGPUProcTable* CGPU_D3D12ProcTable();
RUNTIME_API const CGPUSurfacesProcTable* CGPU_D3D12SurfacesProcTable();

// Instance APIs
RUNTIME_API CGPUInstanceId cgpu_create_instance_d3d12(CGPUInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_d3d12(CGPUInstanceId instance, struct CGPUInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_d3d12(CGPUInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters_d3d12(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API const struct CGPUAdapterDetail* cgpu_query_adapter_detail_d3d12(const CGPUAdapterId adapter);
RUNTIME_API void cgpu_query_video_memory_info_d3d12(const CGPUAdapterId adapter, uint64_t* total, uint64_t* used_bytes);
RUNTIME_API uint32_t cgpu_query_queue_count_d3d12(const CGPUAdapterId adapter, const ECGPUQueueType type);

// Device APIs
RUNTIME_API CGPUDeviceId cgpu_create_device_d3d12(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_d3d12(CGPUDeviceId device);

// API Object APIs
RUNTIME_API CGPUFenceId cgpu_create_fence_d3d12(CGPUDeviceId device);
RUNTIME_API void cgpu_wait_fences_d3d12(const CGPUFenceId* fences, uint32_t fence_count);
ECGPUFenceStatus cgpu_query_fence_status_d3d12(CGPUFenceId fence);
RUNTIME_API void cgpu_free_fence_d3d12(CGPUFenceId fence);
RUNTIME_API CGPUSemaphoreId cgpu_create_semaphore_d3d12(CGPUDeviceId device);
RUNTIME_API void cgpu_free_semaphore_d3d12(CGPUSemaphoreId semaphore);
RUNTIME_API CGPURootSignaturePoolId cgpu_create_root_signature_pool_d3d12(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature_pool_d3d12(CGPURootSignaturePoolId pool);
RUNTIME_API CGPURootSignatureId cgpu_create_root_signature_d3d12(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature_d3d12(CGPURootSignatureId signature);
RUNTIME_API CGPUDescriptorSetId cgpu_create_descriptor_set_d3d12(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc);
RUNTIME_API void cgpu_update_descriptor_set_d3d12(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count);
RUNTIME_API void cgpu_free_descriptor_set_d3d12(CGPUDescriptorSetId set);
RUNTIME_API CGPUComputePipelineId cgpu_create_compute_pipeline_d3d12(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc);
RUNTIME_API void cgpu_free_compute_pipeline_d3d12(CGPUComputePipelineId pipeline);
RUNTIME_API CGPURenderPipelineId cgpu_create_render_pipeline_d3d12(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc);
RUNTIME_API void cgpu_free_render_pipeline_d3d12(CGPURenderPipelineId pipeline);
RUNTIME_API CGPUQueryPoolId cgpu_create_query_pool_d3d12(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc);
RUNTIME_API void cgpu_free_query_pool_d3d12(CGPUQueryPoolId pool);

// Queue APIs
RUNTIME_API CGPUQueueId cgpu_get_queue_d3d12(CGPUDeviceId device, ECGPUQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue_d3d12(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle_d3d12(CGPUQueueId queue);
RUNTIME_API void cgpu_queue_present_d3d12(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc);
RUNTIME_API float cgpu_queue_get_timestamp_period_ns_d3d12(CGPUQueueId queue);
RUNTIME_API void cgpu_free_queue_d3d12(CGPUQueueId queue);

// Command APIs
RUNTIME_API CGPUCommandPoolId cgpu_create_command_pool_d3d12(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc);
RUNTIME_API CGPUCommandBufferId cgpu_create_command_buffer_d3d12(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_reset_command_pool_d3d12(CGPUCommandPoolId pool);
RUNTIME_API void cgpu_free_command_buffer_d3d12(CGPUCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool_d3d12(CGPUCommandPoolId pool);

// Event & Markers
RUNTIME_API void cgpu_cmd_begin_event_d3d12(CGPUCommandBufferId cmd, const CGPUEventInfo* event);
RUNTIME_API void cgpu_cmd_set_marker_d3d12(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker);
RUNTIME_API void cgpu_cmd_end_event_d3d12(CGPUCommandBufferId cmd);

// Shader APIs
RUNTIME_API CGPUShaderLibraryId cgpu_create_shader_library_d3d12(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_d3d12(CGPUShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGPUBufferId cgpu_create_buffer_d3d12(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer_d3d12(CGPUBufferId buffer, const struct CGPUBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer_d3d12(CGPUBufferId buffer);
RUNTIME_API void cgpu_free_buffer_d3d12(CGPUBufferId buffer);

// Sampler APIs
RUNTIME_API CGPUSamplerId cgpu_create_sampler_d3d12(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc);
RUNTIME_API void cgpu_free_sampler_d3d12(CGPUSamplerId sampler);

// Texture/TextureView APIs
RUNTIME_API CGPUTextureId cgpu_create_texture_d3d12(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc);
RUNTIME_API void cgpu_free_texture_d3d12(CGPUTextureId texture);
RUNTIME_API CGPUTextureViewId cgpu_create_texture_view_d3d12(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc);
RUNTIME_API void cgpu_free_texture_view_d3d12(CGPUTextureViewId render_target);
RUNTIME_API bool cgpu_try_bind_aliasing_texture_d3d12(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc);

// Swapchain APIs
RUNTIME_API CGPUSwapChainId cgpu_create_swapchain_d3d12(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc);
RUNTIME_API uint32_t cgpu_acquire_next_image_d3d12(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_d3d12(CGPUSwapChainId swapchain);

// CMDs
RUNTIME_API void cgpu_cmd_begin_d3d12(CGPUCommandBufferId cmd);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_buffer_d3d12(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc);
RUNTIME_API void cgpu_cmd_transfer_texture_to_texture_d3d12(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_texture_d3d12(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc);
RUNTIME_API void cgpu_cmd_resource_barrier_d3d12(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc);
RUNTIME_API void cgpu_cmd_begin_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
RUNTIME_API void cgpu_cmd_end_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc);
RUNTIME_API void cgpu_cmd_reset_query_pool_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId, uint32_t start_query, uint32_t query_count);
RUNTIME_API void cgpu_cmd_resolve_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count);
RUNTIME_API void cgpu_cmd_end_d3d12(CGPUCommandBufferId cmd);

// Compute CMDs
RUNTIME_API CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_d3d12(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc);
RUNTIME_API void cgpu_compute_encoder_bind_descriptor_set_d3d12(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set);
RUNTIME_API void cgpu_compute_encoder_push_constants_d3d12(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
RUNTIME_API void cgpu_compute_encoder_bind_pipeline_d3d12(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline);
RUNTIME_API void cgpu_compute_encoder_dispatch_d3d12(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
RUNTIME_API void cgpu_cmd_end_compute_pass_d3d12(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder);

// Render CMDs
RUNTIME_API CGPURenderPassEncoderId cgpu_cmd_begin_render_pass_d3d12(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc);
RUNTIME_API void cgpu_render_encoder_set_shading_rate_d3d12(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate);
RUNTIME_API void cgpu_render_encoder_bind_descriptor_set_d3d12(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set);
RUNTIME_API void cgpu_render_encoder_set_viewport_d3d12(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
RUNTIME_API void cgpu_render_encoder_set_scissor_d3d12(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
RUNTIME_API void cgpu_render_encoder_bind_pipeline_d3d12(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline);
RUNTIME_API void cgpu_render_encoder_bind_vertex_buffers_d3d12(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
RUNTIME_API void cgpu_render_encoder_bind_index_buffer_d3d12(CGPURenderPassEncoderId encoder, CGPUBufferId buffer, uint32_t index_stride, uint64_t offset);
RUNTIME_API void cgpu_render_encoder_push_constants_d3d12(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data);
RUNTIME_API void cgpu_render_encoder_draw_d3d12(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_instanced_d3d12(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance);
RUNTIME_API void cgpu_render_encoder_draw_indexed_d3d12(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
RUNTIME_API void cgpu_render_encoder_draw_indexed_instanced_d3d12(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
RUNTIME_API void cgpu_cmd_end_render_pass_d3d12(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder);

// dstorage
RUNTIME_API ECGPUDStorageAvailability cgpu_query_dstorage_availability_d3d12(CGPUDeviceId device);
RUNTIME_API CGPUDStorageQueueId cgpu_create_dstorage_queue_d3d12(CGPUDeviceId device, const CGPUDStorageQueueDescriptor* descriptor);
RUNTIME_API CGPUDStorageFileHandle cgpu_dstorage_open_file_d3d12(CGPUDStorageQueueId queue, const char* abs_path);
RUNTIME_API void cgpu_dstorage_query_file_info_d3d12(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file, CGPUDStorageFileInfo* info);
RUNTIME_API void cgpu_dstorage_enqueue_buffer_request_d3d12(CGPUDStorageQueueId queue, const CGPUDStorageBufferIODescriptor* desc);
RUNTIME_API void cgpu_dstorage_enqueue_texture_request_d3d12(CGPUDStorageQueueId queue, const CGPUDStorageTextureIODescriptor* desc);
RUNTIME_API void cgpu_dstorage_queue_submit_d3d12(CGPUDStorageQueueId queue, CGPUFenceId fence);
RUNTIME_API void cgpu_dstorage_close_file_d3d12(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file);
RUNTIME_API void cgpu_free_dstorage_queue_d3d12(CGPUDStorageQueueId queue);

#ifdef __cplusplus
} // end extern "C"
namespace D3D12MA
{
class Allocator;
class Allocation;
} // namespace D3D12MA
#endif

typedef struct CGPUInstance_D3D12 {
    CGPUInstance super;
#if defined(XBOX)
    IDXGIFactory2* pDXGIFactory;
#elif defined(_WIN32)
    struct IDXGIFactory6* pDXGIFactory;
#endif
    struct ID3D12Debug* pDXDebug;
    struct CGPUAdapter_D3D12* pAdapters;
    uint32_t mAdaptersCount;
#if defined(__cplusplus)

#endif
} CGPUInstance_D3D12;

typedef struct CGPUAdapter_D3D12 {
    CGPUAdapter super;
#if defined(XBOX)
    IDXGIAdapter* pDxActiveGPU;
#elif defined(_WIN32)
    struct IDXGIAdapter4* pDxActiveGPU;
#endif
    D3D_FEATURE_LEVEL mFeatureLevel;
    CGPUAdapterDetail adapter_detail;
    bool mEnhancedBarriersSupported : 1;
} CGPUAdapter_D3D12;

typedef struct CGPUDevice_D3D12 {
    CGPUDevice super;
    struct D3D12Util_DescriptorHeap** pCPUDescriptorHeaps;
    struct D3D12Util_DescriptorHeap** pCbvSrvUavHeaps;
    struct D3D12Util_DescriptorHeap** pSamplerHeaps;
    ID3D12Device* pDxDevice;
    struct ID3D12CommandQueue** const ppCommandQueues[CGPU_QUEUE_TYPE_COUNT]
#ifdef __cplusplus
    = {}
#endif
    ;
    const uint32_t pCommandQueueCounts[CGPU_QUEUE_TYPE_COUNT]
#ifdef __cplusplus
    = {}
#endif
    ;
#ifdef __cplusplus
    class D3D12MA::Allocator* pResourceAllocator;
#else
    struct DMA_Allocator* pResourceAllocator;
#endif
    struct D3D12Util_StaticSamplerMap* pStaticSamplersMap;
    ID3D12PipelineLibrary* pPipelineLibrary;
    void* pPSOCacheData;
} CGPUDevice_D3D12;

typedef struct CGPUFence_D3D12 {
    CGPUFence super;
    ID3D12Fence* pDxFence;
    HANDLE pDxWaitIdleFenceEvent;
    uint64_t mFenceValue;
    uint64_t mPadA;
} CGPUFence_D3D12;

typedef struct CGPUQueue_D3D12 {
    CGPUQueue super;
    struct ID3D12CommandQueue* pCommandQueue;
    struct CGPUFence_D3D12* pFence;
} CGPUQueue_D3D12;

typedef struct CGPUCommandPool_D3D12 {
    CGPUCommandPool super;
    struct ID3D12CommandAllocator* pDxCmdAlloc;
} CGPUCommandPool_D3D12;

typedef struct CGPUQueryPool_D3D12 {
    CGPUQueryPool super;
    struct ID3D12QueryHeap* pDxQueryHeap;
    D3D12_QUERY_TYPE mType;
} CGPUQueryPool_D3D12;

typedef struct CGPUCommandBuffer_D3D12 {
    CGPUCommandBuffer super;
#if defined(XBOX)
    DmaCmd mDma;
#endif
    ID3D12GraphicsCommandList* pDxCmdList;
    // Cached in beginCmd to avoid fetching them during rendering
    struct D3D12Util_DescriptorHeap* pBoundHeaps[2]; // pCbvSrvUavHeaps, pSamplerHeaps
    D3D12_GPU_DESCRIPTOR_HANDLE mBoundHeapStartHandles[2];
    // Command buffer state
    const ID3D12RootSignature* pBoundRootSignature;
    uint32_t mNodeIndex : 4;
    uint32_t mType : 3;
    CGPUCommandPool_D3D12* pCmdPool;
    D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS mSubResolveResource;
} CGPUCommandBuffer_D3D12;

typedef struct CGPUShaderLibrary_D3D12 {
    CGPUShaderLibrary super;
    struct IDxcBlobEncoding* pShaderBlob;
} CGPUShaderLibrary_D3D12;

typedef struct CGPURootSignature_D3D12 {
    CGPURootSignature super;
    ID3D12RootSignature* pDxRootSignature;
    D3D12_ROOT_PARAMETER1 mRootConstantParam;
    uint32_t mRootParamIndex;
} CGPURootSignature_D3D12;

typedef struct CGPUDescriptorSet_D3D12 {
    CGPUDescriptorSet super;
    /// Start handle to cbv srv uav descriptor table
    uint64_t mCbvSrvUavHandle;
    /// Stride of the cbv srv uav descriptor table (number of descriptors * descriptor size)
    uint32_t mCbvSrvUavStride;
    /// Start handle to sampler descriptor table
    uint64_t mSamplerHandle;
    /// Stride of the sampler descriptor table (number of descriptors * descriptor size)
    uint32_t mSamplerStride;
    // TODO: Support root descriptors
    // D3D12_GPU_VIRTUAL_ADDRESS* pRootAddresses;
} CGPUDescriptorSet_D3D12;

typedef struct CGPUComputePipeline_D3D12 {
    CGPUComputePipeline super;
    ID3D12PipelineState* pDxPipelineState;
    ID3D12RootSignature* pRootSignature;
} CGPUComputePipeline_D3D12;

typedef struct CGPURenderPipeline_D3D12 {
    CGPURenderPipeline super;
    ID3D12PipelineState* pDxPipelineState;
    ID3D12RootSignature* pRootSignature;
    D3D_PRIMITIVE_TOPOLOGY mDxPrimitiveTopology;
} CGPURenderPipeline_D3D12;

typedef struct CGPUBuffer_D3D12 {
    CGPUBuffer super;
    /// GPU Address - Cache to avoid calls to ID3D12Resource::GetGpuVirtualAddress
    D3D12_GPU_VIRTUAL_ADDRESS mDxGpuAddress;
    /// Descriptor handle of the CBV in a CPU visible descriptor heap (applicable to BUFFER_USAGE_UNIFORM)
    D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
    /// Offset from mDxDescriptors for srv descriptor handle
    uint64_t mDxSrvOffset : 8;
    /// Offset from mDxDescriptors for uav descriptor handle
    uint64_t mDxUavOffset : 8;
    /// Native handle of the underlying resource
    struct ID3D12Resource* pDxResource;
    /// Contains resource allocation info such as parent heap, offset in heap
#ifdef __cplusplus
    D3D12MA::Allocation* pDxAllocation;
#else
    struct DMA_Allocation* pDxAllocation;
#endif
} CGPUBuffer_D3D12;

typedef struct CGPUTexture_D3D12 {
    CGPUTexture super;
    ID3D12Resource* pDxResource;
#ifdef __cplusplus
    D3D12MA::Allocation* pDxAllocation;
    CGPUTexture_D3D12();
#else
    struct DMA_Allocation* pDxAllocation;
#endif
} CGPUTexture_D3D12;

typedef struct CGPUTextureView_D3D12 {
    CGPUTextureView super;
    D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
    /// Offset from mDxDescriptors for srv descriptor handle
    uint64_t mDxSrvOffset : 8;
    /// Offset from mDxDescriptors for uav descriptor handle
    uint64_t mDxUavOffset : 8;
    /// Offset from mDxDescriptors for rtv descriptor handle
    D3D12_CPU_DESCRIPTOR_HANDLE mDxRtvDsvDescriptorHandle;
} CGPUTextureView_D3D12;

typedef struct CGPUSampler_D3D12 {
    CGPUSampler super;
    /// Description for creating the Sampler descriptor for this sampler
    D3D12_SAMPLER_DESC mDxDesc;
    /// Descriptor handle of the Sampler in a CPU visible descriptor heap
    D3D12_CPU_DESCRIPTOR_HANDLE mDxHandle;
} CGPUSampler_D3D12;

typedef struct CGPUSwapChain_D3D12 {
    CGPUSwapChain super;
    struct IDXGISwapChain3* pDxSwapChain;
    uint32_t mDxSyncInterval : 3;
    uint32_t mFlags : 10;
    uint32_t mImageCount : 3;
    uint32_t mEnableVsync : 1;
} CGPUSwapChain_D3D12;

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE(p_var) \
        if (p_var)              \
        {                       \
            p_var->Release();   \
            p_var = NULL;       \
        }
#endif

static const D3D_FEATURE_LEVEL d3d_feature_levels[] = {
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0
};

static const D3D12_COMMAND_LIST_TYPE gDx12CmdTypeTranslator[CGPU_QUEUE_TYPE_COUNT] = {
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    D3D12_COMMAND_LIST_TYPE_COMPUTE,
    D3D12_COMMAND_LIST_TYPE_COPY
};

#define IID_ARGS IID_PPV_ARGS