// clang-format off
#include "SkrGraphics/backend/d3d12/cgpu_d3d12.h"
#include "SkrGraphics/backend/d3d12/cgpu_d3d12_raytracing.h"
#include "../common/common_utils.h"
#include "d3d12_utils.h"

#if !defined(XBOX)
    #pragma comment(lib, "d3d12.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "dxguid.lib")
#endif

const D3D12_CPU_DESCRIPTOR_HANDLE kInvalidCPUHandle = { .ptr = 0 };
inline static D3D12_QUERY_TYPE D3D12Util_ToD3D12QueryType(ECGPUQueryType type);
inline static D3D12_QUERY_HEAP_TYPE D3D12Util_ToD3D12QueryHeapType(ECGPUQueryType type);

void cgpu_query_instance_features_d3d12(CGPUInstanceId instance, struct CGPUInstanceFeatures* features)
{
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)instance;
    (void)I;
    features->specialization_constant = false;
}

void cgpu_enum_adapters_d3d12(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: null instance!");
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)instance;
    *adapters_num = I->mAdaptersCount;
    if (!adapters)
    {
        return;
    }
    else
    {
        for (uint32_t i = 0u; i < *adapters_num; i++)
            adapters[i] = &(I->pAdapters[i].super);
    }
}

const CGPUAdapterDetail* cgpu_query_adapter_detail_d3d12(const CGPUAdapterId adapter)
{
    const CGPUAdapter_D3D12* A = (CGPUAdapter_D3D12*)adapter;
    return &A->adapter_detail;
}

uint32_t cgpu_query_queue_count_d3d12(const CGPUAdapterId adapter, const ECGPUQueueType type)
{
    // queues are virtual in d3d12.
    /*
    switch(type)
    {
        case CGPU_QUEUE_TYPE_GRAPHICS: return 1;
        case CGPU_QUEUE_TYPE_COMPUTE: return 2;
        case CGPU_QUEUE_TYPE_TRANSFER: return 2;
        default: return 0;
    }
    */
    // return UINT32_MAX;
    return 8; // Avoid returning unreasonable large number
}

CGPUInstanceId cgpu_create_instance_d3d12(CGPUInstanceDescriptor const* descriptor)
{
#if !defined(XBOX) && defined(_WIN32)
    D3D12Util_LoadDxcDLL();
#endif

    CGPUInstance_D3D12* result = cgpu_calloc(1, sizeof(CGPUInstance_D3D12));

    D3D12Util_InitializeEnvironment(&result->super);

    D3D12Util_Optionalenable_debug_layer(result, descriptor);

    UINT flags = 0;
    if (descriptor->enable_debug_layer) flags = DXGI_CREATE_FACTORY_DEBUG;
#if defined(XBOX)
#else
    if (SUCCEEDED(CreateDXGIFactory2(flags, IID_ARGS(IDXGIFactory6, &result->pDXGIFactory))))
    {
        uint32_t gpuCount             = 0;
        bool     foundSoftwareAdapter = false;
        D3D12Util_QueryAllAdapters(result, &gpuCount, &foundSoftwareAdapter);
        // If the only adapter we found is a software adapter, log error message for QA
        if (!gpuCount && foundSoftwareAdapter)
        {
            cgpu_assert(0 && "The only available GPU has DXGI_ADAPTER_FLAG_SOFTWARE. Early exiting");
            return CGPU_NULLPTR;
        }
    }
    else
    {
        cgpu_assert("[D3D12 Fatal]: Create DXGIFactory2 Failed!");
    }
#endif
    return &result->super;
}

void cgpu_free_instance_d3d12(CGPUInstanceId instance)
{
    CGPUInstance_D3D12* to_destroy = (CGPUInstance_D3D12*)instance;
    D3D12Util_DeInitializeEnvironment(&to_destroy->super);
    if (to_destroy->mAdaptersCount > 0)
    {
        for (uint32_t i = 0; i < to_destroy->mAdaptersCount; i++)
        {
            SAFE_RELEASE(to_destroy->pAdapters[i].pDxActiveGPU);
        }
    }
    cgpu_free(to_destroy->pAdapters);
    SAFE_RELEASE(to_destroy->pDXGIFactory);
    if (to_destroy->pDXDebug)
    {
        SAFE_RELEASE(to_destroy->pDXDebug);
    }
    cgpu_free(to_destroy);

#if !defined(XBOX) && defined(_WIN32)
    D3D12Util_UnloadDxcDLL();
#endif

#if !SKR_SHIPPING
    {
        IDXGIDebug1* dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_ARGS(IDXGIDebug1, &dxgiDebug))))
        {
            COM_CALL(ReportLiveObjects, dxgiDebug, DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        }
        SAFE_RELEASE(dxgiDebug);
    }
#endif
}

CGPUDeviceId cgpu_create_device_d3d12(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc)
{
    CGPUAdapter_D3D12*  A                          = (CGPUAdapter_D3D12*)adapter;
    CGPUInstance_D3D12* I                          = (CGPUInstance_D3D12*)A->super.instance;
    CGPUDevice_D3D12*   D                          = cgpu_calloc(1, sizeof(CGPUDevice_D3D12));
    *(CGPUAdapterId*)&D->super.adapter = adapter;

    if (!SUCCEEDED(D3D12CreateDevice((IUnknown*)A->pDxActiveGPU, A->mFeatureLevel, IID_ARGS(ID3D12Device, &D->pDxDevice))))
    {
        cgpu_assert("[D3D12 Fatal]: Create D3D12Device Failed!");
    }

    // Create Requested Queues.
    for (uint32_t i = 0u; i < desc->queue_group_count; i++)
    {
        const CGPUQueueGroupDescriptor* pQueueGroup = desc->queue_groups + i;
        const ECGPUQueueType type = pQueueGroup->queue_type;

        *(uint32_t*)&D->pCommandQueueCounts[type] = pQueueGroup->queue_count;
        *(ID3D12CommandQueue***)&D->ppCommandQueues[type] = (ID3D12CommandQueue**)cgpu_malloc(sizeof(ID3D12CommandQueue*) * pQueueGroup->queue_count);

        for (uint32_t j = 0u; j < pQueueGroup->queue_count; j++)
        {
            SKR_DECLARE_ZERO(D3D12_COMMAND_QUEUE_DESC, queueDesc)
            switch (type)
            {
                case CGPU_QUEUE_TYPE_GRAPHICS:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    break;
                case CGPU_QUEUE_TYPE_COMPUTE:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
                case CGPU_QUEUE_TYPE_TRANSFER:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;
                default:
                    cgpu_assert(0 && "[D3D12 Fatal]: Unsupported ECGPUQueueType!");
                    return CGPU_NULLPTR;
            }
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            if (!SUCCEEDED(COM_CALL(CreateCommandQueue, D->pDxDevice, &queueDesc, IID_ARGS(ID3D12CommandQueue, &D->ppCommandQueues[type][j]))))
            {
                cgpu_assert("[D3D12 Fatal]: Create D3D12CommandQueue Failed!");
            }
        }
    }
    // Create D3D12MA Allocator
    D3D12Util_CreateDMAAllocator(I, A, D);
    cgpu_assert(D->pResourceAllocator && "DMA Allocator Must be Created!");

    // Create Tiled Memory Pool
    const uint64_t kTileSize = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    CGPUMemoryPoolDescriptor poolDesc  = {};
    poolDesc.type = CGPU_MEM_POOL_TYPE_TILED;
    poolDesc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    poolDesc.min_alloc_alignment = kTileSize;
    poolDesc.block_size = kTileSize * 256;
    poolDesc.min_block_count = 32;
    poolDesc.max_block_count = 256;
    D->pTiledMemoryPool = (struct CGPUTiledMemoryPool_D3D12*)cgpu_create_memory_pool_d3d12(&D->super, &poolDesc);
    if (A->mTiledResourceTier <= D3D12_TILED_RESOURCES_TIER_1)
    {
        const uint64_t kPageSize = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        D3D12_HEAP_DESC heapDesc = {};
        heapDesc.Alignment = kPageSize;
        heapDesc.SizeInBytes = kPageSize;
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapDesc.Properties.VisibleNodeMask = CGPU_SINGLE_GPU_NODE_MASK;
        HRESULT hres = COM_CALL(CreateHeap, D->pDxDevice, &heapDesc, IID_ARGS(ID3D12Heap, &D->pUndefinedTileHeap));
        CHECK_HRESULT(hres);
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        queueDesc.NodeMask = CGPU_SINGLE_GPU_NODE_MASK;
        hres = COM_CALL(CreateCommandQueue, D->pDxDevice, &queueDesc, IID_ARGS(ID3D12CommandQueue, &D->pUndefinedTileMappingQueue));
        CHECK_HRESULT(hres);
        COM_CALL(SetName, D->pUndefinedTileHeap, TEXT("UndefinedTileHeap"));
        COM_CALL(SetName, D->pUndefinedTileMappingQueue, TEXT("MappingQueue"));
    }

    // Create Descriptor Heaps
    D->pCPUDescriptorHeaps = (D3D12Util_DescriptorHeap**)cgpu_malloc(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES * sizeof(D3D12Util_DescriptorHeap*));
    D->pCbvSrvUavHeaps = (D3D12Util_DescriptorHeap**)cgpu_malloc(sizeof(D3D12Util_DescriptorHeap*));
    D->pSamplerHeaps = (D3D12Util_DescriptorHeap**)cgpu_malloc(sizeof(D3D12Util_DescriptorHeap*));
    D->pNullDescriptors = (CGPUEmptyDescriptors_D3D12*)cgpu_calloc(1, sizeof(CGPUEmptyDescriptors_D3D12));
    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Flags = gCpuDescriptorHeapProperties[i].mFlags;
        desc.NodeMask = 0; // CPU Descriptor Heap - Node mask is irrelevant
        desc.NumDescriptors = gCpuDescriptorHeapProperties[i].mMaxDescriptors;
        desc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
#ifdef _DEBUG
        auto        sz      = D->pDxDevice->GetDescriptorHandleIncrementSize(desc.Type);
        const char* types[] = {
            "CBV_SRV_UAV",
            "SAMPLER",
            "RTV",
            "DSV",
        };
        cgpu_trace(u8"D3D12 descriptor heap type: %s, increment size: %d, Total allocated: %d(bytes)", types[desc.Type], sz, sz * desc.NumDescriptors);
#endif
        D3D12Util_CreateDescriptorHeap(D->pDxDevice, &desc, &D->pCPUDescriptorHeaps[i]);
    }
    // One shader visible heap for each linked node
    for (uint32_t i = 0; i < CGPU_SINGLE_GPU_NODE_COUNT; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = CGPU_SINGLE_GPU_NODE_MASK;

        desc.NumDescriptors = 1 << 16;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        D3D12Util_CreateDescriptorHeap(D->pDxDevice, &desc, &D->pCbvSrvUavHeaps[i]);

        desc.NumDescriptors = 1 << 11;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        D3D12Util_CreateDescriptorHeap(D->pDxDevice, &desc, &D->pSamplerHeaps[i]);
    }
    // Allocate NULL Descriptors
    {
        D->pNullDescriptors->Sampler = kInvalidCPUHandle;
        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        D->pNullDescriptors->Sampler = D3D12Util_ConsumeDescriptorHandles(
            D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], 1
        ).mCpu;
        COM_CALL(CreateSampler, D->pDxDevice, &samplerDesc, D->pNullDescriptors->Sampler);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc  = {};
        srvDesc.Format = DXGI_FORMAT_R8_UINT;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_R8_UINT;

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_1D]);
        D3D12Util_CreateUAV(D, NULL, NULL, &uavDesc, &D->pNullDescriptors->TextureUAV[CGPU_TEX_DIMENSION_1D]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_2D]);
        D3D12Util_CreateUAV(D, NULL, NULL, &uavDesc, &D->pNullDescriptors->TextureUAV[CGPU_TEX_DIMENSION_2D]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_2DMS]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_3D]);
        D3D12Util_CreateUAV(D, NULL, NULL, &uavDesc, &D->pNullDescriptors->TextureUAV[CGPU_TEX_DIMENSION_3D]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_CUBE]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_1D_ARRAY]);
        D3D12Util_CreateUAV(D, NULL, NULL, &uavDesc, &D->pNullDescriptors->TextureUAV[CGPU_TEX_DIMENSION_1D_ARRAY]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_2D_ARRAY]);
        D3D12Util_CreateUAV(D, NULL, NULL, &uavDesc, &D->pNullDescriptors->TextureUAV[CGPU_TEX_DIMENSION_2D_ARRAY]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_2DMS_ARRAY]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->TextureSRV[CGPU_TEX_DIMENSION_CUBE_ARRAY]);

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        D3D12Util_CreateSRV(D, NULL, &srvDesc, &D->pNullDescriptors->BufferSRV);
        D3D12Util_CreateUAV(D, NULL, NULL, &uavDesc, &D->pNullDescriptors->BufferUAV);
        D3D12Util_CreateCBV(D, NULL, &D->pNullDescriptors->BufferCBV);
    }
    // pipeline cache
    D3D12_FEATURE_DATA_SHADER_CACHE feature = {};
    HRESULT result = COM_CALL(CheckFeatureSupport, D->pDxDevice, D3D12_FEATURE_SHADER_CACHE, &feature, sizeof(feature));
    if (SUCCEEDED(result))
    {
        result = E_NOTIMPL;
        if (feature.SupportFlags & D3D12_SHADER_CACHE_SUPPORT_LIBRARY)
        {
            ID3D12Device1* device1 = NULL;
            result = COM_CALL(QueryInterface, D->pDxDevice, IID_ARGS(ID3D12Device1, &device1));
            if (SUCCEEDED(result))
            {
                result = COM_CALL(CreatePipelineLibrary, device1, D->pPSOCacheData, 0, IID_ARGS(ID3D12PipelineLibrary, &D->pPipelineLibrary));
            }
            SAFE_RELEASE(device1);
        }
    }
    // for ray tracing
    result = COM_CALL(QueryInterface, D->pDxDevice, IID_ARGS(ID3D12Device5, &D->pDxDevice5));
    return &D->super;
}

