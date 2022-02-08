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

RUNTIME_API const CGpuProcTable* CGPU_D3D12ProcTable();
RUNTIME_API const CGpuSurfacesProcTable* CGPU_D3D12SurfacesProcTable();

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance_d3d12(CGpuInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_d3d12(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_d3d12(CGpuInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters_d3d12(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API const struct CGpuAdapterDetail* cgpu_query_adapter_detail_d3d12(const CGpuAdapterId adapter);
RUNTIME_API uint32_t cgpu_query_queue_count_d3d12(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device_d3d12(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_d3d12(CGpuDeviceId device);

// API Object APIs
RUNTIME_API CGpuFenceId cgpu_create_fence_d3d12(CGpuDeviceId device);
RUNTIME_API void cgpu_wait_fences_d3d12(const CGpuFenceId* fences, uint32_t fence_count);
ECGpuFenceStatus cgpu_query_fence_status_d3d12(CGpuFenceId fence);
RUNTIME_API void cgpu_free_fence_d3d12(CGpuFenceId fence);
RUNTIME_API CGpuSemaphoreId cgpu_create_semaphore_d3d12(CGpuDeviceId device);
RUNTIME_API void cgpu_free_semaphore_d3d12(CGpuSemaphoreId semaphore);
RUNTIME_API CGpuRootSignatureId cgpu_create_root_signature_d3d12(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc);
RUNTIME_API void cgpu_free_root_signature_d3d12(CGpuRootSignatureId signature);
RUNTIME_API CGpuDescriptorSetId cgpu_create_descriptor_set_d3d12(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc);
RUNTIME_API void cgpu_update_descriptor_set_d3d12(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count);
RUNTIME_API void cgpu_free_descriptor_set_d3d12(CGpuDescriptorSetId set);
RUNTIME_API CGpuComputePipelineId cgpu_create_compute_pipeline_d3d12(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc);
RUNTIME_API void cgpu_free_compute_pipeline_d3d12(CGpuComputePipelineId pipeline);
RUNTIME_API CGpuRenderPipelineId cgpu_create_render_pipeline_d3d12(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc);
RUNTIME_API void cgpu_free_render_pipeline_d3d12(CGpuRenderPipelineId pipeline);

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue_d3d12(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue_d3d12(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle_d3d12(CGpuQueueId queue);
RUNTIME_API void cgpu_queue_present_d3d12(CGpuQueueId queue, const struct CGpuQueuePresentDescriptor* desc);
RUNTIME_API void cgpu_free_queue_d3d12(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool_d3d12(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc);
RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer_d3d12(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_reset_command_pool_d3d12(CGpuCommandPoolId pool);
RUNTIME_API void cgpu_free_command_buffer_d3d12(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool_d3d12(CGpuCommandPoolId pool);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library_d3d12(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_d3d12(CGpuShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGpuBufferId cgpu_create_buffer_d3d12(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer_d3d12(CGpuBufferId buffer, const struct CGpuBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer_d3d12(CGpuBufferId buffer);
RUNTIME_API void cgpu_free_buffer_d3d12(CGpuBufferId buffer);

// Sampler APIs
RUNTIME_API CGpuSamplerId cgpu_create_sampler_d3d12(CGpuDeviceId device, const struct CGpuSamplerDescriptor* desc);
RUNTIME_API void cgpu_free_sampler_d3d12(CGpuSamplerId sampler);

// Texture/TextureView APIs
RUNTIME_API CGpuTextureId cgpu_create_texture_d3d12(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc);
RUNTIME_API void cgpu_free_texture_d3d12(CGpuTextureId texture);
RUNTIME_API CGpuTextureViewId cgpu_create_texture_view_d3d12(CGpuDeviceId device, const struct CGpuTextureViewDescriptor* desc);
RUNTIME_API void cgpu_free_texture_view_d3d12(CGpuTextureViewId render_target);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain_d3d12(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
RUNTIME_API uint32_t cgpu_acquire_next_image_d3d12(CGpuSwapChainId swapchain, const struct CGpuAcquireNextDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_d3d12(CGpuSwapChainId swapchain);

// CMDs
RUNTIME_API void cgpu_cmd_begin_d3d12(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_buffer_d3d12(CGpuCommandBufferId cmd, const struct CGpuBufferToBufferTransfer* desc);
RUNTIME_API void cgpu_cmd_transfer_buffer_to_texture_d3d12(CGpuCommandBufferId cmd, const struct CGpuBufferToTextureTransfer* desc);
RUNTIME_API void cgpu_cmd_resource_barrier_d3d12(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc);
RUNTIME_API void cgpu_cmd_end_d3d12(CGpuCommandBufferId cmd);

// Compute CMDs
RUNTIME_API CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass_d3d12(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc);
RUNTIME_API void cgpu_compute_encoder_bind_descriptor_set_d3d12(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId set);
RUNTIME_API void cgpu_compute_encoder_bind_pipeline_d3d12(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline);
RUNTIME_API void cgpu_compute_encoder_dispatch_d3d12(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z);
RUNTIME_API void cgpu_cmd_end_compute_pass_d3d12(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder);

// Render CMDs
RUNTIME_API CGpuRenderPassEncoderId cgpu_cmd_begin_render_pass_d3d12(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc);
RUNTIME_API void cgpu_render_encoder_bind_descriptor_set_d3d12(CGpuRenderPassEncoderId encoder, CGpuDescriptorSetId set);
RUNTIME_API void cgpu_render_encoder_set_viewport_d3d12(CGpuRenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth);
RUNTIME_API void cgpu_render_encoder_set_scissor_d3d12(CGpuRenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
RUNTIME_API void cgpu_render_encoder_bind_pipeline_d3d12(CGpuRenderPassEncoderId encoder, CGpuRenderPipelineId pipeline);
RUNTIME_API void cgpu_render_encoder_bind_vertex_buffers_d3d12(CGpuRenderPassEncoderId encoder, uint32_t buffer_count,
    const CGpuBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
RUNTIME_API void cgpu_render_encoder_push_constants_d3d12(CGpuRenderPassEncoderId encoder, CGpuRootSignatureId rs, const char8_t* name, const void* data);
RUNTIME_API void cgpu_render_encoder_draw_d3d12(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex);
RUNTIME_API void cgpu_cmd_end_render_pass_d3d12(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder);

#ifdef __cplusplus
} // end extern "C"
namespace D3D12MA
{
class Allocator;
class Allocation;
} // namespace D3D12MA
#endif

typedef struct CGpuInstance_D3D12 {
    CGpuInstance super;
#if defined(XBOX)
    IDXGIFactory2* pDXGIFactory;
#elif defined(_WIN32)
    struct IDXGIFactory6* pDXGIFactory;
#endif
    struct ID3D12Debug* pDXDebug;
    struct CGpuAdapter_D3D12* pAdapters;
    uint32_t mAdaptersCount;
#if defined(__cplusplus)

#endif
} CGpuInstance_D3D12;

typedef struct CGpuAdapter_D3D12 {
    CGpuAdapter super;
#if defined(XBOX)
    IDXGIAdapter* pDxActiveGPU;
#elif defined(_WIN32)
    struct IDXGIAdapter4* pDxActiveGPU;
#endif
    D3D_FEATURE_LEVEL mFeatureLevel;
    CGpuAdapterDetail adapter_detail;
    bool mEnhancedBarriersSupported : 1;
} CGpuAdapter_D3D12;

typedef struct CGpuDevice_D3D12 {
    CGpuDevice super;
    struct D3D12Util_DescriptorHeap** pCPUDescriptorHeaps;
    struct D3D12Util_DescriptorHeap** pCbvSrvUavHeaps;
    struct D3D12Util_DescriptorHeap** pSamplerHeaps;
    ID3D12Device* pDxDevice;
    struct ID3D12CommandQueue** const ppCommandQueues[QUEUE_TYPE_COUNT]
#ifdef __cplusplus
        = {}
#endif
    ;
    const uint32_t pCommandQueueCounts[QUEUE_TYPE_COUNT]
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
} CGpuDevice_D3D12;

typedef struct CGpuFence_D3D12 {
    CGpuFence super;
    ID3D12Fence* pDxFence;
    HANDLE pDxWaitIdleFenceEvent;
    uint64_t mFenceValue;
    uint64_t mPadA;
} CGpuFence_D3D12;

typedef struct CGpuQueue_D3D12 {
    CGpuQueue super;
    struct ID3D12CommandQueue* pCommandQueue;
    struct CGpuFence_D3D12* pFence;
} CGpuQueue_D3D12;

typedef struct CGpuCommandPool_D3D12 {
    CGpuCommandPool super;
    struct ID3D12CommandAllocator* pDxCmdAlloc;
} CGpuCommandPool_D3D12;

typedef struct CGpuCommandBuffer_D3D12 {
    CGpuCommandBuffer super;
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
    CGpuCommandPool_D3D12* pCmdPool;
    uint32_t mPadA;
#if !defined(XBOX)
    uint64_t mPadB;
#endif
} CGpuCommandBuffer_D3D12;

typedef struct CGpuShaderLibrary_D3D12 {
    CGpuShaderLibrary super;
    struct IDxcBlobEncoding* pShaderBlob;
} CGpuShaderLibrary_D3D12;

typedef struct CGpuRootSignature_D3D12 {
    CGpuRootSignature super;
    ID3D12RootSignature* pDxRootSignature;
    D3D12_ROOT_PARAMETER1 mRootConstantParam;
    uint32_t mRootParamIndex;
} CGpuRootSignature_D3D12;

typedef struct CGpuDescriptorSet_D3D12 {
    CGpuDescriptorSet super;
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
} CGpuDescriptorSet_D3D12;

typedef struct CGpuComputePipeline_D3D12 {
    CGpuComputePipeline super;
    ID3D12PipelineState* pDxPipelineState;
    ID3D12RootSignature* pRootSignature;
} CGpuComputePipeline_D3D12;

typedef struct CGpuRenderPipeline_D3D12 {
    CGpuRenderPipeline super;
    ID3D12PipelineState* pDxPipelineState;
    ID3D12RootSignature* pRootSignature;
    D3D_PRIMITIVE_TOPOLOGY mDxPrimitiveTopology;
} CGpuRenderPipeline_D3D12;

typedef struct CGpuBuffer_D3D12 {
    CGpuBuffer super;
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
} CGpuBuffer_D3D12;

typedef struct CGpuTexture_D3D12 {
    CGpuTexture super;
    ID3D12Resource* pDxResource;
#ifdef __cplusplus
    D3D12MA::Allocation* pDxAllocation;
#else
    struct DMA_Allocation* pDxAllocation;
#endif
} CGpuTexture_D3D12;

typedef struct CGpuTextureView_D3D12 {
    CGpuTextureView super;
    D3D12_CPU_DESCRIPTOR_HANDLE mDxDescriptorHandles;
    /// Offset from mDxDescriptors for srv descriptor handle
    uint64_t mDxSrvOffset : 8;
    /// Offset from mDxDescriptors for uav descriptor handle
    uint64_t mDxUavOffset : 8;
    /// Offset from mDxDescriptors for rtv descriptor handle
    D3D12_CPU_DESCRIPTOR_HANDLE mDxRtxDescriptorHandle;
} CGpuTextureView_D3D12;

typedef struct CGpuSampler_D3D12 {
    CGpuSampler super;
    /// Description for creating the Sampler descriptor for this sampler
    D3D12_SAMPLER_DESC mDxDesc;
    /// Descriptor handle of the Sampler in a CPU visible descriptor heap
    D3D12_CPU_DESCRIPTOR_HANDLE mDxHandle;
} CGpuSampler_D3D12;

typedef struct CGpuSwapChain_D3D12 {
    CGpuSwapChain super;
    struct IDXGISwapChain3* pDxSwapChain;
    uint32_t mDxSyncInterval : 3;
    uint32_t mFlags : 10;
    uint32_t mImageCount : 3;
    uint32_t mEnableVsync : 1;
} CGpuSwapChain_D3D12;

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

static const D3D12_COMMAND_LIST_TYPE gDx12CmdTypeTranslator[QUEUE_TYPE_COUNT] = {
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    D3D12_COMMAND_LIST_TYPE_COMPUTE,
    D3D12_COMMAND_LIST_TYPE_COPY
};

#define IID_ARGS IID_PPV_ARGS