#pragma once
#include "cgpu/api.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#ifdef __cplusplus
extern "C" {
#endif

const CGpuProcTable* CGPU_D3D12ProcTable();
const CGpuSurfacesProcTable* CGPU_D3D12SurfacesProcTable();

CGpuInstanceId cgpu_create_instance_d3d12(CGpuInstanceDescriptor const* descriptor);
void cgpu_free_instance_d3d12(CGpuInstanceId instance);
void cgpu_enum_adapters_d3d12(CGpuInstanceId instance, CGpuAdapterId* const adapters, size_t* adapters_num);
void cgpu_query_adapter_detail_d3d12(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail);
uint32_t cgpu_query_queue_count_d3d12(const CGpuAdapterId adapter, const ECGpuQueueType type);
CGpuDeviceId cgpu_create_device_d3d12(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
void cgpu_free_device_d3d12(CGpuDeviceId device);
CGpuQueueId cgpu_get_queue_d3d12(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
void cgpu_free_queue_d3d12(CGpuQueueId queue);
CGpuCommandEncoderId cgpu_create_command_encoder_d3d12(CGpuQueueId queue, const CGpuCommandEncoderDescriptor* desc);
void cgpu_free_command_encoder_d3d12(CGpuCommandEncoderId pool);
CGpuSwapChainId cgpu_create_swapchain_d3d12(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
void cgpu_free_swapchain_d3d12(CGpuSwapChainId swapchain);

typedef struct CGpuInstance_D3D12 {
    CGpuInstance super;
#if defined(XBOX)
    IDXGIFactory2*                  pDXGIFactory;
#elif defined(_WIN32)
    IDXGIFactory6*                  pDXGIFactory;
#endif
    ID3D12Debug*                    pDXDebug;

    struct CGpuAdapter_D3D12*       pAdapters;
    uint32_t                        mAdaptersCount;
#if defined(__cplusplus)

#endif

} CGpuInstance_D3D12;

typedef struct CGpuAdapter_D3D12 {
    CGpuAdapter super;
#if defined(XBOX)
	IDXGIAdapter*                   pDxActiveGPU;
#elif defined(_WIN32)
	IDXGIAdapter4*                  pDxActiveGPU;
#endif
    D3D_FEATURE_LEVEL               mFeatureLevel;
    uint32_t                        mDeviceId;
    uint32_t                        mVendorId;
    char                            mDescription[128];
} CGpuAdapter_D3D12;

typedef struct CGpuDevice_D3D12 {
    CGpuDevice super;
    ID3D12Device* pDxDevice; 
    ID3D12CommandQueue** const ppCommandQueues[ECGpuQueueType_Count]
#ifdef __cplusplus
    = {}
#endif
    ;
    const uint32_t pCommandQueueCounts[ECGpuQueueType_Count]
#ifdef __cplusplus
    = {}
#endif
    ;
} CGpuDevice_D3D12;

typedef struct CGpuQueue_D3D12 {
    CGpuQueue super;
    ID3D12CommandQueue* pCommandQueue; 
} CGpuQueue_D3D12;

typedef struct CGpuCommandEncoder_D3D12 {
    CGpuCommandEncoder super;
    ID3D12CommandAllocator* pCommandAllocator;
} CGpuCommandEncoder_D3D12;

typedef struct CGpuSwapChain_D3D12 {
    CGpuSwapChain            super;
    IDXGISwapChain3*         pDxSwapChain;
    uint32_t                 mDxSyncInterval : 3;
    uint32_t                 mFlags : 10;
    uint32_t                 mImageCount : 3;
    uint32_t                 mEnableVsync : 1;
} CGpuSwapChain_D3D12;

#ifdef __cplusplus
} // end extern "C"
#endif