void cgpu_free_device_d3d12(CGPUDeviceId device)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    for (uint32_t t = 0u; t < CGPU_QUEUE_TYPE_COUNT; t++)
    {
        for (uint32_t i = 0; i < D->pCommandQueueCounts[t]; i++)
        {
            SAFE_RELEASE(D->ppCommandQueues[t][i]);
        }
        cgpu_free((ID3D12CommandQueue**)D->ppCommandQueues[t]);
    }
    // Free Tiled Pool
    SAFE_RELEASE(D->pUndefinedTileHeap);
    SAFE_RELEASE(D->pUndefinedTileMappingQueue);
    cgpu_free_memory_pool_d3d12((CGPUMemoryPoolId)D->pTiledMemoryPool);
    // Free D3D12MA Allocator
    D3D12Util_FreeDMAAllocator(D);
    // Free Descriptor Heaps
    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
    {
        D3D12Util_FreeDescriptorHeap(D->pCPUDescriptorHeaps[i]);
    }
    D3D12Util_FreeDescriptorHeap(D->pCbvSrvUavHeaps[0]);
    D3D12Util_FreeDescriptorHeap(D->pSamplerHeaps[0]);
    cgpu_free(D->pCPUDescriptorHeaps);
    cgpu_free(D->pCbvSrvUavHeaps);
    cgpu_free(D->pSamplerHeaps);
    cgpu_free(D->pNullDescriptors);
    SAFE_RELEASE(D->pDxDevice);
    SAFE_RELEASE(D->pPipelineLibrary);
    if (D->pPSOCacheData) cgpu_free(D->pPSOCacheData);
    SAFE_RELEASE(D->pDxDevice5);
    cgpu_free(D);
}

void cgpu_query_video_memory_info_d3d12(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    const CGPUAdapter_D3D12*     A    = (CGPUAdapter_D3D12*)device->adapter;
    DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
    COM_CALL(QueryVideoMemoryInfo, A->pDxActiveGPU, CGPU_SINGLE_GPU_NODE_INDEX, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
    *total      = info.Budget;
    *used_bytes = info.CurrentUsage;
}

void cgpu_query_shared_memory_info_d3d12(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    const CGPUAdapter_D3D12*     A    = (CGPUAdapter_D3D12*)device->adapter;
    DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
    COM_CALL(QueryVideoMemoryInfo, A->pDxActiveGPU, CGPU_SINGLE_GPU_NODE_INDEX, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &info);
    *total      = info.Budget;
    *used_bytes = info.CurrentUsage;
}

// API Objects APIs
CGPUFenceId cgpu_create_fence_d3d12(CGPUDeviceId device)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    // create a Fence and cgpu_assert that it is valid
    CGPUFence_D3D12* F = cgpu_calloc(1, sizeof(CGPUFence_D3D12));
    cgpu_assert(F);

    CHECK_HRESULT(COM_CALL(CreateFence, D->pDxDevice, 0, D3D12_FENCE_FLAG_NONE, IID_ARGS(ID3D12Fence, &F->pDxFence)));
    F->mFenceValue = 1;

    F->pDxWaitIdleFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    return &F->super;
}

void cgpu_wait_fences_d3d12(const CGPUFenceId* fences, uint32_t fence_count)
{
    const CGPUFence_D3D12** Fences = (const CGPUFence_D3D12**)fences;
    for (uint32_t i = 0; i < fence_count; ++i)
    {
        ECGPUFenceStatus fenceStatus = cgpu_query_fence_status(fences[i]);
        uint64_t         fenceValue  = Fences[i]->mFenceValue - 1;
        if (fenceStatus == CGPU_FENCE_STATUS_INCOMPLETE)
        {
            COM_CALL(SetEventOnCompletion, Fences[i]->pDxFence, fenceValue, Fences[i]->pDxWaitIdleFenceEvent);
            WaitForSingleObject(Fences[i]->pDxWaitIdleFenceEvent, INFINITE);
        }
    }
}

ECGPUFenceStatus cgpu_query_fence_status_d3d12(CGPUFenceId fence)
{
    ECGPUFenceStatus status = CGPU_FENCE_STATUS_COMPLETE;
    CGPUFence_D3D12* F      = (CGPUFence_D3D12*)fence;
    const uint64_t Value = COM_CALL(GetCompletedValue, F->pDxFence);
    if (Value < F->mFenceValue - 1)
        status = CGPU_FENCE_STATUS_INCOMPLETE;
    else
        status = CGPU_FENCE_STATUS_COMPLETE;
    return status;
}

void cgpu_free_fence_d3d12(CGPUFenceId fence)
{
    CGPUFence_D3D12* F = (CGPUFence_D3D12*)fence;
    SAFE_RELEASE(F->pDxFence);
    CloseHandle(F->pDxWaitIdleFenceEvent);
    cgpu_free(F);
}

CGPUSemaphoreId cgpu_create_semaphore_d3d12(CGPUDeviceId device)
{
    return (CGPUSemaphoreId)cgpu_create_fence(device);
}

void cgpu_free_semaphore_d3d12(CGPUSemaphoreId semaphore)
{
    return cgpu_free_fence((CGPUFenceId)semaphore);
}

CGPURootSignaturePoolId cgpu_create_root_signature_pool_d3d12(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc)
{
    return CGPUUtil_CreateRootSignaturePool(desc);
}

void cgpu_free_root_signature_pool_d3d12(CGPURootSignaturePoolId pool)
{
    CGPUUtil_FreeRootSignaturePool(pool);
}

// for example, shader register set: (s0t0) (s0b1) [s0b0[root]] (s1t0) (s1t1) {s2s0{static}}
// rootParams: |   s0    |   s1    |  [s0b0]  |
// rootRanges: | s0t0 | s0b1 | s1t0 | s1t1 |
// staticSamplers: |  s2s0  |   ...   |
CGPURootSignatureId cgpu_create_root_signature_d3d12(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc)
{
    CGPUDevice_D3D12*        D  = (CGPUDevice_D3D12*)device;
    CGPURootSignature_D3D12* RS = cgpu_calloc(1, sizeof(CGPURootSignature_D3D12));
    // Pick root parameters from desc data
    CGPUShaderStages shaderStages = 0;
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        CGPUShaderEntryDescriptor* shader_desc = desc->shaders + i;
        shaderStages |= shader_desc->stage;
    }
    // Pick shader reflection data
    CGPUUtil_InitRSParamTables((CGPURootSignature*)RS, desc);
    // [RS POOL] ALLOCATION
    if (desc->pool)
    {
        CGPURootSignature_D3D12* poolSig =
        (CGPURootSignature_D3D12*)CGPUUtil_TryAllocateSignature(desc->pool, &RS->super, desc);
        if (poolSig != CGPU_NULLPTR)
        {
            RS->mRootConstantParam = poolSig->mRootConstantParam;
            RS->pDxRootSignature = poolSig->pDxRootSignature;
            RS->mRootParamIndex = poolSig->mRootParamIndex;
            RS->super.pool = desc->pool;
            RS->super.pool_sig = &poolSig->super;
            return &RS->super;
        }
    }
    // [RS POOL] END ALLOCATION
    // Fill resource slots
    // Only support descriptor tables now
    // TODO: Support root CBVs
    //       Add backend sort for better performance
    const UINT tableCount = RS->super.table_count;
    UINT descRangeCount = 0;
    for (uint32_t i = 0; i < tableCount; i++)
    {
        descRangeCount += RS->super.tables[i].resources_count;
    }
    D3D12_ROOT_PARAMETER1* rootParams = (D3D12_ROOT_PARAMETER1*)cgpu_calloc(
    tableCount + RS->super.push_constant_count,
    sizeof(D3D12_ROOT_PARAMETER1));
    D3D12_DESCRIPTOR_RANGE1* cbvSrvUavRanges = (D3D12_DESCRIPTOR_RANGE1*)cgpu_calloc(descRangeCount, sizeof(D3D12_DESCRIPTOR_RANGE1));
    // Create descriptor tables
    UINT valid_root_tables = 0;
    for (uint32_t i_set = 0, i_range = 0; i_set < RS->super.table_count; i_set++)
    {
        CGPUParameterTable* paramTable = &RS->super.tables[i_set];
        D3D12_ROOT_PARAMETER1* rootParam = &rootParams[valid_root_tables];
        rootParam->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        CGPUShaderStages visStages = CGPU_SHADER_STAGE_NONE;
        const D3D12_DESCRIPTOR_RANGE1* descRangeCursor = &cbvSrvUavRanges[i_range];
        for (uint32_t i_register = 0; i_register < paramTable->resources_count; i_register++)
        {
            CGPUShaderResource* reflSlot = &paramTable->resources[i_register];
            visStages |= reflSlot->stages;
            {
                D3D12_DESCRIPTOR_RANGE1* descRange = &cbvSrvUavRanges[i_range];
                descRange->RegisterSpace = reflSlot->set;
                descRange->BaseShaderRegister = reflSlot->binding;
                descRange->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                descRange->NumDescriptors = (reflSlot->type != CGPU_RESOURCE_TYPE_UNIFORM_BUFFER) ? reflSlot->size : 1;
                descRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                descRange->RangeType = D3D12Util_ResourceTypeToDescriptorRangeType(reflSlot->type);
                rootParam->DescriptorTable.NumDescriptorRanges++;
                i_range++;
            }
        }
        if (visStages != 0)
        {
            rootParam->ShaderVisibility = D3D12Util_TranslateShaderStages(visStages);
            rootParam->DescriptorTable.pDescriptorRanges = descRangeCursor;
            valid_root_tables++;
        }
    }
    // Root Const
    assert(RS->super.push_constant_count <= 1 && "Only support 1 push const now!");
    if (RS->super.push_constant_count)
    {
        CGPUShaderResource* reflSlot = RS->super.push_constants;
        RS->mRootConstantParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        RS->mRootConstantParam.Constants.RegisterSpace = reflSlot->set;
        RS->mRootConstantParam.Constants.ShaderRegister = reflSlot->binding;
        RS->mRootConstantParam.Constants.Num32BitValues = reflSlot->size / sizeof(uint32_t);
        RS->mRootConstantParam.ShaderVisibility = D3D12Util_TranslateShaderStages(reflSlot->stages);
    }
    // Create static samplers
    UINT staticSamplerCount = desc->static_sampler_count;
    D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs = CGPU_NULLPTR;
    if (staticSamplerCount > 0)
    {
        staticSamplerDescs = (D3D12_STATIC_SAMPLER_DESC*)cgpu_calloc(staticSamplerCount, sizeof(D3D12_STATIC_SAMPLER_DESC));
        for (uint32_t i = 0; i < RS->super.static_sampler_count; i++)
        {
            CGPUShaderResource* RST_slot = &RS->super.static_samplers[i];
            for (uint32_t j = 0; j < desc->static_sampler_count; j++)
            {
                CGPUSampler_D3D12* input_slot = (CGPUSampler_D3D12*)desc->static_samplers[j];
                if (strcmp((const char*)RST_slot->name, (const char*)desc->static_sampler_names[j]) == 0)
                {
                    D3D12_SAMPLER_DESC* dxSamplerDesc = &input_slot->mDxDesc;
                    staticSamplerDescs[i].Filter = dxSamplerDesc->Filter;
                    staticSamplerDescs[i].AddressU = dxSamplerDesc->AddressU;
                    staticSamplerDescs[i].AddressV = dxSamplerDesc->AddressV;
                    staticSamplerDescs[i].AddressW = dxSamplerDesc->AddressW;
                    staticSamplerDescs[i].MipLODBias = dxSamplerDesc->MipLODBias;
                    staticSamplerDescs[i].MaxAnisotropy = dxSamplerDesc->MaxAnisotropy;
                    staticSamplerDescs[i].ComparisonFunc = dxSamplerDesc->ComparisonFunc;
                    staticSamplerDescs[i].MinLOD = dxSamplerDesc->MinLOD;
                    staticSamplerDescs[i].MaxLOD = dxSamplerDesc->MaxLOD;
                    staticSamplerDescs[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

                    CGPUShaderResource* samplerResource = RST_slot;
                    staticSamplerDescs[i].RegisterSpace = samplerResource->set;
                    staticSamplerDescs[i].ShaderRegister = samplerResource->binding;
                    staticSamplerDescs[i].ShaderVisibility = D3D12Util_TranslateShaderStages(samplerResource->stages);
                }
            }
        }
    }
    bool useInputLayout = shaderStages & CGPU_SHADER_STAGE_VERT; // VertexStage uses input layout
    // Fill RS flags
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    if (useInputLayout)
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    if (!(shaderStages & CGPU_SHADER_STAGE_VERT))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    if (!(shaderStages & CGPU_SHADER_STAGE_HULL))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if (!(shaderStages & CGPU_SHADER_STAGE_DOMAIN))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if (!(shaderStages & CGPU_SHADER_STAGE_GEOM))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    if (!(shaderStages & CGPU_SHADER_STAGE_FRAG))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    // Serialize versioned RS
    const UINT paramCount = valid_root_tables + RS->super.push_constant_count /*must be 0 or 1 now*/;
    // Root Constant
    assert(RS->super.push_constant_count <= 1 && "Only support 1 push const now!");
    for (uint32_t i = 0; i < RS->super.push_constant_count; i++)
    {
        rootParams[valid_root_tables + i] = RS->mRootConstantParam;
        RS->mRootParamIndex               = valid_root_tables + i;
    }
    // Serialize root signature
    ID3DBlob* error               = NULL;
    ID3DBlob* rootSignatureString = NULL;
    SKR_DECLARE_ZERO(D3D12_VERSIONED_ROOT_SIGNATURE_DESC, sig_desc);
    sig_desc.Version                    = D3D_ROOT_SIGNATURE_VERSION_1_1;
    sig_desc.Desc_1_1.NumParameters     = paramCount;
    sig_desc.Desc_1_1.pParameters       = rootParams;
    sig_desc.Desc_1_1.NumStaticSamplers = staticSamplerCount;
    sig_desc.Desc_1_1.pStaticSamplers   = staticSamplerDescs;
    sig_desc.Desc_1_1.Flags             = rootSignatureFlags;
    HRESULT hres                        = D3D12SerializeVersionedRootSignature(&sig_desc, &rootSignatureString, &error);
    if (staticSamplerDescs != CGPU_NULLPTR)
        cgpu_free(staticSamplerDescs);

    if (!SUCCEEDED(hres))
        cgpu_error(u8"Failed to serialize root signature with error (%s)", (char*)COM_CALL(GetBufferPointer, error));

        // If running Linked Mode (SLI) create root signature for all nodes
    // #NOTE : In non SLI mode, mNodeCount will be 0 which sets nodeMask to
    // default value
    LPVOID BufferPointer = COM_CALL(GetBufferPointer, rootSignatureString);
    SIZE_T BufferSize = COM_CALL(GetBufferSize, rootSignatureString);
    CHECK_HRESULT(COM_CALL(CreateRootSignature, D->pDxDevice, CGPU_SINGLE_GPU_NODE_MASK, BufferPointer, BufferSize, IID_ARGS(ID3D12RootSignature, &RS->pDxRootSignature)));
    
    cgpu_free(rootParams);
    cgpu_free(cbvSrvUavRanges);
    // [RS POOL] INSERTION
    if (desc->pool)
    {
        CGPURootSignatureId result = CGPUUtil_AddSignature(desc->pool, &RS->super, desc);
        cgpu_assert(result && u8"Root signature pool insertion failed!");
        return result;
    }
    // [RS POOL] END INSERTION
    return &RS->super;
}

