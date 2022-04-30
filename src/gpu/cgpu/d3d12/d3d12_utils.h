#pragma once
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "./../common/common_utils.h"
#include "D3D12MemAlloc.h"
#ifdef CGPU_THREAD_SAFETY
    #include "platform/thread.h"
    #include "platform/atomic.h"
#endif
#include <EASTL/vector.h>

#define CALC_SUBRESOURCE_INDEX(MipSlice, ArraySlice, PlaneSlice, MipLevels, \
ArraySize)                                                                  \
    ((MipSlice) + ((ArraySlice) * (MipLevels)) +                            \
     ((PlaneSlice) * (MipLevels) * (ArraySize)))

// Instance Helpers
void D3D12Util_QueryAllAdapters(CGPUInstance_D3D12* I, uint32_t* count, bool* foundSoftwareAdapter);
void D3D12Util_Optionalenable_debug_layer(CGPUInstance_D3D12* result, CGPUInstanceDescriptor const* descriptor);

// Device Helpers
void D3D12Util_CreateDMAAllocator(CGPUInstance_D3D12* I, CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D);

// API Objects Helpers
void D3D12Util_SignalFence(CGPUQueue_D3D12* Q, ID3D12Fence* DxF, uint64_t fenceValue);
void D3D12Util_InitializeShaderReflection(CGPUDevice_D3D12* device, CGPUShaderLibrary_D3D12* library, const struct CGPUShaderLibraryDescriptor* desc);
void D3D12Util_FreeShaderReflection(CGPUShaderLibrary_D3D12* library);

// Feature Select Helpers
void D3D12Util_RecordAdapterDetail(struct CGPUAdapter_D3D12* D3D12Adapter);

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
void D3D12Util_CreateSRV(CGPUDevice_D3D12* D, ID3D12Resource* pResource,
const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);
void D3D12Util_CreateUAV(CGPUDevice_D3D12* D, ID3D12Resource* pResource,
ID3D12Resource* pCounterResource,
const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);
void D3D12Util_CreateCBV(CGPUDevice_D3D12* D,
const D3D12_CONSTANT_BUFFER_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);
void D3D12Util_CreateRTV(CGPUDevice_D3D12* D, ID3D12Resource* pResource,
const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle);

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

static const D3D12_BLEND_OP gDx12BlendOpTranslator[CGPU_BLEND_MODE_COUNT] = {
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_OP_SUBTRACT,
    D3D12_BLEND_OP_REV_SUBTRACT,
    D3D12_BLEND_OP_MIN,
    D3D12_BLEND_OP_MAX,
};

static const D3D12_BLEND gDx12BlendConstantTranslator[CGPU_BLEND_CONST_COUNT] = {
    D3D12_BLEND_ZERO,
    D3D12_BLEND_ONE,
    D3D12_BLEND_SRC_COLOR,
    D3D12_BLEND_INV_SRC_COLOR,
    D3D12_BLEND_DEST_COLOR,
    D3D12_BLEND_INV_DEST_COLOR,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_INV_SRC_ALPHA,
    D3D12_BLEND_DEST_ALPHA,
    D3D12_BLEND_INV_DEST_ALPHA,
    D3D12_BLEND_SRC_ALPHA_SAT,
    D3D12_BLEND_BLEND_FACTOR,
    D3D12_BLEND_INV_BLEND_FACTOR,
};

static const D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE gDx12PassBeginOpTranslator[CGPU_LOAD_ACTION_COUNT] = {
    D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD,
    D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE,
    D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR
};

static const D3D12_RENDER_PASS_ENDING_ACCESS_TYPE gDx12PassEndOpTranslator[CGPU_STORE_ACTION_COUNT] = {
    D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE,
    D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD
};

static const D3D12_COMPARISON_FUNC gDx12ComparisonFuncTranslator[CMP_COUNT] = {
    D3D12_COMPARISON_FUNC_NEVER,
    D3D12_COMPARISON_FUNC_LESS,
    D3D12_COMPARISON_FUNC_EQUAL,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    D3D12_COMPARISON_FUNC_GREATER,
    D3D12_COMPARISON_FUNC_NOT_EQUAL,
    D3D12_COMPARISON_FUNC_GREATER_EQUAL,
    D3D12_COMPARISON_FUNC_ALWAYS,
};

static const D3D12_STENCIL_OP gDx12StencilOpTranslator[CGPU_STENCIL_OP_COUNT] = {
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_ZERO,
    D3D12_STENCIL_OP_REPLACE,
    D3D12_STENCIL_OP_INVERT,
    D3D12_STENCIL_OP_INCR,
    D3D12_STENCIL_OP_DECR,
    D3D12_STENCIL_OP_INCR_SAT,
    D3D12_STENCIL_OP_DECR_SAT,
};

D3D12_CULL_MODE gDx12CullModeTranslator[CGPU_CULL_MODE_COUNT] = {
    D3D12_CULL_MODE_NONE,
    D3D12_CULL_MODE_BACK,
    D3D12_CULL_MODE_FRONT,
};

D3D12_FILL_MODE gDx12FillModeTranslator[FILL_MODE_COUNT] = {
    D3D12_FILL_MODE_SOLID,
    D3D12_FILL_MODE_WIREFRAME,
};

#include "d3d12_utils.inl"
//
// C++ is the only language supported by D3D12:
//   https://msdn.microsoft.com/en-us/library/windows/desktop/dn899120(v=vs.85).aspx
//
#if !defined(__cplusplus)
    #error "D3D12 requires C++! Sorry!"
#endif