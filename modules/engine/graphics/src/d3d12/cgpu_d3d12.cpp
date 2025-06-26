#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrBase/misc/defer.hpp"
#include "SkrGraphics/containers.hpp"
#include "SkrGraphics/backend/d3d12/cgpu_d3d12.h"
#include "SkrGraphics/drivers/cgpu_nvapi.h"

#include "d3d12_utils.h"
#include <dxcapi.h>

#include "SkrProfile/profile.h"

// Inline Utils
D3D12_RESOURCE_DESC      D3D12Util_CreateBufferDesc(CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D, const struct CGPUBufferDescriptor* desc);
D3D12MA::ALLOCATION_DESC D3D12Util_CreateAllocationDesc(const struct CGPUBufferDescriptor* desc);

inline D3D12_HEAP_TYPE D3D12Util_TranslateHeapType(ECGPUMemoryUsage usage)
{
    if (usage == CGPU_MEM_USAGE_CPU_ONLY || usage == CGPU_MEM_USAGE_CPU_TO_GPU)
        return D3D12_HEAP_TYPE_UPLOAD;
    else if (usage == CGPU_MEM_USAGE_GPU_TO_CPU)
        return D3D12_HEAP_TYPE_READBACK;
    else
        return D3D12_HEAP_TYPE_DEFAULT;
}

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

        int32_t expect_mapped = D3D12_TILE_MAPPING_STATUS_MAPPED;
        if (skr_atomic_compare_exchange_strong(&mapping->status,
                                        &expect_mapped, D3D12_TILE_MAPPING_STATUS_UNMAPPING))
        {
            SAFE_RELEASE(mapping->pDxAllocation);
            skr_atomic_fetch_add_relaxed(&pTiledInfo->alive_tiles_count, -1);
        }

        int32_t expect_unmapping = D3D12_TILE_MAPPING_STATUS_UNMAPPING;
        skr_atomic_compare_exchange_strong(&mapping->status,
                                 &expect_unmapping, D3D12_TILE_MAPPING_STATUS_UNMAPPED);
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
        int32_t expect_mapped = D3D12_TILE_MAPPING_STATUS_MAPPED;
        if (skr_atomic_compare_exchange_strong(&status,
                &expect_mapped, D3D12_TILE_MAPPING_STATUS_UNMAPPING))
        {
            SAFE_RELEASE(pAllocation);
            skr_atomic_fetch_add_relaxed(&pTiledInfo->alive_tiles_count, -1);
        }

        int32_t expect_unmapping = D3D12_TILE_MAPPING_STATUS_UNMAPPING;
        skr_atomic_compare_exchange_strong(&status,
                                 &expect_unmapping, D3D12_TILE_MAPPING_STATUS_UNMAPPED);
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

CGPUMemoryPoolId cgpu_create_memory_pool_d3d12(CGPUDeviceId device, const struct CGPUMemoryPoolDescriptor* desc)
{
    CGPUDevice_D3D12*     D    = (CGPUDevice_D3D12*)device;
    CGPUMemoryPool_D3D12* pool = CGPU_NULLPTR;

    D3D12MA::POOL_DESC poolDesc = {};
    switch (desc->type)
    {
        case CGPU_MEM_POOL_TYPE_TILED:
            poolDesc.HeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
            pool = cgpu_new<CGPUTiledMemoryPool_D3D12>();
            break;
        default:
            poolDesc.HeapFlags = D3D12_HEAP_FLAG_NONE;
            pool = cgpu_new<CGPUMemoryPool_D3D12>();
            break;
    }
    poolDesc.HeapProperties.Type = D3D12Util_TranslateHeapType(desc->memory_usage);
    poolDesc.MinAllocationAlignment = desc->min_alloc_alignment;
    poolDesc.BlockSize = desc->block_size;
    poolDesc.MinBlockCount = desc->min_block_count;
    poolDesc.MaxBlockCount = desc->max_block_count;
    auto hres = D->pResourceAllocator->CreatePool(&poolDesc, &pool->pDxPool);
    CHECK_HRESULT(hres);

    pool->super.device = device;
    pool->super.type   = desc->type;
    return &pool->super;
}

void cgpu_free_memory_pool_d3d12(CGPUMemoryPoolId pool)
{
    switch (pool->type)
    {
        case CGPU_MEM_POOL_TYPE_TILED:
            cgpu_delete((CGPUTiledMemoryPool_D3D12*)pool);
            break;
        default:
            cgpu_delete((CGPUMemoryPool_D3D12*)pool);
            break;
    }
}