void cgpu_free_root_signature_d3d12(CGPURootSignatureId signature)
{
    CGPURootSignature_D3D12* RS = (CGPURootSignature_D3D12*)signature;
    // [RS POOL] FREE
    if (signature->pool)
    {
        CGPUUtil_PoolFreeSignature(signature->pool, signature);
        return;
    }
    // [RS POOL] END FREE
    CGPUUtil_FreeRSParamTables((CGPURootSignature*)signature);
    SAFE_RELEASE(RS->pDxRootSignature);
    cgpu_free(RS);
    return;
}

inline static uint32_t D3D12Util_ComputeNeededDescriptorCount(CGPUShaderResource* resource)
{
    if (resource->dim == CGPU_TEX_DIMENSION_1D_ARRAY ||
        resource->dim == CGPU_TEX_DIMENSION_2D_ARRAY ||
        resource->dim == CGPU_TEX_DIMENSION_2DMS_ARRAY ||
        resource->dim == CGPU_TEX_DIMENSION_CUBE_ARRAY)
        return resource->size;
    else
        return 1;
}

CGPUDescriptorSetId cgpu_create_descriptor_set_d3d12(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc)
{
    CGPUDevice_D3D12*                D              = (CGPUDevice_D3D12*)device;
    const CGPURootSignature_D3D12*   RS             = (const CGPURootSignature_D3D12*)desc->root_signature;
    CGPUDescriptorSet_D3D12*         Set            = cgpu_calloc(1, sizeof(CGPUDescriptorSet_D3D12));
    const uint32_t                   nodeIndex      = CGPU_SINGLE_GPU_NODE_INDEX;
    struct D3D12Util_DescriptorHeap* pCbvSrvUavHeap = D->pCbvSrvUavHeaps[nodeIndex];
    struct D3D12Util_DescriptorHeap* pSamplerHeap   = D->pSamplerHeaps[nodeIndex];
    (void)pSamplerHeap;
    CGPUParameterTable* param_table = &RS->super.tables[desc->set_index];
    uint32_t CbvSrvUavCount = 0;
    uint32_t SamplerCount   = 0;
    for (uint32_t i = 0; i < param_table->resources_count; i++)
    {
        if (param_table->resources[i].type == CGPU_RESOURCE_TYPE_SAMPLER)
        {
            SamplerCount++;
        }
        else if (param_table->resources[i].type == CGPU_RESOURCE_TYPE_TEXTURE ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_RW_TEXTURE ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_BUFFER ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_BUFFER_RAW ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_RW_BUFFER ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_RW_BUFFER_RAW ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_TEXTURE_CUBE ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
        {
            CbvSrvUavCount += D3D12Util_ComputeNeededDescriptorCount(&param_table->resources[i]);
        }
    }
    // CBV/SRV/UAV
    Set->mCbvSrvUavHandle = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    Set->mSamplerHandle = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    if (CbvSrvUavCount)
    {
        D3D12Util_DescriptorHandle StartHandle = D3D12Util_ConsumeDescriptorHandles(pCbvSrvUavHeap, CbvSrvUavCount);
        Set->mCbvSrvUavHandle = StartHandle.mGpu.ptr - D3D12Util_GetStartHandle(pCbvSrvUavHeap).mGpu.ptr;
        Set->mCbvSrvUavStride = CbvSrvUavCount * D3D12Util_GetDescriptorSize(pCbvSrvUavHeap);
    }
    if (SamplerCount)
    {
        D3D12Util_DescriptorHandle StartHandle = D3D12Util_ConsumeDescriptorHandles(pSamplerHeap, SamplerCount);
        Set->mSamplerHandle = StartHandle.mGpu.ptr - D3D12Util_GetStartHandle(pSamplerHeap).mGpu.ptr;
        Set->mSamplerStride = SamplerCount * D3D12Util_GetDescriptorSize(pSamplerHeap);
    }
    // Bind NULL handles on creation
    if (CbvSrvUavCount || SamplerCount)
    {
        uint32_t CbvSrvUavHeapOffset = 0;
        uint32_t SamplerHeapOffset   = 0;
        for (uint32_t i = 0; i < param_table->resources_count; ++i)
        {
            const ECGPUTextureDimension dimension = param_table->resources[i].dim;
            D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = kInvalidCPUHandle;
            D3D12_CPU_DESCRIPTOR_HANDLE srcSamplerHandle = kInvalidCPUHandle;
            switch (param_table->resources[i].type)
            {
                case CGPU_RESOURCE_TYPE_TEXTURE:
                    srcHandle = D->pNullDescriptors->TextureSRV[dimension];
                    break;
                case CGPU_RESOURCE_TYPE_BUFFER:
                    srcHandle = D->pNullDescriptors->BufferSRV;
                    break;
                case CGPU_RESOURCE_TYPE_RW_BUFFER:
                    srcHandle = D->pNullDescriptors->BufferUAV;
                    break;
                case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER:
                    srcHandle = D->pNullDescriptors->BufferCBV;
                    break;
                case CGPU_RESOURCE_TYPE_SAMPLER:
                    srcSamplerHandle = D->pNullDescriptors->Sampler;
                    break;
                default:
                    break;
            }
            if (srcHandle.ptr != kInvalidCPUHandle.ptr)
            {
                for (uint32_t j = 0; j < param_table->resources[i].size; j++)
                {
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, srcHandle, Set->mCbvSrvUavHandle, CbvSrvUavHeapOffset);
                    CbvSrvUavHeapOffset++;
                }
            }
            if (srcSamplerHandle.ptr != kInvalidCPUHandle.ptr)
            {
                for (uint32_t j = 0; j < param_table->resources[i].size; j++)
                {
                    D3D12Util_CopyDescriptorHandle(pSamplerHeap, srcSamplerHandle, Set->mSamplerHandle, SamplerHeapOffset);
                    SamplerHeapOffset++;
                }
            }
        }
    }
    // TODO: Support root descriptors
    return &Set->super;
}

void cgpu_update_descriptor_set_d3d12(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count)
{
    CGPUDescriptorSet_D3D12*         Set            = (CGPUDescriptorSet_D3D12*)set;
    const CGPURootSignature_D3D12*   RS             = (const CGPURootSignature_D3D12*)set->root_signature;
    CGPUDevice_D3D12*                D              = (CGPUDevice_D3D12*)set->root_signature->device;
    CGPUParameterTable*              ParamTable     = &RS->super.tables[set->index];
    const uint32_t                   nodeIndex      = CGPU_SINGLE_GPU_NODE_INDEX;
    struct D3D12Util_DescriptorHeap* pCbvSrvUavHeap = D->pCbvSrvUavHeaps[nodeIndex];
    struct D3D12Util_DescriptorHeap* pSamplerHeap   = D->pSamplerHeaps[nodeIndex];
    for (uint32_t i = 0; i < count; i++)
    {
        // Descriptor Info
        const CGPUDescriptorData* pParam     = datas + i;
        CGPUShaderResource*       ResData    = CGPU_NULLPTR;
        uint32_t                  HeapOffset = 0;
        if (pParam->name != CGPU_NULLPTR)
        {
            size_t argNameHash = cgpu_name_hash(pParam->name, strlen((const char*)pParam->name));
            for (uint32_t j = 0; j < ParamTable->resources_count; j++)
            {
                if (ParamTable->resources[j].name_hash == argNameHash &&
                    strcmp((const char*)ParamTable->resources[j].name, (const char*)pParam->name) == 0)
                {
                    ResData = ParamTable->resources + j;
                    break;
                }
                HeapOffset += D3D12Util_ComputeNeededDescriptorCount(&ParamTable->resources[j]);
            }
        }
        else
        {
            for (uint32_t j = 0; j < ParamTable->resources_count; j++)
            {
                if (ParamTable->resources[j].type == pParam->binding_type &&
                    ParamTable->resources[j].binding == pParam->binding)
                {
                    ResData = &ParamTable->resources[j];
                    break;
                }
                HeapOffset += D3D12Util_ComputeNeededDescriptorCount(&ParamTable->resources[j]);
            }
        }
        // Update Info
        const uint32_t arrayCount = cgpu_max(1U, pParam->count);
        switch (ResData->type)
        {
            case CGPU_RESOURCE_TYPE_SAMPLER: {
                cgpu_assert(pParam->samplers && "cgpu_assert: Binding NULL Sampler(s)!");
                const CGPUSampler_D3D12** Samplers = (const CGPUSampler_D3D12**)pParam->samplers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->samplers[arr] && "cgpu_assert: Binding NULL Sampler!");
                    const D3D12_CPU_DESCRIPTOR_HANDLE ToCopy = { Samplers[arr]->mDxHandle.ptr };
                    D3D12Util_CopyDescriptorHandle(pSamplerHeap, ToCopy, Set->mSamplerHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_TEXTURE:
            case CGPU_RESOURCE_TYPE_TEXTURE_CUBE: {
                cgpu_assert(pParam->textures && "cgpu_assert: Binding NULL Textures(s)!");
                const CGPUTextureView_D3D12** Textures = (const CGPUTextureView_D3D12**)pParam->textures;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->textures[arr] && "cgpu_assert: Binding NULL Textures!");
                    const D3D12_CPU_DESCRIPTOR_HANDLE ToCopy = { Textures[arr]->mDxDescriptorHandles.ptr + Textures[arr]->mDxSrvOffset };
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, ToCopy, Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_BUFFER:
            case CGPU_RESOURCE_TYPE_BUFFER_RAW: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                const CGPUBuffer_D3D12** Buffers = (const CGPUBuffer_D3D12**)pParam->buffers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    const D3D12_CPU_DESCRIPTOR_HANDLE ToCopy = { Buffers[arr]->mDxDescriptorHandles.ptr + Buffers[arr]->mDxSrvOffset };
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, ToCopy, Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                const CGPUBuffer_D3D12** Buffers = (const CGPUBuffer_D3D12**)pParam->buffers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    const D3D12_CPU_DESCRIPTOR_HANDLE ToCopy = { Buffers[arr]->mDxDescriptorHandles.ptr };
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, ToCopy, Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_RW_TEXTURE: {
                cgpu_assert(pParam->textures && "cgpu_assert: Binding NULL Texture(s)!");
                const CGPUTextureView_D3D12** Textures = (const CGPUTextureView_D3D12**)pParam->textures;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->textures[arr] && "cgpu_assert: Binding NULL Texture!");
                    const D3D12_CPU_DESCRIPTOR_HANDLE ToCopy = { Textures[arr]->mDxDescriptorHandles.ptr + Textures[arr]->mDxUavOffset };
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, ToCopy, Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_RW_BUFFER:
            case CGPU_RESOURCE_TYPE_RW_BUFFER_RAW: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                const CGPUBuffer_D3D12** Buffers = (const CGPUBuffer_D3D12**)pParam->buffers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    const D3D12_CPU_DESCRIPTOR_HANDLE ToCopy = { Buffers[arr]->mDxDescriptorHandles.ptr + Buffers[arr]->mDxUavOffset };
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, ToCopy, Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_ACCELERATION_STRUCTURE: {
                cgpu_assert(pParam->acceleration_structures && "NULL Acceleration Structure(s)");
                CGPUAccelerationStructure_D3D12* pAccel = (CGPUAccelerationStructure_D3D12*)pParam->acceleration_structures[0];
                const CGPUBuffer_D3D12* pASBuffer = (const CGPUBuffer_D3D12*)pAccel->pASBuffer;
                const D3D12_CPU_DESCRIPTOR_HANDLE ToCopy = { pASBuffer->mDxDescriptorHandles.ptr + pASBuffer->mDxSrvOffset };
                D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap, ToCopy, Set->mCbvSrvUavHandle, HeapOffset);
                Set->pBoundAccel = pAccel;
            }
            break;
            default:
                break;
        }
    }
}

void cgpu_free_descriptor_set_d3d12(CGPUDescriptorSetId set)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)set->root_signature->device;
    CGPUDescriptorSet_D3D12* Set = (CGPUDescriptorSet_D3D12*)set;
    (void)D; // TODO: recycle of descriptor set heap handles
    cgpu_free(Set);
}

CGPUComputePipelineId cgpu_create_compute_pipeline_d3d12(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc)
{
    CGPUDevice_D3D12*          D   = (CGPUDevice_D3D12*)device;
    CGPUComputePipeline_D3D12* PPL = cgpu_calloc(1, sizeof(CGPUComputePipeline_D3D12));
    CGPURootSignature_D3D12*   RS  = (CGPURootSignature_D3D12*)desc->root_signature;
    CGPUShaderLibrary_D3D12*   SL  = (CGPUShaderLibrary_D3D12*)desc->compute_shader->library;
    PPL->pRootSignature            = RS->pDxRootSignature;
    // Add pipeline specifying its for compute purposes
    SKR_DECLARE_ZERO(D3D12_SHADER_BYTECODE, CS);
    CS.BytecodeLength = D3D12Util_GetShaderBlobSize(SL->pShaderBlob);
    CS.pShaderBytecode = D3D12Util_GetShaderBlobData(SL->pShaderBlob);
    SKR_DECLARE_ZERO(D3D12_CACHED_PIPELINE_STATE, cached_pso_desc);
    cached_pso_desc.pCachedBlob           = NULL;
    cached_pso_desc.CachedBlobSizeInBytes = 0;
    SKR_DECLARE_ZERO(D3D12_COMPUTE_PIPELINE_STATE_DESC, pipeline_state_desc);
    pipeline_state_desc.pRootSignature = RS->pDxRootSignature;
    pipeline_state_desc.CS             = CS;
    pipeline_state_desc.CachedPSO      = cached_pso_desc;
    pipeline_state_desc.Flags          = D3D12_PIPELINE_STATE_FLAG_NONE;
    pipeline_state_desc.NodeMask       = CGPU_SINGLE_GPU_NODE_MASK;
    // Pipeline cache
    HRESULT result                        = E_FAIL;
    wchar_t pipelineName[PSO_NAME_LENGTH] = {};
    size_t  psoShaderHash                 = 0;
    size_t  psoComputeHash                = 0;
    if (D->pPipelineLibrary)
    {
        if (CS.BytecodeLength)
            psoShaderHash = cgpu_hash(CS.pShaderBytecode, CS.BytecodeLength, psoShaderHash);
        psoComputeHash = cgpu_hash(&pipeline_state_desc.Flags, sizeof(D3D12_PIPELINE_STATE_FLAGS), psoComputeHash);
        psoComputeHash = cgpu_hash(&pipeline_state_desc.NodeMask, sizeof(UINT), psoComputeHash);
        swprintf(pipelineName, PSO_NAME_LENGTH, L"%S_S%zuR%zu", "COMPUTEPSO", psoShaderHash, psoComputeHash);
        result = COM_CALL(LoadComputePipeline, D->pPipelineLibrary, pipelineName, &pipeline_state_desc, IID_ARGS(ID3D12PipelineState, &PPL->pDxPipelineState));
    }
    if (!SUCCEEDED(result))
    {
        // XBOX: Support PSO extensions
        CHECK_HRESULT(COM_CALL(CreateComputePipelineState, D->pDxDevice, &pipeline_state_desc, IID_ARGS(ID3D12PipelineState, &PPL->pDxPipelineState)));
    }
    return &PPL->super;
}

