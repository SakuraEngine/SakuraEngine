#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/drivers/cgpu_nvapi.h"
#include "cgpu/drivers/cgpu_ags.h"
#include "d3d12_utils.h"
#include "platform/memory.h"
#include "misc/make_zeroed.hpp"
#include "misc/defer.hpp"
#include <containers/string.hpp>
#include <containers/hashmap.hpp>
#include <containers/concurrent_queue.h>
#include <dxcapi.h>

#include <EASTL/string.h>

#include "tracy/Tracy.hpp"

// Inline Utils
D3D12_RESOURCE_DESC D3D12Util_CreateBufferDesc(CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D, const struct CGPUBufferDescriptor* desc);
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

struct CGPUTiledMemoryPool_D3D12 : public CGPUMemoryPool_D3D12
{
    D3D12MA::Allocation* AllocateTiles(uint32_t n) SKR_NOEXCEPT
    {
        CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)super.device;
        const auto kPageSize = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        D3D12MA::Allocation* pAllocation = nullptr;
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.CustomPool = pDxPool;
        D3D12_RESOURCE_ALLOCATION_INFO allocInfo = {};
        allocInfo.Alignment = kPageSize;
        allocInfo.SizeInBytes = n * kPageSize;
        D->pResourceAllocator->AllocateMemory(&allocDesc, &allocInfo, &pAllocation);
        allocated.insert(pAllocation);
        return pAllocation;
    }

    ~CGPUTiledMemoryPool_D3D12() SKR_NOEXCEPT
    {
        for (auto pAlloc : allocated)
        {
            SAFE_RELEASE(pAlloc);
        }
        allocated.clear();
        SAFE_RELEASE(pDxPool);
    }

private:
    skr::parallel_flat_hash_set<D3D12MA::Allocation*> allocated;
};