// Buffer APIs
cgpu_static_assert(sizeof(CGPUBuffer_D3D12) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line
CGPUBufferId cgpu_create_buffer_d3d12(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc)
{
    CGPUBuffer_D3D12*   B              = cgpu_new_sized<CGPUBuffer_D3D12>(sizeof(CGPUBuffer_D3D12) + sizeof(CGPUBufferInfo));
    auto                pInfo          = (CGPUBufferInfo*)(B + 1);
    CGPUDevice_D3D12*   D              = (CGPUDevice_D3D12*)device;
    CGPUAdapter_D3D12*  A              = (CGPUAdapter_D3D12*)device->adapter;
    D3D12_RESOURCE_DESC bufDesc        = D3D12Util_CreateBufferDesc(A, D, desc);
    uint64_t            allocationSize = bufDesc.Width;
    // Handle Resource Start State
    ECGPUResourceState start_state = desc->start_state;
    if (desc->memory_usage == CGPU_MEM_USAGE_CPU_TO_GPU || desc->memory_usage == CGPU_MEM_USAGE_CPU_ONLY)
    {
        start_state = CGPU_RESOURCE_STATE_GENERIC_READ;
    }
    else if (desc->memory_usage == CGPU_MEM_USAGE_GPU_TO_CPU)
    {
        start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    }
    D3D12_RESOURCE_STATES InitialState = D3D12Util_TranslateResourceState(start_state);
    // Do Allocation
    const bool               log_allocation = false;
    D3D12MA::ALLOCATION_DESC alloc_desc     = D3D12Util_CreateAllocationDesc(desc);
#ifdef CGPU_USE_NVAPI
    if ((desc->memory_usage == CGPU_MEM_USAGE_GPU_ONLY && desc->flags & CGPU_BCF_HOST_VISIBLE) ||
        (desc->memory_usage & CGPU_MEM_USAGE_GPU_ONLY && desc->flags == CGPU_BCF_PERSISTENT_MAP_BIT))
    {
        bool                  cpuVisibleVRamSupported = false;
        D3D12_HEAP_PROPERTIES heapProps               = {};
        heapProps.Type                                = D3D12_HEAP_TYPE_CUSTOM;
        heapProps.CPUPageProperty                     = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
        heapProps.MemoryPoolPreference                = D3D12_MEMORY_POOL_L0;
        heapProps.VisibleNodeMask                     = CGPU_SINGLE_GPU_NODE_MASK;
        heapProps.CreationNodeMask                    = CGPU_SINGLE_GPU_NODE_MASK;
        NV_RESOURCE_PARAMS nvParams                   = {};
        nvParams.NVResourceFlags                      = NV_D3D12_RESOURCE_FLAGS::NV_D3D12_RESOURCE_FLAG_CPUVISIBLE_VIDMEM;
        if (InitialState == D3D12_RESOURCE_STATE_GENERIC_READ)
            InitialState = D3D12_RESOURCE_STATE_COMMON; // [STATE_CREATION WARNING #1328: CREATERESOURCE_STATE_IGNORED]
        NvAPI_D3D12_CreateCommittedResource(D->pDxDevice, &heapProps,
                                            alloc_desc.ExtraHeapFlags,
                                            &bufDesc, InitialState,
                                            nullptr, &nvParams, IID_ARGS(&B->pDxResource),
                                            &cpuVisibleVRamSupported);
        if (!cpuVisibleVRamSupported)
            B->pDxResource = nullptr;
        else if (log_allocation)
        {
            cgpu_trace(u8"[D3D12] Create CVV Buffer Resource Succeed! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d",
                       desc->name ? desc->name : u8"", allocationSize, desc->format);
        }
    }
#endif
    if (!B->pDxResource)
    {
        if (D3D12_HEAP_TYPE_DEFAULT != alloc_desc.HeapType &&
            (bufDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
        {
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type                  = D3D12_HEAP_TYPE_CUSTOM;
            heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_L0;
            heapProps.VisibleNodeMask       = CGPU_SINGLE_GPU_NODE_MASK;
            heapProps.CreationNodeMask      = CGPU_SINGLE_GPU_NODE_MASK;
            if (InitialState == D3D12_RESOURCE_STATE_GENERIC_READ)
                InitialState = D3D12_RESOURCE_STATE_COMMON; // [STATE_CREATION WARNING #1328: CREATERESOURCE_STATE_IGNORED]
            CHECK_HRESULT(D->pDxDevice->CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags,
                                                                &bufDesc, InitialState, NULL, IID_ARGS(&B->pDxResource)));
            if (log_allocation)
            {
                cgpu_trace(u8"[D3D12] Create Committed Buffer Resource Succeed! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d",
                           desc->name ? desc->name : u8"", allocationSize, desc->format);
            }
        }
        else
        {
            {
                SkrZoneScopedN("Allocation(Buffer)");
                CHECK_HRESULT(D->pResourceAllocator->CreateResource(&alloc_desc, &bufDesc, InitialState,
                                                                    NULL, &B->pDxAllocation, IID_ARGS(&B->pDxResource)));
            }
            if (log_allocation)
            {
                SkrZoneScopedN("Log(Allocation)");
                cgpu_trace(u8"[D3D12] Create Buffer Resource Succeed! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d",
                           desc->name ? desc->name : u8"", allocationSize, desc->format);
            }
        }
    }

    // MemMaps
    if (desc->flags & CGPU_BCF_PERSISTENT_MAP_BIT)
    {
        SkrZoneScopedN("Map(Buffer)");

        auto mapResult = B->pDxResource->Map(0, NULL, &pInfo->cpu_mapped_address);
        if (!SUCCEEDED(mapResult))
        {
            cgpu_warn(u8"[D3D12] Map Buffer Resource Failed %d! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d",
                      mapResult, desc->name ? desc->name : u8"", allocationSize, desc->format);
        }
    }
    B->mDxGpuAddress = B->pDxResource->GetGPUVirtualAddress();
#if defined(XBOX)
    B->super.cpu_mapped_address->pCpuMappedAddress = (void*)B->mDxGpuAddress;
#endif

    // Create Descriptors
    if (!(desc->flags & CGPU_BCF_NO_DESCRIPTOR_VIEW_CREATION))
    {
        D3D12Util_DescriptorHeap* pHeap = D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
        const size_t kDescriptorSize = D3D12Util_GetDescriptorSize(pHeap);

        uint32_t handleCount = ((desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER) ? 1 : 0) +
                               ((desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER) ? 1 : 0) +
                               ((desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER) ? 1 : 0);
        B->mDxDescriptorHandles = D3D12Util_ConsumeDescriptorHandles(pHeap, handleCount).mCpu;
        // Create CBV
        if (desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE cbv = { B->mDxDescriptorHandles.ptr };
            B->mDxSrvOffset = kDescriptorSize * 1;

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = B->mDxGpuAddress;
            cbvDesc.SizeInBytes = (UINT)allocationSize;
            D3D12Util_CreateCBV(D, &cbvDesc, &cbv);
        }
        // Create SRV
        if (desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srv = { B->mDxDescriptorHandles.ptr + B->mDxSrvOffset };
            B->mDxUavOffset = B->mDxSrvOffset + kDescriptorSize * 1;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Buffer.FirstElement = desc->first_element;
            srvDesc.Buffer.NumElements = (UINT)(desc->element_count);
            srvDesc.Buffer.StructureByteStride = (UINT)(desc->element_stride);
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            srvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, false);
            if (CGPU_RESOURCE_TYPE_BUFFER_RAW == (desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER_RAW))
            {
                if (desc->format != CGPU_FORMAT_UNDEFINED)
                {
                    cgpu_warn(u8"Raw buffers use R32 typeless format. Format will be ignored");
                }
                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
            }
            // Cannot create a typed StructuredBuffer
            if (srvDesc.Format != DXGI_FORMAT_UNKNOWN)
            {
                srvDesc.Buffer.StructureByteStride = 0;
            }
            D3D12Util_CreateSRV(D, B->pDxResource, &srvDesc, &srv);
        }
        // Create UAV
        if (desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE uav = { B->mDxDescriptorHandles.ptr + B->mDxUavOffset };

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = desc->first_element;
            uavDesc.Buffer.NumElements = (UINT)(desc->element_count);
            uavDesc.Buffer.StructureByteStride = (UINT)(desc->element_stride);
            uavDesc.Buffer.CounterOffsetInBytes = 0;
            uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_NONE;
            if (CGPU_RESOURCE_TYPE_RW_BUFFER_RAW == (desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER_RAW))
            {
                if (desc->format != CGPU_FORMAT_UNDEFINED)
                {
                    cgpu_warn(u8"Raw buffers use R32 typeless format. Format will be ignored");
                }
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
            }
            else if (desc->format != CGPU_FORMAT_UNDEFINED)
            {
                uavDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, false);
                D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = {
                    uavDesc.Format,
                    D3D12_FORMAT_SUPPORT1_NONE,
                    D3D12_FORMAT_SUPPORT2_NONE
                };
                HRESULT hr = D->pDxDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
                if (!SUCCEEDED(hr) || !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) ||
                    !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE))
                {
                    // Format does not support UAV Typed Load
                    cgpu_warn(u8"Cannot use Typed UAV for buffer format %u", (uint32_t)desc->format);
                    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                }
            }
            // Cannot create a typed RWStructuredBuffer
            if (uavDesc.Format != DXGI_FORMAT_UNKNOWN)
            {
                uavDesc.Buffer.StructureByteStride = 0;
            }
            CGPUBuffer_D3D12* pCountBuffer     = (CGPUBuffer_D3D12*)desc->count_buffer;
            ID3D12Resource*   pCounterResource = pCountBuffer ? pCountBuffer->pDxResource : NULL;
            D3D12Util_CreateUAV(D, B->pDxResource, pCounterResource, &uavDesc, &uav);
        }
    }
    // Set Debug Name
    if (device->adapter->instance->enable_set_name && desc->name)
    {
        wchar_t debugName[MAX_GPU_DEBUG_NAME_LENGTH] = {};
        mbstowcs(debugName, (const char*)desc->name, MAX_GPU_DEBUG_NAME_LENGTH);
        if (B->pDxAllocation)
        {
            B->pDxAllocation->SetName(debugName);
        }
        B->pDxResource->SetName(debugName);
    }

    // Set Buffer Object Props
    B->super.info = pInfo;
    pInfo->size = allocationSize;
    pInfo->memory_usage = desc->memory_usage;
    pInfo->descriptors = desc->descriptors;
    return &B->super;
}

void cgpu_map_buffer_d3d12(CGPUBufferId buffer, const struct CGPUBufferRange* range)
{
    CGPUBuffer_D3D12* B = (CGPUBuffer_D3D12*)buffer;
    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)B->super.info;
    cgpu_assert(pInfo->memory_usage != CGPU_MEM_USAGE_GPU_ONLY && "Trying to map non-cpu accessible resource");

    D3D12_RANGE dxrange = { 0, pInfo->size };
    if (range)
    {
        dxrange.Begin += range->offset;
        dxrange.End = dxrange.Begin + range->size;
    }
    CHECK_HRESULT(B->pDxResource->Map(0, &dxrange, &pInfo->cpu_mapped_address));
}

void cgpu_unmap_buffer_d3d12(CGPUBufferId buffer)
{
    CGPUBuffer_D3D12* B = (CGPUBuffer_D3D12*)buffer;
    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)B->super.info;
    cgpu_assert(pInfo->memory_usage != CGPU_MEM_USAGE_GPU_ONLY && "Trying to unmap non-cpu accessible resource");

    B->pDxResource->Unmap(0, NULL);
    pInfo->cpu_mapped_address = NULL;
}

