#pragma once
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "D3D12MemAlloc.h"
#ifdef CGPU_THREAD_SAFETY
    #include "platform/thread.h"
    #include "platform/atomic.h"
#endif
#include <EASTL/vector.h>

// Instance Helpers
void D3D12Util_QueryAllAdapters(CGpuInstance_D3D12* I, uint32_t* count, bool* foundSoftwareAdapter);
void D3D12Util_Optionalenable_debug_layer(CGpuInstance_D3D12* result, CGpuInstanceDescriptor const* descriptor);

// Device Helpers
void D3D12Util_CreateDMAAllocator(CGpuInstance_D3D12* I, CGpuAdapter_D3D12* A, CGpuDevice_D3D12* D);

// API Objects Helpers
void D3D12Util_SignalFence(CGpuQueue_D3D12* Q, ID3D12Fence* DxF, uint64_t fenceValue);
void D3D12Util_InitializeShaderReflection(CGpuDevice_D3D12* device, CGpuShaderLibrary_D3D12* library, const struct CGpuShaderLibraryDescriptor* desc);
void D3D12Util_FreeShaderReflection(CGpuShaderLibrary_D3D12* library);

// Feature Select Helpers
void D3D12Util_RecordAdapterDetail(struct CGpuAdapter_D3D12* D3D12Adapter);

// Descriptor Heap Helpers

/// CPU Visible Heap to store all the resources needing CPU read / write operations - Textures/Buffers/RTV
typedef struct D3D12Util_DescriptorHandle {
    D3D12_CPU_DESCRIPTOR_HANDLE mCpu;
    D3D12_GPU_DESCRIPTOR_HANDLE mGpu;
} D3D12Util_DescriptorHandle;

void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice,
    const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, struct D3D12Util_DescriptorHeap** ppDescHeap);
void D3D12Util_ResetDescriptorHeap(struct D3D12Util_DescriptorHeap* pHeap);
void D3D12Util_FreeDescriptorHeap(struct D3D12Util_DescriptorHeap* pHeap);
// Consume & Return
D3D12Util_DescriptorHandle D3D12Util_ConsumeDescriptorHandles(
    struct D3D12Util_DescriptorHeap* pHeap, uint32_t count);
void D3D12Util_ReturnDescriptorHandles(
    struct D3D12Util_DescriptorHeap* pHeap, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t count);

// Use Views
void D3D12Util_CreateSRV(CGpuDevice_D3D12* D, ID3D12Resource* pResource,
    const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);
void D3D12Util_CreateUAV(CGpuDevice_D3D12* D, ID3D12Resource* pResource,
    ID3D12Resource* pCounterResource,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);
void D3D12Util_CreateCBV(CGpuDevice_D3D12* D,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);

typedef struct D3D12Util_DescriptorHeap {
    /// DX Heap
    ID3D12DescriptorHeap* pCurrentHeap;
    ID3D12Device* pDevice;
    D3D12_CPU_DESCRIPTOR_HANDLE* pHandles;
    /// Start position in the heap
    D3D12Util_DescriptorHandle mStartHandle;
    /// Free List used for CPU only descriptor heaps
    eastl::vector<D3D12Util_DescriptorHandle> mFreeList;
    /// Description
    D3D12_DESCRIPTOR_HEAP_DESC mDesc;
    /// DescriptorInfo Increment Size
    uint32_t mDescriptorSize;
#ifdef CGPU_THREAD_SAFETY
    /// Lock for multi-threaded descriptor allocations
    struct SMutex* pMutex;
    /// Used
    SAtomic32 mUsedDescriptors;
#else
    /// Used
    uint32_t mUsedDescriptors;
#endif
} D3D12Util_DescriptorHeap;

typedef struct DescriptorHeapProperties {
    uint32_t mMaxDescriptors;
    D3D12_DESCRIPTOR_HEAP_FLAGS mFlags;
} DescriptorHeapProperties;

static const DescriptorHeapProperties gCpuDescriptorHeapProperties[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {
    { 1024 * 256, D3D12_DESCRIPTOR_HEAP_FLAG_NONE }, // CBV SRV UAV
    { 2048, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },       // Sampler
    { 512, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },        // RTV
    { 512, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },        // DSV
};

#include "d3d12_utils.inl"
//
// C++ is the only language supported by D3D12:
//   https://msdn.microsoft.com/en-us/library/windows/desktop/dn899120(v=vs.85).aspx
//
#if !defined(__cplusplus)
    #error "D3D12 requires C++! Sorry!"
#endif