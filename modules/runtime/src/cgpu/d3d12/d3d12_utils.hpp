#pragma once
#include "cgpu/d3d12/D3D12MemAlloc.h"
#include "SkrRT/platform/configure.h"
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "./../common/common_utils.h"
#ifdef __cplusplus
    #include "D3D12MemAlloc.h"
    #include <EASTL/vector.h>
    #include <SkrRT/containers/hashmap.hpp>
#endif
#ifdef CGPU_THREAD_SAFETY
    #include "SkrRT/platform/thread.h"
    #include "SkrRT/platform/atomic.h"
#endif

#define CALC_SUBRESOURCE_INDEX(MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize) ((MipSlice) + ((ArraySlice) * (MipLevels)) + ((PlaneSlice) * (MipLevels) * (ArraySize)))

// Instance Helpers
void D3D12Util_QueryAllAdapters(CGPUInstance_D3D12* I, uint32_t* count, bool* foundSoftwareAdapter);
bool D3D12Util_InitializeEnvironment(struct CGPUInstance* Inst);
void D3D12Util_DeInitializeEnvironment(struct CGPUInstance* Inst);
void D3D12Util_Optionalenable_debug_layer(CGPUInstance_D3D12* result, CGPUInstanceDescriptor const* descriptor);

// Device Helpers
void D3D12Util_CreateDMAAllocator(CGPUInstance_D3D12* I, CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D);

#if !defined(XBOX) && defined(_WIN32)
    #include "dxc/dxcapi.h"

void D3D12Util_LoadDxcDLL();
void D3D12Util_UnloadDxcDLL();
DxcCreateInstanceProc D3D12Util_GetDxcCreateInstanceProc();
#endif

// Crash Report Helpers
void D3D12Util_LogDREDPageFault(const D3D12_DRED_PAGE_FAULT_OUTPUT* pageFault);
void D3D12Util_LogDREDBreadcrumbs(const D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT* breadcrumbs);
#ifdef __ID3D12DeviceRemovedExtendedData1_INTERFACE_DEFINED__
void D3D12Util_LogDREDBreadcrumbs1(const D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1* breadcrumbs);
#endif
void D3D12Util_ReportGPUCrash(ID3D12Device* device);

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
D3D12Util_DescriptorHandle D3D12Util_ConsumeDescriptorHandles(struct D3D12Util_DescriptorHeap* pHeap, uint32_t count);
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
    SAtomicU32 mUsedDescriptors;
#else
    /// Used
    uint32_t mUsedDescriptors;
#endif
} D3D12Util_DescriptorHeap;

struct CGPUTiledMemoryPool_D3D12 : public CGPUMemoryPool_D3D12 {
    void AllocateTiles(uint32_t N, D3D12MA::Allocation** ppAllocation, uint32_t Scale = 1) SKR_NOEXCEPT
    {
        CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)super.device;
        const auto kPageSize = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.CustomPool = pDxPool;
        D3D12_RESOURCE_ALLOCATION_INFO allocInfo = {};
        allocInfo.Alignment = kPageSize;
        allocInfo.SizeInBytes = kPageSize * Scale;
        for (uint32_t i = 0; i < N; ++i)
        {
            D->pResourceAllocator->AllocateMemory(&allocDesc, &allocInfo, &ppAllocation[i]);
        }
    }

    ~CGPUTiledMemoryPool_D3D12() SKR_NOEXCEPT
    {
        SAFE_RELEASE(pDxPool);
    }
};

enum ETileMappingStatus_D3D12
{
    D3D12_TILE_MAPPING_STATUS_UNMAPPED = 0,
    D3D12_TILE_MAPPING_STATUS_PENDING = 1,
    D3D12_TILE_MAPPING_STATUS_MAPPING = 2,
    D3D12_TILE_MAPPING_STATUS_MAPPED = 3,
    D3D12_TILE_MAPPING_STATUS_UNMAPPING = 4
};

struct TileMapping_D3D12 {
    D3D12MA::Allocation* pDxAllocation;
    SAtomic32 status;
};
static_assert(std::is_trivially_constructible_v<TileMapping_D3D12>, "TileMapping_D3D12 Must Be Trivially Constructible!");