void cgpu_free_buffer_d3d12(CGPUBufferId buffer)
{
    CGPUBuffer_D3D12* B = (CGPUBuffer_D3D12*)buffer;
    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)B->super.info;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)B->super.device;
    if (B->mDxDescriptorHandles.ptr != D3D12_GPU_VIRTUAL_ADDRESS_NULL)
    {
        uint32_t handleCount = ((pInfo->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER) ? 1 : 0) +
                               ((pInfo->descriptors & CGPU_RESOURCE_TYPE_BUFFER) ? 1 : 0) +
                               ((pInfo->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER) ? 1 : 0);
        D3D12Util_ReturnDescriptorHandles(
        D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], B->mDxDescriptorHandles,
        handleCount);
    }
    SAFE_RELEASE(B->pDxAllocation);
    SAFE_RELEASE(B->pDxResource);
    cgpu_delete(B);
}

CGPUTexture_D3D12::CGPUTexture_D3D12()
{
    memset(&super, 0, sizeof(super));
}

// Texture/TextureView APIs
inline D3D12_RESOURCE_DIMENSION D3D12Util_CalculateTextureDimension(const struct CGPUTextureDescriptor* desc)
{
    D3D12_RESOURCE_DIMENSION resDim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
    if (desc->flags & CGPU_TCF_FORCE_2D)
    {
        cgpu_assert(desc->depth == 1);
        resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    }
    else if (desc->flags & CGPU_TCF_FORCE_3D)
        resDim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    else
    {
        if (desc->depth > 1)
            resDim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        else if (desc->height > 1)
            resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        else
            resDim = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    }
    return resDim;
}

inline CGPUResourceStates D3D12Util_CalculateTextureStartState(const struct CGPUTextureDescriptor* desc)
{
    CGPUResourceStates start_state = 0;
    if (desc->start_state & CGPU_RESOURCE_STATE_COPY_DEST)
    {
        start_state = CGPU_RESOURCE_STATE_COMMON;
    }
    else if (desc->start_state & CGPU_RESOURCE_STATE_RENDER_TARGET)
    {
        start_state = (desc->start_state > CGPU_RESOURCE_STATE_RENDER_TARGET) ?
                      (desc->start_state & (ECGPUResourceState)~CGPU_RESOURCE_STATE_RENDER_TARGET) :
                      CGPU_RESOURCE_STATE_RENDER_TARGET;
    }
    else if (desc->start_state & CGPU_RESOURCE_STATE_DEPTH_WRITE)
    {
        start_state = (desc->start_state > CGPU_RESOURCE_STATE_DEPTH_WRITE) ?
                      (desc->start_state & (ECGPUResourceState)~CGPU_RESOURCE_STATE_DEPTH_WRITE) :
                      CGPU_RESOURCE_STATE_DEPTH_WRITE;
    }
    if (desc->flags & CGPU_TCF_ALLOW_DISPLAY_TARGET)
    {
        start_state = CGPU_RESOURCE_STATE_PRESENT;
    }
    return start_state;
}

inline uint32_t D3D12Util_CalculateTextureSampleCount(ID3D12Device* pDxDevice, DXGI_FORMAT Format, uint32_t Count)
{
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaFeature;
    msaaFeature.Format      = Format;
    msaaFeature.Flags       = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msaaFeature.SampleCount = Count;
    if (msaaFeature.SampleCount > 1)
    {
        pDxDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaFeature, sizeof(msaaFeature));
        while (msaaFeature.NumQualityLevels == 0 && msaaFeature.SampleCount > 0)
        {
            cgpu_warn(u8"Sample Count (%u) not supported. Trying a lower sample count (%u)",
                      msaaFeature.SampleCount, msaaFeature.SampleCount / 2);
            msaaFeature.SampleCount = Count / 2;
            pDxDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                           &msaaFeature, sizeof(msaaFeature));
        }
        return msaaFeature.SampleCount;
    }
    return 1;
}

inline CGPUTexture_D3D12* D3D12Util_AllocateFromAllocator(CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D, const struct CGPUTextureDescriptor* desc,
                                                          D3D12_RESOURCE_DESC resDesc, D3D12_RESOURCE_STATES startStates, const D3D12_CLEAR_VALUE* pClearValue)
{
    auto T                                 = cgpu_new_sized<CGPUTexture_D3D12>(sizeof(CGPUTexture_D3D12) + sizeof(CGPUTextureInfo));
    auto pInfo                             = (CGPUTextureInfo*)(T + 1);
    T->super.info                          = pInfo;
    ID3D12Resource*          pDxResource   = nullptr;
    D3D12MA::Allocation*     pDxAllocation = nullptr;
    D3D12MA::ALLOCATION_DESC allocDesc     = {};
    // Do allocation (TODO: mGPU)
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    // for smaller alignment that not suitable for MSAA
    if (desc->is_restrict_dedicated || desc->flags & CGPU_TCF_DEDICATED_BIT || desc->sample_count != CGPU_SAMPLE_COUNT_1)
    {
        allocDesc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
    }
    bool is_allocation_dedicated = allocDesc.Flags & D3D12MA::ALLOCATION_FLAG_COMMITTED;
    bool is_restrict_dedicated   = is_allocation_dedicated;
    if (!desc->is_restrict_dedicated && desc->sample_count == CGPU_SAMPLE_COUNT_1 && !(desc->flags & CGPU_TCF_EXPORT_BIT))
    {
        allocDesc.Flags |= D3D12MA::ALLOCATION_FLAG_CAN_ALIAS;
        is_restrict_dedicated = false;
    }
    bool can_alias_allocation = allocDesc.Flags & D3D12MA::ALLOCATION_FLAG_CAN_ALIAS;
    if (desc->flags & CGPU_TCF_EXPORT_BIT)
    {
        allocDesc.ExtraHeapFlags |= D3D12_HEAP_FLAG_SHARED;
    }

    auto hres = D->pResourceAllocator->CreateResource(
    &allocDesc, &resDesc, startStates, pClearValue,
    &pDxAllocation, IID_ARGS(&pDxResource));
    if (hres != S_OK)
    {
        auto fallbackHres = hres;
        cgpu_error(u8"[D3D12] Create Texture Resorce Failed With HRESULT %d! \n\t With Name: %s\n\t Size: %dx%d \n\t Format: %d \n\t Sample Count: %d",
                   hres,
                   desc->name ? desc->name : u8"", desc->width, desc->height,
                   desc->format, desc->sample_count);
        cgpu_error(u8"[D3D12] Format Support For this Format: RenderTarget %d Read %d Write %d",
                   A->adapter_detail.format_supports[desc->format].render_target_write,
                   A->adapter_detail.format_supports[desc->format].shader_read,
                   A->adapter_detail.format_supports[desc->format].shader_write);
        const bool use_fallback_commited = true;
        if (use_fallback_commited)
        {
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type                  = D3D12_HEAP_TYPE_DEFAULT;
            heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.CreationNodeMask      = CGPU_SINGLE_GPU_NODE_MASK;
            heapProps.VisibleNodeMask       = CGPU_SINGLE_GPU_NODE_MASK;
            fallbackHres                    = D->pDxDevice->CreateCommittedResource(&heapProps,
                                                                                    allocDesc.ExtraHeapFlags, &resDesc, startStates,
                                                                                    pClearValue, IID_ARGS(&pDxResource));
            if (fallbackHres == S_OK)
            {
                is_restrict_dedicated = true;
                can_alias_allocation  = false;
                pDxAllocation         = nullptr;
                cgpu_trace(u8"[D3D12] Create Texture With Fallback Driver API Succeed!");
            }
            else
            {
                cgpu_fatal(u8"[D3D12] Create Texture With Fallback Driver API Failed! Please Update Your Driver or Contact With us!");
            }
        }
        CHECK_HRESULT(fallbackHres);
        if (fallbackHres != S_OK) goto FAIL;
    }
    else
    {
        cgpu_trace(u8"[D3D12] Create Texture Resource Succeed! \n\t With Name: %s\n\t Size: %dx%d \n\t Format: %d \n\t Sample Count: %d",
                   desc->name ? desc->name : u8"", desc->width, desc->height,
                   desc->format, desc->sample_count);
        cgpu_trace(u8"[D3D12] Format Support For this Format: RenderTarget %d Read %d Write %d",
                   A->adapter_detail.format_supports[desc->format].render_target_write,
                   A->adapter_detail.format_supports[desc->format].shader_read,
                   A->adapter_detail.format_supports[desc->format].shader_write);
    }
    T->pDxAllocation               = pDxAllocation;
    T->pDxResource                 = pDxResource;
    pInfo->is_restrict_dedicated   = is_restrict_dedicated;
    pInfo->is_allocation_dedicated = is_allocation_dedicated;
    pInfo->can_alias               = can_alias_allocation || (desc->flags & CGPU_TCF_ALIASING_RESOURCE);
    pInfo->can_export              = (allocDesc.ExtraHeapFlags & D3D12_HEAP_FLAG_SHARED);
    return T;
FAIL:
    SAFE_RELEASE(pDxAllocation);
    SAFE_RELEASE(pDxResource);
    if (T) cgpu_delete(T);
    return nullptr;
}