CGPUMemoryPoolId cgpu_create_memory_pool_d3d12(CGPUDeviceId device, const struct CGPUMemoryPoolDescriptor* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
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
    pool->super.type = desc->type;
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
    CGPUBuffer_D3D12* B = cgpu_new_sized<CGPUBuffer_D3D12>(sizeof(CGPUBuffer_D3D12) + sizeof(CGPUBufferInfo));
    auto pInfo = (CGPUBufferInfo*)(B + 1);
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUAdapter_D3D12* A = (CGPUAdapter_D3D12*)device->adapter;
    D3D12_RESOURCE_DESC bufDesc = D3D12Util_CreateBufferDesc(A, D, desc);
    uint64_t allocationSize = bufDesc.Width;
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
    D3D12_RESOURCE_STATES res_states = D3D12Util_TranslateResourceState(start_state);

    // Do Allocation
    const bool log_allocation = false;
    D3D12MA::ALLOCATION_DESC alloc_desc = D3D12Util_CreateAllocationDesc(desc);
#ifdef CGPU_USE_NVAPI
    if ((desc->memory_usage == CGPU_MEM_USAGE_GPU_ONLY && desc->flags & CGPU_BCF_HOST_VISIBLE) ||
        (desc->memory_usage & CGPU_MEM_USAGE_GPU_ONLY && desc->flags == CGPU_BCF_PERSISTENT_MAP_BIT))
    {
        bool cpuVisibleVRamSupported = false;
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
        heapProps.VisibleNodeMask = CGPU_SINGLE_GPU_NODE_MASK;
        heapProps.CreationNodeMask = CGPU_SINGLE_GPU_NODE_MASK;
        NV_RESOURCE_PARAMS nvParams = {};
        nvParams.NVResourceFlags = NV_D3D12_RESOURCE_FLAGS::NV_D3D12_RESOURCE_FLAG_CPUVISIBLE_VIDMEM;
        NvAPI_D3D12_CreateCommittedResource(D->pDxDevice, &heapProps,
            alloc_desc.ExtraHeapFlags,
            &bufDesc, res_states,
            nullptr, &nvParams, IID_ARGS(&B->pDxResource),
            &cpuVisibleVRamSupported);
        if (!cpuVisibleVRamSupported)
            B->pDxResource = nullptr;
        else if (log_allocation)
        {
            SKR_LOG_TRACE("[D3D12] Create CVV Buffer Resource Succeed! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d", 
                desc->name ? desc->name : u8"", allocationSize, desc->format);
        }
    }
#endif
    if (!B->pDxResource)
    {
        if (D3D12_HEAP_TYPE_DEFAULT != alloc_desc.HeapType &&
            (alloc_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
        {
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
            heapProps.VisibleNodeMask = CGPU_SINGLE_GPU_NODE_MASK;
            heapProps.CreationNodeMask = CGPU_SINGLE_GPU_NODE_MASK;
            CHECK_HRESULT(D->pDxDevice->CreateCommittedResource(&heapProps, alloc_desc.ExtraHeapFlags, 
            &bufDesc, res_states, NULL, IID_ARGS(&B->pDxResource)));
            if (log_allocation)
            {
                SKR_LOG_TRACE("[D3D12] Create Committed Buffer Resource Succeed! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d", 
                    desc->name ? desc->name : u8"", allocationSize, desc->format);
            }
        }
        else
        {
            {
                ZoneScopedN("Allocation(Buffer)");
                CHECK_HRESULT(D->pResourceAllocator->CreateResource(&alloc_desc, &bufDesc, res_states, 
                    NULL, &B->pDxAllocation, IID_ARGS(&B->pDxResource)));
            }            
            if (log_allocation)
            {
                ZoneScopedN("Log(Allocation)");
                SKR_LOG_TRACE("[D3D12] Create Buffer Resource Succeed! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d", 
                    desc->name ? desc->name : u8"", allocationSize, desc->format);
            }
        }
    }

    // MemMaps
    if (desc->flags & CGPU_BCF_PERSISTENT_MAP_BIT)
    {
        ZoneScopedN("Map(Buffer)");

        auto mapResult = B->pDxResource->Map(0, NULL, &pInfo->cpu_mapped_address);
        if (!SUCCEEDED(mapResult))
        {
            cgpu_warn("[D3D12] Map Buffer Resource Failed %d! \n\t With Name: %s\n\t Size: %lld \n\t Format: %d", 
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
        uint32_t handleCount = ((desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER) ? 1 : 0) +
                               ((desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER) ? 1 : 0) +
                               ((desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER) ? 1 : 0);
        B->mDxDescriptorHandles = D3D12Util_ConsumeDescriptorHandles(pHeap, handleCount).mCpu;
        // Create CBV
        if (desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE cbv = { B->mDxDescriptorHandles.ptr };
            B->mDxSrvOffset = pHeap->mDescriptorSize * 1;

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = B->mDxGpuAddress;
            cbvDesc.SizeInBytes = (UINT)allocationSize;
            D3D12Util_CreateCBV(D, &cbvDesc, &cbv);
        }
        // Create SRV
        if (desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srv = { B->mDxDescriptorHandles.ptr + B->mDxSrvOffset };
            B->mDxUavOffset = B->mDxSrvOffset + pHeap->mDescriptorSize * 1;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Buffer.FirstElement = desc->first_element;
            srvDesc.Buffer.NumElements = (UINT)(desc->elemet_count);
            srvDesc.Buffer.StructureByteStride = (UINT)(desc->element_stride);
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            srvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format);
            if (CGPU_RESOURCE_TYPE_BUFFER_RAW == (desc->descriptors & CGPU_RESOURCE_TYPE_BUFFER_RAW))
            {
                if (desc->format != CGPU_FORMAT_UNDEFINED)
                {
                    cgpu_warn("Raw buffers use R32 typeless format. Format will be ignored");
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
            uavDesc.Buffer.NumElements = (UINT)(desc->elemet_count);
            uavDesc.Buffer.StructureByteStride = (UINT)(desc->element_stride);
            uavDesc.Buffer.CounterOffsetInBytes = 0;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            if (CGPU_RESOURCE_TYPE_RW_BUFFER_RAW == (desc->descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER_RAW))
            {
                if (desc->format != CGPU_FORMAT_UNDEFINED)
                {
                    cgpu_warn("Raw buffers use R32 typeless format. Format will be ignored");
                }
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
            }
            else if (desc->format != CGPU_FORMAT_UNDEFINED)
            {
                uavDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format);
                D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = { uavDesc.Format, D3D12_FORMAT_SUPPORT1_NONE,
                    D3D12_FORMAT_SUPPORT2_NONE };
                HRESULT hr =
                D->pDxDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
                if (!SUCCEEDED(hr) || !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) ||
                    !(FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE))
                {
                    // Format does not support UAV Typed Load
                    cgpu_warn("Cannot use Typed UAV for buffer format %u", (uint32_t)desc->format);
                    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                }
            }
            // Cannot create a typed RWStructuredBuffer
            if (uavDesc.Format != DXGI_FORMAT_UNKNOWN)
            {
                uavDesc.Buffer.StructureByteStride = 0;
            }
            CGPUBuffer_D3D12* pCountBuffer = (CGPUBuffer_D3D12*)desc->count_buffer;
            ID3D12Resource* pCounterResource = pCountBuffer ? pCountBuffer->pDxResource : NULL;
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

void cgpu_cmd_transfer_buffer_to_buffer_d3d12(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUBuffer_D3D12* Src = (CGPUBuffer_D3D12*)desc->src;
    CGPUBuffer_D3D12* Dst = (CGPUBuffer_D3D12*)desc->dst;
#if defined(XBOX)
    Cmd->mDma.pDxCmdList->CopyBufferRegion(Dst->pDxResource, desc->dst_offset, Src->pDxResource, desc->src_offset, desc->size);
#else
    Cmd->pDxCmdList->CopyBufferRegion(Dst->pDxResource, desc->dst_offset, Src->pDxResource, desc->src_offset, desc->size);
#endif
}

void cgpu_cmd_transfer_texture_to_texture_d3d12(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)cmd->device;
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUTexture_D3D12* Src = (CGPUTexture_D3D12*)desc->src;
    CGPUTexture_D3D12* Dst = (CGPUTexture_D3D12*)desc->dst;

    uint32_t src_subresource = CALC_SUBRESOURCE_INDEX(
    desc->src_subresource.mip_level, desc->src_subresource.base_array_layer,
    0, 1,
    desc->src_subresource.layer_count);
    uint32_t dst_subresource = CALC_SUBRESOURCE_INDEX(
    desc->dst_subresource.mip_level, desc->dst_subresource.base_array_layer,
    0, 1,
    desc->dst_subresource.layer_count);
    D3D12_RESOURCE_DESC src_resourceDesc = Src->pDxResource->GetDesc();
    D3D12_RESOURCE_DESC dst_resourceDesc = Dst->pDxResource->GetDesc();

    D3D12_TEXTURE_COPY_LOCATION src = {};
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    D->pDxDevice->GetCopyableFootprints(
        &src_resourceDesc, src_subresource, 1,
        0, &src.PlacedFootprint,
        NULL, NULL, NULL);
    D->pDxDevice->GetCopyableFootprints(
        &dst_resourceDesc, dst_subresource, 1,
        0, &dst.PlacedFootprint,
        NULL, NULL, NULL);
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.pResource = Src->pDxResource;
    src.SubresourceIndex = src_subresource;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.pResource = Dst->pDxResource;
    dst.SubresourceIndex = dst_subresource;

#if defined(XBOX)
    Cmd->mDma.pDxCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
#else
    Cmd->pDxCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
#endif
}

void cgpu_cmd_transfer_buffer_to_texture_d3d12(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)cmd->device;
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUBuffer_D3D12* Src = (CGPUBuffer_D3D12*)desc->src;
    CGPUTexture_D3D12* Dst = (CGPUTexture_D3D12*)desc->dst;

    uint32_t subresource = CALC_SUBRESOURCE_INDEX(
        desc->dst_subresource.mip_level,
        desc->dst_subresource.base_array_layer,
        0, 1,
        desc->dst_subresource.layer_count);
    D3D12_RESOURCE_DESC resourceDesc = Dst->pDxResource->GetDesc();

    D3D12_TEXTURE_COPY_LOCATION src = {};
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.pResource = Src->pDxResource;
    D->pDxDevice->GetCopyableFootprints(
        &resourceDesc, subresource, 1,
        desc->src_offset, &src.PlacedFootprint,
        NULL, NULL, NULL);
    src.PlacedFootprint.Offset = desc->src_offset;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.pResource = Dst->pDxResource;
    dst.SubresourceIndex = subresource;
#if defined(XBOX)
    Cmd->mDma.pDxCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
#else
    Cmd->pDxCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
#endif
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
    SAFE_RELEASE(B->pDxAllocation)
    SAFE_RELEASE(B->pDxResource)
    cgpu_delete(B);
}

// Sampler APIs
CGPUSamplerId cgpu_create_sampler_d3d12(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc)
{
    // initialize to zero
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUSampler_D3D12* pSampler = cgpu_new<CGPUSampler_D3D12>();

    D3D12_SAMPLER_DESC add_desc = {};
    // add sampler to gpu
    add_desc.Filter = D3D12Util_TranslateFilter(
    desc->min_filter, desc->mag_filter,
    desc->mipmap_mode, desc->max_anisotropy > 0.0f,
    (desc->compare_func != CGPU_CMP_NEVER ? true : false));
    add_desc.AddressU = D3D12Util_TranslateAddressMode(desc->address_u);
    add_desc.AddressV = D3D12Util_TranslateAddressMode(desc->address_v);
    add_desc.AddressW = D3D12Util_TranslateAddressMode(desc->address_w);
    add_desc.MipLODBias = desc->mip_lod_bias;
    add_desc.MaxAnisotropy = cgpu_max((UINT)desc->max_anisotropy, 1U);
    add_desc.ComparisonFunc = gDx12ComparisonFuncTranslator[desc->compare_func];
    add_desc.BorderColor[0] = 0.0f;
    add_desc.BorderColor[1] = 0.0f;
    add_desc.BorderColor[2] = 0.0f;
    add_desc.BorderColor[3] = 0.0f;
    add_desc.MinLOD = 0.0f;
    add_desc.MaxLOD =
    ((desc->mipmap_mode == CGPU_MIPMAP_MODE_LINEAR) ? D3D12_FLOAT32_MAX : 0.0f);
    ;
    pSampler->mDxDesc = add_desc;
    pSampler->mDxHandle = D3D12Util_ConsumeDescriptorHandles(
        D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], 1).mCpu;
        D->pDxDevice->CreateSampler(&pSampler->mDxDesc, pSampler->mDxHandle);
    return &pSampler->super;
}

void cgpu_free_sampler_d3d12(CGPUSamplerId sampler)
{
    CGPUSampler_D3D12* pSampler = (CGPUSampler_D3D12*)sampler;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)sampler->device;

    D3D12Util_ReturnDescriptorHandles(
        D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER],
        pSampler->mDxHandle, 1);
    cgpu_delete(pSampler);
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
    msaaFeature.Format = Format;
    msaaFeature.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msaaFeature.SampleCount = Count;
    if (msaaFeature.SampleCount > 1)
    {
        pDxDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaFeature, sizeof(msaaFeature));
        while (msaaFeature.NumQualityLevels == 0 && msaaFeature.SampleCount > 0)
        {
            cgpu_warn("Sample Count (%u) not supported. Trying a lower sample count (%u)",
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
    auto T = cgpu_new_sized<CGPUTexture_D3D12>(sizeof(CGPUTexture_D3D12) + sizeof(CGPUTextureInfo));
    auto pInfo = (CGPUTextureInfo*)(T + 1);
    T->super.info = pInfo;
    ID3D12Resource* pDxResource = nullptr;
    D3D12MA::Allocation* pDxAllocation = nullptr;
    D3D12MA::ALLOCATION_DESC allocDesc = {};
    // Do allocation (TODO: mGPU)
    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    // for smaller alignment that not suitable for MSAA
    if (desc->flags & CGPU_TCF_OWN_MEMORY_BIT || desc->sample_count != CGPU_SAMPLE_COUNT_1)
    {
        allocDesc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
    }
    bool is_heap_dedicated = allocDesc.Flags & D3D12MA::ALLOCATION_FLAG_COMMITTED;
    bool is_driver_dedicated = is_heap_dedicated;
    if (!desc->is_dedicated && desc->sample_count == CGPU_SAMPLE_COUNT_1 && !(desc->flags & CGPU_TCF_EXPORT_BIT))
    {
        allocDesc.Flags |= D3D12MA::ALLOCATION_FLAG_CAN_ALIAS;
        is_driver_dedicated = false;
    }
    bool can_alias_allocation = allocDesc.Flags & D3D12MA::ALLOCATION_FLAG_CAN_ALIAS;
    if ( desc->flags & CGPU_TCF_EXPORT_BIT )
    {
        allocDesc.ExtraHeapFlags |= D3D12_HEAP_FLAG_SHARED;
    } 

    auto hres = D->pResourceAllocator->CreateResource(
        &allocDesc, &resDesc, startStates, pClearValue,
        &pDxAllocation, IID_ARGS(&pDxResource));
    if (hres != S_OK)
    {
        auto fallbackHres = hres;
        SKR_LOG_ERROR("[D3D12] Create Texture Resorce Failed With HRESULT %d! \n\t With Name: %s\n\t Size: %dx%d \n\t Format: %d \n\t Sample Count: %d", 
            hres,
            desc->name ? desc->name : u8"", desc->width, desc->height, 
            desc->format, desc->sample_count);
        SKR_LOG_ERROR("[D3D12] Format Support For this Format: RenderTarget %d Read %d Write %d", 
            A->adapter_detail.format_supports[desc->format].render_target_write,
            A->adapter_detail.format_supports[desc->format].shader_read,
            A->adapter_detail.format_supports[desc->format].shader_write);
        const bool use_fallback_commited = true;
        if (use_fallback_commited)
        {
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.CreationNodeMask = CGPU_SINGLE_GPU_NODE_MASK;
            heapProps.VisibleNodeMask = CGPU_SINGLE_GPU_NODE_MASK;
            fallbackHres = D->pDxDevice->CreateCommittedResource(&heapProps, 
                allocDesc.ExtraHeapFlags, &resDesc, startStates, 
                pClearValue, IID_ARGS(&pDxResource));
            if (fallbackHres == S_OK)
            {
                is_driver_dedicated = true;
                can_alias_allocation = false;
                pDxAllocation = nullptr;
                SKR_LOG_TRACE("[D3D12] Create Texture With Fallback Driver API Succeed!");
            }
            else
            {
                SKR_LOG_FATAL("[D3D12] Create Texture With Fallback Driver API Failed! Please Update Your Driver or Contact With us!");
            }
        }
        CHECK_HRESULT(fallbackHres);
        if (fallbackHres != S_OK) goto FAIL;
    }
    else
    {
        SKR_LOG_TRACE("[D3D12] Create Texture Resource Succeed! \n\t With Name: %s\n\t Size: %dx%d \n\t Format: %d \n\t Sample Count: %d", 
            desc->name ? desc->name : u8"", desc->width, desc->height, 
            desc->format, desc->sample_count);
        SKR_LOG_TRACE("[D3D12] Format Support For this Format: RenderTarget %d Read %d Write %d", 
            A->adapter_detail.format_supports[desc->format].render_target_write,
            A->adapter_detail.format_supports[desc->format].shader_read,
            A->adapter_detail.format_supports[desc->format].shader_write);
    }
    T->pDxAllocation = pDxAllocation;
    T->pDxResource = pDxResource;
    pInfo->is_driver_dedicated = is_driver_dedicated;
    pInfo->is_heap_dedicated = is_heap_dedicated;
    pInfo->can_alias = can_alias_allocation || (desc->flags & CGPU_TCF_ALIASING_RESOURCE);
    pInfo->can_export = (allocDesc.ExtraHeapFlags & D3D12_HEAP_FLAG_SHARED);
    return T;
FAIL:
    SAFE_RELEASE(pDxAllocation);
    SAFE_RELEASE(pDxResource);
    if (T) cgpu_delete(T);
    return nullptr;
}

struct CGPUTextureAliasing_D3D12 : public CGPUTexture_D3D12 {
    D3D12_RESOURCE_DESC mDxDesc;
    skr::string name;
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
        resDesc, desc->name
    );
    T->super.info = (CGPUTextureInfo*)(T + 1);
    return T;
}

inline static void alignedDivision(const D3D12_TILED_RESOURCE_COORDINATE& extent, const D3D12_TILED_RESOURCE_COORDINATE& granularity, D3D12_TILED_RESOURCE_COORDINATE* out)
{
    out->X = (extent.X / granularity.X + ((extent.X % granularity.X) ? 1u : 0u));
    out->Y = (extent.Y / granularity.Y + ((extent.Y % granularity.Y) ? 1u : 0u));
    out->Z = (extent.Z / granularity.Z + ((extent.Z % granularity.Z) ? 1u : 0u));
}

struct TileMapping_D3D12
{
    D3D12MA::Allocation* pDxAllocation;
};
static_assert(std::is_trivially_constructible_v<TileMapping_D3D12>, "TileMapping_D3D12 Must Be Trivially Constructible!");

struct SubresTileMappings_D3D12
{
    SubresTileMappings_D3D12(uint32_t X, uint32_t Y, uint32_t Z) SKR_NOEXCEPT
        : X(X), Y(Y), Z(Z)
    {
        if (X * Y * Z)
            mappings = (TileMapping_D3D12*)cgpu_calloc(1, X * Y * Z * sizeof(TileMapping_D3D12));
    }
    ~SubresTileMappings_D3D12() SKR_NOEXCEPT
    {
        if (mappings)
            cgpu_free(mappings);
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

private:
    const uint32_t X = 0;
    const uint32_t Y = 0;
    const uint32_t Z = 0;
    TileMapping_D3D12* mappings = nullptr;
};

struct CGPUTiledTexture_D3D12 : public CGPUTexture_D3D12 {
    CGPUTiledTexture_D3D12(SubresTileMappings_D3D12* pMappings) SKR_NOEXCEPT
        : CGPUTexture_D3D12(), pMappings(pMappings)
    {

    }
    ~CGPUTiledTexture_D3D12() SKR_NOEXCEPT
    {
        const auto N = super.info->mip_levels * (super.info->array_size_minus_one + 1);
        for (uint32_t i = 0; i < N; i++)
        {
            pMappings[i].~SubresTileMappings_D3D12();
        }
    }
    SubresTileMappings_D3D12* getSubresTileMappings(uint32_t mip_level, uint32_t array_index)
    {
        SKR_ASSERT(mip_level < super.info->mip_levels && array_index < super.info->array_size_minus_one + 1);
        return pMappings + (mip_level * (super.info->array_size_minus_one + 1) + array_index);
    }
    
private:
    SubresTileMappings_D3D12* pMappings;
};
cgpu_static_assert(sizeof(CGPUTiledTexture_D3D12) <= 8 * sizeof(uint64_t), "Acquire Single CacheLine"); // Cache Line

inline CGPUTexture_D3D12* D3D12Util_AllocateTiled(CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D, const struct CGPUTextureDescriptor* desc,
    D3D12_RESOURCE_DESC resDesc, D3D12_RESOURCE_STATES startStates, const D3D12_CLEAR_VALUE* pClearValue)
{
    ID3D12Resource* pDxResource = nullptr;
    auto res = D->pDxDevice->CreateReservedResource(
        &resDesc, startStates, pClearValue, IID_PPV_ARGS(&pDxResource));
    CHECK_HRESULT(res);
    SKR_ASSERT(resDesc.DepthOrArraySize == 1);

    // query page informations
    UINT numTiles = 0;
	D3D12_PACKED_MIP_INFO packedMipInfo;
	D3D12_TILE_SHAPE tileShape = {};
	UINT subresourceCount = resDesc.MipLevels;
    auto tilings = (D3D12_SUBRESOURCE_TILING*)cgpu_calloc(subresourceCount, sizeof(D3D12_SUBRESOURCE_TILING));
    SKR_DEFER({ cgpu_free(tilings);});
	D->pDxDevice->GetResourceTiling(pDxResource, &numTiles, 
        &packedMipInfo, &tileShape, 
        &subresourceCount, 0, 
        tilings);

    const auto objSize = sizeof(CGPUTiledTexture_D3D12) + sizeof(CGPUTextureInfo) + sizeof(CGPUTiledTextureInfo);
    const auto subresSize = (sizeof(CGPUTiledSubresourceInfo) + sizeof(SubresTileMappings_D3D12)) * subresourceCount;
    const auto totalSize = objSize + subresSize;
    auto T = (CGPUTiledTexture_D3D12*)cgpu_calloc(1, totalSize);
    auto pInfo = (CGPUTextureInfo*)(T + 1);
    auto pTiledInfo = (CGPUTiledTextureInfo*)(pInfo + 1);
    auto pSubresInfo = (CGPUTiledSubresourceInfo*)(pTiledInfo + 1);
    auto pSubresMapping = (SubresTileMappings_D3D12*)(pSubresInfo + subresourceCount);
    new (T) CGPUTiledTexture_D3D12(pSubresMapping);

    pTiledInfo->total_tiles_count = numTiles;
    pTiledInfo->alive_tiles_count = 0;
    pTiledInfo->tile_size = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    
    pTiledInfo->tile_width_in_texels = tileShape.WidthInTexels;
    pTiledInfo->tile_height_in_texels = tileShape.HeightInTexels;
    pTiledInfo->tile_depth_in_texels = tileShape.DepthInTexels;
    pTiledInfo->subresources = pSubresInfo;

    pTiledInfo->tail_mip_start = packedMipInfo.NumStandardMips;
    pTiledInfo->tail_mip_count = packedMipInfo.NumPackedMips;
    pTiledInfo->tail_tiles_count = packedMipInfo.NumTilesForPackedMips;

    for (uint32_t i = 0; i < subresourceCount; i++)
    {
        auto& SubresInfo = pSubresInfo[i];
        SubresInfo.width_in_tiles = tilings[i].WidthInTiles;
        SubresInfo.height_in_tiles = tilings[i].HeightInTiles;
        SubresInfo.depth_in_tiles = tilings[i].DepthInTiles;
        SubresInfo.layer = 0;
        SubresInfo.mip_level = i;

        new (&pSubresMapping[i]) SubresTileMappings_D3D12(SubresInfo.width_in_tiles, SubresInfo.height_in_tiles, SubresInfo.depth_in_tiles);
    }

    T->super.info = pInfo;
    T->super.tiled_resource = pTiledInfo;
    T->pDxResource = pDxResource;

    return T;
}

void cgpu_queue_map_tiled_texture_d3d12(CGPUQueueId queue, const struct CGPUMapTiledTextureDescriptor* desc)
{
    const auto kPageSize = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    const uint32_t RegionCount = desc->region_count;
    uint32_t PageCount = 0;
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const auto& Region = desc->regions[i];
        const auto Width = Region.end.x - Region.start.x;
        const auto Height = Region.end.y - Region.start.y;
        const auto Depth = (1 + Region.end.z - Region.start.z);
        const auto RegionPageCount = Width * Height * Depth;
        PageCount += RegionPageCount;
    }
    if (!PageCount) return;

    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)queue->device;
    CGPUQueue_D3D12* Q = (CGPUQueue_D3D12*)queue;
    CGPUTiledTexture_D3D12* T = (CGPUTiledTexture_D3D12*)desc->texture;

    // do allocations
    D3D12MA::Allocation* pAllocation = D->pTiledMemoryPool->AllocateTiles(PageCount);
    struct ID3D12Heap* pHeap = pAllocation->GetHeap();
    auto BytesOffset = pAllocation->GetOffset();

    // calc mapping batch
    auto ArgsMemory = sakura_calloc(RegionCount, sizeof(D3D12_TILED_RESOURCE_COORDINATE) + sizeof(D3D12_TILE_REGION_SIZE) + sizeof(UINT) + sizeof(UINT));
    SKR_DEFER( { sakura_free(ArgsMemory); } );
    auto pTileCoordinates = (D3D12_TILED_RESOURCE_COORDINATE*)ArgsMemory;
    auto pTileSizes = (D3D12_TILE_REGION_SIZE*)(pTileCoordinates + RegionCount);
    UINT* pRangeOffsets = (UINT*)(pTileSizes + RegionCount);
    UINT* pTileCounts = (UINT*)(pRangeOffsets + RegionCount);
    uint32_t ActualRegionCount = 0;
    for (uint32_t i = 0; i < RegionCount; i++)
    {
        const auto& Region = desc->regions[i];
        // auto& Mappings = *T->getSubresTileMappings(Region.mip_level, Region.layer);
        pTileSizes[ActualRegionCount].UseBox = TRUE;
        const auto Width = pTileSizes[ActualRegionCount].Width = Region.end.x - Region.start.x;
        const auto Height = pTileSizes[ActualRegionCount].Height = Region.end.y - Region.start.y;
        const auto Depth = pTileSizes[ActualRegionCount].Depth = cgpu_max(1, Region.end.z - Region.start.z);
        const auto NumTiles = pTileSizes[ActualRegionCount].NumTiles = Width * Height * Depth;
        if (NumTiles == 0)
            continue;
            
        pTileCoordinates[ActualRegionCount].X = Region.start.x;
        pTileCoordinates[ActualRegionCount].Y = Region.start.y;
        pTileCoordinates[ActualRegionCount].Z = Region.start.z;
        pTileCoordinates[ActualRegionCount].Subresource = CALC_SUBRESOURCE_INDEX(
            Region.mip_level, Region.layer, 0, 1, 1);

        pTileCounts[ActualRegionCount] = pTileSizes[i].NumTiles;
        
        pRangeOffsets[ActualRegionCount] = (uint32_t)(BytesOffset / kPageSize);
        BytesOffset += pTileCounts[ActualRegionCount] * kPageSize;

        ActualRegionCount++;
    }
    
    // do mapping
    {
		Q->pCommandQueue->UpdateTileMappings(
			T->pDxResource,
			ActualRegionCount,
			pTileCoordinates,
			pTileSizes,  // All regions are single tiles
			pHeap, // ID3D12Heap*
			ActualRegionCount,
			NULL,  // All ranges are sequential tiles in the heap
			pRangeOffsets,
			pTileCounts,
			D3D12_TILE_MAPPING_FLAG_NONE);
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
    DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValue);
    clearValue.Format = dxFormat;
    if (desc->descriptors & CGPU_RESOURCE_TYPE_DEPTH_STENCIL)
    {
        clearValue.DepthStencil.Depth = desc->clear_value.depth;
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
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUAdapter_D3D12* A = (CGPUAdapter_D3D12*)device->adapter;
    CGPUTexture_D3D12* T = nullptr;
    D3D12_RESOURCE_DESC resDesc = make_zeroed<D3D12_RESOURCE_DESC>();
    DXGI_FORMAT dxFormat = DXGIUtil_TranslatePixelFormat(desc->format);
    if (desc->native_handle == CGPU_NULLPTR)
    {
        // On PC, If Alignment is set to 0, the runtime will use 4MB for MSAA
        // textures and 64KB for everything else. On XBox, We have to explicitlly
        // assign D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT if MSAA is used
        resDesc.Alignment = (UINT)desc->sample_count > 1 ? D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT : 0;
        resDesc.Width = desc->width;
        resDesc.Height = (UINT)desc->height;
        resDesc.DepthOrArraySize = (UINT16)(desc->array_size != 1 ? desc->array_size : desc->depth);
        resDesc.MipLevels = (UINT16)desc->mip_levels;
        resDesc.Dimension = D3D12Util_CalculateTextureDimension(desc);
        resDesc.Format = DXGIUtil_FormatToTypeless(dxFormat);
#if defined(XBOX)
        if (desc->flags & CGPU_TCF_ALLOW_DISPLAY_TARGET)
        {
            allocDesc.ExtraHeapFlags |= D3D12_HEAP_FLAG_ALLOW_DISPLAY;
            resDesc.Format = dxFormat;
        }
#endif
        // Sample Count
        resDesc.SampleDesc.Quality = (UINT)desc->sample_quality;
        resDesc.SampleDesc.Count = (UINT)desc->sample_count ? desc->sample_count : 1;
        resDesc.SampleDesc.Count = D3D12Util_CalculateTextureSampleCount(
            D->pDxDevice, resDesc.Format, resDesc.SampleDesc.Count);

        // Layout & Flags
        resDesc.Layout = D3D12Util_CalculateTextureLayout(desc);
        resDesc.Flags = D3D12Util_CalculateTextureFlags(desc);

        // Decide start states
        CGPUResourceStates start_state = desc->start_state;
        start_state |= D3D12Util_CalculateTextureStartState(desc);
        D3D12_RESOURCE_STATES startStates = D3D12Util_TranslateResourceState(start_state);

        // Decide clear value
        const auto clearValue = D3D12Util_CalculateClearValue(dxFormat, desc);
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
        T = cgpu_new_sized<CGPUTexture_D3D12>(sizeof(CGPUTexture_D3D12) + sizeof(CGPUTextureInfo));
        T->super.info = (CGPUTextureInfo*)(T + 1);
        T->pDxResource = (ID3D12Resource*)desc->native_handle;
        resDesc = ((ID3D12Resource*)desc->native_handle)->GetDesc();
        dxFormat = resDesc.Format;
    }
    // Set Texture Object Props
    auto pInfo = const_cast<CGPUTextureInfo*>(T->super.info);
    pInfo->is_imported = desc->native_handle ? 1 : 0;
    pInfo->is_aliasing = (desc->flags & CGPU_TCF_ALIASING_RESOURCE) ? 1 : 0;
    pInfo->is_tiled = (desc->flags & CGPU_TCF_TILED_RESOURCE) ? 1 : 0;
    pInfo->is_cube = (CGPU_RESOURCE_TYPE_TEXTURE_CUBE == (desc->descriptors & CGPU_RESOURCE_TYPE_TEXTURE_CUBE)) ? 1 : 0;
    pInfo->owns_image = !pInfo->is_aliasing && !pInfo->is_imported;
    pInfo->sample_count = desc->sample_count;
    pInfo->width = desc->width;
    pInfo->height = desc->height;
    pInfo->depth = desc->depth;
    pInfo->mip_levels = desc->mip_levels;
    pInfo->array_size_minus_one = desc->array_size - 1;
    pInfo->format = desc->format;
    if (T->pDxResource)
    {
        const auto Desc = T->pDxResource->GetDesc();
        auto allocDesc = D->pDxDevice->GetResourceAllocationInfo(
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
    HRESULT result = E_INVALIDARG;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    if (desc->aliased)
    {
        CGPUTexture_D3D12* Aliased = (CGPUTexture_D3D12*)desc->aliased;
        CGPUTextureAliasing_D3D12* Aliasing = (CGPUTextureAliasing_D3D12*)desc->aliasing;
        auto AliasingInfo = const_cast<CGPUTextureInfo*>(Aliasing->super.info);
        const auto AliasedInfo = Aliased->super.info;
        cgpu_assert(AliasingInfo->is_aliasing && "aliasing texture need to be created as aliasing!");
        if (Aliased->pDxResource != nullptr && Aliased->pDxAllocation != nullptr &&
            !AliasedInfo->is_driver_dedicated && AliasingInfo->is_aliasing)
        {
            result = D->pResourceAllocator->CreateAliasingResource(
                Aliased->pDxAllocation,
                0, &Aliasing->mDxDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&Aliasing->pDxResource));
            if (result == S_OK)
            {
                Aliasing->pDxAllocation = Aliased->pDxAllocation;
                AliasingInfo->size_in_bytes = AliasedInfo->size_in_bytes;
                // Set debug name
                if (device->adapter->instance->enable_set_name)
                {
                    wchar_t debugName[MAX_GPU_DEBUG_NAME_LENGTH] = {};
                    auto alisingName = Aliasing->name.append(u8"[aliasing]");
                    if (!Aliasing->name.is_empty())
                        mbstowcs(debugName, alisingName.c_str(), MAX_GPU_DEBUG_NAME_LENGTH);
                    Aliasing->pDxResource->SetName(debugName);
                }
            }
        }
    }
    return result == S_OK;
}

const eastl::wstring shared_texture_name_format = L"cgpu-shared-texture-";
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
    eastl::wstring name = shared_texture_name_format;
    name += eastl::to_wstring(hdl);

    // create shared resource handle
    result = D->pDxDevice->CreateSharedHandle(T->pDxResource, 
        CGPU_NULLPTR, GENERIC_ALL, name.c_str(), &winHdl);
    auto winHdlLong = HandleToLong(winHdl);

    // deal with info & error
    const auto pInfo = T->super.info;
    if (FAILED(result))
    {
        cgpu_error("Create Shared Handle Failed! Error Code: %d\n\tcan_export: %d\n\tsize:%dx%dx%d" ,
            result, pInfo->can_export, pInfo->width, pInfo->height, pInfo->depth);
    }
    else
    {
        cgpu_trace("Create Shared Handle Success! Handle: %lld(Windows handle %lld, As Long %d)\n\tcan_export: %d\n\tsize:%dx%dx%d",
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

    eastl::wstring name = shared_texture_name_format;
    name += eastl::to_wstring(desc->shared_handle);

    result = D->pDxDevice->OpenSharedHandleByName(name.c_str(), GENERIC_ALL, &namedResourceHandle);
    if (FAILED(result))
    {
        cgpu_error("Open Shared Handle %ls Failed! Error Code: %d size:%dx%dx%d" ,
            name.c_str(), result, desc->width, desc->height, desc->depth);
        return CGPU_NULLPTR;
    }
    else
    {
        cgpu_trace("Open Shared Handle %ls Success! Handle: %lld backend: %d is_dedicated: %d", 
            name.c_str(), desc->shared_handle, desc->backend, desc->is_dedicated);
    }

    if (desc->backend == CGPU_BACKEND_D3D12)
    {
        result = D->pDxDevice->OpenSharedHandle(namedResourceHandle, IID_PPV_ARGS(&imported));
    }
    else if (desc->backend == CGPU_BACKEND_VULKAN)
    {
        CloseHandle(namedResourceHandle);
        cgpu_warn("Not implementated!");
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
        cgpu_error("Import Shared Handle %ls(Windows handle %lld, As Long %d) Failed! Error Code: %d", 
            name.c_str(), namedResourceHandle, winHdlLong, result);
        return nullptr;
    }
    else
    {
        cgpu_trace("Import Shared Handle %ls(Windows handle %lld, As Long %d) Succeed!", 
            name.c_str(), namedResourceHandle, desc->shared_handle);
    }
    CloseHandle(namedResourceHandle);

    auto imported_desc = imported->GetDesc();
    D3D12_RESOURCE_ALLOCATION_INFO alloc_info 
        = D->pDxDevice->GetResourceAllocationInfo(CGPU_SINGLE_GPU_NODE_MASK, 1, &imported_desc);
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
    pInfo->is_driver_dedicated = false;
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
        SAFE_RELEASE(TT->pDxResource);
        cgpu_delete(TT);
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

CGPUTextureViewId cgpu_create_texture_view_d3d12(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc)
{
    CGPUTextureView_D3D12* TV = cgpu_new<CGPUTextureView_D3D12>();
    CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)desc->texture;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    // Consume handles
    const auto usages = desc->usages;
    uint32_t handleCount = ((usages & CGPU_TVU_SRV) ? 1 : 0) +
                           ((usages & CGPU_TVU_UAV) ? 1 : 0);
    if (handleCount > 0)
    {
        D3D12Util_DescriptorHeap* pHeap = D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
        TV->mDxDescriptorHandles = D3D12Util_ConsumeDescriptorHandles(pHeap, handleCount).mCpu;
        TV->mDxSrvOffset = 0;
        uint64_t CurrentOffsetCursor = TV->mDxSrvOffset;
        // Create SRV
        if (usages & CGPU_TVU_SRV)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srv = { TV->mDxDescriptorHandles.ptr + TV->mDxSrvOffset };
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, true);
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            switch (desc->dims)
            {
                case CGPU_TEX_DIMENSION_1D: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                    srvDesc.Texture1D.MipLevels = desc->mip_level_count;
                    srvDesc.Texture1D.MostDetailedMip = desc->base_mip_level;
                }
                break;
                case CGPU_TEX_DIMENSION_1D_ARRAY: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    srvDesc.Texture1DArray.MipLevels = desc->mip_level_count;
                    srvDesc.Texture1DArray.MostDetailedMip = desc->base_mip_level;
                    srvDesc.Texture1DArray.FirstArraySlice = desc->base_array_layer;
                    srvDesc.Texture1DArray.ArraySize = desc->array_layer_count;
                }
                break;
                case CGPU_TEX_DIMENSION_2DMS: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                break;
                case CGPU_TEX_DIMENSION_2D: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MipLevels = desc->mip_level_count;
                    srvDesc.Texture2D.MostDetailedMip = desc->base_mip_level;
                    srvDesc.Texture2D.PlaneSlice = 0;
                }
                break;
                case CGPU_TEX_DIMENSION_2DMS_ARRAY: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    srvDesc.Texture2DMSArray.ArraySize = desc->array_layer_count;
                    srvDesc.Texture2DMSArray.FirstArraySlice = desc->base_array_layer;
                }
                break;
                case CGPU_TEX_DIMENSION_2D_ARRAY: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srvDesc.Texture2DArray.MipLevels = desc->mip_level_count;
                    srvDesc.Texture2DArray.MostDetailedMip = desc->base_mip_level;
                    srvDesc.Texture2DArray.PlaneSlice = 0;
                    srvDesc.Texture2DArray.FirstArraySlice = desc->base_array_layer;
                    srvDesc.Texture2DArray.ArraySize = desc->array_layer_count;
                }
                break;
                case CGPU_TEX_DIMENSION_3D: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                    srvDesc.Texture3D.MipLevels = desc->mip_level_count;
                    srvDesc.Texture3D.MostDetailedMip = desc->base_mip_level;
                }
                break;
                case CGPU_TEX_DIMENSION_CUBE: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    srvDesc.TextureCube.MipLevels = desc->mip_level_count;
                    srvDesc.TextureCube.MostDetailedMip = desc->base_mip_level;
                }
                break;
                case CGPU_TEX_DIMENSION_CUBE_ARRAY: {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    srvDesc.TextureCubeArray.MipLevels = desc->mip_level_count;
                    srvDesc.TextureCubeArray.MostDetailedMip = desc->base_mip_level;
                    srvDesc.TextureCubeArray.NumCubes = desc->array_layer_count;
                    srvDesc.TextureCubeArray.First2DArrayFace = desc->array_layer_count;
                }
                break;
                default:
                    cgpu_assert(0 && "Unsupported texture dimension!");
                    break;
            }
            D3D12Util_CreateSRV(D, T->pDxResource, &srvDesc, &srv);
            CurrentOffsetCursor += pHeap->mDescriptorSize * 1;
        }
        // Create UAV
        if (usages & CGPU_TVU_UAV)
        {
            TV->mDxUavOffset = CurrentOffsetCursor;
            CurrentOffsetCursor += pHeap->mDescriptorSize * 1;
            D3D12_CPU_DESCRIPTOR_HANDLE uav = { TV->mDxDescriptorHandles.ptr + TV->mDxUavOffset };
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, true);
            cgpu_assert(desc->mip_level_count <= 1 && "UAV must be created with non-multi mip slices!");
            switch (desc->dims)
            {
                case CGPU_TEX_DIMENSION_1D: {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                    uavDesc.Texture1D.MipSlice = desc->base_mip_level;
                }
                break;
                case CGPU_TEX_DIMENSION_1D_ARRAY: {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                    uavDesc.Texture1DArray.MipSlice = desc->base_mip_level;
                    uavDesc.Texture1DArray.FirstArraySlice = desc->base_array_layer;
                    uavDesc.Texture1DArray.ArraySize = desc->array_layer_count;
                }
                break;
                case CGPU_TEX_DIMENSION_2D: {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                    uavDesc.Texture2D.MipSlice = desc->base_mip_level;
                    uavDesc.Texture2D.PlaneSlice = 0;
                }
                break;
                case CGPU_TEX_DIMENSION_2D_ARRAY: {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    uavDesc.Texture2DArray.MipSlice = desc->base_mip_level;
                    uavDesc.Texture2DArray.PlaneSlice = 0;
                    uavDesc.Texture2DArray.FirstArraySlice = desc->base_array_layer;
                    uavDesc.Texture2DArray.ArraySize = desc->array_layer_count;
                }
                break;
                case CGPU_TEX_DIMENSION_3D: {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                    uavDesc.Texture3D.MipSlice = desc->base_mip_level;
                    uavDesc.Texture3D.FirstWSlice = desc->base_array_layer;
                    uavDesc.Texture3D.WSize = desc->array_layer_count;
                }
                break;
                default:
                    cgpu_assert(0 && "Unsupported texture dimension!");
                    break;
            }
            D3D12Util_CreateUAV(D, T->pDxResource, CGPU_NULLPTR, &uavDesc, &uav);
        }
    }
    // Create RTV
    if (usages & CGPU_TVU_RTV_DSV)
    {
        const bool isDSV = FormatUtil_IsDepthStencilFormat(desc->format);
        if (isDSV)
        {
            D3D12Util_DescriptorHeap* pDsvHeap = D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
            TV->mDxRtvDsvDescriptorHandle = D3D12Util_ConsumeDescriptorHandles(pDsvHeap, 1).mCpu;
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format);
            switch (desc->dims)
            {
                case CGPU_TEX_DIMENSION_1D: {
                    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                    dsvDesc.Texture1D.MipSlice = desc->base_mip_level;
                }
                break;
                case CGPU_TEX_DIMENSION_1D_ARRAY: {
                    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                    dsvDesc.Texture1DArray.MipSlice = desc->base_mip_level;
                    dsvDesc.Texture1DArray.FirstArraySlice = desc->base_array_layer;
                    dsvDesc.Texture1DArray.ArraySize = desc->array_layer_count;
                }
                break;
                case CGPU_TEX_DIMENSION_2DMS: {
                    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                }
                break;
                case CGPU_TEX_DIMENSION_2D: {
                    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    dsvDesc.Texture2D.MipSlice = desc->base_mip_level;
                }
                break;
                case CGPU_TEX_DIMENSION_2DMS_ARRAY: {
                    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    dsvDesc.Texture2DMSArray.FirstArraySlice = desc->base_array_layer;
                    dsvDesc.Texture2DMSArray.ArraySize = desc->array_layer_count;
                }
                case CGPU_TEX_DIMENSION_2D_ARRAY: {
                    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsvDesc.Texture2DArray.MipSlice = desc->base_mip_level;
                    dsvDesc.Texture2DArray.FirstArraySlice = desc->base_array_layer;
                    dsvDesc.Texture2DArray.ArraySize = desc->array_layer_count;
                }
                break;
                default:
                    cgpu_assert(0 && "Unsupported texture dimension!");
                    break;
            }
            D3D12Util_CreateDSV(D, T->pDxResource, &dsvDesc, &TV->mDxRtvDsvDescriptorHandle);
        }
        else
        {
            D3D12Util_DescriptorHeap* pRtvHeap = D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
            TV->mDxRtvDsvDescriptorHandle = D3D12Util_ConsumeDescriptorHandles(pRtvHeap, 1).mCpu;
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format);
            switch (desc->dims)
            {
                case CGPU_TEX_DIMENSION_1D: {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                    rtvDesc.Texture1D.MipSlice = desc->base_mip_level;
                }
                break;
                case CGPU_TEX_DIMENSION_1D_ARRAY: {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                    rtvDesc.Texture1DArray.MipSlice = desc->base_mip_level;
                    rtvDesc.Texture1DArray.FirstArraySlice = desc->base_array_layer;
                    rtvDesc.Texture1DArray.ArraySize = desc->array_layer_count;
                }
                break;
                case CGPU_TEX_DIMENSION_2DMS: {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                }
                break;
                case CGPU_TEX_DIMENSION_2D: {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    rtvDesc.Texture2D.MipSlice = desc->base_mip_level;
                    rtvDesc.Texture2D.PlaneSlice = 0;
                }
                break;
                case CGPU_TEX_DIMENSION_2DMS_ARRAY: {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    rtvDesc.Texture2DMSArray.FirstArraySlice = desc->base_array_layer;
                    rtvDesc.Texture2DMSArray.ArraySize = desc->array_layer_count;
                }
                case CGPU_TEX_DIMENSION_2D_ARRAY: {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtvDesc.Texture2DArray.MipSlice = desc->base_mip_level;
                    rtvDesc.Texture2DArray.PlaneSlice = 0;
                    rtvDesc.Texture2DArray.FirstArraySlice = desc->base_array_layer;
                    rtvDesc.Texture2DArray.ArraySize = desc->array_layer_count;
                }
                break;
                case CGPU_TEX_DIMENSION_3D: {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                    rtvDesc.Texture3D.MipSlice = desc->base_mip_level;
                    rtvDesc.Texture3D.FirstWSlice = desc->base_array_layer;
                    rtvDesc.Texture3D.WSize = desc->array_layer_count;
                }
                break;
                default:
                    cgpu_assert(0 && "Unsupported texture dimension!");
                    break;
            }
            D3D12Util_CreateRTV(D, T->pDxResource, &rtvDesc, &TV->mDxRtvDsvDescriptorHandle);
        }
    }
    return &TV->super;
}