struct SubresTileMappings_D3D12 {
    SubresTileMappings_D3D12(CGPUTexture_D3D12* T, uint32_t X, uint32_t Y, uint32_t Z) SKR_NOEXCEPT
        : T(T),
          X(X),
          Y(Y),
          Z(Z)
    {
        if (X * Y * Z)
            mappings = (TileMapping_D3D12*)cgpu_calloc(1, X * Y * Z * sizeof(TileMapping_D3D12));
    }
    ~SubresTileMappings_D3D12() SKR_NOEXCEPT
    {
        if (mappings)
        {
            for (uint32_t x = 0; x < X; x++)
                for (uint32_t y = 0; y < Y; y++)
                    for (uint32_t z = 0; z < Z; z++)
                    {
                        unmap(x, y, z);
                    }
            cgpu_free(mappings);
        }
    }
    TileMapping_D3D12* at(uint32_t x, uint32_t y, uint32_t z)
    {
        SKR_ASSERT(mappings && x < X && y < Y && z < Z && "SubresTileMappings::at: Out of Range!");
        return mappings + (x + y * X + z * X * Y);
    }
    const TileMapping_D3D12* at(uint32_t x, uint32_t y, uint32_t z) const
    {
        SKR_ASSERT(mappings && x < X && y < Y && z < Z && "SubresTileMappings::at: Out of Range!");
        return mappings + (x + y * X + z * X * Y);
    }
    void unmap(uint32_t x, uint32_t y, uint32_t z)
    {
        auto pTiledInfo = const_cast<CGPUTiledTextureInfo*>(T->super.tiled_resource);
        auto* mapping = at(x, y, z);
        const auto status = skr_atomic32_cas_relaxed(&mapping->status,
                                                     D3D12_TILE_MAPPING_STATUS_MAPPED, D3D12_TILE_MAPPING_STATUS_UNMAPPING);
        if (status == D3D12_TILE_MAPPING_STATUS_MAPPED)
        {
            SAFE_RELEASE(mapping->pDxAllocation);
            skr_atomicu64_add_relaxed(&pTiledInfo->alive_tiles_count, -1);
        }
        skr_atomic32_cas_relaxed(&mapping->status,
                                 D3D12_TILE_MAPPING_STATUS_UNMAPPING, D3D12_TILE_MAPPING_STATUS_UNMAPPED);
    }

private:
    CGPUTexture_D3D12* T = nullptr;
    const uint32_t X = 0;
    const uint32_t Y = 0;
    const uint32_t Z = 0;
    TileMapping_D3D12* mappings = nullptr;
};

struct PackedMipMapping_D3D12 {
    PackedMipMapping_D3D12(CGPUTexture_D3D12* T, uint32_t N) SKR_NOEXCEPT
        : N(N),
          T(T)
    {
    }
    ~PackedMipMapping_D3D12() SKR_NOEXCEPT
    {
        unmap();
    }
    void unmap()
    {
        auto pTiledInfo = const_cast<CGPUTiledTextureInfo*>(T->super.tiled_resource);
        const auto prev = skr_atomic32_cas_relaxed(&status,
                                                   D3D12_TILE_MAPPING_STATUS_MAPPED, D3D12_TILE_MAPPING_STATUS_UNMAPPING);
        if (prev == D3D12_TILE_MAPPING_STATUS_MAPPED)
        {
            SAFE_RELEASE(pAllocation);
            skr_atomicu64_add_relaxed(&pTiledInfo->alive_tiles_count, -1);
        }
        skr_atomic32_cas_relaxed(&status,
                                 D3D12_TILE_MAPPING_STATUS_UNMAPPING, D3D12_TILE_MAPPING_STATUS_UNMAPPED);
    }
    D3D12MA::Allocation* pAllocation = nullptr;
    const uint32_t N = 0;
    SAtomic32 status;

private:
    CGPUTexture_D3D12* T = nullptr;
};

struct CGPUTiledTexture_D3D12 : public CGPUTexture_D3D12 {
    CGPUTiledTexture_D3D12(SubresTileMappings_D3D12* pMappings, PackedMipMapping_D3D12* pPackedMips, uint32_t NumPacks) SKR_NOEXCEPT
        : CGPUTexture_D3D12(),
          pMappings(pMappings),
          pPackedMips(pPackedMips),
          NumPacks(NumPacks)
    {
    }
    ~CGPUTiledTexture_D3D12() SKR_NOEXCEPT
    {
        const auto N = super.info->mip_levels * (super.info->array_size_minus_one + 1);
        for (uint32_t i = 0; i < N; i++)
            pMappings[i].~SubresTileMappings_D3D12();
        for (uint32_t i = 0; i < NumPacks; i++)
            pPackedMips[i].~PackedMipMapping_D3D12();
    }
    SubresTileMappings_D3D12* getSubresTileMappings(uint32_t mip_level, uint32_t array_index)
    {
        SKR_ASSERT(mip_level < super.info->mip_levels && array_index < super.info->array_size_minus_one + 1);
        return pMappings + (mip_level * (super.info->array_size_minus_one + 1) + array_index);
    }
    PackedMipMapping_D3D12* getPackedMipMapping(uint32_t layer)
    {
        return pPackedMips + layer;
    }

private:
    SubresTileMappings_D3D12* pMappings;
    PackedMipMapping_D3D12* pPackedMips;
    uint32_t NumPacks;
};

typedef struct DescriptorHeapProperties {
    uint32_t mMaxDescriptors;
    D3D12_DESCRIPTOR_HEAP_FLAGS mFlags;
} DescriptorHeapProperties;