struct CGPUTextureAliasing_D3D12 : public CGPUTexture_D3D12 {
    D3D12_RESOURCE_DESC mDxDesc;
    cgpu::String        name;
    CGPUTextureAliasing_D3D12(const D3D12_RESOURCE_DESC& dxDesc, const char8_t* name)
        : CGPUTexture_D3D12()
        , mDxDesc(dxDesc)
        , name(name)
    {
    }
};

inline CGPUTexture_D3D12* D3D12Util_AllocateAliasing(const struct CGPUTextureDescriptor* desc, D3D12_RESOURCE_DESC resDesc)
{
    auto T = cgpu_new_sized<CGPUTextureAliasing_D3D12>(
    sizeof(CGPUTextureAliasing_D3D12) + sizeof(CGPUTextureInfo),
    resDesc, desc->name);
    T->super.info = (CGPUTextureInfo*)(T + 1);
    return T;
}

inline static void alignedDivision(const D3D12_TILED_RESOURCE_COORDINATE& extent, const D3D12_TILED_RESOURCE_COORDINATE& granularity, D3D12_TILED_RESOURCE_COORDINATE* out)
{
    out->X = (extent.X / granularity.X + ((extent.X % granularity.X) ? 1u : 0u));
    out->Y = (extent.Y / granularity.Y + ((extent.Y % granularity.Y) ? 1u : 0u));
    out->Z = (extent.Z / granularity.Z + ((extent.Z % granularity.Z) ? 1u : 0u));
}

inline void D3D12Util_MapAllTilesAsUndefined(ID3D12CommandQueue* pDxQueue, ID3D12Resource* pDxResource, ID3D12Heap* pHeap)
{
    D3D12_TILE_RANGE_FLAGS RangeFlags  = D3D12_TILE_RANGE_FLAG_REUSE_SINGLE_TILE;
    UINT                   StartOffset = 0;
    pDxQueue->UpdateTileMappings(pDxResource, 1,
                                 NULL, NULL,
                                 pHeap, 1,
                                 &RangeFlags, &StartOffset,
                                 NULL, D3D12_TILE_MAPPING_FLAG_NONE);
}

cgpu_static_assert(sizeof(CGPUTiledTexture_D3D12) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line
inline CGPUTexture_D3D12* D3D12Util_AllocateTiled(CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D, const struct CGPUTextureDescriptor* desc,
                                                  D3D12_RESOURCE_DESC resDesc, D3D12_RESOURCE_STATES startStates, const D3D12_CLEAR_VALUE* pClearValue)
{
    ID3D12Resource* pDxResource = nullptr;
    auto            res         = D->pDxDevice->CreateReservedResource(
    &resDesc, startStates, pClearValue, IID_PPV_ARGS(&pDxResource));
    CHECK_HRESULT(res);
    SKR_ASSERT(resDesc.DepthOrArraySize == 1);
    uint32_t layers = resDesc.DepthOrArraySize;

    // query page informations
    UINT numTiles = 0;
    D3D12_PACKED_MIP_INFO packedMipInfo;
    D3D12_TILE_SHAPE tileShape        = {};
    UINT subresourceCount = resDesc.MipLevels;
    auto tilings = (D3D12_SUBRESOURCE_TILING*)cgpu_calloc(subresourceCount, sizeof(D3D12_SUBRESOURCE_TILING));
    SKR_DEFER({ cgpu_free(tilings); });
    D->pDxDevice->GetResourceTiling(pDxResource, &numTiles,
                                    &packedMipInfo, &tileShape,
                                    &subresourceCount, 0,
                                    tilings);

    const auto objSize = sizeof(CGPUTiledTexture_D3D12) + sizeof(CGPUTextureInfo) + sizeof(CGPUTiledTextureInfo);
    const auto subresSize = (sizeof(CGPUTiledSubresourceInfo) + sizeof(SubresTileMappings_D3D12)) * subresourceCount;
    const auto packedMipSize = sizeof(PackedMipMapping_D3D12);
    const auto totalPackedMipSize = layers * packedMipSize;
    const auto totalSize = objSize + subresSize + totalPackedMipSize;
    auto T = (CGPUTiledTexture_D3D12*)cgpu_calloc(1, totalSize);
    auto pInfo = (CGPUTextureInfo*)(T + 1);
    auto pTiledInfo = (CGPUTiledTextureInfo*)(pInfo + 1);
    auto pSubresInfo = (CGPUTiledSubresourceInfo*)(pTiledInfo + 1);
    auto pSubresMapping = (SubresTileMappings_D3D12*)(pSubresInfo + subresourceCount);
    auto pPackedMipsMapping = (PackedMipMapping_D3D12*)(pSubresMapping + subresourceCount);
    for (uint32_t i = 0; i < layers; i++)
    {
        new (pPackedMipsMapping + i) PackedMipMapping_D3D12(T, packedMipInfo.NumTilesForPackedMips);
    }
    new (T) CGPUTiledTexture_D3D12(pSubresMapping, pPackedMipsMapping, layers);

    pTiledInfo->total_tiles_count = numTiles;
    pTiledInfo->alive_tiles_count = 0;
    pTiledInfo->tile_size = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

    pTiledInfo->tile_width_in_texels = tileShape.WidthInTexels;
    pTiledInfo->tile_height_in_texels = tileShape.HeightInTexels;
    pTiledInfo->tile_depth_in_texels = tileShape.DepthInTexels;
    pTiledInfo->subresources = pSubresInfo;

    if (A->mTiledResourceTier <= D3D12_TILED_RESOURCES_TIER_1)
        pTiledInfo->pack_unaligned = true;
    else
        pTiledInfo->pack_unaligned = false;

    pTiledInfo->packed_mip_start = packedMipInfo.NumStandardMips;
    pTiledInfo->packed_mip_count = packedMipInfo.NumPackedMips;

    for (uint32_t i = 0; i < subresourceCount; i++)
    {
        auto& SubresInfo = pSubresInfo[i];
        SubresInfo.width_in_tiles = tilings[i].WidthInTiles;
        SubresInfo.height_in_tiles = tilings[i].HeightInTiles;
        SubresInfo.depth_in_tiles = tilings[i].DepthInTiles;
        SubresInfo.layer = 0;
        SubresInfo.mip_level = i;
        new (&pSubresMapping[i]) SubresTileMappings_D3D12(T, SubresInfo.width_in_tiles, SubresInfo.height_in_tiles, SubresInfo.depth_in_tiles);
    }
    T->super.info = pInfo;
    T->super.tiled_resource = pTiledInfo;
    T->pDxResource = pDxResource;
    // GPU reads or writes to NULL mappings are undefined under Tier1.
    if (desc->owner_queue && A->mTiledResourceTier <= D3D12_TILED_RESOURCES_TIER_1)
    {
        auto Q = (CGPUQueue_D3D12*)desc->owner_queue;
        D3D12Util_MapAllTilesAsUndefined(Q->pCommandQueue, T->pDxResource, D->pUndefinedTileHeap);
    }
    return T;
}

void cgpu_queue_map_packed_mips_d3d12(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)queue->device;
    CGPUQueue_D3D12*  Q = (CGPUQueue_D3D12*)queue;
    for (uint32_t i = 0; i < regions->packed_mip_count; i++)
    {
        CGPUTiledTexture_D3D12* T        = (CGPUTiledTexture_D3D12*)regions->packed_mips[i].texture;
        uint32_t                layer    = regions->packed_mips[i].layer;
        auto*                   pMapping = T->getPackedMipMapping(layer);
        
        int32_t expect_unmapped = D3D12_TILE_MAPPING_STATUS_MAPPED;
        if (!skr_atomic_compare_exchange_strong(&pMapping->status, 
                &expect_unmapped, D3D12_TILE_MAPPING_STATUS_PENDING)) 
            continue;

        D->pTiledMemoryPool->AllocateTiles(1, &pMapping->pAllocation, pMapping->N);

        int32_t expect_pending = D3D12_TILE_MAPPING_STATUS_PENDING;
        if (!skr_atomic_compare_exchange_strong(&pMapping->status, 
                &expect_pending, D3D12_TILE_MAPPING_STATUS_MAPPING)) continue;

        const auto                      HeapOffset       = (UINT32)pMapping->pAllocation->GetOffset();
        const auto                      firstSubresource = CALC_SUBRESOURCE_INDEX(T->super.tiled_resource->packed_mip_start, layer, 0, 1, 1);
        D3D12_TILED_RESOURCE_COORDINATE resourceRegionStartCoordinates{ 0, 0, 0, firstSubresource };
        D3D12_TILE_REGION_SIZE          resourceRegionSizes{ pMapping->N, FALSE, 0, 0, 0 };
        Q->pCommandQueue->UpdateTileMappings(
        T->pDxResource,
        1,
        &resourceRegionStartCoordinates,
        &resourceRegionSizes,
        pMapping->pAllocation->GetHeap(), // ID3D12Heap*
        pMapping->N,
        NULL, // All ranges are sequential tiles in the heap
        &HeapOffset,
        nullptr,
        D3D12_TILE_MAPPING_FLAG_NONE);

        int32_t expect_mapping = D3D12_TILE_MAPPING_STATUS_MAPPING;
        skr_atomic_compare_exchange_strong(&pMapping->status, &expect_mapping, D3D12_TILE_MAPPING_STATUS_MAPPED);
    }
}