void cgpu_free_texture_view_d3d12(CGPUTextureViewId view)
{
    CGPUTextureView_D3D12* TV = (CGPUTextureView_D3D12*)view;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)view->device;
    const auto usages = TV->super.info.usages;
    const bool isDSV = FormatUtil_IsDepthStencilFormat(view->info.format);
    if (TV->mDxDescriptorHandles.ptr != D3D12_GPU_VIRTUAL_ADDRESS_NULL)
    {
        uint32_t handleCount = ((usages & CGPU_TVU_SRV) ? 1 : 0) +
                               ((usages & CGPU_TVU_UAV) ? 1 : 0);
        D3D12Util_ReturnDescriptorHandles(
        D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], TV->mDxDescriptorHandles,
        handleCount);
    }
    if (TV->mDxRtvDsvDescriptorHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_NULL)
    {
        if (usages & CGPU_TVU_RTV_DSV)
            D3D12Util_ReturnDescriptorHandles(
            isDSV ? D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] : D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV],
            TV->mDxRtvDsvDescriptorHandle,
            1);
    }
    cgpu_delete(TV);
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
    const DxilMinimalHeader* header = reinterpret_cast<const DxilMinimalHeader*>(buffer);
    bool has_digest = false;
    has_digest |= header->hash_digest[0] != 0x0;
    has_digest |= header->hash_digest[1] != 0x0;
    has_digest |= header->hash_digest[2] != 0x0;
    has_digest |= header->hash_digest[3] != 0x0;
    return has_digest;
}