static const DescriptorHeapProperties gCpuDescriptorHeapProperties[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {
    { 1024 * 256, D3D12_DESCRIPTOR_HEAP_FLAG_NONE }, // CBV SRV UAV
    { 2048, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },       // Sampler
    { 1024 * 64, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },  // RTV
    { 1024 * 64, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },  // DSV
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

static const D3D12_COMPARISON_FUNC gDx12ComparisonFuncTranslator[CGPU_CMP_COUNT] = {
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

static const D3D12_CULL_MODE gDx12CullModeTranslator[CGPU_CULL_MODE_COUNT] = {
    D3D12_CULL_MODE_NONE,
    D3D12_CULL_MODE_BACK,
    D3D12_CULL_MODE_FRONT,
};

static const D3D12_FILL_MODE gDx12FillModeTranslator[CGPU_FILL_MODE_COUNT] = {
    D3D12_FILL_MODE_SOLID,
    D3D12_FILL_MODE_WIREFRAME,
};

//
// C++ is the only language supported by D3D12:
//   https://msdn.microsoft.com/en-us/library/windows/desktop/dn899120(v=vs.85).aspx
//
#if !defined(__cplusplus)
    #error "D3D12 requires C++! Sorry!"
#endif

/* clang-format off */
FORCEINLINE static void D3D12Util_CopyDescriptorHandle(D3D12Util_DescriptorHeap *pHeap,
                                   const D3D12_CPU_DESCRIPTOR_HANDLE &srcHandle,
                                   const uint64_t &dstHandle, uint32_t index) {
  pHeap->pHandles[(dstHandle / pHeap->mDescriptorSize) + index] = srcHandle;
  pHeap->pDevice->CopyDescriptorsSimple(1,
                                        {pHeap->mStartHandle.mCpu.ptr +
                                         dstHandle +
                                         (index * pHeap->mDescriptorSize)},
                                        srcHandle, pHeap->mDesc.Type);
}

FORCEINLINE static void D3D12Util_CreateSRV(CGPUDevice_D3D12* D, ID3D12Resource* pResource,
    const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
    D->pDxDevice->CreateShaderResourceView(pResource, pSrvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateUAV(CGPUDevice_D3D12* D, ID3D12Resource* pResource,
    ID3D12Resource* pCounterResource,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
    D->pDxDevice->CreateUnorderedAccessView(pResource, pCounterResource, pSrvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateCBV(CGPUDevice_D3D12* D,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
    D->pDxDevice->CreateConstantBufferView(pSrvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateRTV(CGPUDevice_D3D12* D,
    ID3D12Resource* pResource,
    const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], 1).mCpu;
    D->pDxDevice->CreateRenderTargetView(pResource, pRtvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateDSV(CGPUDevice_D3D12* D,
    ID3D12Resource* pResource,
    const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], 1).mCpu;
    D->pDxDevice->CreateDepthStencilView(pResource, pDsvDesc, *pHandle);
}

#ifdef __ID3D12GraphicsCommandList5_INTERFACE_DEFINED__
#define D3D12_HEADER_SUPPORT_VRS
inline static D3D12_SHADING_RATE_COMBINER D3D12Util_TranslateShadingRateCombiner(ECGPUShadingRateCombiner combiner)
{
	switch (combiner)
	{
		case CGPU_SHADING_RATE_COMBINER_PASSTHROUGH: return D3D12_SHADING_RATE_COMBINER_PASSTHROUGH;
		case CGPU_SHADING_RATE_COMBINER_OVERRIDE: return D3D12_SHADING_RATE_COMBINER_OVERRIDE;
		case CGPU_SHADING_RATE_COMBINER_MAX: return D3D12_SHADING_RATE_COMBINER_MAX;
		case CGPU_SHADING_RATE_COMBINER_SUM: return D3D12_SHADING_RATE_COMBINER_SUM;
		default: cgpu_assert(false && "Invalid shading rate combiner type"); return D3D12_SHADING_RATE_COMBINER_PASSTHROUGH;
	}
}

inline static D3D12_SHADING_RATE D3D12Util_TranslateShadingRate(ECGPUShadingRate shadingRate)
{
	switch (shadingRate)
	{
		case CGPU_SHADING_RATE_FULL: return D3D12_SHADING_RATE_1X1;
		case CGPU_SHADING_RATE_1X2: return D3D12_SHADING_RATE_1X2;
		case CGPU_SHADING_RATE_2X1: return D3D12_SHADING_RATE_2X1;
		case CGPU_SHADING_RATE_HALF: return D3D12_SHADING_RATE_2X2;
		case CGPU_SHADING_RATE_2X4: return D3D12_SHADING_RATE_2X4;
		case CGPU_SHADING_RATE_4X2: return D3D12_SHADING_RATE_4X2;
		case CGPU_SHADING_RATE_QUARTER: return D3D12_SHADING_RATE_4X4;
		default: cgpu_assert(false && "Invalid shading rate"); return D3D12_SHADING_RATE_1X1;
	}
}
#endif

FORCEINLINE static D3D12_DESCRIPTOR_RANGE_TYPE D3D12Util_ResourceTypeToDescriptorRangeType(ECGPUResourceType type) {
  switch (type) {
  case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER:
  case CGPU_RESOURCE_TYPE_PUSH_CONSTANT:
    return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
  case CGPU_RESOURCE_TYPE_RW_BUFFER:
  case CGPU_RESOURCE_TYPE_RW_TEXTURE:
    return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
  case CGPU_RESOURCE_TYPE_SAMPLER:
    return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  case CGPU_RESOURCE_TYPE_RAY_TRACING:
  case CGPU_RESOURCE_TYPE_TEXTURE:
  case CGPU_RESOURCE_TYPE_BUFFER:
    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  default:
    cgpu_assert(false && "Invalid DescriptorInfo Type");
    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  }
}

FORCEINLINE static D3D12_BLEND_DESC D3D12Util_TranslateBlendState(const CGPUBlendStateDescriptor* pDesc)
{
  int blendDescIndex = 0;
  D3D12_BLEND_DESC ret = {};
  ret.AlphaToCoverageEnable = (BOOL)pDesc->alpha_to_coverage;
  ret.IndependentBlendEnable = TRUE;
  for (int i = 0; i < CGPU_MAX_MRT_COUNT; i++) {
      BOOL blendEnable =
        (gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]] !=
            D3D12_BLEND_ONE ||
        gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]] !=
            D3D12_BLEND_ZERO ||
        gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]] !=
            D3D12_BLEND_ONE ||
           gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]] !=
            D3D12_BLEND_ZERO);

      ret.RenderTarget[i].BlendEnable = blendEnable;
      ret.RenderTarget[i].RenderTargetWriteMask = (UINT8)pDesc->masks[blendDescIndex];
      ret.RenderTarget[i].BlendOp =
          gDx12BlendOpTranslator[pDesc->blend_modes[blendDescIndex]];
      ret.RenderTarget[i].SrcBlend =
          gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]];
      ret.RenderTarget[i].DestBlend =
          gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]];
      ret.RenderTarget[i].BlendOpAlpha =
          gDx12BlendOpTranslator[pDesc->blend_alpha_modes[blendDescIndex]];
      ret.RenderTarget[i].SrcBlendAlpha =
          gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]];
      ret.RenderTarget[i].DestBlendAlpha =
          gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]];

    if (pDesc->independent_blend)
      ++blendDescIndex;
  }
  return ret;
}