void cgpu_free_compute_pipeline_d3d12(CGPUComputePipelineId pipeline)
{
    CGPUComputePipeline_D3D12* PPL = (CGPUComputePipeline_D3D12*)pipeline;
    SAFE_RELEASE(PPL->pDxPipelineState);
    cgpu_free(PPL);
}

static const char* kD3D12PSOMemoryPoolName = "cgpu::d3d12_pso";
static const D3D12_DEPTH_STENCIL_DESC gDefaultDepthDesc = {};
static const D3D12_BLEND_DESC gDefaultBlendDesc = {};
static const D3D12_RASTERIZER_DESC gDefaultRasterizerDesc = {};
CGPURenderPipelineId cgpu_create_render_pipeline_d3d12(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc)
{
    CGPUDevice_D3D12*        D  = (CGPUDevice_D3D12*)device;
    CGPURootSignature_D3D12* RS = (CGPURootSignature_D3D12*)desc->root_signature;

    uint32_t input_elem_count = 0;
    if (desc->vertex_layout != CGPU_NULLPTR)
    {
        for (uint32_t attrib_index = 0; attrib_index < desc->vertex_layout->attribute_count; ++attrib_index)
        {
            const CGPUVertexAttribute* attrib = &(desc->vertex_layout->attributes[attrib_index]);
            for (uint32_t arr_index = 0; arr_index < attrib->array_size; arr_index++)
            {
                input_elem_count++;
            }
        }
    }
    uint64_t       dsize                 = sizeof(CGPURenderPipeline_D3D12);
    const uint64_t input_elements_offset = dsize;
    dsize += (sizeof(D3D12_INPUT_ELEMENT_DESC) * input_elem_count);

    uint8_t* ptr = (uint8_t*)cgpu_callocN(1, dsize, kD3D12PSOMemoryPoolName);
    CGPURenderPipeline_D3D12* PPL = (CGPURenderPipeline_D3D12*)ptr;
    D3D12_INPUT_ELEMENT_DESC* input_elements = (D3D12_INPUT_ELEMENT_DESC*)(ptr + input_elements_offset);

    // Vertex input state
    typedef struct Semantic
    {
        const char8_t* name;
        uint32_t array_size;
    } Semantic;
    if (desc->vertex_layout != CGPU_NULLPTR)
    {
        Semantic* semanticRecords = (Semantic*)cgpu_calloc(desc->vertex_layout->attribute_count, sizeof(Semantic));
        uint32_t semanticRecordCount = 0;
        uint32_t elementIndex = 0;
        for (uint32_t attrib_index = 0; attrib_index < desc->vertex_layout->attribute_count; ++attrib_index)
        {
            const CGPUVertexAttribute* attrib = &(desc->vertex_layout->attributes[attrib_index]);
            // find existed semantic
            Semantic* currentSemantic = CGPU_NULLPTR;
            {
                for (uint32_t i = currentSemantic ? UINT32_MAX : 0; i < semanticRecordCount; i++)
                {
                    if (semanticRecords[i].name)
                    {
                        if (semanticRecords[i].name && attrib->semantic_name == semanticRecords[i].name)
                        {
                            currentSemantic = &semanticRecords[i];
                            break;
                        }
                    }
                }
                for (uint32_t i = currentSemantic ? UINT32_MAX : 0; i < semanticRecordCount; i++)
                {
                    if (semanticRecords[i].name)
                    {
                        if (semanticRecords[i].name && strcmp(attrib->semantic_name, semanticRecords[i].name) == 0)
                        {
                            currentSemantic = &semanticRecords[i];
                            break;
                        }
                    }
                }
                if (!currentSemantic)
                {
                    currentSemantic = &semanticRecords[semanticRecordCount++];
                    currentSemantic->name = attrib->semantic_name;
                    currentSemantic->array_size = attrib->array_size;
                }
            }
            // fill input element
            for (uint32_t arr_index = 0; arr_index < attrib->array_size; arr_index++)
            {
                input_elements[elementIndex].SemanticName = (const char*)attrib->semantic_name;
                input_elements[elementIndex].SemanticIndex = arr_index;
                input_elements[elementIndex].Format = DXGIUtil_TranslatePixelFormat(attrib->format, false);
                input_elements[elementIndex].InputSlot = attrib->binding;
                input_elements[elementIndex].AlignedByteOffset = attrib->offset + arr_index * FormatUtil_BitSizeOfBlock(attrib->format) / 8;
                if (attrib->rate == CGPU_INPUT_RATE_INSTANCE)
                {
                    input_elements[elementIndex].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                    input_elements[elementIndex].InstanceDataStepRate = 1;
                }
                else
                {
                    input_elements[elementIndex].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    input_elements[elementIndex].InstanceDataStepRate = 0;
                }
                elementIndex++;
            }
        }
    }
    SKR_DECLARE_ZERO(D3D12_INPUT_LAYOUT_DESC, input_layout_desc);
    input_layout_desc.pInputElementDescs = input_elem_count ? input_elements : NULL;
    input_layout_desc.NumElements        = input_elem_count;
    // Shader stages
    SKR_DECLARE_ZERO(D3D12_SHADER_BYTECODE, VS);
    SKR_DECLARE_ZERO(D3D12_SHADER_BYTECODE, PS);
    SKR_DECLARE_ZERO(D3D12_SHADER_BYTECODE, DS);
    SKR_DECLARE_ZERO(D3D12_SHADER_BYTECODE, HS);
    SKR_DECLARE_ZERO(D3D12_SHADER_BYTECODE, GS);
    for (uint32_t i = 0; i < 5; ++i)
    {
        ECGPUShaderStage stage_mask = (ECGPUShaderStage)(1 << i);
        switch (stage_mask)
        {
            case CGPU_SHADER_STAGE_VERT: {
                if (desc->vertex_shader)
                {
                    CGPUShaderLibrary_D3D12* VertLib = (CGPUShaderLibrary_D3D12*)desc->vertex_shader->library;
                    VS.BytecodeLength = D3D12Util_GetShaderBlobSize(VertLib->pShaderBlob);
                    VS.pShaderBytecode = D3D12Util_GetShaderBlobData(VertLib->pShaderBlob);
                }
            }
            break;
            case CGPU_SHADER_STAGE_TESC: {
                if (desc->tesc_shader)
                {
                    CGPUShaderLibrary_D3D12* TescLib = (CGPUShaderLibrary_D3D12*)desc->tesc_shader->library;
                    HS.BytecodeLength = D3D12Util_GetShaderBlobSize(TescLib->pShaderBlob);
                    HS.pShaderBytecode = D3D12Util_GetShaderBlobData(TescLib->pShaderBlob);
                }
            }
            break;
            case CGPU_SHADER_STAGE_TESE: {
                if (desc->tese_shader)
                {
                    CGPUShaderLibrary_D3D12* TeseLib = (CGPUShaderLibrary_D3D12*)desc->tese_shader->library;
                    DS.BytecodeLength = D3D12Util_GetShaderBlobSize(TeseLib->pShaderBlob);
                    DS.pShaderBytecode = D3D12Util_GetShaderBlobData(TeseLib->pShaderBlob);
                }
            }
            break;
            case CGPU_SHADER_STAGE_GEOM: {
                if (desc->geom_shader)
                {
                    CGPUShaderLibrary_D3D12* GeomLib = (CGPUShaderLibrary_D3D12*)desc->geom_shader->library;
                    GS.BytecodeLength = D3D12Util_GetShaderBlobSize(GeomLib->pShaderBlob);
                    GS.pShaderBytecode = D3D12Util_GetShaderBlobData(GeomLib->pShaderBlob);
                }
            }
            break;
            case CGPU_SHADER_STAGE_FRAG: {
                if (desc->fragment_shader)
                {
                    CGPUShaderLibrary_D3D12* FragLib = (CGPUShaderLibrary_D3D12*)desc->fragment_shader->library;
                    PS.BytecodeLength = D3D12Util_GetShaderBlobSize(FragLib->pShaderBlob);
                    PS.pShaderBytecode = D3D12Util_GetShaderBlobData(FragLib->pShaderBlob);
                }
            }
            break;
            default:
                cgpu_assert(false && "Shader Stage not supported!");
                break;
        }
    }
    // Stream out
    SKR_DECLARE_ZERO(D3D12_STREAM_OUTPUT_DESC, stream_output_desc);
    stream_output_desc.pSODeclaration = NULL;
    stream_output_desc.NumEntries = 0;
    stream_output_desc.pBufferStrides = NULL;
    stream_output_desc.NumStrides = 0;
    stream_output_desc.RasterizedStream = 0;
    // Sample
    SKR_DECLARE_ZERO(DXGI_SAMPLE_DESC, sample_desc);
    sample_desc.Count   = (UINT)(desc->sample_count ? desc->sample_count : 1);
    sample_desc.Quality = (UINT)(desc->sample_quality);
    SKR_DECLARE_ZERO(D3D12_CACHED_PIPELINE_STATE, cached_pso_desc);
    cached_pso_desc.pCachedBlob = NULL;
    cached_pso_desc.CachedBlobSizeInBytes = 0;
    // Fill pipeline object desc
    PPL->mDxGfxPipelineStateDesc.pRootSignature = RS->pDxRootSignature;
    // Single GPU
    PPL->mDxGfxPipelineStateDesc.NodeMask = CGPU_SINGLE_GPU_NODE_MASK;
    PPL->mDxGfxPipelineStateDesc.VS = VS;
    PPL->mDxGfxPipelineStateDesc.PS = PS;
    PPL->mDxGfxPipelineStateDesc.DS = DS;
    PPL->mDxGfxPipelineStateDesc.HS = HS;
    PPL->mDxGfxPipelineStateDesc.GS = GS;
    PPL->mDxGfxPipelineStateDesc.StreamOutput = stream_output_desc;
    PPL->mDxGfxPipelineStateDesc.BlendState = desc->blend_state ? D3D12Util_TranslateBlendState(desc->blend_state) : gDefaultBlendDesc;
    PPL->mDxGfxPipelineStateDesc.SampleMask = UINT_MAX;
    PPL->mDxGfxPipelineStateDesc.RasterizerState = desc->rasterizer_state ? D3D12Util_TranslateRasterizerState(desc->rasterizer_state) : gDefaultRasterizerDesc;
    // Depth stencil
    PPL->mDxGfxPipelineStateDesc.DepthStencilState = desc->depth_state ? D3D12Util_TranslateDephStencilState(desc->depth_state) : gDefaultDepthDesc;
    PPL->mDxGfxPipelineStateDesc.InputLayout = input_layout_desc;
    PPL->mDxGfxPipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    PPL->mDxGfxPipelineStateDesc.PrimitiveTopologyType = D3D12Util_TranslatePrimitiveTopology(desc->prim_topology);
    PPL->mDxGfxPipelineStateDesc.NumRenderTargets = desc->render_target_count;
    PPL->mDxGfxPipelineStateDesc.DSVFormat = DXGIUtil_TranslatePixelFormat(desc->depth_stencil_format, false);
    PPL->mDxGfxPipelineStateDesc.SampleDesc = sample_desc;
    PPL->mDxGfxPipelineStateDesc.CachedPSO = cached_pso_desc;
    PPL->mDxGfxPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    for (uint32_t i = 0; i < PPL->mDxGfxPipelineStateDesc.NumRenderTargets; ++i)
    {
        PPL->mDxGfxPipelineStateDesc.RTVFormats[i] = DXGIUtil_TranslatePixelFormat(desc->color_formats[i], false);
    }
    // Create pipeline object
    HRESULT result = E_FAIL;
    wchar_t pipelineName[PSO_NAME_LENGTH] = {};
    size_t  psoShaderHash = 0;
    size_t  psoRenderHash = 0;
    size_t  rootSignatureNumber = (size_t)RS->pDxRootSignature;
    if (D->pPipelineLibrary)
    {
        // Calculate graphics pso shader hash
        if (VS.BytecodeLength)
            psoShaderHash = cgpu_hash(VS.pShaderBytecode, VS.BytecodeLength, psoShaderHash);
        if (PS.BytecodeLength)
            psoShaderHash = cgpu_hash(PS.pShaderBytecode, PS.BytecodeLength, psoShaderHash);
        if (DS.BytecodeLength)
            psoShaderHash = cgpu_hash(DS.pShaderBytecode, DS.BytecodeLength, psoShaderHash);
        if (HS.BytecodeLength)
            psoShaderHash = cgpu_hash(HS.pShaderBytecode, HS.BytecodeLength, psoShaderHash);
        if (GS.BytecodeLength)
            psoShaderHash = cgpu_hash(GS.pShaderBytecode, GS.BytecodeLength, psoShaderHash);

        // Calculate graphics pso desc hash
        psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.BlendState, sizeof(D3D12_BLEND_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.RasterizerState, sizeof(D3D12_RASTERIZER_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.DepthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.IBStripCutValue, sizeof(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE), psoRenderHash);
        psoRenderHash = cgpu_hash(PPL->mDxGfxPipelineStateDesc.RTVFormats,
                                  PPL->mDxGfxPipelineStateDesc.NumRenderTargets * sizeof(DXGI_FORMAT), psoRenderHash);
        psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.DSVFormat, sizeof(DXGI_FORMAT), psoRenderHash);
        psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.SampleDesc, sizeof(DXGI_SAMPLE_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.Flags, sizeof(D3D12_PIPELINE_STATE_FLAGS), psoRenderHash);
        for (uint32_t i = 0; i < PPL->mDxGfxPipelineStateDesc.InputLayout.NumElements; i++)
        {
            psoRenderHash = cgpu_hash(&PPL->mDxGfxPipelineStateDesc.InputLayout.pInputElementDescs[i],
                                      sizeof(D3D12_INPUT_ELEMENT_DESC), psoRenderHash);
        }
        swprintf(pipelineName, PSO_NAME_LENGTH, L"%S_S%zu_R%zu_RS%zu", "GRAPHICSPSO", psoShaderHash, psoRenderHash, rootSignatureNumber);
        result = COM_CALL(LoadGraphicsPipeline, D->pPipelineLibrary, pipelineName, &PPL->mDxGfxPipelineStateDesc, IID_ARGS(ID3D12PipelineState, &PPL->pDxPipelineState));
    }
    if (!SUCCEEDED(result))
    {
        CHECK_HRESULT(COM_CALL(CreateGraphicsPipelineState, D->pDxDevice, &PPL->mDxGfxPipelineStateDesc, IID_ARGS(ID3D12PipelineState, &PPL->pDxPipelineState)));
        // Pipeline cache
        if (D->pPipelineLibrary)
        {
            result = COM_CALL(StorePipeline, D->pPipelineLibrary, pipelineName, PPL->pDxPipelineState);
            if (!SUCCEEDED(result))
            {
                cgpu_warn(u8"Failed to store pipeline state object to pipeline library %ls. hash: (%lld) hr:(0x%08x)", pipelineName, psoRenderHash, result);
            }
            else
            {
                cgpu_trace(u8"Succeeded to store pipeline state object to pipeline library %ls. hash: (%lld) hr:(0x%08x)", pipelineName, psoRenderHash, result);
            }
        }
    }
    D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    switch (desc->prim_topology)
    {
        case CGPU_PRIM_TOPO_POINT_LIST:
            topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case CGPU_PRIM_TOPO_LINE_LIST:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case CGPU_PRIM_TOPO_LINE_STRIP:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case CGPU_PRIM_TOPO_TRI_LIST:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case CGPU_PRIM_TOPO_TRI_STRIP:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        case CGPU_PRIM_TOPO_PATCH_LIST: {
            // TODO: Support D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST with Hull Shaders
            cgpu_assert(0 && "Unsupported primitive topology!");
        }
        default:
            break;
    }
    PPL->mDxPrimitiveTopology = topology;
    PPL->pRootSignature       = RS->pDxRootSignature;
    return &PPL->super;
}

