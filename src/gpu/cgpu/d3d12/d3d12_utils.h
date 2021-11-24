#pragma once
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/d3d12_bridge.h"
#include "D3D12MemAlloc.h"
#include "platform/thread.h"
#include "platform/atomic.h"
#include <EASTL/vector.h>

#define SINGLE_GPU_NODE_COUNT 1
#define SINGLE_GPU_NODE_MASK 1

// Instance Helpers
void D3D12Util_QueryAllAdapters(CGpuInstance_D3D12* I, uint32_t* count, bool* foundSoftwareAdapter);
void D3D12Util_Optionalenable_debug_layer(CGpuInstance_D3D12* result, CGpuInstanceDescriptor const* descriptor);

// Device Helpers
void D3D12Util_CreateDMAAllocator(CGpuInstance_D3D12* I, CGpuAdapter_D3D12* A, CGpuDevice_D3D12* D);

// Feature Select Helpers
void D3D12Util_RecordAdapterDetail(struct CGpuAdapter_D3D12* D3D12Adapter);

// Helper Structures
void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice,
    const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, struct D3D12Util_DescriptorHeap** ppDescHeap);
void D3D12Util_ResetDescriptorHeap(struct D3D12Util_DescriptorHeap* pHeap);
void D3D12Util_FreeDescriptorHeap(struct D3D12Util_DescriptorHeap* pHeap);

/// CPU Visible Heap to store all the resources needing CPU read / write operations - Textures/Buffers/RTV
typedef struct D3D12Util_DescriptorHandle {
    D3D12_CPU_DESCRIPTOR_HANDLE mCpu;
    D3D12_GPU_DESCRIPTOR_HANDLE mGpu;
} D3D12Util_DescriptorHandle;

typedef struct D3D12Util_DescriptorHeap {
    /// DX Heap
    ID3D12DescriptorHeap* pCurrentHeap;
    /// Lock for multi-threaded descriptor allocations
    struct SMutex* pMutex;
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
    /// Used
    SAtomic32 mUsedDescriptors;
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
//
// C++ is the only language supported by D3D12:
//   https://msdn.microsoft.com/en-us/library/windows/desktop/dn899120(v=vs.85).aspx
//
#if !defined(__cplusplus)
    #error "D3D12 requires C++! Sorry!"
#endif