FORCEINLINE static D3D12_FILTER D3D12Util_TranslateFilter(ECGPUFilterType minFilter, ECGPUFilterType magFilter,
    ECGPUMipMapMode mipMapMode, bool aniso, bool comparisonFilterEnabled) 
{
  if (aniso)
    return (comparisonFilterEnabled ? D3D12_FILTER_COMPARISON_ANISOTROPIC
                                    : D3D12_FILTER_ANISOTROPIC);

  // control bit : minFilter  magFilter   mipMapMode
  //   point   :   00	  00	   00
  //   linear  :   01	  01	   01
  // ex : trilinear == 010101
  int filter = (minFilter << 4) | (magFilter << 2) | mipMapMode;
  int baseFilter = comparisonFilterEnabled
                       ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT
                       : D3D12_FILTER_MIN_MAG_MIP_POINT;
  return (D3D12_FILTER)(baseFilter + filter);
}

inline static D3D12_TEXTURE_ADDRESS_MODE D3D12Util_TranslateAddressMode(ECGPUAddressMode addressMode) 
{
  switch (addressMode) {
  case CGPU_ADDRESS_MODE_MIRROR:
    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
  case CGPU_ADDRESS_MODE_REPEAT:
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  case CGPU_ADDRESS_MODE_CLAMP_TO_EDGE:
    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  case CGPU_ADDRESS_MODE_CLAMP_TO_BORDER:
    return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  default:
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  }
}

FORCEINLINE static D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const CGPURasterizerStateDescriptor *pDesc) {
  cgpu_assert(pDesc->fill_mode < CGPU_FILL_MODE_COUNT);
  cgpu_assert(pDesc->cull_mode < CGPU_CULL_MODE_COUNT);
  cgpu_assert(pDesc->front_face == CGPU_FRONT_FACE_CCW ||  pDesc->front_face == CGPU_FRONT_FACE_CW);
  D3D12_RASTERIZER_DESC ret = {};
  ret.FillMode = gDx12FillModeTranslator[pDesc->fill_mode];
  ret.CullMode = gDx12CullModeTranslator[pDesc->cull_mode];
  ret.FrontCounterClockwise = pDesc->front_face == CGPU_FRONT_FACE_CCW;
  ret.DepthBias = pDesc->depth_bias;
  ret.DepthBiasClamp = 0.0f;
  ret.SlopeScaledDepthBias = pDesc->slope_scaled_depth_bias;
  ret.DepthClipEnable = !pDesc->enable_depth_clamp;
  ret.MultisampleEnable = pDesc->enable_multi_sample ? TRUE : FALSE;
  ret.AntialiasedLineEnable = FALSE;
  ret.ForcedSampleCount = 0;
  ret.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
  return ret;
}

FORCEINLINE static  D3D12_DEPTH_STENCIL_DESC D3D12Util_TranslateDephStencilState(const CGPUDepthStateDescriptor *pDesc) {
  cgpu_assert(pDesc->depth_func < CGPU_CMP_COUNT);
  cgpu_assert(pDesc->stencil_front_func < CGPU_CMP_COUNT);
  cgpu_assert(pDesc->stencil_front_fail < CGPU_STENCIL_OP_COUNT);
  cgpu_assert(pDesc->depth_front_fail < CGPU_STENCIL_OP_COUNT);
  cgpu_assert(pDesc->stencil_front_pass < CGPU_STENCIL_OP_COUNT);
  cgpu_assert(pDesc->stencil_back_func < CGPU_CMP_COUNT);
  cgpu_assert(pDesc->stencil_back_fail < CGPU_STENCIL_OP_COUNT);
  cgpu_assert(pDesc->depth_back_fail < CGPU_STENCIL_OP_COUNT);
  cgpu_assert(pDesc->stencil_back_pass < CGPU_STENCIL_OP_COUNT);

  D3D12_DEPTH_STENCIL_DESC ret = {};
  ret.DepthEnable = (BOOL)pDesc->depth_test;
  ret.DepthWriteMask = pDesc->depth_write ? D3D12_DEPTH_WRITE_MASK_ALL
                                          : D3D12_DEPTH_WRITE_MASK_ZERO;
  ret.DepthFunc = gDx12ComparisonFuncTranslator[pDesc->depth_func];
  ret.StencilEnable = (BOOL)pDesc->stencil_test;
  ret.StencilReadMask = pDesc->stencil_read_mask;
  ret.StencilWriteMask = pDesc->stencil_write_mask;
  ret.BackFace.StencilFunc =
      gDx12ComparisonFuncTranslator[pDesc->stencil_back_func];
  ret.FrontFace.StencilFunc =
      gDx12ComparisonFuncTranslator[pDesc->stencil_front_func];
  ret.BackFace.StencilDepthFailOp =
      gDx12StencilOpTranslator[pDesc->depth_back_fail];
  ret.FrontFace.StencilDepthFailOp =
      gDx12StencilOpTranslator[pDesc->depth_front_fail];
  ret.BackFace.StencilFailOp =
      gDx12StencilOpTranslator[pDesc->stencil_back_fail];
  ret.FrontFace.StencilFailOp =
      gDx12StencilOpTranslator[pDesc->stencil_front_fail];
  ret.BackFace.StencilPassOp =
      gDx12StencilOpTranslator[pDesc->stencil_back_pass];
  ret.FrontFace.StencilPassOp =
      gDx12StencilOpTranslator[pDesc->stencil_front_pass];

  return ret;
}