void cgpu_free_render_pipeline_d3d12(CGPURenderPipelineId pipeline)
{
    CGPURenderPipeline_D3D12* PPL = (CGPURenderPipeline_D3D12*)pipeline;
    SAFE_RELEASE(PPL->pDxPipelineState);
    cgpu_freeN(PPL, kD3D12PSOMemoryPoolName);
}

CGPUQueryPoolId cgpu_create_query_pool_d3d12(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc)
{
    CGPUDevice_D3D12*    D          = (CGPUDevice_D3D12*)device;
    CGPUQueryPool_D3D12* pQueryPool = cgpu_calloc(1, sizeof(CGPUQueryPool_D3D12));
    pQueryPool->mType               = D3D12Util_ToD3D12QueryType(desc->type);
    pQueryPool->super.count         = desc->query_count;

    D3D12_QUERY_HEAP_DESC Desc = {};
    Desc.Count                 = desc->query_count;
    Desc.NodeMask              = CGPU_SINGLE_GPU_NODE_MASK;
    Desc.Type                  = D3D12Util_ToD3D12QueryHeapType(desc->type);
    COM_CALL(CreateQueryHeap, D->pDxDevice, &Desc, IID_ARGS(ID3D12QueryHeap, &pQueryPool->pDxQueryHeap));

    return &pQueryPool->super;
}

void cgpu_free_query_pool_d3d12(CGPUQueryPoolId pool)
{
    CGPUQueryPool_D3D12* QP = (CGPUQueryPool_D3D12*)pool;
    SAFE_RELEASE(QP->pDxQueryHeap);
    cgpu_free(QP);
}

// Queue APIs
CGPUQueueId cgpu_get_queue_d3d12(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUQueue_D3D12*  Q = cgpu_calloc(1, sizeof(CGPUQueue_D3D12));
    Q->pCommandQueue    = D->ppCommandQueues[type][index];
    Q->pFence           = (CGPUFence_D3D12*)cgpu_create_fence_d3d12(device);
    return &Q->super;
}

void cgpu_submit_queue_d3d12(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc)
{
    uint32_t                  CmdCount = desc->cmds_count;
    CGPUCommandBuffer_D3D12** Cmds     = (CGPUCommandBuffer_D3D12**)desc->cmds;
    CGPUQueue_D3D12*          Q        = (CGPUQueue_D3D12*)queue;
    CGPUFence_D3D12*          F        = (CGPUFence_D3D12*)desc->signal_fence;

    // cgpu_assert that given cmd list and given params are valid
    cgpu_assert(CmdCount > 0);
    cgpu_assert(Cmds);
    // execute given command list
    cgpu_assert(Q->pCommandQueue);

    ID3D12CommandList* cmds[CmdCount];
    for (uint32_t i = 0; i < CmdCount; ++i)
    {
        cmds[i] = (ID3D12CommandList*)Cmds[i]->pDxCmdList;
    }
    // Wait semaphores
    CGPUFence_D3D12** WaitSemaphores = (CGPUFence_D3D12**)desc->wait_semaphores;
    for (uint32_t i = 0; i < desc->wait_semaphore_count; ++i)
        COM_CALL(Wait, Q->pCommandQueue, WaitSemaphores[i]->pDxFence, WaitSemaphores[i]->mFenceValue - 1);
    // Execute
    COM_CALL(ExecuteCommandLists, Q->pCommandQueue, CmdCount, cmds);
    // Signal fences
    if (F)
        D3D12Util_SignalFence(Q, F->pDxFence, F->mFenceValue++);
    // Signal Semaphores
    CGPUFence_D3D12** SignalSemaphores = (CGPUFence_D3D12**)desc->signal_semaphores;
    for (uint32_t i = 0; i < desc->signal_semaphore_count; i++)
        D3D12Util_SignalFence(Q, SignalSemaphores[i]->pDxFence, SignalSemaphores[i]->mFenceValue++);
}

void cgpu_wait_queue_idle_d3d12(CGPUQueueId queue)
{
    CGPUQueue_D3D12* Q = (CGPUQueue_D3D12*)queue;
    D3D12Util_SignalFence(Q, Q->pFence->pDxFence, Q->pFence->mFenceValue++);

    uint64_t fenceValue = Q->pFence->mFenceValue - 1;
    if (COM_CALL(GetCompletedValue, Q->pFence->pDxFence) < Q->pFence->mFenceValue - 1)
    {
        COM_CALL(SetEventOnCompletion, Q->pFence->pDxFence, fenceValue, Q->pFence->pDxWaitIdleFenceEvent);
        WaitForSingleObject(Q->pFence->pDxWaitIdleFenceEvent, INFINITE);
    }
}

void cgpu_queue_present_d3d12(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc)
{
    CGPUSwapChain_D3D12* S  = (CGPUSwapChain_D3D12*)desc->swapchain;
    if (FAILED(COM_CALL(Present, S->pDxSwapChain, S->mDxSyncInterval, S->mFlags)))
    {
        cgpu_error(u8"Failed to present swapchain render target!");
#if defined(_WIN32)
        ID3D12Device* device = NULL;
        COM_CALL(GetDevice, S->pDxSwapChain, IID_ARGS(ID3D12Device, &device));
        D3D12Util_ReportGPUCrash(device);
        SAFE_RELEASE(device);
        ((CGPUDevice*)queue->device)->is_lost = true;
#endif
    }
}

float cgpu_queue_get_timestamp_period_ns_d3d12(CGPUQueueId queue)
{
    CGPUQueue_D3D12* Q    = (CGPUQueue_D3D12*)queue;
    UINT64           freq = 0;
    // ticks per second
    COM_CALL(GetTimestampFrequency, Q->pCommandQueue, &freq);
    // ns per tick
    const double ms_period = 1e9 / (double)freq;
    return (float)ms_period;
}

void cgpu_free_queue_d3d12(CGPUQueueId queue)
{
    CGPUQueue_D3D12* Q = (CGPUQueue_D3D12*)queue;
    cgpu_assert(queue && "D3D12 ERROR: FREE NULL QUEUE!");
    cgpu_free_fence_d3d12(&Q->pFence->super);
    cgpu_free(Q);
}

CGPUSamplerId cgpu_create_sampler_d3d12(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc)
{
    // initialize to zero
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUSampler_D3D12* pSampler = cgpu_calloc(1, sizeof(CGPUSampler_D3D12));

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
    add_desc.MaxLOD = (desc->mipmap_mode == CGPU_MIPMAP_MODE_LINEAR) ? D3D12_FLOAT32_MAX : 0.0f;
    pSampler->mDxDesc = add_desc;
    pSampler->mDxHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], 1).mCpu;
    COM_CALL(CreateSampler, D->pDxDevice, &pSampler->mDxDesc, pSampler->mDxHandle);
    return &pSampler->super;
}

void cgpu_free_sampler_d3d12(CGPUSamplerId sampler)
{
    CGPUSampler_D3D12* pSampler = (CGPUSampler_D3D12*)sampler;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)sampler->device;

    D3D12Util_ReturnDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], pSampler->mDxHandle, 1);
    cgpu_free(pSampler);
}

CGPUTextureViewId cgpu_create_texture_view_d3d12(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc)
{
    CGPUTextureView_D3D12* TV = cgpu_calloc(1, sizeof(CGPUTextureView_D3D12));
    CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)desc->texture;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    // Consume handles
    const CGPUMemoryUsages usages = desc->usages;
    uint32_t handleCount = ((usages & CGPU_TVU_SRV) ? 1 : 0) +
                           ((usages & CGPU_TVU_UAV) ? 1 : 0);
    if (handleCount > 0)
    {
        D3D12Util_DescriptorHeap* pHeap = D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
        const size_t kDescriptorSize = D3D12Util_GetDescriptorSize(pHeap);
        TV->mDxDescriptorHandles = D3D12Util_ConsumeDescriptorHandles(pHeap, handleCount).mCpu;
        TV->mDxSrvOffset = 0;
        uint64_t CurrentOffsetCursor = TV->mDxSrvOffset;
        // Create SRV
        if (usages & CGPU_TVU_SRV)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srv = { TV->mDxDescriptorHandles.ptr + TV->mDxSrvOffset };
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, true);
            // TODO: SUPPORT RGBA COMPONENT VIEW MAPPING SWIZZLE
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
                    srvDesc.TextureCubeArray.First2DArrayFace = desc->base_array_layer;
                }
                break;
                default:
                    cgpu_assert(0 && "Unsupported texture dimension!");
                    break;
            }
            D3D12Util_CreateSRV(D, T->pDxResource, &srvDesc, &srv);
            CurrentOffsetCursor += kDescriptorSize * 1;
        }
        // Create UAV
        if (usages & CGPU_TVU_UAV)
        {
            TV->mDxUavOffset = CurrentOffsetCursor;
            CurrentOffsetCursor += kDescriptorSize * 1;
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
            dsvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, false);
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
            rtvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, false);
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
    const CGPUMemoryUsages usages = TV->super.info.usages;
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
    cgpu_free(TV);
}

// Command Objects
inline static ID3D12CommandAllocator* D3D12Util_AllocateTransientCommandAllocator(CGPUCommandPool_D3D12* E, CGPUQueueId queue)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)queue->device;

    D3D12_COMMAND_LIST_TYPE type =
        queue->type == CGPU_QUEUE_TYPE_TRANSFER ?
        D3D12_COMMAND_LIST_TYPE_COPY :
        queue->type == CGPU_QUEUE_TYPE_COMPUTE ?
        D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (SUCCEEDED(COM_CALL(CreateCommandAllocator, D->pDxDevice, type, IID_ARGS(ID3D12CommandAllocator, &E->pDxCmdAlloc))))
    {
        return E->pDxCmdAlloc;
    }
    return CGPU_NULLPTR;
}

inline static void D3D12Util_FreeTransientCommandAllocator(ID3D12CommandAllocator* allocator) 
{ 
    SAFE_RELEASE(allocator); 
}

CGPUCommandPoolId cgpu_create_command_pool_d3d12(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc)
{
    CGPUCommandPool_D3D12* P = cgpu_calloc(1, sizeof(CGPUCommandPool_D3D12));
    P->pDxCmdAlloc = D3D12Util_AllocateTransientCommandAllocator(P, queue);
    return &P->super;
}

void cgpu_reset_command_pool_d3d12(CGPUCommandPoolId pool)
{
    CGPUCommandPool_D3D12* P = (CGPUCommandPool_D3D12*)pool;
    CHECK_HRESULT(COM_CALL(Reset, P->pDxCmdAlloc));
}

CGPUCommandBufferId cgpu_create_command_buffer_d3d12(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc)
{
    // initialize to zero
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cgpu_calloc(1, sizeof(CGPUCommandBuffer_D3D12));
    CGPUCommandPool_D3D12*   P   = (CGPUCommandPool_D3D12*)pool;
    CGPUQueue_D3D12*         Q   = (CGPUQueue_D3D12*)P->super.queue;
    CGPUDevice_D3D12*        D   = (CGPUDevice_D3D12*)Q->super.device;
    cgpu_assert(Cmd);

    // set command pool of new command
    Cmd->mNodeIndex = CGPU_SINGLE_GPU_NODE_INDEX;
    Cmd->mType      = Q->super.type;

    Cmd->pBoundHeaps[0] = D->pCbvSrvUavHeaps[Cmd->mNodeIndex];
    Cmd->pBoundHeaps[1] = D->pSamplerHeaps[Cmd->mNodeIndex];
    Cmd->pCmdPool       = P;

    uint32_t nodeMask = Cmd->mNodeIndex;

    ID3D12PipelineState* initialState = NULL;
    CHECK_HRESULT(COM_CALL(CreateCommandList, D->pDxDevice, 
        nodeMask, gDx12CmdTypeTranslator[Cmd->mType], P->pDxCmdAlloc, initialState, IID_ARGS(ID3D12CommandList, &Cmd->pDxCmdList)));

    // Command lists are addd in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    CHECK_HRESULT(COM_CALL(Close, Cmd->pDxCmdList));
    return &Cmd->super;
}

void cgpu_free_command_buffer_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    SAFE_RELEASE(Cmd->pDxCmdList);
    cgpu_free(Cmd);
}