void cgpu_queue_unmap_packed_mips_d3d12(CGPUQueueId queue, const struct CGPUTiledTexturePackedMips* regions)
{
    for (uint32_t i = 0; i < regions->packed_mip_count; i++)
    {
        CGPUTiledTexture_D3D12* T        = (CGPUTiledTexture_D3D12*)regions->packed_mips[i].texture;
        uint32_t                layer    = regions->packed_mips[i].layer;
        auto*                   pMapping = T->getPackedMipMapping(layer);
        pMapping->unmap();
    }
}

void cgpu_queue_map_tiled_texture_d3d12(CGPUQueueId queue, const struct CGPUTiledTextureRegions* regions)
{
    const auto                  kPageSize   = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    const uint32_t              RegionCount = regions->region_count;
    CGPUDevice_D3D12*           D           = (CGPUDevice_D3D12*)queue->device;
    CGPUQueue_D3D12*            Q           = (CGPUQueue_D3D12*)queue;
    CGPUTiledTexture_D3D12*     T           = (CGPUTiledTexture_D3D12*)regions->texture;
    const CGPUTiledTextureInfo* pTiledInfo  = T->super.tiled_resource;

    // calculate page count
    uint32_t TotalTileCount = 0;
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const auto& Region          = regions->regions[i];
        auto&       Mappings        = *T->getSubresTileMappings(Region.mip_level, Region.layer);
        uint32_t    RegionTileCount = 0;
        for (uint32_t x = Region.start.x; x < Region.end.x; x++)
            for (uint32_t y = Region.start.y; y < Region.end.y; y++)
                for (uint32_t z = Region.start.z; z < Region.end.z; z++)
                {
                    SKR_ASSERT(Region.mip_level < pTiledInfo->packed_mip_start &&
                               "cgpu_queue_map_tiled_texture_d3d12: Mip level must be less than packed mip start!");
                    auto&      Mapping = *Mappings.at(x, y, z);
                    int32_t expect_unmapped = D3D12_TILE_MAPPING_STATUS_MAPPED;
                    if (skr_atomic_compare_exchange_strong(&Mapping.status, 
                        &expect_unmapped, D3D12_TILE_MAPPING_STATUS_PENDING))
                    {
                        RegionTileCount += 1;
                    }
                }
        TotalTileCount += RegionTileCount;
    }
    if (!TotalTileCount) return;

    // allocate memory for arguments
    auto ArgsMemory = sakura_calloc(TotalTileCount,
                                    sizeof(D3D12_TILED_RESOURCE_COORDINATE) + sizeof(UINT) + sizeof(UINT) +
                                    sizeof(ID3D12Heap*) + sizeof(D3D12MA::Allocation*) + sizeof(struct TileMapping_D3D12*));
    SKR_DEFER({ sakura_free(ArgsMemory); });
    auto                  pTileCoordinates = (D3D12_TILED_RESOURCE_COORDINATE*)ArgsMemory;
    UINT*                 pRangeTileCounts = (UINT*)(pTileCoordinates + TotalTileCount);
    UINT*                 pRangeOffsets    = (UINT*)(pRangeTileCounts + TotalTileCount);
    ID3D12Heap**          ppHeaps          = (ID3D12Heap**)(pRangeOffsets + TotalTileCount);
    D3D12MA::Allocation** ppAllocations    = (D3D12MA::Allocation**)(ppHeaps + TotalTileCount);
    TileMapping_D3D12**   ppMappings       = (TileMapping_D3D12**)(ppAllocations + TotalTileCount);

    // do allocations
    D->pTiledMemoryPool->AllocateTiles(TotalTileCount, ppAllocations);
    for (uint32_t i = 0; i < TotalTileCount; i++)
    {
        auto pHeap = ppAllocations[i]->GetHeap();
        ppHeaps[i] = pHeap;
    }

    // calc mapping batch
    uint32_t AllocateTileCount = 0;
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const auto& Region      = regions->regions[i];
        const auto  SubresIndex = CALC_SUBRESOURCE_INDEX(Region.mip_level, Region.layer, 0, 1, 1);
        auto&       Mappings    = *T->getSubresTileMappings(Region.mip_level, Region.layer);
        for (uint32_t x = Region.start.x; x < Region.end.x; x++)
            for (uint32_t y = Region.start.y; y < Region.end.y; y++)
                for (uint32_t z = Region.start.z; z < Region.end.z; z++)
                {
                    auto&      Mapping = *Mappings.at(x, y, z);
                    int32_t expect_pending = D3D12_TILE_MAPPING_STATUS_PENDING;
                    if (!skr_atomic_compare_exchange_strong(&Mapping.status, 
                        &expect_pending, D3D12_TILE_MAPPING_STATUS_MAPPING)) 
                        continue; // skip if already mapped

                    // calc mapping args
                    Mapping.pDxAllocation                           = ppAllocations[AllocateTileCount];
                    ppMappings[AllocateTileCount]                   = &Mapping;
                    pTileCoordinates[AllocateTileCount]             = { x, y, z, {} };
                    pTileCoordinates[AllocateTileCount].Subresource = SubresIndex;
                    pRangeOffsets[AllocateTileCount]                = (uint32_t)(ppAllocations[AllocateTileCount]->GetOffset() / kPageSize);
                    pRangeTileCounts[AllocateTileCount]             = 1;
                    AllocateTileCount++;
                }
    }

    // do mapping
    const auto fnMap = [&](uint32_t N, uint32_t Offset, ID3D12Heap* pHeap) {
        if (N == 0) return;

        Q->pCommandQueue->UpdateTileMappings(
        T->pDxResource,
        N,
        pTileCoordinates + Offset,
        NULL,  // All regions are single tiles
        pHeap, // ID3D12Heap*
        N,
        NULL, // All ranges are sequential tiles in the heap
        pRangeOffsets + Offset,
        pRangeTileCounts + Offset,
        D3D12_TILE_MAPPING_FLAG_NONE);

        for (uint32_t i = 0; i < N; i++)
        {
            auto& TiledInfo = *const_cast<CGPUTiledTextureInfo*>(T->super.tiled_resource);
            auto& Mapping   = *ppMappings[Offset + i];
            int32_t expect_mapping = D3D12_TILE_MAPPING_STATUS_MAPPING;
            skr_atomic_compare_exchange_strong(&Mapping.status, &expect_mapping, D3D12_TILE_MAPPING_STATUS_MAPPED);
            skr_atomic_fetch_add_relaxed(&TiledInfo.alive_tiles_count, 1);
        }
    };
    uint32_t TileIndex     = 0;
    uint32_t SeqTileOffset = 0;
    uint32_t SeqTileCount  = 0;
    auto     LastHeap      = ppHeaps[0];
    while (TileIndex < AllocateTileCount)
    {
        auto CurrentHeap = ppHeaps[TileIndex];
        if (CurrentHeap != LastHeap)
        {
            fnMap(SeqTileCount, SeqTileOffset, LastHeap);
            LastHeap = CurrentHeap;
            SeqTileOffset += SeqTileCount;
            SeqTileCount = 0;
        }
        TileIndex++;
        SeqTileCount++;
    }
    fnMap(SeqTileCount, SeqTileOffset, LastHeap);
}