FORCEINLINE static D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Util_TranslatePrimitiveTopology(ECGPUPrimitiveTopology topology) {
switch (topology) {
  case CGPU_PRIM_TOPO_POINT_LIST:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
  case CGPU_PRIM_TOPO_LINE_LIST:
  case CGPU_PRIM_TOPO_LINE_STRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
  case CGPU_PRIM_TOPO_TRI_LIST:
  case CGPU_PRIM_TOPO_TRI_STRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  case CGPU_PRIM_TOPO_PATCH_LIST:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
default:
  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
  }
  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

FORCEINLINE static D3D12_RESOURCE_STATES D3D12Util_TranslateResourceState(CGPUResourceStates state)
{
    D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

    // These states cannot be combined with other states so we just do an == check
    if (state == CGPU_RESOURCE_STATE_GENERIC_READ)
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    if (state == CGPU_RESOURCE_STATE_COMMON)
        return D3D12_RESOURCE_STATE_COMMON;
    if (state == CGPU_RESOURCE_STATE_PRESENT)
        return D3D12_RESOURCE_STATE_PRESENT;

    if (state & CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
        ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (state & CGPU_RESOURCE_STATE_RESOLVE_DEST)
        ret |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
    if (state & CGPU_RESOURCE_STATE_INDEX_BUFFER)
        ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if (state & CGPU_RESOURCE_STATE_RENDER_TARGET)
        ret |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (state & CGPU_RESOURCE_STATE_UNORDERED_ACCESS)
        ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (state & CGPU_RESOURCE_STATE_DEPTH_WRITE)
        ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
    if (state & CGPU_RESOURCE_STATE_DEPTH_READ)
        ret |= D3D12_RESOURCE_STATE_DEPTH_READ;
    if (state & CGPU_RESOURCE_STATE_STREAM_OUT)
        ret |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if (state & CGPU_RESOURCE_STATE_INDIRECT_ARGUMENT)
        ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    if (state & CGPU_RESOURCE_STATE_COPY_DEST)
        ret |= D3D12_RESOURCE_STATE_COPY_DEST;
    if (state & CGPU_RESOURCE_STATE_COPY_SOURCE)
        ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (state & CGPU_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
        ret |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    if (state & CGPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        ret |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
#ifdef ENABLE_RAYTRACING
    if (state & CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE)
        ret |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif
#ifdef ENABLE_VRS
    if (state & CGPU_RESOURCE_STATE_SHADING_RATE_SOURCE)
        ret |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
#endif
    return ret;
}

FORCEINLINE static D3D12_SHADER_VISIBILITY D3D12Util_TranslateShaderStages(CGPUShaderStages stages) {
  D3D12_SHADER_VISIBILITY res = D3D12_SHADER_VISIBILITY_ALL;
  uint32_t stageCount = 0;
  if (stages == CGPU_SHADER_STAGE_COMPUTE) {
    return D3D12_SHADER_VISIBILITY_ALL;
  }
  if (stages & CGPU_SHADER_STAGE_VERT) {
    res = D3D12_SHADER_VISIBILITY_VERTEX;
    ++stageCount;
  }
  if (stages & CGPU_SHADER_STAGE_GEOM) {
    res = D3D12_SHADER_VISIBILITY_GEOMETRY;
    ++stageCount;
  }
  if (stages & CGPU_SHADER_STAGE_HULL) {
    res = D3D12_SHADER_VISIBILITY_HULL;
    ++stageCount;
  }
  if (stages & CGPU_SHADER_STAGE_DOMAIN) {
    res = D3D12_SHADER_VISIBILITY_DOMAIN;
    ++stageCount;
  }
  if (stages & CGPU_SHADER_STAGE_FRAG) {
    res = D3D12_SHADER_VISIBILITY_PIXEL;
    ++stageCount;
  }
  if (stages == CGPU_SHADER_STAGE_RAYTRACING) {
    return D3D12_SHADER_VISIBILITY_ALL;
  }
  cgpu_assert(stageCount > 0);
  return stageCount > 1 ? D3D12_SHADER_VISIBILITY_ALL : res;
}


FORCEINLINE static DXGI_FORMAT DXGIUtil_TranslatePixelFormat(const ECGPUFormat fmt, bool ShaderResource = false)
{
	switch (fmt) {
	case CGPU_FORMAT_R1_UNORM: return DXGI_FORMAT_R1_UNORM;
	case CGPU_FORMAT_R5G6B5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
	case CGPU_FORMAT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
	case CGPU_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
	case CGPU_FORMAT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
	case CGPU_FORMAT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
	case CGPU_FORMAT_R8_UINT: return DXGI_FORMAT_R8_UINT;
	case CGPU_FORMAT_R8_SINT: return DXGI_FORMAT_R8_SINT;
	case CGPU_FORMAT_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
	case CGPU_FORMAT_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
	case CGPU_FORMAT_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
	case CGPU_FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
	case CGPU_FORMAT_B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;

	case CGPU_FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case CGPU_FORMAT_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
	case CGPU_FORMAT_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
	case CGPU_FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
	case CGPU_FORMAT_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	case CGPU_FORMAT_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case CGPU_FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
	case CGPU_FORMAT_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

	case CGPU_FORMAT_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
	case CGPU_FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;

	case CGPU_FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
	case CGPU_FORMAT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
	case CGPU_FORMAT_R16_UINT: return DXGI_FORMAT_R16_UINT;
	case CGPU_FORMAT_R16_SINT: return DXGI_FORMAT_R16_SINT;
	case CGPU_FORMAT_R16_SFLOAT: return DXGI_FORMAT_R16_FLOAT;
	case CGPU_FORMAT_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
	case CGPU_FORMAT_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
	case CGPU_FORMAT_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
	case CGPU_FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
	case CGPU_FORMAT_R16G16_SFLOAT: return DXGI_FORMAT_R16G16_FLOAT;
	case CGPU_FORMAT_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
	case CGPU_FORMAT_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
	case CGPU_FORMAT_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
	case CGPU_FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
	case CGPU_FORMAT_R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case CGPU_FORMAT_R32_UINT: return DXGI_FORMAT_R32_UINT;
	case CGPU_FORMAT_R32_SINT: return DXGI_FORMAT_R32_SINT;
	case CGPU_FORMAT_R32_SFLOAT: return DXGI_FORMAT_R32_FLOAT;
	case CGPU_FORMAT_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
	case CGPU_FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
	case CGPU_FORMAT_R32G32_SFLOAT: return DXGI_FORMAT_R32G32_FLOAT;
	case CGPU_FORMAT_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
	case CGPU_FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
	case CGPU_FORMAT_R32G32B32_SFLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
	case CGPU_FORMAT_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
	case CGPU_FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
	case CGPU_FORMAT_R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case CGPU_FORMAT_B10G11R11_UFLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
	case CGPU_FORMAT_E5B9G9R9_UFLOAT: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	case CGPU_FORMAT_D16_UNORM: return ShaderResource ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_D16_UNORM;
	case CGPU_FORMAT_X8_D24_UNORM: return ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
	case CGPU_FORMAT_D32_SFLOAT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT :DXGI_FORMAT_D32_FLOAT;
	case CGPU_FORMAT_D24_UNORM_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS :DXGI_FORMAT_D24_UNORM_S8_UINT;
	case CGPU_FORMAT_D32_SFLOAT_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS :DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case CGPU_FORMAT_DXBC1_RGB_UNORM: return DXGI_FORMAT_BC1_UNORM;
	case CGPU_FORMAT_DXBC1_RGB_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
	case CGPU_FORMAT_DXBC1_RGBA_UNORM: return DXGI_FORMAT_BC1_UNORM;
	case CGPU_FORMAT_DXBC1_RGBA_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
	case CGPU_FORMAT_DXBC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
	case CGPU_FORMAT_DXBC2_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
	case CGPU_FORMAT_DXBC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
	case CGPU_FORMAT_DXBC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
	case CGPU_FORMAT_DXBC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
	case CGPU_FORMAT_DXBC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
	case CGPU_FORMAT_DXBC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
	case CGPU_FORMAT_DXBC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
	case CGPU_FORMAT_DXBC6H_UFLOAT: return DXGI_FORMAT_BC6H_UF16;
	case CGPU_FORMAT_DXBC6H_SFLOAT: return DXGI_FORMAT_BC6H_SF16;
	case CGPU_FORMAT_DXBC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
	case CGPU_FORMAT_DXBC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

	case CGPU_FORMAT_D16_UNORM_S8_UINT:
	case CGPU_FORMAT_R4G4_UNORM: 
	default: return DXGI_FORMAT_UNKNOWN;
	}
	return DXGI_FORMAT_UNKNOWN;
}

FORCEINLINE static ECGPUFormat DXGIUtil_FormatToCGPU(DXGI_FORMAT fmt) {
	switch (fmt) {
	case DXGI_FORMAT_R1_UNORM: return CGPU_FORMAT_R1_UNORM;
	case DXGI_FORMAT_B5G6R5_UNORM: return CGPU_FORMAT_B5G6R5_UNORM;
	case DXGI_FORMAT_B5G5R5A1_UNORM: return CGPU_FORMAT_B5G5R5A1_UNORM;

	case DXGI_FORMAT_R8_UNORM: return CGPU_FORMAT_R8_UNORM;
	case DXGI_FORMAT_R8_SNORM: return CGPU_FORMAT_R8_SNORM;
	case DXGI_FORMAT_R8_UINT: return CGPU_FORMAT_R8_UINT;
	case DXGI_FORMAT_R8_SINT: return CGPU_FORMAT_R8_SINT;
	case DXGI_FORMAT_R8_TYPELESS: return CGPU_FORMAT_R8_UNORM;

	case DXGI_FORMAT_R8G8_UNORM: return CGPU_FORMAT_R8G8_UNORM;
	case DXGI_FORMAT_R8G8_SNORM: return CGPU_FORMAT_R8G8_SNORM;
	case DXGI_FORMAT_R8G8_UINT: return CGPU_FORMAT_R8G8_UINT;
	case DXGI_FORMAT_R8G8_SINT: return CGPU_FORMAT_R8G8_SINT;
	case DXGI_FORMAT_R8G8_TYPELESS: return CGPU_FORMAT_R8G8_UNORM;

	case DXGI_FORMAT_B4G4R4A4_UNORM: return CGPU_FORMAT_B4G4R4A4_UNORM;

	case DXGI_FORMAT_R8G8B8A8_UNORM: return CGPU_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_SNORM: return CGPU_FORMAT_R8G8B8A8_SNORM;
	case DXGI_FORMAT_R8G8B8A8_UINT: return CGPU_FORMAT_R8G8B8A8_UINT;
	case DXGI_FORMAT_R8G8B8A8_SINT: return CGPU_FORMAT_R8G8B8A8_SINT;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return CGPU_FORMAT_R8G8B8A8_SRGB;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS: return CGPU_FORMAT_R8G8B8A8_UNORM;

	case DXGI_FORMAT_B8G8R8A8_UNORM: return CGPU_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_UNORM: return CGPU_FORMAT_B8G8R8X8_UNORM;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return CGPU_FORMAT_B8G8R8A8_SRGB;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS: return CGPU_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS: return CGPU_FORMAT_B8G8R8A8_UNORM;

	case DXGI_FORMAT_R10G10B10A2_UNORM: return CGPU_FORMAT_R10G10B10A2_UNORM;
	case DXGI_FORMAT_R10G10B10A2_UINT: return CGPU_FORMAT_R10G10B10A2_UINT;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS: return CGPU_FORMAT_R10G10B10A2_UNORM;

	case DXGI_FORMAT_R16_UNORM: return CGPU_FORMAT_R16_UNORM;
	case DXGI_FORMAT_R16_SNORM: return CGPU_FORMAT_R16_SNORM;
	case DXGI_FORMAT_R16_UINT: return CGPU_FORMAT_R16_UINT;
	case DXGI_FORMAT_R16_SINT: return CGPU_FORMAT_R16_SINT;
	case DXGI_FORMAT_R16_FLOAT: return CGPU_FORMAT_R16_SFLOAT;
	case DXGI_FORMAT_R16_TYPELESS: return CGPU_FORMAT_R16_UNORM;

	case DXGI_FORMAT_R16G16_UNORM: return CGPU_FORMAT_R16G16_UNORM;
	case DXGI_FORMAT_R16G16_SNORM: return CGPU_FORMAT_R16G16_SNORM;
	case DXGI_FORMAT_R16G16_UINT: return CGPU_FORMAT_R16G16_UINT;
	case DXGI_FORMAT_R16G16_SINT: return CGPU_FORMAT_R16G16_SINT;
	case DXGI_FORMAT_R16G16_FLOAT: return CGPU_FORMAT_R16G16_SFLOAT;
	case DXGI_FORMAT_R16G16_TYPELESS: return CGPU_FORMAT_R16G16_SFLOAT;

	case DXGI_FORMAT_R16G16B16A16_UNORM: return CGPU_FORMAT_R16G16B16A16_UNORM;
	case DXGI_FORMAT_R16G16B16A16_SNORM: return CGPU_FORMAT_R16G16B16A16_SNORM;
	case DXGI_FORMAT_R16G16B16A16_UINT: return CGPU_FORMAT_R16G16B16A16_UINT;
	case DXGI_FORMAT_R16G16B16A16_SINT: return CGPU_FORMAT_R16G16B16A16_SINT;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return CGPU_FORMAT_R16G16B16A16_SFLOAT;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS: return CGPU_FORMAT_R16G16B16A16_SFLOAT;

	case DXGI_FORMAT_R32_UINT: return CGPU_FORMAT_R32_UINT;
	case DXGI_FORMAT_R32_SINT: return CGPU_FORMAT_R32_SINT;
	case DXGI_FORMAT_R32_FLOAT: return CGPU_FORMAT_R32_SFLOAT;
	case DXGI_FORMAT_R32_TYPELESS: return CGPU_FORMAT_R32_SFLOAT;

	case DXGI_FORMAT_R32G32_UINT: return CGPU_FORMAT_R32G32_UINT;
	case DXGI_FORMAT_R32G32_SINT: return CGPU_FORMAT_R32G32_SINT;
	case DXGI_FORMAT_R32G32_FLOAT: return CGPU_FORMAT_R32G32_SFLOAT;
	case DXGI_FORMAT_R32G32_TYPELESS: return CGPU_FORMAT_R32G32_SFLOAT;

	case DXGI_FORMAT_R32G32B32_UINT: return CGPU_FORMAT_R32G32B32_UINT;
	case DXGI_FORMAT_R32G32B32_SINT: return CGPU_FORMAT_R32G32B32_SINT;
	case DXGI_FORMAT_R32G32B32_FLOAT: return CGPU_FORMAT_R32G32B32_SFLOAT;
	case DXGI_FORMAT_R32G32B32_TYPELESS: return CGPU_FORMAT_R32G32B32_SFLOAT;

	case DXGI_FORMAT_R32G32B32A32_UINT: return CGPU_FORMAT_R32G32B32A32_UINT;
	case DXGI_FORMAT_R32G32B32A32_SINT: return CGPU_FORMAT_R32G32B32A32_SINT;
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return CGPU_FORMAT_R32G32B32A32_SFLOAT;
	case DXGI_FORMAT_R32G32B32A32_TYPELESS: return CGPU_FORMAT_R32G32B32A32_SFLOAT;
	case DXGI_FORMAT_R11G11B10_FLOAT: return CGPU_FORMAT_B10G11R11_UFLOAT;

	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return CGPU_FORMAT_E5B9G9R9_UFLOAT;
	case DXGI_FORMAT_D16_UNORM: return CGPU_FORMAT_D16_UNORM;
	case DXGI_FORMAT_D32_FLOAT: return CGPU_FORMAT_D32_SFLOAT;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return CGPU_FORMAT_D24_UNORM_S8_UINT;
	case DXGI_FORMAT_R24G8_TYPELESS: return CGPU_FORMAT_D24_UNORM_S8_UINT;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return CGPU_FORMAT_D32_SFLOAT_S8_UINT;
	case DXGI_FORMAT_R32G8X24_TYPELESS: return CGPU_FORMAT_D32_SFLOAT_S8_UINT;

	case DXGI_FORMAT_BC1_UNORM: return CGPU_FORMAT_DXBC1_RGB_UNORM;
	case DXGI_FORMAT_BC1_UNORM_SRGB: return CGPU_FORMAT_DXBC1_RGB_SRGB;
	case DXGI_FORMAT_BC1_TYPELESS: return CGPU_FORMAT_DXBC1_RGB_UNORM;

	case DXGI_FORMAT_BC2_UNORM: return CGPU_FORMAT_DXBC2_UNORM;
	case DXGI_FORMAT_BC2_UNORM_SRGB: return CGPU_FORMAT_DXBC2_SRGB;
	case DXGI_FORMAT_BC2_TYPELESS: return CGPU_FORMAT_DXBC2_UNORM;

	case DXGI_FORMAT_BC3_UNORM: return CGPU_FORMAT_DXBC3_UNORM;
	case DXGI_FORMAT_BC3_UNORM_SRGB: return CGPU_FORMAT_DXBC3_SRGB;
	case DXGI_FORMAT_BC3_TYPELESS: return CGPU_FORMAT_DXBC3_UNORM;

	case DXGI_FORMAT_BC4_UNORM: return CGPU_FORMAT_DXBC4_UNORM;
	case DXGI_FORMAT_BC4_SNORM: return CGPU_FORMAT_DXBC4_SNORM;
	case DXGI_FORMAT_BC4_TYPELESS: return CGPU_FORMAT_DXBC4_UNORM;

	case DXGI_FORMAT_BC5_UNORM: return CGPU_FORMAT_DXBC5_UNORM;
	case DXGI_FORMAT_BC5_SNORM: return CGPU_FORMAT_DXBC5_SNORM;
	case DXGI_FORMAT_BC5_TYPELESS: return CGPU_FORMAT_DXBC5_UNORM;

	case DXGI_FORMAT_BC6H_UF16: return CGPU_FORMAT_DXBC6H_UFLOAT;
	case DXGI_FORMAT_BC6H_SF16: return CGPU_FORMAT_DXBC6H_SFLOAT;
	case DXGI_FORMAT_BC6H_TYPELESS: return CGPU_FORMAT_DXBC6H_SFLOAT;

	case DXGI_FORMAT_BC7_UNORM: return CGPU_FORMAT_DXBC7_UNORM;
	case DXGI_FORMAT_BC7_UNORM_SRGB: return CGPU_FORMAT_DXBC7_SRGB;
	case DXGI_FORMAT_BC7_TYPELESS: return CGPU_FORMAT_DXBC7_SRGB;

	default: return CGPU_FORMAT_UNDEFINED;
	}
	return CGPU_FORMAT_UNDEFINED;
}

FORCEINLINE static DXGI_FORMAT DXGIUtil_FormatToTypeless(DXGI_FORMAT fmt) {
	switch (fmt) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_TYPELESS;

	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_TYPELESS;

	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_TYPELESS;

	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_TYPELESS;

	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_TYPELESS;

	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT: return DXGI_FORMAT_R32_TYPELESS;
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_TYPELESS;
	case DXGI_FORMAT_B4G4R4A4_UNORM: // just treats a 16 raw bits
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT: return DXGI_FORMAT_R16_TYPELESS;
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT: return DXGI_FORMAT_R8_TYPELESS;
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_TYPELESS;
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_TYPELESS;
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_TYPELESS;
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_TYPELESS;
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_TYPELESS;
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_R16_TYPELESS;

	case DXGI_FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;

	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_TYPELESS;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;

	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_TYPELESS;

	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_TYPELESS;

	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;

		// typeless just return the input format
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC7_TYPELESS: return fmt;

	case DXGI_FORMAT_R1_UNORM:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_YUY2:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
	case DXGI_FORMAT_NV11:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_P208:
	case DXGI_FORMAT_V208:
	case DXGI_FORMAT_V408:
	case DXGI_FORMAT_UNKNOWN:
    default:
    return DXGI_FORMAT_UNKNOWN;
	}
	return DXGI_FORMAT_UNKNOWN;
}