void cgpu_free_command_pool_d3d12(CGPUCommandPoolId pool)
{
    CGPUCommandPool_D3D12* P = (CGPUCommandPool_D3D12*)pool;
    cgpu_assert(pool && "D3D12 ERROR: FREE NULL COMMAND POOL!");
    cgpu_assert(P->pDxCmdAlloc && "D3D12 ERROR: FREE NULL pDxCmdAlloc!");

    D3D12Util_FreeTransientCommandAllocator(P->pDxCmdAlloc);
    cgpu_free(P);
}

// CMDs
void cgpu_cmd_begin_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUCommandPool_D3D12*   P   = (CGPUCommandPool_D3D12*)Cmd->pCmdPool;
    CHECK_HRESULT(COM_CALL(Reset, Cmd->pDxCmdList, P->pDxCmdAlloc, NULL));

    if (Cmd->mType != CGPU_QUEUE_TYPE_TRANSFER)
    {
        ID3D12DescriptorHeap* heaps[] = {
            D3D12Util_GetUnderlyingHeap(Cmd->pBoundHeaps[0]),
            D3D12Util_GetUnderlyingHeap(Cmd->pBoundHeaps[1])
        };
        COM_CALL(SetDescriptorHeaps, Cmd->pDxCmdList, 2, heaps);

        COM_CALL(GetGPUDescriptorHandleForHeapStart, heaps[0], &Cmd->mBoundHeapStartHandles[0]);
        COM_CALL(GetGPUDescriptorHandleForHeapStart, heaps[1], &Cmd->mBoundHeapStartHandles[1]);
    }
    // Reset CPU side data
    Cmd->pBoundRootSignature = NULL;
}

void cgpu_cmd_transfer_buffer_to_buffer_d3d12(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUBuffer_D3D12* Src = (CGPUBuffer_D3D12*)desc->src;
    CGPUBuffer_D3D12* Dst = (CGPUBuffer_D3D12*)desc->dst;
#if defined(XBOX)
    COM_CALL(CopyBufferRegion, Cmd->mDma.pDxCmdList, Dst->pDxResource, desc->dst_offset, Src->pDxResource, desc->src_offset, desc->size);
#else
    COM_CALL(CopyBufferRegion, Cmd->pDxCmdList, Dst->pDxResource, desc->dst_offset, Src->pDxResource, desc->src_offset, desc->size);
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
        desc->src_subresource.layer_count
    );
    uint32_t dst_subresource = CALC_SUBRESOURCE_INDEX(
        desc->dst_subresource.mip_level, desc->dst_subresource.base_array_layer,
        0, 1,
        desc->dst_subresource.layer_count
    );
    D3D12_RESOURCE_DESC srcResourceDesc = { 0 };
    COM_CALL(GetDesc, Src->pDxResource, &srcResourceDesc);
    D3D12_RESOURCE_DESC dstResourceDesc = { 0 };
    COM_CALL(GetDesc, Dst->pDxResource, &dstResourceDesc);

    D3D12_TEXTURE_COPY_LOCATION src = {};
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    COM_CALL(GetCopyableFootprints, D->pDxDevice,
        &srcResourceDesc, src_subresource, 1,
        0, &src.PlacedFootprint,
        NULL, NULL, NULL
    );
    COM_CALL(GetCopyableFootprints, D->pDxDevice,
        &dstResourceDesc, dst_subresource, 1,
        0, &dst.PlacedFootprint,
        NULL, NULL, NULL
    );
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.pResource = Src->pDxResource;
    src.SubresourceIndex = src_subresource;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.pResource = Dst->pDxResource;
    dst.SubresourceIndex = dst_subresource;

#if defined(XBOX)
    COM_CALL(CopyTextureRegion, Cmd->mDma.pDxCmdList, &dst, 0, 0, 0, &src, NULL);
#else
    COM_CALL(CopyTextureRegion, Cmd->pDxCmdList, &dst, 0, 0, 0, &src, NULL);
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
    D3D12_RESOURCE_DESC resourceDesc = { 0 };
    COM_CALL(GetDesc, Dst->pDxResource, &resourceDesc);

    D3D12_TEXTURE_COPY_LOCATION src = {};
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.pResource = Src->pDxResource;
    COM_CALL(GetCopyableFootprints, D->pDxDevice,
        &resourceDesc, subresource, 1,
        desc->src_offset, &src.PlacedFootprint,
        NULL, NULL, NULL);
    src.PlacedFootprint.Offset = desc->src_offset;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.pResource = Dst->pDxResource;
    dst.SubresourceIndex = subresource;
#if defined(XBOX)
    COM_CALL(CopyTextureRegion, Cmd->mDma.pDxCmdList, &dst, 0, 0, 0, &src, CGPU_NULLPTR);
#else
    COM_CALL(CopyTextureRegion, Cmd->pDxCmdList, &dst, 0, 0, 0, &src, CGPU_NULLPTR);
#endif
}

void cgpu_cmd_transfer_buffer_to_tiles_d3d12(CGPUCommandBufferId cmd, const struct CGPUBufferToTilesTransfer* desc)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUBuffer_D3D12* Src = (CGPUBuffer_D3D12*)desc->src;
    CGPUTexture_D3D12* Dst = (CGPUTexture_D3D12*)desc->dst;

    uint32_t subresource = CALC_SUBRESOURCE_INDEX(
        desc->region.mip_level,
        desc->region.layer,
        0, 1, 1
    );
    D3D12_TILED_RESOURCE_COORDINATE DstCoord = {
        desc->region.start.x, desc->region.start.y, desc->region.start.z, subresource
    };
    const uint32_t Width = desc->region.end.x - desc->region.start.x;
    const uint32_t Height = desc->region.end.y - desc->region.start.y;
    const uint32_t Depth = desc->region.end.z - desc->region.start.z;
    const uint32_t N = Width * Height * Depth;
    D3D12_TILE_REGION_SIZE CoordSize = {
        N, TRUE, Width, (UINT16)Height, (UINT16)Depth
    };

    COM_CALL(CopyTiles, Cmd->pDxCmdList, Dst->pDxResource, &DstCoord,
                               &CoordSize, Src->pDxResource, desc->src_offset,
                               D3D12_TILE_COPY_FLAG_LINEAR_BUFFER_TO_SWIZZLED_TILED_RESOURCE);
}

void cgpu_cmd_fill_buffer_d3d12(CGPUCommandBufferId cmd, CGPUBufferId buffer, const struct CGPUFillBufferDescriptor* desc)
{
    const CGPUCommandBuffer_D3D12* CMD = (const CGPUCommandBuffer_D3D12*)cmd;
    const CGPUBuffer_D3D12* B = (const CGPUBuffer_D3D12*)buffer;

    ID3D12GraphicsCommandList2* pCommandList2 = CGPU_NULLPTR;
    COM_CALL(QueryInterface, CMD->pDxCmdList, IID_ARGS(ID3D12GraphicsCommandList, &pCommandList2));

    D3D12_WRITEBUFFERIMMEDIATE_PARAMETER param = { B->mDxGpuAddress + desc->offset, desc->value };
    D3D12_WRITEBUFFERIMMEDIATE_MODE mode = D3D12_WRITEBUFFERIMMEDIATE_MODE_MARKER_OUT;
    COM_CALL(WriteBufferImmediate, pCommandList2, 1, &param, &mode);
    SAFE_RELEASE(pCommandList2);
}

void cgpu_cmd_fill_buffer_n_d3d12(CGPUCommandBufferId cmd, CGPUBufferId buffer, const struct CGPUFillBufferDescriptor* desc, uint32_t count)
{
    const CGPUCommandBuffer_D3D12* CMD = (const CGPUCommandBuffer_D3D12*)cmd;
    const CGPUBuffer_D3D12* B = (const CGPUBuffer_D3D12*)buffer;

    ID3D12GraphicsCommandList2* pCommandList2 = CGPU_NULLPTR;
    COM_CALL(QueryInterface, CMD->pDxCmdList, IID_ARGS(ID3D12GraphicsCommandList, &pCommandList2));

    D3D12_WRITEBUFFERIMMEDIATE_PARAMETER params[count];
    for (uint32_t i = 0; i < count; i++)
    {
        params[i].Dest = B->mDxGpuAddress + desc->offset;
        params[i].Value = desc->value;
    }
    D3D12_WRITEBUFFERIMMEDIATE_MODE mode = D3D12_WRITEBUFFERIMMEDIATE_MODE_MARKER_OUT;
    COM_CALL(WriteBufferImmediate, pCommandList2, count, params, &mode);
    SAFE_RELEASE(pCommandList2);
}

void cgpu_cmd_begin_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    CGPUCommandBuffer_D3D12* CMD = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUQueryPool_D3D12* pQueryPool = (CGPUQueryPool_D3D12*)pool;
    D3D12_QUERY_TYPE type = pQueryPool->mType;
    switch (type)
    {
        case D3D12_QUERY_TYPE_TIMESTAMP:
            COM_CALL(EndQuery, CMD->pDxCmdList, pQueryPool->pDxQueryHeap, type, desc->index);
            break;
        case D3D12_QUERY_TYPE_OCCLUSION:
        case D3D12_QUERY_TYPE_PIPELINE_STATISTICS:
        case D3D12_QUERY_TYPE_BINARY_OCCLUSION:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM1:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM2:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM3:
        default:
            break;
    }
}

void cgpu_cmd_end_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    cgpu_cmd_begin_query(cmd, pool, desc);
}

void cgpu_cmd_reset_query_pool_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, uint32_t start_query, uint32_t query_count)
{

}

void cgpu_cmd_resolve_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count)
{
    CGPUCommandBuffer_D3D12* CMD = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUQueryPool_D3D12* pQueryPool = (CGPUQueryPool_D3D12*)pool;
    CGPUBuffer_D3D12* pReadbackBuffer = (CGPUBuffer_D3D12*)readback;
    COM_CALL(ResolveQueryData, CMD->pDxCmdList, 
        pQueryPool->pDxQueryHeap, pQueryPool->mType, start_query, query_count,
        pReadbackBuffer->pDxResource, start_query * 8
    );
}

// TODO: https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#introduction
// Enhanced Barriers is not currently a hardware or driver requirement
void cgpu_cmd_resource_barrier_d3d12(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc)
{
    CGPUCommandBuffer_D3D12* Cmd             = (CGPUCommandBuffer_D3D12*)cmd;
    const uint32_t barriers_count  = desc->buffer_barriers_count + desc->texture_barriers_count;
    D3D12_RESOURCE_BARRIER barriers[barriers_count];
    uint32_t transitionCount = 0;
    for (uint32_t i = 0; i < desc->buffer_barriers_count; ++i)
    {
        const CGPUBufferBarrier* pTransBarrier = &desc->buffer_barriers[i];
        D3D12_RESOURCE_BARRIER* pBarrier = &barriers[transitionCount];
        CGPUBuffer_D3D12* pBuffer = (CGPUBuffer_D3D12*)pTransBarrier->buffer;
        const CGPUMemoryUsages memory_usage = pTransBarrier->buffer->info->memory_usage;
        const CGPUResourceTypes descriptors = pTransBarrier->buffer->info->descriptors;
        if (memory_usage == CGPU_MEM_USAGE_GPU_ONLY || memory_usage == CGPU_MEM_USAGE_GPU_TO_CPU ||
            (memory_usage == CGPU_MEM_USAGE_CPU_TO_GPU && (descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER)))
        {
            if (CGPU_RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->src_state &&
                CGPU_RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->dst_state)
            {
                pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                pBarrier->UAV.pResource = pBuffer->pDxResource;
                ++transitionCount;
            }
            else if (CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE == pTransBarrier->src_state &&
                CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_READ == pTransBarrier->dst_state)
            {
                pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                pBarrier->UAV.pResource = pBuffer->pDxResource;
                ++transitionCount;
            }
            else
            {
                cgpu_assert((pTransBarrier->src_state != pTransBarrier->dst_state) && "D3D12 ERROR: Buffer Barrier with same src and dst state!");

                pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                if (pTransBarrier->d3d12_begin_only)
                {
                    pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
                }
                else if (pTransBarrier->d3d12_end_only)
                {
                    pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                }
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                pBarrier->Transition.pResource = pBuffer->pDxResource;
                pBarrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

                if (pTransBarrier->queue_acquire)
                    pBarrier->Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
                else
                    pBarrier->Transition.StateBefore = D3D12Util_TranslateResourceState(pTransBarrier->src_state);

                if (pTransBarrier->queue_release)
                    pBarrier->Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                else
                    pBarrier->Transition.StateAfter = D3D12Util_TranslateResourceState(pTransBarrier->dst_state);

                cgpu_assert((pBarrier->Transition.StateBefore != pBarrier->Transition.StateAfter) && "D3D12 ERROR: Buffer Barrier with same src and dst state!");

                ++transitionCount;
            }
        }
    }
    for (uint32_t i = 0; i < desc->texture_barriers_count; ++i)
    {
        const CGPUTextureBarrier* pTransBarrier = &desc->texture_barriers[i];
        D3D12_RESOURCE_BARRIER* pBarrier = &barriers[transitionCount];
        CGPUTexture_D3D12* pTexture = (CGPUTexture_D3D12*)pTransBarrier->texture;
        const CGPUTextureInfo* pTexInfo = pTexture->super.info;
        if (CGPU_RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->src_state &&
            CGPU_RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->dst_state)
        {
            pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            pBarrier->UAV.pResource = pTexture->pDxResource;
            ++transitionCount;
        }
        else
        {
            cgpu_assert((pTransBarrier->src_state != pTransBarrier->dst_state) && "D3D12 ERROR: Texture Barrier with same src and dst state!");

            pBarrier->Type  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            if (pTransBarrier->d3d12_begin_only)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
            }
            else if (pTransBarrier->d3d12_end_only)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            }
            pBarrier->Transition.pResource   = pTexture->pDxResource;
            pBarrier->Transition.Subresource = pTransBarrier->subresource_barrier ? CALC_SUBRESOURCE_INDEX(
                                                                                    pTransBarrier->mip_level, pTransBarrier->array_layer,
                                                                                    0, pTexInfo->mip_levels, pTexInfo->array_size_minus_one + 1) :
                                                                                    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            if (pTransBarrier->queue_acquire)
                pBarrier->Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            else
                pBarrier->Transition.StateBefore = D3D12Util_TranslateResourceState(pTransBarrier->src_state);

            if (pTransBarrier->queue_release)
                pBarrier->Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
            else
                pBarrier->Transition.StateAfter = D3D12Util_TranslateResourceState(pTransBarrier->dst_state);

            if (pBarrier->Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON && pBarrier->Transition.StateAfter == D3D12_RESOURCE_STATE_COMMON)
            {
                if (pTransBarrier->dst_state == CGPU_RESOURCE_STATE_PRESENT || pTransBarrier->src_state == CGPU_RESOURCE_STATE_PRESENT)
                {
                    continue;
                }
            }
            cgpu_assert((pBarrier->Transition.StateBefore != pBarrier->Transition.StateAfter) && "D3D12 ERROR: Texture Barrier with same src and dst state!");
            ++transitionCount;
        }
    }
    if (transitionCount)
    {
        COM_CALL(ResourceBarrier, Cmd->pDxCmdList, transitionCount, barriers);
    }
}