void cgpu_queue_unmap_tiled_texture_d3d12(CGPUQueueId queue, const struct CGPUTiledTextureRegions* regions)
{
    CGPUTiledTexture_D3D12* T           = (CGPUTiledTexture_D3D12*)regions->texture;
    CGPUQueue_D3D12*        Q           = (CGPUQueue_D3D12*)queue;
    const uint32_t          RegionCount = regions->region_count;

    // calculate page count
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const auto& Region   = regions->regions[i];
        auto&       Mappings = *T->getSubresTileMappings(Region.mip_level, Region.layer);
        for (uint32_t x = Region.start.x; x < Region.end.x; x++)
            for (uint32_t y = Region.start.y; y < Region.end.y; y++)
                for (uint32_t z = Region.start.z; z < Region.end.z; z++)
                {
                    Mappings.unmap(x, y, z);

                    const bool ForceUnmap = false;
                    if (ForceUnmap) // slow and only useful for debugging
                    {
                        D3D12_TILE_RANGE_FLAGS          Flags = D3D12_TILE_RANGE_FLAG_NULL;
                        UINT                            N     = 1;
                        D3D12_TILED_RESOURCE_COORDINATE Coord = { x, y, z, {} };
                        Q->pCommandQueue->UpdateTileMappings(
                        T->pDxResource,
                        1,
                        &Coord,
                        NULL, // All regions are single tiles
                        NULL, // ID3D12Heap*
                        1,
                        &Flags, // All ranges are sequential tiles in the heap
                        NULL,
                        &N,
                        D3D12_TILE_MAPPING_FLAG_NONE);
                    }
                }
    }
}