CGPUShaderLibraryId cgpu_create_shader_library_d3d12(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUShaderLibrary_D3D12* S = cgpu_new<CGPUShaderLibrary_D3D12>();
    IDxcLibrary* pUtils;
    auto procDxcCreateInstnace = D3D12Util_GetDxcCreateInstanceProc();
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
inline D3D12_RESOURCE_DESC D3D12Util_CreateBufferDesc(CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D, const struct CGPUBufferDescriptor* desc)
{
    DECLARE_ZERO(D3D12_RESOURCE_DESC, bufDesc);
    uint64_t allocationSize = desc->size;
    // Align the buffer size to multiples of the dynamic uniform buffer minimum size
    if (desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
    {
        uint64_t minAlignment = A->adapter_detail.uniform_buffer_alignment;
        allocationSize = cgpu_round_up(allocationSize, minAlignment);
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
        bufDesc.Width = allocationSize;
    }
    // Mark DENY_SHADER_RESOURCE
    if (desc->memory_usage == CGPU_MEM_USAGE_GPU_TO_CPU)
    {
        bufDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }
    return bufDesc;
}

inline D3D12MA::ALLOCATION_DESC D3D12Util_CreateAllocationDesc(const struct CGPUBufferDescriptor* desc)
{
    // Alloc Info
    DECLARE_ZERO(D3D12MA::ALLOCATION_DESC, alloc_desc)
    alloc_desc.HeapType = D3D12Util_TranslateHeapType(desc->memory_usage);

    if (desc->flags & CGPU_BCF_OWN_MEMORY_BIT)
        alloc_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
    return alloc_desc;
}

// Descriptor Heap
D3D12Util_DescriptorHandle D3D12Util_ConsumeDescriptorHandles(D3D12Util_DescriptorHeap* pHeap, uint32_t descriptorCount)
{
    if (pHeap->mUsedDescriptors + descriptorCount > pHeap->mDesc.NumDescriptors)
    {
#ifdef CGPU_THREAD_SAFETY
        SMutexLock lock(*pHeap->pMutex);
#endif
        if ((pHeap->mDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE))
        {
            uint32_t currentOffset = pHeap->mUsedDescriptors;
            (void)currentOffset;
            D3D12_DESCRIPTOR_HEAP_DESC desc = pHeap->mDesc;
            while (pHeap->mUsedDescriptors + descriptorCount > desc.NumDescriptors)
            {
                desc.NumDescriptors <<= 1;
            }
            ID3D12Device* pDevice = pHeap->pDevice;
            SAFE_RELEASE(pHeap->pCurrentHeap);
            pDevice->CreateDescriptorHeap(&desc, IID_ARGS(&pHeap->pCurrentHeap));
            pHeap->mDesc = desc;
            pHeap->mStartHandle.mCpu = pHeap->pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
            pHeap->mStartHandle.mGpu = pHeap->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();

            uint32_t* rangeSizes = (uint32_t*)alloca(pHeap->mUsedDescriptors * sizeof(uint32_t));
#ifdef CGPU_THREAD_SAFETY
            uint32_t usedDescriptors = skr_atomicu32_load_relaxed(&pHeap->mUsedDescriptors);
#else
            uint32_t usedDescriptors = pHeap->mUsedDescriptors;
#endif
            for (uint32_t i = 0; i < pHeap->mUsedDescriptors; ++i)
                rangeSizes[i] = 1;
            pDevice->CopyDescriptors(
            1, &pHeap->mStartHandle.mCpu, &usedDescriptors, pHeap->mUsedDescriptors, pHeap->pHandles, rangeSizes, pHeap->mDesc.Type);
            D3D12_CPU_DESCRIPTOR_HANDLE* pNewHandles =
            (D3D12_CPU_DESCRIPTOR_HANDLE*)cgpu_calloc(pHeap->mDesc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
            memcpy(pNewHandles, pHeap->pHandles, pHeap->mUsedDescriptors * sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
            cgpu_free(pHeap->pHandles);
            pHeap->pHandles = pNewHandles;
        }
        else if (pHeap->mFreeList.size() >= descriptorCount)
        {
            if (descriptorCount == 1)
            {
                D3D12Util_DescriptorHandle ret = pHeap->mFreeList.back();
                pHeap->mFreeList.pop_back();
                return ret;
            }

            // search for continuous free items in the list
            uint32_t freeCount = 1;
            for (size_t i = pHeap->mFreeList.size() - 1; i > 0; --i)
            {
                size_t index = i - 1;
                D3D12Util_DescriptorHandle mDescHandle = pHeap->mFreeList[index];
                if (mDescHandle.mCpu.ptr + pHeap->mDescriptorSize == pHeap->mFreeList[i].mCpu.ptr)
                    ++freeCount;
                else
                    freeCount = 1;

                if (freeCount == descriptorCount)
                {
                    pHeap->mFreeList.erase(pHeap->mFreeList.begin() + index, pHeap->mFreeList.begin() + index + descriptorCount);
                    return mDescHandle;
                }
            }
        }
    }
#ifdef CGPU_THREAD_SAFETY
    uint32_t usedDescriptors = skr_atomicu32_add_relaxed(&pHeap->mUsedDescriptors, descriptorCount);
#else
    uint32_t usedDescriptors = pHeap->mUsedDescriptors = pHeap->mUsedDescriptors + descriptorCount;
#endif
    cgpu_assert(usedDescriptors + descriptorCount <= pHeap->mDesc.NumDescriptors);
    D3D12Util_DescriptorHandle ret = {
        { pHeap->mStartHandle.mCpu.ptr + usedDescriptors * pHeap->mDescriptorSize },
        { pHeap->mStartHandle.mGpu.ptr + usedDescriptors * pHeap->mDescriptorSize },
    };
    return ret;
}

void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice,
    const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, struct D3D12Util_DescriptorHeap** ppDescHeap)
{
    uint32_t numDescriptors = pDesc->NumDescriptors;
    D3D12Util_DescriptorHeap* pHeap = (D3D12Util_DescriptorHeap*)cgpu_calloc(1, sizeof(*pHeap));
#ifdef CGPU_THREAD_SAFETY
    pHeap->pMutex = (SMutex*)cgpu_calloc(1, sizeof(SMutex));
    skr_init_mutex(pHeap->pMutex);
#endif
    pHeap->pDevice = pDevice;

    // Keep 32 aligned for easy remove
    numDescriptors = cgpu_round_up(numDescriptors, 32);

    D3D12_DESCRIPTOR_HEAP_DESC Desc = *pDesc;
    Desc.NumDescriptors = numDescriptors;
    pHeap->mDesc = Desc;

    CHECK_HRESULT(pDevice->CreateDescriptorHeap(&Desc, IID_ARGS(&pHeap->pCurrentHeap)));

    pHeap->mStartHandle.mCpu = pHeap->pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
    if (pHeap->mDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        pHeap->mStartHandle.mGpu = pHeap->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
    }
    pHeap->mDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(pHeap->mDesc.Type);
    if (Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        pHeap->pHandles = (D3D12_CPU_DESCRIPTOR_HANDLE*)cgpu_calloc(Desc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));

    *ppDescHeap = pHeap;
}

void D3D12Util_ReturnDescriptorHandles(
    struct D3D12Util_DescriptorHeap* pHeap, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t count)
{
    cgpu_assert((pHeap->mDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) == 0);
#ifdef CGPU_THREAD_SAFETY
    SMutexLock lock(*pHeap->pMutex);
#endif
    for (uint32_t i = 0; i < count; ++i)
    {
        DECLARE_ZERO(D3D12Util_DescriptorHandle, Free)
        Free.mCpu = { handle.ptr + pHeap->mDescriptorSize * i };
        Free.mGpu = { D3D12_GPU_VIRTUAL_ADDRESS_NULL };
        pHeap->mFreeList.push_back(Free);
    }
}

void D3D12Util_ResetDescriptorHeap(struct D3D12Util_DescriptorHeap* pHeap)
{
    pHeap->mUsedDescriptors = 0;
    pHeap->mFreeList.clear();
}

static void D3D12Util_FreeDescriptorHeap(D3D12Util_DescriptorHeap* pHeap)
{
    if (pHeap == nullptr) return;
    SAFE_RELEASE(pHeap->pCurrentHeap);

// Need delete since object frees allocated memory in destructor
#ifdef CGPU_THREAD_SAFETY
    skr_destroy_mutex(pHeap->pMutex);
    cgpu_free(pHeap->pMutex);
#endif

    pHeap->mFreeList.~vector();

    cgpu_free(pHeap->pHandles);
    cgpu_free(pHeap);
}