void cgpu_cmd_end_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    cgpu_assert(Cmd->pDxCmdList);
    CHECK_HRESULT(COM_CALL(Close, Cmd->pDxCmdList));
}

inline static bool D3D12Util_ResetRootSignature(CGPUCommandBuffer_D3D12* pCmd, ECGPUPipelineType type, ID3D12RootSignature* pRootSignature)
{
    // Set root signature if the current one differs from pRootSignature
    if (pCmd->pBoundRootSignature != pRootSignature)
    {
        pCmd->pBoundRootSignature = pRootSignature;
        if (type == CGPU_PIPELINE_TYPE_GRAPHICS)
            COM_CALL(SetGraphicsRootSignature, pCmd->pDxCmdList, pRootSignature);
        else
            COM_CALL(SetComputeRootSignature, pCmd->pDxCmdList, pRootSignature);
    }
    return true;
}

inline static bool D3D12Util_UseAccel(CGPUCommandBufferId cmd, const CGPUDescriptorSet_D3D12* Set)
{
    if (Set->pBoundAccel)
    {
        if (Set->pBoundAccel->bIsDirty) // issue uav barrier
        {
            CGPUResourceBarrierDescriptor b = { 0 };
            CGPUBufferBarrier bb = {
                .buffer = Set->pBoundAccel->pASBuffer,
                .src_state = CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE,
                .dst_state = CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_READ,
            };
            b.buffer_barriers = &bb;
            b.buffer_barriers_count = 1;
            cgpu_cmd_resource_barrier(cmd, &b);
            Set->pBoundAccel->bIsDirty = false;
        }
    }
    return true;
}

// Compute CMDs
CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_d3d12(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc)
{
    // DO NOTHING NOW
    return (CGPUComputePassEncoderId)cmd;
}

void cgpu_compute_encoder_bind_descriptor_set_d3d12(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set)
{
    CGPUCommandBuffer_D3D12*       Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    const CGPUDescriptorSet_D3D12* Set = (CGPUDescriptorSet_D3D12*)set;
    D3D12_GPU_DESCRIPTOR_HANDLE HeapToBind = { Cmd->mBoundHeapStartHandles[0].ptr + Set->mCbvSrvUavHandle };
    D3D12Util_UseAccel(&Cmd->super, Set);
    COM_CALL(SetComputeRootDescriptorTable, Cmd->pDxCmdList, set->index, HeapToBind);
}

void cgpu_compute_encoder_bind_pipeline_d3d12(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline)
{
    CGPUCommandBuffer_D3D12*   Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPUComputePipeline_D3D12* PPL = (CGPUComputePipeline_D3D12*)pipeline;
    D3D12Util_ResetRootSignature(Cmd, CGPU_PIPELINE_TYPE_COMPUTE, PPL->pRootSignature);
    COM_CALL(SetPipelineState, Cmd->pDxCmdList, PPL->pDxPipelineState);
}

void cgpu_compute_encoder_push_constants_d3d12(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPURootSignature_D3D12* RS  = (CGPURootSignature_D3D12*)rs;
    D3D12Util_ResetRootSignature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, RS->pDxRootSignature);
    if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_GRAPHICS)
    {
        COM_CALL(SetGraphicsRoot32BitConstants, Cmd->pDxCmdList,
            RS->mRootParamIndex, RS->mRootConstantParam.Constants.Num32BitValues, data, 0);
    }
    else if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_COMPUTE)
    {
        COM_CALL(SetComputeRoot32BitConstants, Cmd->pDxCmdList,
            RS->mRootParamIndex, RS->mRootConstantParam.Constants.Num32BitValues, data, 0);
    }
}

// Render CMDs

void cgpu_render_encoder_bind_vertex_buffers_d3d12(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
                                                   const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;

    const CGPUBuffer_D3D12** Buffers = (const CGPUBuffer_D3D12**)buffers;
    SKR_DECLARE_ZERO(D3D12_VERTEX_BUFFER_VIEW, views[CGPU_MAX_VERTEX_ATTRIBS]);
    for (uint32_t i = 0; i < buffer_count; ++i)
    {
        cgpu_assert(D3D12_GPU_VIRTUAL_ADDRESS_NULL != Buffers[i]->mDxGpuAddress);

        views[i].BufferLocation =
        (Buffers[i]->mDxGpuAddress + (offsets ? offsets[i] : 0));
        views[i].SizeInBytes =
        (UINT)(Buffers[i]->super.info->size - (offsets ? offsets[i] : 0));
        views[i].StrideInBytes = (UINT)strides[i];
    }
    COM_CALL(IASetVertexBuffers, Cmd->pDxCmdList, 0, buffer_count, views);
}

void cgpu_render_encoder_bind_index_buffer_d3d12(CGPURenderPassEncoderId encoder, CGPUBufferId buffer,
                                                 uint32_t index_stride, uint64_t offset)
{
    CGPUCommandBuffer_D3D12* Cmd    = (CGPUCommandBuffer_D3D12*)encoder;
    const CGPUBuffer_D3D12*  Buffer = (const CGPUBuffer_D3D12*)buffer;
    cgpu_assert(Cmd);
    cgpu_assert(buffer);
    cgpu_assert(CGPU_NULLPTR != Cmd->pDxCmdList);
    cgpu_assert(CGPU_NULLPTR != Buffer->pDxResource);

    SKR_DECLARE_ZERO(D3D12_INDEX_BUFFER_VIEW, view);
    view.BufferLocation = Buffer->mDxGpuAddress + offset;
    view.Format =
        (sizeof(uint16_t) == index_stride) ?
        DXGI_FORMAT_R16_UINT :
        ((sizeof(uint8_t) == index_stride) ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32_UINT);
    view.SizeInBytes = (UINT)(Buffer->super.info->size - offset);
    COM_CALL(IASetIndexBuffer, Cmd->pDxCmdList, &view);
}

void cgpu_compute_encoder_dispatch_d3d12(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    COM_CALL(Dispatch, Cmd->pDxCmdList, X, Y, Z);
}

void cgpu_cmd_end_compute_pass_d3d12(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder)
{
    // DO NOTHING NOW
}

CGPURenderPassEncoderId cgpu_cmd_begin_render_pass_d3d12(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
#ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
    ID3D12GraphicsCommandList4* CmdList4 = (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList;
    SKR_DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValues[CGPU_MAX_MRT_COUNT]);
    SKR_DECLARE_ZERO(D3D12_CLEAR_VALUE, clearDepth);
    SKR_DECLARE_ZERO(D3D12_CLEAR_VALUE, clearStencil);
    SKR_DECLARE_ZERO(D3D12_RENDER_PASS_RENDER_TARGET_DESC, renderPassRenderTargetDescs[CGPU_MAX_MRT_COUNT]);
    SKR_DECLARE_ZERO(D3D12_RENDER_PASS_DEPTH_STENCIL_DESC, renderPassDepthStencilDesc);
    uint32_t colorTargetCount = 0;
    // color
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        CGPUTextureView_D3D12* TV = (CGPUTextureView_D3D12*)desc->color_attachments[i].view;

        clearValues[i].Format  = DXGIUtil_TranslatePixelFormat(TV->super.info.format, false);
        clearValues[i].Color[0] = desc->color_attachments[i].clear_color.r;
        clearValues[i].Color[1] = desc->color_attachments[i].clear_color.g;
        clearValues[i].Color[2] = desc->color_attachments[i].clear_color.b;
        clearValues[i].Color[3] = desc->color_attachments[i].clear_color.a;
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE BeginningAccessType = gDx12PassBeginOpTranslator[desc->color_attachments[i].load_action];
        CGPUTextureView_D3D12* TV_Resolve = (CGPUTextureView_D3D12*)desc->color_attachments[i].resolve_view;
        if (desc->sample_count != CGPU_SAMPLE_COUNT_1 && TV_Resolve)
        {
            CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)TV->super.info.texture;
            CGPUTexture_D3D12* T_Resolve = (CGPUTexture_D3D12*)TV_Resolve->super.info.texture;
            const CGPUTextureInfo* pTexInfo = T->super.info;
            const CGPUTextureInfo* pResolveTexInfo = T_Resolve->super.info;
            
            D3D12_RENDER_PASS_BEGINNING_ACCESS BeginningAccess = { .Type = BeginningAccessType, .Clear = { .ClearValue = clearValues[i] } };
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE EndingAccessType = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
            D3D12_RENDER_PASS_ENDING_ACCESS EndingAccess = { .Type = EndingAccessType, .Resolve = { 0 } };

            renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = TV->mDxRtvDsvDescriptorHandle;
            renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = BeginningAccess;
            renderPassRenderTargetDescs[colorTargetCount].EndingAccess = EndingAccess;

            D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_PARAMETERS* pResolve = &renderPassRenderTargetDescs[colorTargetCount].EndingAccess.Resolve;
            pResolve->ResolveMode = D3D12_RESOLVE_MODE_AVERAGE; // TODO: int->MODE_MAX
            pResolve->Format = clearValues[i].Format;
            pResolve->pSrcResource = T->pDxResource;
            pResolve->pDstResource = T_Resolve->pDxResource;
            // Cmd->mSubResolveResource[i].SrcRect = { 0, 0, 0, 0 };
            // RenderDoc has a bug, it will crash if SrcRect is zero
            D3D12_RECT SrcRect = { 0, 0, (LONG)pTexInfo->width, (LONG)pTexInfo->height };
            Cmd->mSubResolveResource[i].SrcRect = SrcRect;
            Cmd->mSubResolveResource[i].DstX = 0;
            Cmd->mSubResolveResource[i].DstY = 0;
            Cmd->mSubResolveResource[i].SrcSubresource = 0;
            Cmd->mSubResolveResource[i].DstSubresource = CALC_SUBRESOURCE_INDEX(
                0, 0, 0,
                pResolveTexInfo->mip_levels, pResolveTexInfo->array_size_minus_one + 1
            );
            pResolve->PreserveResolveSource  = false;
            pResolve->SubresourceCount       = 1;
            pResolve->pSubresourceParameters = &Cmd->mSubResolveResource[i];
        }
        else
        {
            // Load & Store action
            D3D12_RENDER_PASS_BEGINNING_ACCESS BeginningAccess = { .Type = BeginningAccessType, .Clear = { .ClearValue = clearValues[i] } };
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE EndingAccessType = gDx12PassEndOpTranslator[desc->color_attachments[i].store_action];
            D3D12_RENDER_PASS_ENDING_ACCESS EndingAccess = { .Type = EndingAccessType, .Resolve = { 0 } };
            renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor   = TV->mDxRtvDsvDescriptorHandle;
            renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = BeginningAccess;
            renderPassRenderTargetDescs[colorTargetCount].EndingAccess    = EndingAccess;
        }
        colorTargetCount++;
    }
    // depth stencil
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pRenderPassDepthStencilDesc = CGPU_NULLPTR;
    if (desc->depth_stencil != CGPU_NULLPTR && desc->depth_stencil->view != CGPU_NULLPTR)
    {
        CGPUTextureView_D3D12*                  DTV             = (CGPUTextureView_D3D12*)desc->depth_stencil->view;
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE dBeginingAccess = gDx12PassBeginOpTranslator[desc->depth_stencil->depth_load_action];
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE sBeginingAccess = gDx12PassBeginOpTranslator[desc->depth_stencil->stencil_load_action];
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE    dEndingAccess   = gDx12PassEndOpTranslator[desc->depth_stencil->depth_store_action];
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE    sEndingAccess   = gDx12PassEndOpTranslator[desc->depth_stencil->stencil_store_action];
        clearDepth.Format                                       = DXGIUtil_TranslatePixelFormat(desc->depth_stencil->view->info.format, false);
        clearDepth.DepthStencil.Depth                           = desc->depth_stencil->clear_depth;
        clearStencil.Format                                     = DXGIUtil_TranslatePixelFormat(desc->depth_stencil->view->info.format, false);
        clearStencil.DepthStencil.Stencil                       = desc->depth_stencil->clear_stencil;
        renderPassDepthStencilDesc.cpuDescriptor                = DTV->mDxRtvDsvDescriptorHandle;
        
        D3D12_RENDER_PASS_BEGINNING_ACCESS DepthBeginningAccess = { dBeginingAccess, { clearDepth } };
        renderPassDepthStencilDesc.DepthBeginningAccess = DepthBeginningAccess;
        
        D3D12_RENDER_PASS_ENDING_ACCESS DepthEndingAccess = { dEndingAccess, { 0 } };
        renderPassDepthStencilDesc.DepthEndingAccess = DepthEndingAccess;
        
        D3D12_RENDER_PASS_BEGINNING_ACCESS StencilBeginningAccess = { sBeginingAccess, { clearStencil } };
        renderPassDepthStencilDesc.StencilBeginningAccess = StencilBeginningAccess;

        D3D12_RENDER_PASS_ENDING_ACCESS StencilEndingAccess = { sEndingAccess, { 0 } };
        renderPassDepthStencilDesc.StencilEndingAccess = StencilEndingAccess;

        pRenderPassDepthStencilDesc                             = &renderPassDepthStencilDesc;
    }
    D3D12_RENDER_PASS_RENDER_TARGET_DESC* pRenderPassRenderTargetDesc = renderPassRenderTargetDescs;
    COM_CALL(BeginRenderPass, CmdList4, colorTargetCount,
                              pRenderPassRenderTargetDesc, pRenderPassDepthStencilDesc,
                              D3D12_RENDER_PASS_FLAG_NONE);
    return (CGPURenderPassEncoderId)&Cmd->super;
#endif
    cgpu_warn(u8"ID3D12GraphicsCommandList4 is not defined!");
    return (CGPURenderPassEncoderId)&Cmd->super;
}