inline D3D12_RESOURCE_FLAGS D3D12Util_CalculateTextureFlags(const struct CGPUTextureDescriptor* desc)
{
    D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
    // Decide UAV flags
    if (desc->descriptors & CGPU_RESOURCE_TYPE_RW_TEXTURE)
    {
        Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    // Decide render target flags
    if (desc->descriptors & CGPU_RESOURCE_TYPE_RENDER_TARGET)
    {
        Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    else if (desc->descriptors & CGPU_RESOURCE_TYPE_DEPTH_STENCIL)
    {
        Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    // Decide sharing flags
    if (desc->flags & CGPU_TCF_EXPORT_ADAPTER_BIT)
    {
        Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
    }
    return Flags;
}

inline D3D12_TEXTURE_LAYOUT D3D12Util_CalculateTextureLayout(const struct CGPUTextureDescriptor* desc)
{
    D3D12_TEXTURE_LAYOUT Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    if (desc->flags & CGPU_TCF_TILED_RESOURCE)
        Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    else if (desc->flags & CGPU_TCF_EXPORT_ADAPTER_BIT)
        Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    return Layout;
}

inline D3D12_CLEAR_VALUE D3D12Util_CalculateClearValue(DXGI_FORMAT dxFormat, const struct CGPUTextureDescriptor* desc)
{
    SKR_DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValue);
    clearValue.Format = dxFormat;
    if (desc->descriptors & CGPU_RESOURCE_TYPE_DEPTH_STENCIL)
    {
        clearValue.DepthStencil.Depth   = desc->clear_value.depth;
        clearValue.DepthStencil.Stencil = (UINT8)desc->clear_value.stencil;
    }
    else
    {
        clearValue.Color[0] = desc->clear_value.r;
        clearValue.Color[1] = desc->clear_value.g;
        clearValue.Color[2] = desc->clear_value.b;
        clearValue.Color[3] = desc->clear_value.a;
    }
    return clearValue;
}

static_assert(sizeof(CGPUTexture_D3D12) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine");
CGPUTextureId cgpu_create_texture_d3d12(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc)
{
    CGPUDevice_D3D12*   D        = (CGPUDevice_D3D12*)device;
    CGPUAdapter_D3D12*  A        = (CGPUAdapter_D3D12*)device->adapter;
    CGPUTexture_D3D12*  T        = nullptr;
    D3D12_RESOURCE_DESC resDesc  = make_zeroed<D3D12_RESOURCE_DESC>();
    DXGI_FORMAT         dxFormat = DXGIUtil_TranslatePixelFormat(desc->format, false);
    if (desc->native_handle == CGPU_NULLPTR)
    {
        // On PC, If Alignment is set to 0, the runtime will use 4MB for MSAA
        // textures and 64KB for everything else. On XBox, We have to explicitlly
        // assign D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT if MSAA is used
        resDesc.Alignment        = (UINT)desc->sample_count > 1 ? D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT : 0;
        resDesc.Width            = desc->width;
        resDesc.Height           = (UINT)desc->height;
        resDesc.DepthOrArraySize = (UINT16)(desc->array_size != 1 ? desc->array_size : desc->depth);
        resDesc.MipLevels        = (UINT16)desc->mip_levels;
        resDesc.Dimension        = D3D12Util_CalculateTextureDimension(desc);
        resDesc.Format           = DXGIUtil_FormatToTypeless(dxFormat);
#if defined(XBOX)
        if (desc->flags & CGPU_TCF_ALLOW_DISPLAY_TARGET)
        {
            allocDesc.ExtraHeapFlags |= D3D12_HEAP_FLAG_ALLOW_DISPLAY;
            resDesc.Format = dxFormat;
        }
#endif
        // Sample Count
        resDesc.SampleDesc.Quality = (UINT)desc->sample_quality;
        resDesc.SampleDesc.Count   = (UINT)desc->sample_count ? desc->sample_count : 1;
        resDesc.SampleDesc.Count   = D3D12Util_CalculateTextureSampleCount(
        D->pDxDevice, resDesc.Format, resDesc.SampleDesc.Count);

        // Layout & Flags
        resDesc.Layout = D3D12Util_CalculateTextureLayout(desc);
        resDesc.Flags  = D3D12Util_CalculateTextureFlags(desc);

        // Decide start states
        CGPUResourceStates start_state = desc->start_state;
        start_state |= D3D12Util_CalculateTextureStartState(desc);
        D3D12_RESOURCE_STATES startStates = D3D12Util_TranslateResourceState(start_state);

        // Decide clear value
        const auto               clearValue  = D3D12Util_CalculateClearValue(dxFormat, desc);
        const D3D12_CLEAR_VALUE* pClearValue = NULL;
        if ((resDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (resDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            pClearValue = &clearValue;

        if (desc->flags & CGPU_TCF_ALIASING_RESOURCE)
        {
            T = D3D12Util_AllocateAliasing(desc, resDesc);
        }
        else if (desc->flags & CGPU_TCF_TILED_RESOURCE)
        {
            T = D3D12Util_AllocateTiled(A, D, desc, resDesc, startStates, pClearValue);
        }
        else
        {
            T = D3D12Util_AllocateFromAllocator(A, D, desc, resDesc, startStates, pClearValue);
        }
    }
    else // do import
    {
        T              = cgpu_new_sized<CGPUTexture_D3D12>(sizeof(CGPUTexture_D3D12) + sizeof(CGPUTextureInfo));
        T->super.info  = (CGPUTextureInfo*)(T + 1);
        T->pDxResource = (ID3D12Resource*)desc->native_handle;
        resDesc        = ((ID3D12Resource*)desc->native_handle)->GetDesc();
        dxFormat       = resDesc.Format;
    }
    // Set Texture Object Props
    auto pInfo                  = const_cast<CGPUTextureInfo*>(T->super.info);
    pInfo->is_imported          = desc->native_handle ? 1 : 0;
    pInfo->is_aliasing          = (desc->flags & CGPU_TCF_ALIASING_RESOURCE) ? 1 : 0;
    pInfo->is_tiled             = (desc->flags & CGPU_TCF_TILED_RESOURCE) ? 1 : 0;
    pInfo->is_cube              = (CGPU_RESOURCE_TYPE_TEXTURE_CUBE == (desc->descriptors & CGPU_RESOURCE_TYPE_TEXTURE_CUBE)) ? 1 : 0;
    pInfo->owns_image           = !pInfo->is_aliasing && !pInfo->is_imported;
    pInfo->sample_count         = desc->sample_count;
    pInfo->width                = desc->width;
    pInfo->height               = desc->height;
    pInfo->depth                = desc->depth;
    pInfo->mip_levels           = desc->mip_levels;
    pInfo->array_size_minus_one = desc->array_size - 1;
    pInfo->format               = desc->format;
    if (T->pDxResource)
    {
        const auto Desc      = T->pDxResource->GetDesc();
        auto       allocDesc = D->pDxDevice->GetResourceAllocationInfo(
        CGPU_SINGLE_GPU_NODE_MASK, 1, &Desc);
        pInfo->size_in_bytes = allocDesc.SizeInBytes;
    }
    // Set debug name
    if (device->adapter->instance->enable_set_name && desc->name && T->pDxResource)
    {
        wchar_t debugName[MAX_GPU_DEBUG_NAME_LENGTH] = {};
        if (desc->name)
            mbstowcs(debugName, (const char*)desc->name, MAX_GPU_DEBUG_NAME_LENGTH);
        T->pDxResource->SetName(debugName);
    }
    return &T->super;
}

bool cgpu_try_bind_aliasing_texture_d3d12(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc)
{
    HRESULT           result = E_INVALIDARG;
    CGPUDevice_D3D12* D      = (CGPUDevice_D3D12*)device;
    if (desc->aliased)
    {
        CGPUTexture_D3D12*         Aliased      = (CGPUTexture_D3D12*)desc->aliased;
        CGPUTextureAliasing_D3D12* Aliasing     = (CGPUTextureAliasing_D3D12*)desc->aliasing;
        auto                       AliasingInfo = const_cast<CGPUTextureInfo*>(Aliasing->super.info);
        const auto                 AliasedInfo  = Aliased->super.info;
        cgpu_assert(AliasingInfo->is_aliasing && "aliasing texture need to be created as aliasing!");
        if (Aliased->pDxResource != nullptr && Aliased->pDxAllocation != nullptr &&
            !AliasedInfo->is_restrict_dedicated && AliasingInfo->is_aliasing)
        {
            result = D->pResourceAllocator->CreateAliasingResource(
            Aliased->pDxAllocation,
            0, &Aliasing->mDxDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&Aliasing->pDxResource));
            if (result == S_OK)
            {
                Aliasing->pDxAllocation     = Aliased->pDxAllocation;
                AliasingInfo->size_in_bytes = AliasedInfo->size_in_bytes;
                // Set debug name
                if (device->adapter->instance->enable_set_name)
                {
                    wchar_t debugName[MAX_GPU_DEBUG_NAME_LENGTH] = {};
                    Aliasing->name.append(u8"[aliasing]");
                    if (!Aliasing->name.is_empty())
                        mbstowcs(debugName, Aliasing->name.c_str_raw(), MAX_GPU_DEBUG_NAME_LENGTH);
                    Aliasing->pDxResource->SetName(debugName);
                }
            }
        }
    }
    return result == S_OK;
}

const cgpu::stl_wstring shared_texture_name_format = L"cgpu-shared-texture-";
uint64_t cgpu_export_shared_texture_handle_d3d12(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc)
{
    HRESULT result = S_OK;
    HANDLE winHdl = INVALID_HANDLE_VALUE;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)desc->texture;

    // encode process id & shared_id into handle
    auto pid = (uint64_t)GetCurrentProcessId();
    uint64_t shared_id = D->next_shared_id++;
    uint64_t hdl = (pid << 32) | shared_id;

    // calculate name
    cgpu::stl_wstring name = shared_texture_name_format;
    name += std::to_wstring(hdl);

    // create shared resource handle
    result = D->pDxDevice->CreateSharedHandle(T->pDxResource,
                                               CGPU_NULLPTR, GENERIC_ALL, name.c_str(), &winHdl);
    auto winHdlLong = HandleToLong(winHdl);

    // deal with info & error
    const auto pInfo = T->super.info;
    if (FAILED(result))
    {
        cgpu_error(u8"Create Shared Handle Failed! Error Code: %d\n\tcan_export: %d\n\tsize:%dx%dx%d",
                   result, pInfo->can_export, pInfo->width, pInfo->height, pInfo->depth);
    }
    else
    {
        cgpu_trace(u8"Create Shared Handle Success! Handle: %lld(Windows handle %lld, As Long %d)\n\tcan_export: %d\n\tsize:%dx%dx%d",
                   hdl, winHdl, winHdlLong, pInfo->can_export, pInfo->width, pInfo->height, pInfo->depth);
    }
    return hdl;
}

CGPUTextureId cgpu_import_shared_texture_handle_d3d12(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc)
{
    HRESULT result = S_OK;
    ID3D12Resource* imported = CGPU_NULLPTR;
    HANDLE namedResourceHandle = (HANDLE)LongToHandle((long)desc->shared_handle);
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;

    cgpu::stl_wstring name = shared_texture_name_format;
    name += std::to_wstring(desc->shared_handle);

    result = D->pDxDevice->OpenSharedHandleByName(name.c_str(), GENERIC_ALL, &namedResourceHandle);
    if (FAILED(result))
    {
        cgpu_error(u8"Open Shared Handle %ls Failed! Error Code: %d size:%dx%dx%d",
                   name.c_str(), result, desc->width, desc->height, desc->depth);
        return CGPU_NULLPTR;
    }
    else
    {
        cgpu_trace(u8"Open Shared Handle %ls Success! Handle: %lld backend: %d",
                   name.c_str(), desc->shared_handle, desc->backend);
    }

    if (desc->backend == CGPU_BACKEND_D3D12)
    {
        result = D->pDxDevice->OpenSharedHandle(namedResourceHandle, IID_PPV_ARGS(&imported));
    }
    else if (desc->backend == CGPU_BACKEND_D3D12)
    {
        CloseHandle(namedResourceHandle);
        cgpu_warn(u8"Not implementated!");
        return nullptr;
        // result = D->pDxDevice->OpenSharedHandle(namedResourceHandle, IID_PPV_ARGS(&imported_heap));
    }
    else
    {
        result = D->pDxDevice->OpenSharedHandle(namedResourceHandle, IID_PPV_ARGS(&imported));
    }

    if (FAILED(result) && !imported)
    {
        auto winHdlLong = HandleToLong(namedResourceHandle);
        cgpu_error(u8"Import Shared Handle %ls(Windows handle %lld, As Long %d) Failed! Error Code: %d",
                   name.c_str(), namedResourceHandle, winHdlLong, result);
        return nullptr;
    }
    else
    {
        cgpu_trace(u8"Import Shared Handle %ls(Windows handle %lld, As Long %d) Succeed!",
                   name.c_str(), namedResourceHandle, desc->shared_handle);
    }
    CloseHandle(namedResourceHandle);

    auto imported_desc = imported->GetDesc();
    D3D12_RESOURCE_ALLOCATION_INFO alloc_info = D->pDxDevice->GetResourceAllocationInfo(CGPU_SINGLE_GPU_NODE_MASK, 1, &imported_desc);
    auto T = cgpu_new_sized<CGPUTexture_D3D12>(sizeof(CGPUTexture_D3D12) + sizeof(CGPUTextureInfo));
    auto pInfo = (CGPUTextureInfo*)(T + 1);
    T->super.info = pInfo;
    T->pDxResource = imported;
    T->pDxAllocation = CGPU_NULLPTR;
    pInfo->format = DXGIUtil_FormatToCGPU(imported_desc.Format);
    pInfo->width = imported_desc.Width;
    cgpu_assert(imported_desc.Width == desc->width);
    pInfo->height = imported_desc.Height;
    cgpu_assert(imported_desc.Height == desc->height);
    pInfo->depth = imported_desc.DepthOrArraySize;
    cgpu_assert(imported_desc.DepthOrArraySize == desc->depth);
    pInfo->mip_levels = imported_desc.MipLevels;
    cgpu_assert(imported_desc.MipLevels == desc->mip_levels);
    pInfo->array_size_minus_one = imported_desc.DepthOrArraySize - 1;
    pInfo->can_alias = false;
    pInfo->is_aliasing = false;
    pInfo->is_restrict_dedicated = false;
    pInfo->owns_image = false;
    pInfo->unique_id = D->super.next_texture_id++;
    pInfo->is_cube = (imported_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D && imported_desc.DepthOrArraySize > 6);
    pInfo->is_imported = true;
    // TODO: mGPU
    pInfo->node_index = CGPU_SINGLE_GPU_NODE_INDEX;
    pInfo->size_in_bytes = alloc_info.SizeInBytes;
    return &T->super;
}

void cgpu_free_texture_d3d12(CGPUTextureId texture)
{
    CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)texture;
    const auto pInfo = texture->info;
    if (pInfo->is_aliasing)
    {
        CGPUTextureAliasing_D3D12* AT = (CGPUTextureAliasing_D3D12*)T;
        SAFE_RELEASE(AT->pDxResource);
        cgpu_delete(AT);
    }
    else if (pInfo->is_tiled)
    {
        CGPUTiledTexture_D3D12* TT = (CGPUTiledTexture_D3D12*)T;
        auto pResource = TT->pDxResource;
        cgpu_delete(TT);
        SAFE_RELEASE(pResource);
    }
    else if (pInfo->is_imported)
    {
        SAFE_RELEASE(T->pDxResource);
        cgpu_delete(T);
    }
    else if (pInfo->owns_image)
    {
        SAFE_RELEASE(T->pDxAllocation);
        SAFE_RELEASE(T->pDxResource);
        cgpu_delete(T);
    }
}

// Shader APIs
#ifndef DXC_CP_ACP
    #define DXC_CP_ACP 0
#endif

template <typename T, typename... Args>
auto try_invoke_pinned_api(T* loader, Args&&... args)
-> decltype(loader->CreateBlobWithEncodingFromPinned(std::forward<Args>(args)...), bool())
{
    loader->CreateBlobWithEncodingFromPinned(std::forward<Args>(args)...);
    return true;
};
template <typename T>
bool try_invoke_pinned_api(T* loader, ...) { return false; }

struct DxilMinimalHeader {
    UINT32 four_cc;
    UINT32 hash_digest[4];
};

inline bool is_dxil_signed(const void* buffer)
{
    const DxilMinimalHeader* header     = reinterpret_cast<const DxilMinimalHeader*>(buffer);
    bool                     has_digest = false;
    has_digest |= header->hash_digest[0] != 0x0;
    has_digest |= header->hash_digest[1] != 0x0;
    has_digest |= header->hash_digest[2] != 0x0;
    has_digest |= header->hash_digest[3] != 0x0;
    return has_digest;
}

CGPUShaderLibraryId cgpu_create_shader_library_d3d12(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc)
{
    CGPUDevice_D3D12*        D = (CGPUDevice_D3D12*)device;
    CGPUShaderLibrary_D3D12* S = cgpu_new<CGPUShaderLibrary_D3D12>();
    IDxcLibrary*             pUtils;
    auto                     procDxcCreateInstnace = D3D12Util_GetDxcCreateInstanceProc();
    SKR_ASSERT(procDxcCreateInstnace && "Failed to get dxc proc!");
    procDxcCreateInstnace(CLSID_DxcLibrary, IID_PPV_ARGS(&pUtils));
    // if (!try_invoke_pinned_api(pUtils, desc->code, (uint32_t)desc->code_size, DXC_CP_ACP, &S->pShaderBlob))
    {
        pUtils->CreateBlobWithEncodingOnHeapCopy(desc->code, (uint32_t)desc->code_size, DXC_CP_ACP, &S->pShaderBlob);
    }
    // Validate & Signing
    // if (!is_dxil_signed(desc->code)) cgpu_assert(0 && "The dxil shader is not signed!");
    // Reflection
    D3D12Util_InitializeShaderReflection(D, S, desc);
    pUtils->Release();
    return &S->super;
}

void cgpu_free_shader_library_d3d12(CGPUShaderLibraryId shader_library)
{
    CGPUShaderLibrary_D3D12* S = (CGPUShaderLibrary_D3D12*)shader_library;
    D3D12Util_FreeShaderReflection(S);
    if (S->pShaderBlob != CGPU_NULLPTR)
    {
        S->pShaderBlob->Release();
    }
    cgpu_delete(S);
}

// Util Implementations
inline static D3D12_RESOURCE_DESC D3D12Util_CreateBufferDesc(CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D, const struct CGPUBufferDescriptor* desc)
{
    SKR_DECLARE_ZERO(D3D12_RESOURCE_DESC, bufDesc);
    uint64_t allocationSize = desc->size;
    // Align the buffer size to multiples of the dynamic uniform buffer minimum size
    if (desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
    {
        uint64_t minAlignment = A->adapter_detail.uniform_buffer_alignment;
        allocationSize        = cgpu_round_up(allocationSize, minAlignment);
    }
    bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    // Alignment must be 64KB (D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) or 0, which is effectively 64KB.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn903813(v=vs.85).aspx
    bufDesc.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufDesc.Width              = allocationSize;
    bufDesc.Height             = 1;
    bufDesc.DepthOrArraySize   = 1;
    bufDesc.MipLevels          = 1;
    bufDesc.Format             = DXGI_FORMAT_UNKNOWN;
    bufDesc.SampleDesc.Count   = 1;
    bufDesc.SampleDesc.Quality = 0;
    bufDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;
    if (desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER)
    {
        bufDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    // Adjust for padding
    UINT64 padded_size = 0;
    D->pDxDevice->GetCopyableFootprints(&bufDesc,
                                        0, 1, 0, NULL,
                                        NULL, NULL, &padded_size);
    if (padded_size != UINT64_MAX)
    {
        allocationSize = (uint64_t)padded_size;
        bufDesc.Width  = allocationSize;
    }
    // Mark DENY_SHADER_RESOURCE
    if (desc->memory_usage == CGPU_MEM_USAGE_GPU_TO_CPU)
    {
        bufDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }
    return bufDesc;
}

inline static D3D12MA::ALLOCATION_DESC D3D12Util_CreateAllocationDesc(const struct CGPUBufferDescriptor* desc)
{
    // Alloc Info
    SKR_DECLARE_ZERO(D3D12MA::ALLOCATION_DESC, alloc_desc)
    alloc_desc.HeapType = D3D12Util_TranslateHeapType(desc->memory_usage);

    if (desc->flags & CGPU_BCF_DEDICATED_BIT)
        alloc_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
    return alloc_desc;
}