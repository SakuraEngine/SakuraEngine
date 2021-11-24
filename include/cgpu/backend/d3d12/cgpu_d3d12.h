#pragma once
#include "cgpu/api.h"
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

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue_d3d12(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_free_queue_d3d12(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool_d3d12(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc);
RUNTIME_API void cgpu_free_command_pool_d3d12(CGpuCommandPoolId pool);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library_d3d12(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_d3d12(CGpuShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGpuBufferId cgpu_create_buffer_d3d12(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer_d3d12(CGpuBufferId buffer, const struct CGpuBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer_d3d12(CGpuBufferId buffer);
RUNTIME_API void cgpu_free_buffer_d3d12(CGpuBufferId buffer);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain_d3d12(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_d3d12(CGpuSwapChainId swapchain);

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
} CGpuAdapter_D3D12;

typedef struct CGpuDevice_D3D12 {
    CGpuDevice super;
    struct D3D12Util_DescriptorHeap** pCPUDescriptorHeaps;
    struct D3D12Util_DescriptorHeap** pCbvSrvUavHeaps;
    struct D3D12Util_DescriptorHeap** pSamplerHeaps;
    ID3D12Device* pDxDevice;
    struct ID3D12CommandQueue** const ppCommandQueues[ECGpuQueueType_Count]
#ifdef __cplusplus
        = {}
#endif
    ;
    const uint32_t pCommandQueueCounts[ECGpuQueueType_Count]
#ifdef __cplusplus
        = {}
#endif
    ;
#ifdef __cplusplus
    class D3D12MA::Allocator* pResourceAllocator;
#else
    struct DMA_Allocator* pResourceAllocator;
#endif
} CGpuDevice_D3D12;

typedef struct CGpuQueue_D3D12 {
    CGpuQueue super;
    struct ID3D12CommandQueue* pCommandQueue;
} CGpuQueue_D3D12;

typedef struct CGpuCommandPool_D3D12 {
    CGpuCommandPool super;
    struct ID3D12CommandAllocator* pCommandAllocator;
} CGpuCommandPool_D3D12;

typedef struct CGpuShaderLibrary_D3D12 {
    CGpuShaderLibrary super;
    struct IDxcBlobEncoding* pShaderBlob;
} CGpuShaderLibrary_D3D12;

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

#define IID_ARGS IID_PPV_ARGS