void cgpu_cmd_end_render_pass_d3d12(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
#ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
    ID3D12GraphicsCommandList4* CmdList4 = (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList;
    COM_CALL(EndRenderPass, CmdList4);
    return;
#endif
    cgpu_warn(u8"ID3D12GraphicsCommandList4 is not defined!");
}

CGPURenderPassEncoderId cgpu_cmd_begin_render_pass_d3d12_fallback(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc)
{
    SKR_UNIMPLEMENTED_FUNCTION();

    CGPUCommandBuffer_D3D12*    Cmd      = (CGPUCommandBuffer_D3D12*)cmd;
    ID3D12GraphicsCommandList*  pCmdList = Cmd->pDxCmdList;
    D3D12_CPU_DESCRIPTOR_HANDLE Rtvs[CGPU_MAX_MRT_COUNT];
    D3D12_CPU_DESCRIPTOR_HANDLE Dtv;
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        CGPUTextureView_D3D12* RTV = (CGPUTextureView_D3D12*)desc->color_attachments[i].view;
        Rtvs[i] = RTV->mDxRtvDsvDescriptorHandle;
    }
    if (desc->depth_stencil && desc->depth_stencil->view)
    {
        CGPUTextureView_D3D12* DTV = (CGPUTextureView_D3D12*)desc->depth_stencil->view;
        Dtv = DTV->mDxRtvDsvDescriptorHandle;
    }

    COM_CALL(OMSetRenderTargets, pCmdList, desc->render_target_count, Rtvs, FALSE, desc->depth_stencil ? &Dtv : CGPU_NULLPTR);

    return (CGPURenderPassEncoderId)&Cmd->super;
}

void cgpu_cmd_end_render_pass_d3d12_fallback(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    // CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    // ID3D12GraphicsCommandList* pCmdList = Cmd->pDxCmdList;
}

void cgpu_render_encoder_bind_descriptor_set_d3d12(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set)
{
    CGPUCommandBuffer_D3D12*       Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    const CGPUDescriptorSet_D3D12* Set = (CGPUDescriptorSet_D3D12*)set;
    CGPURootSignature_D3D12*       RS  = (CGPURootSignature_D3D12*)Set->super.root_signature;
    SKR_ASSERT(RS);
    D3D12Util_ResetRootSignature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, RS->pDxRootSignature);
    D3D12Util_UseAccel(&Cmd->super, Set);
    if (Set->mCbvSrvUavHandle != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE HeapToBind = { Cmd->mBoundHeapStartHandles[0].ptr + Set->mCbvSrvUavHandle };
        COM_CALL(SetGraphicsRootDescriptorTable, Cmd->pDxCmdList, set->index, HeapToBind);
    }
    else if (Set->mSamplerHandle != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE HeapToBind = { Cmd->mBoundHeapStartHandles[1].ptr + Set->mSamplerHandle };
        COM_CALL(SetGraphicsRootDescriptorTable, Cmd->pDxCmdList, set->index, HeapToBind);
    }
}

void cgpu_render_encoder_set_viewport_d3d12(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    D3D12_VIEWPORT           viewport;
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width    = width;
    viewport.Height   = height;
    viewport.MinDepth = min_depth;
    viewport.MaxDepth = max_depth;
    COM_CALL(RSSetViewports, Cmd->pDxCmdList, 1, &viewport);
}

void cgpu_render_encoder_set_scissor_d3d12(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    D3D12_RECT               scissor;
    scissor.left   = x;
    scissor.top    = y;
    scissor.right  = x + width;
    scissor.bottom = y + height;
    COM_CALL(RSSetScissorRects, Cmd->pDxCmdList, 1, &scissor);
}

void cgpu_render_encoder_bind_pipeline_d3d12(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline)
{
    CGPUCommandBuffer_D3D12*  Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPURenderPipeline_D3D12* PPL = (CGPURenderPipeline_D3D12*)pipeline;
    D3D12Util_ResetRootSignature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, PPL->pRootSignature);
    COM_CALL(IASetPrimitiveTopology, Cmd->pDxCmdList, PPL->mDxPrimitiveTopology);
    COM_CALL(SetPipelineState, Cmd->pDxCmdList, PPL->pDxPipelineState);
}

void cgpu_render_encoder_push_constants_d3d12(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPURootSignature_D3D12* RS  = (CGPURootSignature_D3D12*)rs;
    D3D12Util_ResetRootSignature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, RS->pDxRootSignature);
    if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_GRAPHICS)
    {
        COM_CALL(SetGraphicsRoot32BitConstants, Cmd->pDxCmdList, 
                RS->mRootParamIndex, RS->mRootConstantParam.Constants.Num32BitValues, data, 0);
    }
    else if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_COMPUTE)
    {
        COM_CALL(SetComputeRoot32BitConstants, Cmd->pDxCmdList,
            RS->mRootParamIndex, RS->mRootConstantParam.Constants.Num32BitValues, data, 0);
    }
}

void cgpu_render_encoder_draw_d3d12(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    COM_CALL(DrawInstanced, Cmd->pDxCmdList, (UINT)vertex_count, (UINT)1, (UINT)first_vertex, (UINT)0);
}

void cgpu_render_encoder_draw_instanced_d3d12(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    COM_CALL(DrawInstanced, Cmd->pDxCmdList, (UINT)vertex_count, (UINT)instance_count, (UINT)first_vertex, (UINT)first_instance);
}

void cgpu_render_encoder_draw_indexed_d3d12(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    COM_CALL(DrawIndexedInstanced, Cmd->pDxCmdList, (UINT)index_count, (UINT)1, (UINT)first_index, (UINT)first_vertex, (UINT)0);
}

void cgpu_render_encoder_draw_indexed_instanced_d3d12(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    COM_CALL(DrawIndexedInstanced, Cmd->pDxCmdList, (UINT)index_count, (UINT)instance_count, (UINT)first_index, (UINT)first_vertex, (UINT)first_instance);
}

// SwapChain APIs
CGPUSwapChainId cgpu_create_swapchain_d3d12_impl(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc, CGPUSwapChain_D3D12* old)
{
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)device->adapter->instance;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)old;
    const uint32_t buffer_count = desc->image_count;
    if (!old)
    {
        const size_t size = sizeof(CGPUSwapChain_D3D12) +
                          (sizeof(CGPUTexture_D3D12) + sizeof(CGPUTextureInfo)) * buffer_count +
                          sizeof(CGPUTextureId) * buffer_count;
        S = (CGPUSwapChain_D3D12*)cgpu_calloc_aligned(1, size, _Alignof(CGPUSwapChain_D3D12));
    }
    S->mDxSyncInterval = 0;
    SKR_DECLARE_ZERO(DXGI_SWAP_CHAIN_DESC1, chain_desc1)
    chain_desc1.Width = desc->width;
    chain_desc1.Height = desc->height;
    chain_desc1.Format = DXGIUtil_TranslatePixelFormat(desc->format, false);
    chain_desc1.Stereo = false;
    chain_desc1.SampleDesc.Count = 1; // If multisampling is needed, we'll resolve it later
    chain_desc1.SampleDesc.Quality = 0;
    chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    chain_desc1.BufferCount = desc->image_count;
    chain_desc1.Scaling = DXGI_SCALING_STRETCH;
    chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // for better performance.
    chain_desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    chain_desc1.Flags = 0;
    BOOL allowTearing = FALSE;
    COM_CALL(CheckFeatureSupport, I->pDXGIFactory, DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    chain_desc1.Flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    S->mFlags |= (!desc->enable_vsync && allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    IDXGISwapChain1* swapchain;
    HWND             hwnd = (HWND)desc->surface;

    CGPUQueue_D3D12* Q = CGPU_NULLPTR;
    if (desc->present_queues == CGPU_NULLPTR)
    {
        Q = (CGPUQueue_D3D12*)cgpu_get_queue_d3d12(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    }
    else
    {
        Q = (CGPUQueue_D3D12*)desc->present_queues[0];
    }
    bool bCreated = SUCCEEDED(COM_CALL(CreateSwapChainForHwnd, I->pDXGIFactory, (IUnknown*)Q->pCommandQueue, hwnd, &chain_desc1, NULL, NULL, &swapchain));
    (void)bCreated;
    cgpu_assert(bCreated && "Failed to Try to Create SwapChain! An existed swapchain might be destroyed!");

    bool bAssociation = SUCCEEDED(COM_CALL(MakeWindowAssociation, I->pDXGIFactory, hwnd, DXGI_MWA_NO_ALT_ENTER));
    (void)bAssociation;
    cgpu_assert(bAssociation && "Failed to Try to Associate SwapChain With Window!");

    bool bQueryChain3 = SUCCEEDED(COM_CALL(QueryInterface, swapchain, IID_ARGS(IDXGISwapChain3, &S->pDxSwapChain)));
    (void)bQueryChain3;
    cgpu_assert(bQueryChain3 && "Failed to Query IDXGISwapChain3 from Created SwapChain!");

    SAFE_RELEASE(swapchain);

    // Get swapchain images
    ID3D12Resource* backbuffers[desc->image_count];
    for (uint32_t i = 0; i < desc->image_count; ++i)
    {
        CHECK_HRESULT(COM_CALL(GetBuffer, S->pDxSwapChain, i, IID_ARGS(ID3D12Resource, &backbuffers[i])));
    }
    typedef struct THeader {
        CGPUTexture_D3D12 T;
        CGPUTextureInfo   I;
    } THeader;
    THeader* Ts = (THeader*)(S + 1);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Ts[i].T.pDxResource          = backbuffers[i];
        Ts[i].T.pDxAllocation        = CGPU_NULLPTR;
        Ts[i].T.super.device         = &D->super;
        Ts[i].T.super.info           = &Ts[i].I;
        Ts[i].I.is_cube              = false;
        Ts[i].I.array_size_minus_one = 0;
        Ts[i].I.sample_count         = CGPU_SAMPLE_COUNT_1; // TODO: ?
        Ts[i].I.format               = desc->format;
        Ts[i].I.aspect_mask          = 1;
        Ts[i].I.depth                = 1;
        Ts[i].I.width                = desc->width;
        Ts[i].I.height               = desc->height;
        Ts[i].I.mip_levels           = 1;
        Ts[i].I.node_index           = CGPU_SINGLE_GPU_NODE_INDEX;
        Ts[i].I.owns_image           = false;
    }
    CGPUTextureId* Vs = (CGPUTextureId*)(Ts + buffer_count);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Vs[i] = &Ts[i].T.super;
    }
    S->super.back_buffers = Vs;
    S->super.buffer_count = buffer_count;
    return &S->super;
}

void cgpu_free_swapchain_d3d12_impl(CGPUSwapChainId swapchain)
{
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)swapchain;
    for (uint32_t i = 0; i < S->super.buffer_count; i++)
    {
        CGPUTexture_D3D12* Texture = (CGPUTexture_D3D12*)S->super.back_buffers[i];
        SAFE_RELEASE(Texture->pDxResource);
    }
    SAFE_RELEASE(S->pDxSwapChain);
}

CGPUSwapChainId cgpu_create_swapchain_d3d12(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc)
{
    return cgpu_create_swapchain_d3d12_impl(device, desc, CGPU_NULLPTR);
}

void cgpu_free_swapchain_d3d12(CGPUSwapChainId swapchain)
{
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)swapchain;
    cgpu_free_swapchain_d3d12_impl(swapchain);
    cgpu_free_aligned(S, _Alignof(CGPUSwapChain_D3D12));
}

uint32_t cgpu_acquire_next_image_d3d12(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc)
{
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)swapchain;
    // On PC AquireNext is always true
    HRESULT hr = S_OK;
    if (FAILED(hr))
    {
        cgpu_error(u8"Failed to acquire next image");
        return UINT32_MAX;
    }
    return COM_CALL(GetCurrentBackBufferIndex, S->pDxSwapChain);
}

#include "SkrGraphics/extensions/cgpu_d3d12_exts.h"
// extentions
CGPUDREDSettingsId cgpu_d3d12_enable_DRED()
{
    CGPUDREDSettingsId settings = cgpu_calloc(1, sizeof(CGPUDREDSettings));
    SUCCEEDED(D3D12GetDebugInterface(IID_ARGS(ID3D12DeviceRemovedExtendedDataSettings, &settings->pDredSettings)));
    // Turn on auto-breadcrumbs and page fault reporting.
    COM_CALL(SetAutoBreadcrumbsEnablement, settings->pDredSettings, D3D12_DRED_ENABLEMENT_FORCED_ON);
    COM_CALL(SetPageFaultEnablement, settings->pDredSettings, D3D12_DRED_ENABLEMENT_FORCED_ON);
    return settings;
}

void cgpu_d3d12_disable_DRED(CGPUDREDSettingsId settings)
{
    COM_CALL(SetAutoBreadcrumbsEnablement, settings->pDredSettings, D3D12_DRED_ENABLEMENT_FORCED_OFF);
    COM_CALL(SetPageFaultEnablement, settings->pDredSettings, D3D12_DRED_ENABLEMENT_FORCED_OFF);
    SAFE_RELEASE(settings->pDredSettings);
    cgpu_free(settings);
}

ID3D12GraphicsCommandList* cgpu_d3d12_get_command_list(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    return Cmd->pDxCmdList;
}

ID3D12Resource* cgpu_d3d12_get_buffer(CGPUBufferId buffer)
{
    CGPUBuffer_D3D12* Buf = (CGPUBuffer_D3D12*)buffer;
    return Buf->pDxResource;
}

ID3D12Device* cgpu_d3d12_get_device(CGPUDeviceId device)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    return D->pDxDevice;
}

inline static D3D12_QUERY_TYPE D3D12Util_ToD3D12QueryType(ECGPUQueryType type)
{
    switch (type)
    {
        case CGPU_QUERY_TYPE_TIMESTAMP:
            return D3D12_QUERY_TYPE_TIMESTAMP;
        case CGPU_QUERY_TYPE_PIPELINE_STATISTICS:
            return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
        case CGPU_QUERY_TYPE_OCCLUSION:
            return D3D12_QUERY_TYPE_OCCLUSION;
        default:
            cgpu_assert(false && "Invalid query heap type");
            return D3D12_QUERY_TYPE_OCCLUSION;
    }
}

inline static D3D12_QUERY_HEAP_TYPE D3D12Util_ToD3D12QueryHeapType(ECGPUQueryType type)
{
    switch (type)
    {
        case CGPU_QUERY_TYPE_TIMESTAMP:
            return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        case CGPU_QUERY_TYPE_PIPELINE_STATISTICS:
            return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        case CGPU_QUERY_TYPE_OCCLUSION:
            return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        default:
            cgpu_assert(false && "Invalid query heap type");
            return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
    }
}