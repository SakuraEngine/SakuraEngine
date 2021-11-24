#include "math/common.h"
#include "platform/thread.h"
#include "platform/atomic.h"
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/d3d12_bridge.h"
#include "D3D12MemAlloc.h"
#include <EASTL/vector.h>
#include <dxcapi.h>

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

// Inline Utils
D3D12_RESOURCE_STATES D3D12Util_ResourceStateBridge(ECGpuResourceState state);
D3D12_RESOURCE_DESC D3D12Util_CreateBufferDesc(CGpuAdapter_D3D12* A, CGpuDevice_D3D12* D, const struct CGpuBufferDescriptor* desc);
D3D12MA::ALLOCATION_DESC D3D12Util_CreateAllocationDesc(const struct CGpuBufferDescriptor* desc);
// Buffer APIs
static_assert(sizeof(CGpuBuffer_D3D12) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line
CGpuBufferId cgpu_create_buffer_d3d12(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc)
{
    CGpuBuffer_D3D12* B = new CGpuBuffer_D3D12();
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuAdapter_D3D12* A = (CGpuAdapter_D3D12*)device->adapter;
    D3D12_RESOURCE_DESC bufDesc = D3D12Util_CreateBufferDesc(A, D, desc);
    // Handle Resource Start State
    ECGpuResourceState start_state = desc->start_state;
    if (desc->memory_usage == MU_CPU_TO_GPU || desc->memory_usage == MU_CPU_ONLY)
    {
        start_state = RS_GENERIC_READ;
    }
    else if (desc->memory_usage == MU_GPU_TO_CPU)
    {
        start_state = RS_COPY_DEST;
    }
    D3D12_RESOURCE_STATES res_states = D3D12Util_ResourceStateBridge(start_state);

    // Do Allocation
    D3D12MA::ALLOCATION_DESC alloc_desc = D3D12Util_CreateAllocationDesc(desc);
    if (D3D12_HEAP_TYPE_DEFAULT != alloc_desc.HeapType &&
        (alloc_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
    {
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
        heapProps.VisibleNodeMask = 1;
        heapProps.CreationNodeMask = 1;
        CHECK_HRESULT(D->pDxDevice->CreateCommittedResource(
            &heapProps, alloc_desc.ExtraHeapFlags, &bufDesc, res_states, NULL, IID_ARGS(&B->pDxResource)));
    }
    else
    {
        CHECK_HRESULT(D->pResourceAllocator->CreateResource(
            &alloc_desc, &bufDesc, res_states, NULL, &B->pDxAllocation, IID_ARGS(&B->pDxResource)));
    }
    if (desc->memory_usage != MU_GPU_ONLY && desc->flags & BCF_PERSISTENT_MAP_BIT)
        B->pDxResource->Map(0, NULL, &B->super.cpu_mapped_address);

    B->mDxGpuAddress = B->pDxResource->GetGPUVirtualAddress();
#if defined(XBOX)
    B->super.cpu_mapped_address->pCpuMappedAddress = (void*)pBuffer->mD3D12.mDxGpuAddress;
#endif
    // TODO: Create Descriptors
    // Set Debug Name
    if (device->adapter->instance->enable_set_name && desc->name)
    {
        wchar_t debugName[MAX_GPU_DEBUG_NAME_LENGTH] = {};
        mbstowcs(debugName, desc->name, MAX_GPU_DEBUG_NAME_LENGTH);
        if (B->pDxAllocation)
        {
            B->pDxAllocation->SetName(debugName);
        }
        B->pDxResource->SetName(debugName);
    }

    // Set Buffer Object Props
    B->super.size = (uint32_t)desc->size;
    B->super.memory_usage = desc->memory_usage;
    B->super.descriptors = desc->descriptors;
    return &B->super;
}

void cgpu_free_buffer_d3d12(CGpuBufferId buffer)
{
    CGpuBuffer_D3D12* B = (CGpuBuffer_D3D12*)buffer;
    SAFE_RELEASE(B->pDxResource)
    SAFE_RELEASE(B->pDxAllocation)
    delete B;
}

// Shader APIs
#ifndef DXC_CP_ACP
    #define DXC_CP_ACP 0
#endif
CGpuShaderLibraryId cgpu_create_shader_library_d3d12(
    CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc)
{
    CGpuShaderLibrary_D3D12* S = new CGpuShaderLibrary_D3D12();
    IDxcLibrary* pUtils;
    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pUtils));
    pUtils->CreateBlobWithEncodingOnHeapCopy(desc->code, (uint32_t)desc->code_size, DXC_CP_ACP, &S->pShaderBlob);
    pUtils->Release();
    return &S->super;
}

void cgpu_free_shader_library_d3d12(CGpuShaderLibraryId shader_library)
{
    CGpuShaderLibrary_D3D12* S = (CGpuShaderLibrary_D3D12*)shader_library;
    if (S->pShaderBlob != CGPU_NULLPTR)
    {
        S->pShaderBlob->Release();
    }
    delete S;
}

// Util Implementations
inline D3D12_RESOURCE_DESC D3D12Util_CreateBufferDesc(
    CGpuAdapter_D3D12* A, CGpuDevice_D3D12* D, const struct CGpuBufferDescriptor* desc)
{
    DECLARE_ZERO(D3D12_RESOURCE_DESC, bufDesc);
    uint64_t allocationSize = desc->size;
    // Align the buffer size to multiples of the dynamic uniform buffer minimum size
    if (desc->descriptors & RT_UNIFORM_BUFFER)
    {
        uint64_t minAlignment = A->adapter_detail.uniform_buffer_alignment;
        allocationSize = smath_round_up_64(allocationSize, minAlignment);
    }
    bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    // Alignment must be 64KB (D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) or 0, which is effectively 64KB.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn903813(v=vs.85).aspx
    bufDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufDesc.Width = allocationSize;
    bufDesc.Height = 1;
    bufDesc.DepthOrArraySize = 1;
    bufDesc.MipLevels = 1;
    bufDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufDesc.SampleDesc.Count = 1;
    bufDesc.SampleDesc.Quality = 0;
    bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    if (desc->descriptors & RT_RW_BUFFER)
    {
        bufDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    // Adjust for padding
    UINT64 padded_size = 0;
    D->pDxDevice->GetCopyableFootprints(&bufDesc, 0, 1, 0, NULL, NULL, NULL, &padded_size);
    allocationSize = (uint64_t)padded_size;
    bufDesc.Width = padded_size;
    // Mark DENY_SHADER_RESOURCE
    if (desc->memory_usage == MU_GPU_TO_CPU)
    {
        bufDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }
    return bufDesc;
}

inline D3D12_RESOURCE_STATES D3D12Util_ResourceStateBridge(ECGpuResourceState state)
{
    D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

    // These states cannot be combined with other states so we just do an == check
    if (state == RS_GENERIC_READ)
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    if (state == RS_COMMON)
        return D3D12_RESOURCE_STATE_COMMON;
    if (state == RS_PRESENT)
        return D3D12_RESOURCE_STATE_PRESENT;

    if (state & RS_VERTEX_AND_CONSTANT_BUFFER)
        ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (state & RS_INDEX_BUFFER)
        ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if (state & RS_RENDER_TARGET)
        ret |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (state & RS_UNORDERED_ACCESS)
        ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (state & RS_DEPTH_WRITE)
        ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
    if (state & RS_DEPTH_READ)
        ret |= D3D12_RESOURCE_STATE_DEPTH_READ;
    if (state & RS_STREAM_OUT)
        ret |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if (state & RS_INDIRECT_ARGUMENT)
        ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    if (state & RS_COPY_DEST)
        ret |= D3D12_RESOURCE_STATE_COPY_DEST;
    if (state & RS_COPY_SOURCE)
        ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (state & RS_NON_PIXEL_SHADER_RESOURCE)
        ret |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    if (state & RS_PIXEL_SHADER_RESOURCE)
        ret |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
#ifdef ENABLE_RAYTRACING
    if (state & RS_RAYTRACING_ACCELERATION_STRUCTURE)
        ret |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif
#ifdef ENABLE_VRS
    if (state & RS_SHADING_RATE_SOURCE)
        ret |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
#endif
    return ret;
}

inline D3D12MA::ALLOCATION_DESC D3D12Util_CreateAllocationDesc(const struct CGpuBufferDescriptor* desc)
{
    // Alloc Info
    DECLARE_ZERO(D3D12MA::ALLOCATION_DESC, alloc_desc)
    if (desc->memory_usage == MU_CPU_ONLY || desc->memory_usage == MU_CPU_TO_GPU)
        alloc_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    else if (desc->memory_usage == MU_GPU_TO_CPU)
        alloc_desc.HeapType = D3D12_HEAP_TYPE_READBACK;
    else
        alloc_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    if (desc->flags & BCF_OWN_MEMORY_BIT)
        alloc_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
    return alloc_desc;
}
