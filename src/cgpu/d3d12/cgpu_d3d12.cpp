#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include "../common/common_utils.h"
#include <dxcapi.h>
#include <EASTL/string_hash_map.h>

#if !defined(XBOX)
    #pragma comment(lib, "d3d12.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "dxguid.lib")
    #pragma comment(lib, "dxcompiler.lib")
#endif

#include <comdef.h>

// Call this only once.

CGPUInstanceId cgpu_create_instance_d3d12(CGPUInstanceDescriptor const* descriptor)
{
    CGPUInstance_D3D12* result = cgpu_new<CGPUInstance_D3D12>();
    D3D12Util_Optionalenable_debug_layer(result, descriptor);

    UINT flags = 0;
    if (descriptor->enable_debug_layer) flags = DXGI_CREATE_FACTORY_DEBUG;
#if defined(XBOX)
#else
    if (SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&result->pDXGIFactory))))
    {
        uint32_t gpuCount = 0;
        bool foundSoftwareAdapter = false;
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

void cgpu_query_instance_features_d3d12(CGPUInstanceId instance, struct CGPUInstanceFeatures* features)
{
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)instance;
    (void)I;
    features->specialization_constant = false;
}

void cgpu_free_instance_d3d12(CGPUInstanceId instance)
{
    CGPUInstance_D3D12* to_destroy = (CGPUInstance_D3D12*)instance;
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
    cgpu_delete(to_destroy);
#ifdef _DEBUG
    {
        IDXGIDebug1* dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(
            DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
        SAFE_RELEASE(dxgiDebug);
    }
#endif
}

void cgpu_enum_adapters_d3d12(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num)
{
    cgpu_assert(instance != nullptr && "fatal: null instance!");
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)instance;
    *adapters_num = I->mAdaptersCount;
    if (!adapters)
    {
        return;
    }
    else
    {
        for (auto i = 0u; i < *adapters_num; i++)
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
    return UINT32_MAX;
}

CGPUDeviceId cgpu_create_device_d3d12(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc)
{
    CGPUAdapter_D3D12* A = (CGPUAdapter_D3D12*)adapter;
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)A->super.instance;
    CGPUDevice_D3D12* D = cgpu_new<CGPUDevice_D3D12>();
    *const_cast<CGPUAdapterId*>(&D->super.adapter) = adapter;

    if (!SUCCEEDED(D3D12CreateDevice(A->pDxActiveGPU, // default adapter
        A->mFeatureLevel, IID_PPV_ARGS(&D->pDxDevice))))
    {
        cgpu_assert("[D3D12 Fatal]: Create D3D12Device Failed!");
    }

    // Create Requested Queues.
    for (uint32_t i = 0u; i < desc->queue_group_count; i++)
    {
        const auto& queueGroup = desc->queue_groups[i];
        const auto type = queueGroup.queue_type;

        *const_cast<uint32_t*>(&D->pCommandQueueCounts[type]) = queueGroup.queue_count;
        *const_cast<ID3D12CommandQueue***>(&D->ppCommandQueues[type]) =
        (ID3D12CommandQueue**)cgpu_malloc(sizeof(ID3D12CommandQueue*) * queueGroup.queue_count);

        for (uint32_t j = 0u; j < queueGroup.queue_count; j++)
        {
            DECLARE_ZERO(D3D12_COMMAND_QUEUE_DESC, queueDesc)
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
            if (!SUCCEEDED(D->pDxDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&D->ppCommandQueues[type][j]))))
            {
                cgpu_assert("[D3D12 Fatal]: Create D3D12CommandQueue Failed!");
            }
        }
    }
    // Create D3D12MA Allocator
    D3D12Util_CreateDMAAllocator(I, A, D);
    cgpu_assert(D->pResourceAllocator && "DMA Allocator Must be Created!");
    // Create Descriptor Heaps
    D->pCPUDescriptorHeaps = (D3D12Util_DescriptorHeap**)cgpu_malloc(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES * sizeof(D3D12Util_DescriptorHeap*));
    D->pCbvSrvUavHeaps = (D3D12Util_DescriptorHeap**)cgpu_malloc(sizeof(D3D12Util_DescriptorHeap*));
    D->pSamplerHeaps = (D3D12Util_DescriptorHeap**)cgpu_malloc(sizeof(D3D12Util_DescriptorHeap*));
    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Flags = gCpuDescriptorHeapProperties[i].mFlags;
        desc.NodeMask = 0; // CPU Descriptor Heap - Node mask is irrelevant
        desc.NumDescriptors = gCpuDescriptorHeapProperties[i].mMaxDescriptors;
        desc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
        D3D12Util_CreateDescriptorHeap(D->pDxDevice, &desc, &D->pCPUDescriptorHeaps[i]);
    }
    // One shader visible heap for each linked node
    for (uint32_t i = 0; i < SINGLE_GPU_NODE_COUNT; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = SINGLE_GPU_NODE_MASK;

        desc.NumDescriptors = 1 << 16;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        D3D12Util_CreateDescriptorHeap(D->pDxDevice, &desc, &D->pCbvSrvUavHeaps[i]);

        desc.NumDescriptors = 1 << 11;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        D3D12Util_CreateDescriptorHeap(D->pDxDevice, &desc, &D->pSamplerHeaps[i]);
    }
    // Pipeline cache
    D3D12_FEATURE_DATA_SHADER_CACHE feature = {};
    HRESULT result = D->pDxDevice->CheckFeatureSupport(
    D3D12_FEATURE_SHADER_CACHE, &feature, sizeof(feature));
    if (SUCCEEDED(result))
    {
        result = E_NOTIMPL;
        if (feature.SupportFlags & D3D12_SHADER_CACHE_SUPPORT_LIBRARY)
        {
            ID3D12Device1* device1 = NULL;
            result = D->pDxDevice->QueryInterface(IID_ARGS(&device1));
            if (SUCCEEDED(result))
            {
                result = device1->CreatePipelineLibrary(
                D->pPSOCacheData, 0, IID_ARGS(&D->pPipelineLibrary));
            }
            SAFE_RELEASE(device1);
        }
    }
    return &D->super;
}

void cgpu_query_video_memory_info_d3d12(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    const CGPUAdapter_D3D12* A = (CGPUAdapter_D3D12*)device->adapter;
    DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
    A->pDxActiveGPU->QueryVideoMemoryInfo(
        SINGLE_GPU_NODE_INDEX, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
    *total = info.Budget;
    *used_bytes = info.CurrentUsage;
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
    // Free D3D12MA Allocator
    SAFE_RELEASE(D->pResourceAllocator);
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
    // Release D3D12 Device
    SAFE_RELEASE(D->pDxDevice);
    SAFE_RELEASE(D->pPipelineLibrary);
    if (D->pPSOCacheData) cgpu_free(D->pPSOCacheData);
    cgpu_delete(D);
}

// API Objects APIs
CGPUFenceId cgpu_create_fence_d3d12(CGPUDeviceId device)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    // create a Fence and cgpu_assert that it is valid
    CGPUFence_D3D12* F = cgpu_new<CGPUFence_D3D12>();
    cgpu_assert(F);

    CHECK_HRESULT(D->pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ARGS(&F->pDxFence)));
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
        uint64_t fenceValue = Fences[i]->mFenceValue - 1;
        if (fenceStatus == CGPU_FENCE_STATUS_INCOMPLETE)
        {
            Fences[i]->pDxFence->SetEventOnCompletion(
            fenceValue, Fences[i]->pDxWaitIdleFenceEvent);
            WaitForSingleObject(Fences[i]->pDxWaitIdleFenceEvent, INFINITE);
        }
    }
}

ECGPUFenceStatus cgpu_query_fence_status_d3d12(CGPUFenceId fence)
{
    ECGPUFenceStatus status = CGPU_FENCE_STATUS_COMPLETE;
    CGPUFence_D3D12* F = (CGPUFence_D3D12*)fence;
    const auto Value = F->pDxFence->GetCompletedValue();
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
    cgpu_delete(F);
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
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPURootSignature_D3D12* RS = cgpu_new<CGPURootSignature_D3D12>();
    // Pick root parameters from desc data
    CGPUShaderStages shaderStages = 0;
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        CGPUPipelineShaderDescriptor* shader_desc = desc->shaders + i;
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
        auto reflSlot = RS->super.push_constants;
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
        staticSamplerDescs = (D3D12_STATIC_SAMPLER_DESC*)alloca(
        staticSamplerCount * sizeof(D3D12_STATIC_SAMPLER_DESC));
        for (uint32_t i = 0; i < RS->super.static_sampler_count; i++)
        {
            auto& RST_slot = RS->super.static_samplers[i];
            for (uint32_t j = 0; j < desc->static_sampler_count; j++)
            {
                auto input_slot = (CGPUSampler_D3D12*)desc->static_samplers[j];
                if (strcmp(RST_slot.name, desc->static_sampler_names[j]) == 0)
                {
                    D3D12_SAMPLER_DESC& desc = input_slot->mDxDesc;
                    staticSamplerDescs[i].Filter = desc.Filter;
                    staticSamplerDescs[i].AddressU = desc.AddressU;
                    staticSamplerDescs[i].AddressV = desc.AddressV;
                    staticSamplerDescs[i].AddressW = desc.AddressW;
                    staticSamplerDescs[i].MipLODBias = desc.MipLODBias;
                    staticSamplerDescs[i].MaxAnisotropy = desc.MaxAnisotropy;
                    staticSamplerDescs[i].ComparisonFunc = desc.ComparisonFunc;
                    staticSamplerDescs[i].MinLOD = desc.MinLOD;
                    staticSamplerDescs[i].MaxLOD = desc.MaxLOD;
                    staticSamplerDescs[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

                    CGPUShaderResource* samplerResource = &RST_slot;
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
        RS->mRootParamIndex = valid_root_tables + i;
    }
    // Serialize PSO
    ID3DBlob* error = NULL;
    ID3DBlob* rootSignatureString = NULL;
    DECLARE_ZERO(D3D12_VERSIONED_ROOT_SIGNATURE_DESC, sig_desc);
    sig_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    sig_desc.Desc_1_1.NumParameters = paramCount;
    sig_desc.Desc_1_1.pParameters = rootParams;
    sig_desc.Desc_1_1.NumStaticSamplers = staticSamplerCount;
    sig_desc.Desc_1_1.pStaticSamplers = staticSamplerDescs;
    sig_desc.Desc_1_1.Flags = rootSignatureFlags;
    HRESULT hres = D3D12SerializeVersionedRootSignature(&sig_desc, &rootSignatureString, &error);
    if (!SUCCEEDED(hres))
    {
        cgpu_error("Failed to serialize root signature with error (%s)", (char*)error->GetBufferPointer());
    }
    // If running Linked Mode (SLI) create root signature for all nodes
    // #NOTE : In non SLI mode, mNodeCount will be 0 which sets nodeMask to
    // default value
    CHECK_HRESULT(D->pDxDevice->CreateRootSignature(
        SINGLE_GPU_NODE_MASK,
        rootSignatureString->GetBufferPointer(),
        rootSignatureString->GetBufferSize(),
        IID_ARGS(&RS->pDxRootSignature)));
    cgpu_free(rootParams);
    cgpu_free(cbvSrvUavRanges);
    // [RS POOL] INSERTION
    if (desc->pool)
    {
        const bool result = CGPUUtil_AddSignature(desc->pool, &RS->super, desc);
        cgpu_assert(result && "Root signature pool insertion failed!");
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
    cgpu_delete(RS);
    return;
}

CGPUDescriptorSetId cgpu_create_descriptor_set_d3d12(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    const CGPURootSignature_D3D12* RS = (const CGPURootSignature_D3D12*)desc->root_signature;
    CGPUDescriptorSet_D3D12* Set = cgpu_new<CGPUDescriptorSet_D3D12>();
    const uint32_t nodeIndex = SINGLE_GPU_NODE_INDEX;
    struct D3D12Util_DescriptorHeap* pCbvSrvUavHeap = D->pCbvSrvUavHeaps[nodeIndex];
    struct D3D12Util_DescriptorHeap* pSamplerHeap = D->pSamplerHeaps[nodeIndex];
    (void)pSamplerHeap;
    CGPUParameterTable* param_table = &RS->super.tables[desc->set_index];
    uint32_t CbvSrvUavCount = 0;
    uint32_t SamplerCount = 0;
    for (uint32_t i = 0; i < param_table->resources_count; i++)
    {
        if (param_table->resources[i].type == CGPU_RESOURCE_TYPE_SAMPLER)
            SamplerCount++;
        else if (param_table->resources[i].type == CGPU_RESOURCE_TYPE_TEXTURE ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_RW_TEXTURE ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_BUFFER ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_BUFFER_RAW ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_RW_BUFFER ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_RW_BUFFER_RAW ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_TEXTURE_CUBE ||
                 param_table->resources[i].type == CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
        {
            CbvSrvUavCount++;
        }
    }
    // CBV/SRV/UAV
    Set->mCbvSrvUavHandle = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    Set->mSamplerHandle = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    if (CbvSrvUavCount)
    {
        auto StartHandle = D3D12Util_ConsumeDescriptorHandles(pCbvSrvUavHeap, param_table->resources_count);
        Set->mCbvSrvUavHandle = StartHandle.mGpu.ptr - pCbvSrvUavHeap->mStartHandle.mGpu.ptr;
        Set->mCbvSrvUavStride = CbvSrvUavCount * pCbvSrvUavHeap->mDescriptorSize;
    }
    if (SamplerCount)
    {
        auto StartHandle = D3D12Util_ConsumeDescriptorHandles(pSamplerHeap, param_table->resources_count);
        Set->mSamplerHandle = StartHandle.mGpu.ptr - pSamplerHeap->mStartHandle.mGpu.ptr;
        Set->mSamplerStride = SamplerCount * pSamplerHeap->mDescriptorSize;
    }
    // TODO: Bind NULL handles on creation
    // TODO: Support root descriptors
    return &Set->super;
}

uint32_t descriptor_count_needed(CGPUShaderResource* resource)
{
    if (resource->dim == CGPU_TEX_DIMENSION_1D_ARRAY ||
        resource->dim == CGPU_TEX_DIMENSION_2D_ARRAY ||
        resource->dim == CGPU_TEX_DIMENSION_2DMS_ARRAY ||
        resource->dim == CGPU_TEX_DIMENSION_CUBE_ARRAY)
        return resource->size;
    else
        return 1;
}

void cgpu_update_descriptor_set_d3d12(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count)
{
    CGPUDescriptorSet_D3D12* Set = (CGPUDescriptorSet_D3D12*)set;
    const CGPURootSignature_D3D12* RS = (const CGPURootSignature_D3D12*)set->root_signature;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)set->root_signature->device;
    CGPUParameterTable* ParamTable = &RS->super.tables[set->index];
    const uint32_t nodeIndex = SINGLE_GPU_NODE_INDEX;
    struct D3D12Util_DescriptorHeap* pCbvSrvUavHeap = D->pCbvSrvUavHeaps[nodeIndex];
    struct D3D12Util_DescriptorHeap* pSamplerHeap = D->pSamplerHeaps[nodeIndex];
    for (uint32_t i = 0; i < count; i++)
    {
        // Descriptor Info
        const CGPUDescriptorData* pParam = datas + i;
        CGPUShaderResource* ResData = CGPU_NULLPTR;
        uint32_t HeapOffset = 0;
        if (pParam->name != CGPU_NULLPTR)
        {
            size_t argNameHash = cgpu_hash(pParam->name, strlen(pParam->name), *(size_t*)&D);
            for (uint32_t j = 0; j < ParamTable->resources_count; j++)
            {
                if (ParamTable->resources[j].name_hash == argNameHash)
                {
                    ResData = ParamTable->resources + j;
                    break;
                }
                HeapOffset += descriptor_count_needed(&ParamTable->resources[j]);
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
                HeapOffset += descriptor_count_needed(&ParamTable->resources[j]);
            }
        }
        // Update Info
        const uint32_t arrayCount = cgpu_max(1U, pParam->count);
        switch (ResData->type)
        {
            case CGPU_RESOURCE_TYPE_SAMPLER: {
                cgpu_assert(pParam->samplers && "cgpu_assert: Binding NULL Sampler(s)!");
                CGPUSampler_D3D12** Samplers = (CGPUSampler_D3D12**)pParam->samplers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->samplers[arr] && "cgpu_assert: Binding NULL Sampler!");
                    D3D12Util_CopyDescriptorHandle(pSamplerHeap,
                    { Samplers[arr]->mDxHandle.ptr },
                    Set->mSamplerHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_TEXTURE:
            case CGPU_RESOURCE_TYPE_TEXTURE_CUBE: {
                cgpu_assert(pParam->textures && "cgpu_assert: Binding NULL Textures(s)!");
                CGPUTextureView_D3D12** Textures = (CGPUTextureView_D3D12**)pParam->textures;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->textures[arr] && "cgpu_assert: Binding NULL Textures!");
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap,
                    { Textures[arr]->mDxDescriptorHandles.ptr + Textures[arr]->mDxSrvOffset },
                    Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_BUFFER:
            case CGPU_RESOURCE_TYPE_BUFFER_RAW: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                CGPUBuffer_D3D12** Buffers = (CGPUBuffer_D3D12**)pParam->buffers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap,
                    { Buffers[arr]->mDxDescriptorHandles.ptr + Buffers[arr]->mDxSrvOffset },
                    Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                CGPUBuffer_D3D12** Buffers = (CGPUBuffer_D3D12**)pParam->buffers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap,
                    { Buffers[arr]->mDxDescriptorHandles.ptr },
                    Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_RW_TEXTURE: {
                cgpu_assert(pParam->textures && "cgpu_assert: Binding NULL Texture(s)!");
                CGPUTextureView_D3D12** Textures = (CGPUTextureView_D3D12**)pParam->textures;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->textures[arr] && "cgpu_assert: Binding NULL Texture!");
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap,
                    { Textures[arr]->mDxDescriptorHandles.ptr + Textures[arr]->mDxUavOffset },
                    Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
            }
            break;
            case CGPU_RESOURCE_TYPE_RW_BUFFER:
            case CGPU_RESOURCE_TYPE_RW_BUFFER_RAW: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                CGPUBuffer_D3D12** Buffers = (CGPUBuffer_D3D12**)pParam->buffers;
                for (uint32_t arr = 0; arr < arrayCount; arr++)
                {
                    cgpu_assert(pParam->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap,
                    { Buffers[arr]->mDxDescriptorHandles.ptr + Buffers[arr]->mDxUavOffset },
                    Set->mCbvSrvUavHandle, arr + HeapOffset);
                }
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
    cgpu_delete(Set);
}

CGPUComputePipelineId cgpu_create_compute_pipeline_d3d12(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUComputePipeline_D3D12* PPL = cgpu_new<CGPUComputePipeline_D3D12>();
    CGPURootSignature_D3D12* RS = (CGPURootSignature_D3D12*)desc->root_signature;
    CGPUShaderLibrary_D3D12* SL = (CGPUShaderLibrary_D3D12*)desc->compute_shader->library;
    PPL->pRootSignature = RS->pDxRootSignature;
    // Add pipeline specifying its for compute purposes
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, CS);
    CS.BytecodeLength = SL->pShaderBlob->GetBufferSize();
    CS.pShaderBytecode = SL->pShaderBlob->GetBufferPointer();
    DECLARE_ZERO(D3D12_CACHED_PIPELINE_STATE, cached_pso_desc);
    cached_pso_desc.pCachedBlob = NULL;
    cached_pso_desc.CachedBlobSizeInBytes = 0;
    DECLARE_ZERO(D3D12_COMPUTE_PIPELINE_STATE_DESC, pipeline_state_desc);
    pipeline_state_desc.pRootSignature = RS->pDxRootSignature;
    pipeline_state_desc.CS = CS;
    pipeline_state_desc.CachedPSO = cached_pso_desc;
    pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    pipeline_state_desc.NodeMask = SINGLE_GPU_NODE_MASK;
    // Pipeline cache
    HRESULT result = E_FAIL;
    wchar_t pipelineName[PSO_NAME_LENGTH] = {};
    size_t psoShaderHash = 0;
    size_t psoComputeHash = 0;
    if (D->pPipelineLibrary)
    {
        if (CS.BytecodeLength)
            psoShaderHash = cgpu_hash(CS.pShaderBytecode, CS.BytecodeLength, psoShaderHash);
        psoComputeHash = cgpu_hash(&pipeline_state_desc.Flags, sizeof(D3D12_PIPELINE_STATE_FLAGS), psoComputeHash);
        psoComputeHash = cgpu_hash(&pipeline_state_desc.NodeMask, sizeof(UINT), psoComputeHash);
        swprintf(pipelineName, PSO_NAME_LENGTH, L"%S_S%zuR%zu", "COMPUTEPSO", psoShaderHash, psoComputeHash);
        result = D->pPipelineLibrary->LoadComputePipeline(pipelineName,
        &pipeline_state_desc, IID_ARGS(&PPL->pDxPipelineState));
    }
    if (!SUCCEEDED(result))
    {
        // XBOX: Support PSO extensions
        CHECK_HRESULT(D->pDxDevice->CreateComputePipelineState(
        &pipeline_state_desc, IID_PPV_ARGS(&PPL->pDxPipelineState)));
    }
    return &PPL->super;
}

void cgpu_free_compute_pipeline_d3d12(CGPUComputePipelineId pipeline)
{
    CGPUComputePipeline_D3D12* PPL = (CGPUComputePipeline_D3D12*)pipeline;
    SAFE_RELEASE(PPL->pDxPipelineState);
    cgpu_delete(PPL);
}

D3D12_DEPTH_STENCIL_DESC gDefaultDepthDesc = {};
D3D12_BLEND_DESC gDefaultBlendDesc = {};
D3D12_RASTERIZER_DESC gDefaultRasterizerDesc = {};
CGPURenderPipelineId cgpu_create_render_pipeline_d3d12(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPURenderPipeline_D3D12* PPL = cgpu_new<CGPURenderPipeline_D3D12>();
    CGPURootSignature_D3D12* RS = (CGPURootSignature_D3D12*)desc->root_signature;
    // Vertex input state
    DECLARE_ZERO(D3D12_INPUT_ELEMENT_DESC, input_elements[CGPU_MAX_VERTEX_ATTRIBS]);
    uint32_t elem_count = 0;
    if (desc->vertex_layout != nullptr)
    {
        for (uint32_t attrib_index = 0; attrib_index < desc->vertex_layout->attribute_count; ++attrib_index)
        {
            const CGPUVertexAttribute* attrib = &(desc->vertex_layout->attributes[attrib_index]);
            for (uint32_t arr_index = 0; arr_index < attrib->array_size; arr_index++)
            {
                input_elements[elem_count].SemanticIndex = arr_index;
                input_elements[elem_count].SemanticName = attrib->semantic_name;
                input_elements[elem_count].Format = DXGIUtil_TranslatePixelFormat(attrib->format);
                input_elements[elem_count].InputSlot = attrib->binding;
                input_elements[elem_count].AlignedByteOffset = attrib->offset + arr_index * FormatUtil_BitSizeOfBlock(attrib->format) / 8;
                if (attrib->rate == CGPU_INPUT_RATE_INSTANCE)
                {
                    input_elements[elem_count].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                    input_elements[elem_count].InstanceDataStepRate = 1;
                }
                else
                {
                    input_elements[elem_count].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    input_elements[elem_count].InstanceDataStepRate = 0;
                }
                elem_count++;
            }
        }
    }
    DECLARE_ZERO(D3D12_INPUT_LAYOUT_DESC, input_layout_desc);
    input_layout_desc.pInputElementDescs = elem_count ? input_elements : NULL;
    input_layout_desc.NumElements = elem_count;
    // Shader stages
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, VS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, PS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, DS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, HS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, GS);
    for (uint32_t i = 0; i < 5; ++i)
    {
        ECGPUShaderStage stage_mask = (ECGPUShaderStage)(1 << i);
        switch (stage_mask)
        {
            case CGPU_SHADER_STAGE_VERT: {
                if (desc->vertex_shader)
                {
                    CGPUShaderLibrary_D3D12* VertLib = (CGPUShaderLibrary_D3D12*)desc->vertex_shader->library;
                    VS.BytecodeLength = VertLib->pShaderBlob->GetBufferSize();
                    VS.pShaderBytecode = VertLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case CGPU_SHADER_STAGE_TESC: {
                if (desc->tesc_shader)
                {
                    CGPUShaderLibrary_D3D12* TescLib = (CGPUShaderLibrary_D3D12*)desc->tesc_shader->library;
                    HS.BytecodeLength = TescLib->pShaderBlob->GetBufferSize();
                    HS.pShaderBytecode = TescLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case CGPU_SHADER_STAGE_TESE: {
                if (desc->tese_shader)
                {
                    CGPUShaderLibrary_D3D12* TeseLib = (CGPUShaderLibrary_D3D12*)desc->tese_shader->library;
                    DS.BytecodeLength = TeseLib->pShaderBlob->GetBufferSize();
                    DS.pShaderBytecode = TeseLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case CGPU_SHADER_STAGE_GEOM: {
                if (desc->geom_shader)
                {
                    CGPUShaderLibrary_D3D12* GeomLib = (CGPUShaderLibrary_D3D12*)desc->geom_shader->library;
                    GS.BytecodeLength = GeomLib->pShaderBlob->GetBufferSize();
                    GS.pShaderBytecode = GeomLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case CGPU_SHADER_STAGE_FRAG: {
                if (desc->fragment_shader)
                {
                    CGPUShaderLibrary_D3D12* FragLib = (CGPUShaderLibrary_D3D12*)desc->fragment_shader->library;
                    PS.BytecodeLength = FragLib->pShaderBlob->GetBufferSize();
                    PS.pShaderBytecode = FragLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            default:
                cgpu_assert(false && "Shader Stage not supported!");
                break;
        }
    }
    // Stream out
    DECLARE_ZERO(D3D12_STREAM_OUTPUT_DESC, stream_output_desc);
    stream_output_desc.pSODeclaration = NULL;
    stream_output_desc.NumEntries = 0;
    stream_output_desc.pBufferStrides = NULL;
    stream_output_desc.NumStrides = 0;
    stream_output_desc.RasterizedStream = 0;
    // Sample
    DECLARE_ZERO(DXGI_SAMPLE_DESC, sample_desc);
    sample_desc.Count = (UINT)(desc->sample_count);
    sample_desc.Quality = (UINT)(desc->sample_quality);
    DECLARE_ZERO(D3D12_CACHED_PIPELINE_STATE, cached_pso_desc);
    cached_pso_desc.pCachedBlob = NULL;
    cached_pso_desc.CachedBlobSizeInBytes = 0;
    // Fill pipeline object desc
    DECLARE_ZERO(D3D12_GRAPHICS_PIPELINE_STATE_DESC, pipeline_state_desc);
    pipeline_state_desc.pRootSignature = RS->pDxRootSignature;
    // Single GPU
    pipeline_state_desc.NodeMask = SINGLE_GPU_NODE_MASK;
    pipeline_state_desc.VS = VS;
    pipeline_state_desc.PS = PS;
    pipeline_state_desc.DS = DS;
    pipeline_state_desc.HS = HS;
    pipeline_state_desc.GS = GS;
    pipeline_state_desc.StreamOutput = stream_output_desc;
    pipeline_state_desc.BlendState = desc->blend_state ? D3D12Util_TranslateBlendState(desc->blend_state) : gDefaultBlendDesc;
    pipeline_state_desc.SampleMask = UINT_MAX;
    pipeline_state_desc.RasterizerState = desc->rasterizer_state ? D3D12Util_TranslateRasterizerState(desc->rasterizer_state) : gDefaultRasterizerDesc;
    // Depth stencil
    pipeline_state_desc.DepthStencilState = desc->depth_state ? D3D12Util_TranslateDephStencilState(desc->depth_state) : gDefaultDepthDesc;
    pipeline_state_desc.InputLayout = input_layout_desc;
    pipeline_state_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    pipeline_state_desc.PrimitiveTopologyType = D3D12Util_TranslatePrimitiveTopology(desc->prim_topology);
    pipeline_state_desc.NumRenderTargets = desc->render_target_count;
    pipeline_state_desc.DSVFormat = DXGIUtil_TranslatePixelFormat(desc->depth_stencil_format);
    pipeline_state_desc.SampleDesc = sample_desc;
    pipeline_state_desc.CachedPSO = cached_pso_desc;
    pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    for (uint32_t i = 0; i < pipeline_state_desc.NumRenderTargets; ++i)
    {
        pipeline_state_desc.RTVFormats[i] = DXGIUtil_TranslatePixelFormat(desc->color_formats[i]);
    }
    // Create pipeline object
    HRESULT result = E_FAIL;
    wchar_t pipelineName[PSO_NAME_LENGTH] = {};
    size_t psoShaderHash = 0;
    size_t psoRenderHash = 0;
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
        psoRenderHash = cgpu_hash(&pipeline_state_desc.BlendState, sizeof(D3D12_BLEND_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&pipeline_state_desc.RasterizerState, sizeof(D3D12_RASTERIZER_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&pipeline_state_desc.DepthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&pipeline_state_desc.IBStripCutValue, sizeof(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE), psoRenderHash);
        psoRenderHash = cgpu_hash(pipeline_state_desc.RTVFormats,
        pipeline_state_desc.NumRenderTargets * sizeof(DXGI_FORMAT), psoRenderHash);
        psoRenderHash = cgpu_hash(&pipeline_state_desc.DSVFormat, sizeof(DXGI_FORMAT), psoRenderHash);
        psoRenderHash = cgpu_hash(&pipeline_state_desc.SampleDesc, sizeof(DXGI_SAMPLE_DESC), psoRenderHash);
        psoRenderHash = cgpu_hash(&pipeline_state_desc.Flags, sizeof(D3D12_PIPELINE_STATE_FLAGS), psoRenderHash);
        for (uint32_t i = 0; i < pipeline_state_desc.InputLayout.NumElements; i++)
        {
            psoRenderHash = cgpu_hash(&pipeline_state_desc.InputLayout.pInputElementDescs[i],
            sizeof(D3D12_INPUT_ELEMENT_DESC), psoRenderHash);
        }
        swprintf(pipelineName, PSO_NAME_LENGTH, L"%S_S%zuR%zu", "GRAPHICSPSO", psoShaderHash, psoRenderHash);
        result = D->pPipelineLibrary->LoadGraphicsPipeline(pipelineName,
        &pipeline_state_desc, IID_ARGS(&PPL->pDxPipelineState));
    }
    if (!SUCCEEDED(result))
    {
        CHECK_HRESULT(D->pDxDevice->CreateGraphicsPipelineState(
        &pipeline_state_desc, IID_PPV_ARGS(&PPL->pDxPipelineState)));
        // Pipeline cache
        if (D->pPipelineLibrary)
        {
            CHECK_HRESULT(D->pPipelineLibrary->StorePipeline(pipelineName, PPL->pDxPipelineState));
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
    PPL->pRootSignature = RS->pDxRootSignature;
    return &PPL->super;
}

void cgpu_free_render_pipeline_d3d12(CGPURenderPipelineId pipeline)
{
    CGPURenderPipeline_D3D12* PPL = (CGPURenderPipeline_D3D12*)pipeline;
    SAFE_RELEASE(PPL->pDxPipelineState);
    cgpu_delete(PPL);
}

D3D12_QUERY_TYPE D3D12Util_ToD3D12QueryType(ECGPUQueryType type)
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

D3D12_QUERY_HEAP_TYPE D3D12Util_ToD3D12QueryHeapType(ECGPUQueryType type)
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

CGPUQueryPoolId cgpu_create_query_pool_d3d12(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUQueryPool_D3D12* pQueryPool = cgpu_new<CGPUQueryPool_D3D12>();
    pQueryPool->mType = D3D12Util_ToD3D12QueryType(desc->type);
    pQueryPool->super.count = desc->query_count;

    D3D12_QUERY_HEAP_DESC Desc = {};
    Desc.Count = desc->query_count;
    Desc.NodeMask = SINGLE_GPU_NODE_MASK;
    Desc.Type = D3D12Util_ToD3D12QueryHeapType(desc->type);
    D->pDxDevice->CreateQueryHeap(&Desc, IID_ARGS(&pQueryPool->pDxQueryHeap));

    return &pQueryPool->super;
}

void cgpu_free_query_pool_d3d12(CGPUQueryPoolId pool)
{
    CGPUQueryPool_D3D12* QP = (CGPUQueryPool_D3D12*)pool;
    SAFE_RELEASE(QP->pDxQueryHeap);
    cgpu_delete(QP);
}

// Queue APIs
CGPUQueueId cgpu_get_queue_d3d12(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUQueue_D3D12* Q = cgpu_new<CGPUQueue_D3D12>();
    Q->pCommandQueue = D->ppCommandQueues[type][index];
    Q->pFence = (CGPUFence_D3D12*)cgpu_create_fence_d3d12(device);
    return &Q->super;
}

void cgpu_submit_queue_d3d12(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc)
{
    uint32_t CmdCount = desc->cmds_count;
    CGPUCommandBuffer_D3D12** Cmds = (CGPUCommandBuffer_D3D12**)desc->cmds;
    CGPUQueue_D3D12* Q = (CGPUQueue_D3D12*)queue;
    CGPUFence_D3D12* F = (CGPUFence_D3D12*)desc->signal_fence;

    // cgpu_assert that given cmd list and given params are valid
    cgpu_assert(CmdCount > 0);
    cgpu_assert(Cmds);
    // execute given command list
    cgpu_assert(Q->pCommandQueue);

    ID3D12CommandList** cmds = (ID3D12CommandList**)alloca(CmdCount * sizeof(ID3D12CommandList*));
    for (uint32_t i = 0; i < CmdCount; ++i)
    {
        cmds[i] = Cmds[i]->pDxCmdList;
    }
    // Wait semaphores
    CGPUFence_D3D12** WaitSemaphores = (CGPUFence_D3D12**)desc->wait_semaphores;
    for (uint32_t i = 0; i < desc->wait_semaphore_count; ++i)
        Q->pCommandQueue->Wait(WaitSemaphores[i]->pDxFence,
        WaitSemaphores[i]->mFenceValue - 1);
    // Execute
    Q->pCommandQueue->ExecuteCommandLists(CmdCount, cmds);
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
    if (Q->pFence->pDxFence->GetCompletedValue() < Q->pFence->mFenceValue - 1)
    {
        Q->pFence->pDxFence->SetEventOnCompletion(fenceValue, Q->pFence->pDxWaitIdleFenceEvent);
        WaitForSingleObject(Q->pFence->pDxWaitIdleFenceEvent, INFINITE);
    }
}

void cgpu_queue_present_d3d12(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc)
{
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)desc->swapchain;
    HRESULT hr = S->pDxSwapChain->Present(S->mDxSyncInterval, S->mFlags /*desc->index*/);
    if (FAILED(hr))
    {
        cgpu_error("Failed to present swapchain render target!");
#if defined(_WIN32)
        ID3D12Device* device = NULL;
        S->pDxSwapChain->GetDevice(IID_ARGS(&device));
        HRESULT removeHr = -1;
        auto failedCount = 0u;
        const auto retryCount = 8u;
        while (FAILED(removeHr) && (failedCount < retryCount))
        {
            removeHr = device->GetDeviceRemovedReason();
            if (FAILED(removeHr))
            {
                cgpu_info("Device removed, waiting for driver to come back online...");
                Sleep(500); // Wait for a few seconds to allow the driver to come
                            // back online before doing a reset.
                failedCount++;
            }
        }
        if (failedCount >= retryCount)
        {
            cgpu_fatal("Device driver get lost, unable to get removed reason.");
        }
    #ifdef __ID3D12DeviceRemovedExtendedData_FWD_DEFINED__
        ID3D12DeviceRemovedExtendedData* pDread;
        if (SUCCEEDED(device->QueryInterface(IID_ARGS(&pDread))))
        {
            D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs;
            if (SUCCEEDED(pDread->GetAutoBreadcrumbsOutput(&breadcrumbs)))
            {
                cgpu_info("Gathered auto-breadcrumbs output.");
            }
            D3D12_DRED_PAGE_FAULT_OUTPUT pageFault;
            if (SUCCEEDED(pDread->GetPageFaultAllocationOutput(&pageFault)))
            {
                cgpu_info("Gathered page fault allocation output.");
            }
        }
        pDread->Release();
    #endif
        device->Release();
#endif
    }
}

float cgpu_queue_get_timestamp_period_ns_d3d12(CGPUQueueId queue)
{
    CGPUQueue_D3D12* Q = (CGPUQueue_D3D12*)queue;
    UINT64 freq = 0;
    // ticks per second
    Q->pCommandQueue->GetTimestampFrequency(&freq);
    // ns per tick
    const double ms_period = 1e9 / (double)freq;
    return (float)ms_period;
}

void cgpu_free_queue_d3d12(CGPUQueueId queue)
{
    CGPUQueue_D3D12* Q = (CGPUQueue_D3D12*)queue;
    cgpu_assert(queue && "D3D12 ERROR: FREE NULL QUEUE!");
    cgpu_free_fence_d3d12(&Q->pFence->super);
    cgpu_delete(Q);
}

// Command Objects

// allocate_transient_command_allocator
ID3D12CommandAllocator* allocate_transient_command_allocator(CGPUCommandPool_D3D12* E, CGPUQueueId queue)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)queue->device;

    D3D12_COMMAND_LIST_TYPE type =
    queue->type == CGPU_QUEUE_TYPE_TRANSFER ?
    D3D12_COMMAND_LIST_TYPE_COPY :
    queue->type == CGPU_QUEUE_TYPE_COMPUTE ?
    D3D12_COMMAND_LIST_TYPE_COMPUTE :
    D3D12_COMMAND_LIST_TYPE_DIRECT;

    bool res = SUCCEEDED(D->pDxDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&E->pDxCmdAlloc)));
    if (res)
    {
        return E->pDxCmdAlloc;
    }
    return CGPU_NULLPTR;
}

void free_transient_command_allocator(ID3D12CommandAllocator* allocator) { SAFE_RELEASE(allocator); }

CGPUCommandPoolId cgpu_create_command_pool_d3d12(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc)
{
    CGPUCommandPool_D3D12* P = cgpu_new<CGPUCommandPool_D3D12>();
    P->pDxCmdAlloc = allocate_transient_command_allocator(P, queue);
    return &P->super;
}

void cgpu_reset_command_pool_d3d12(CGPUCommandPoolId pool)
{
    CGPUCommandPool_D3D12* P = (CGPUCommandPool_D3D12*)pool;
    CHECK_HRESULT(P->pDxCmdAlloc->Reset());
}

CGPUCommandBufferId cgpu_create_command_buffer_d3d12(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc)
{
    // initialize to zero
    CGPUCommandBuffer_D3D12* Cmd = cgpu_new<CGPUCommandBuffer_D3D12>();
    CGPUCommandPool_D3D12* P = (CGPUCommandPool_D3D12*)pool;
    CGPUQueue_D3D12* Q = (CGPUQueue_D3D12*)P->super.queue;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)Q->super.device;
    cgpu_assert(Cmd);

    // set command pool of new command
    Cmd->mNodeIndex = SINGLE_GPU_NODE_INDEX;
    Cmd->mType = Q->super.type;

    Cmd->pBoundHeaps[0] = D->pCbvSrvUavHeaps[Cmd->mNodeIndex];
    Cmd->pBoundHeaps[1] = D->pSamplerHeaps[Cmd->mNodeIndex];
    Cmd->pCmdPool = P;

    uint32_t nodeMask = Cmd->mNodeIndex;

    ID3D12PipelineState* initialState = NULL;
    CHECK_HRESULT(D->pDxDevice->CreateCommandList(
    nodeMask, gDx12CmdTypeTranslator[Cmd->mType], P->pDxCmdAlloc, initialState,
    __uuidof(Cmd->pDxCmdList), (void**)&(Cmd->pDxCmdList)));

    // Command lists are addd in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    CHECK_HRESULT(Cmd->pDxCmdList->Close());
    return &Cmd->super;
}

void cgpu_free_command_buffer_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    SAFE_RELEASE(Cmd->pDxCmdList);
    cgpu_delete(Cmd);
}

void cgpu_free_command_pool_d3d12(CGPUCommandPoolId pool)
{
    CGPUCommandPool_D3D12* P = (CGPUCommandPool_D3D12*)pool;
    cgpu_assert(pool && "D3D12 ERROR: FREE NULL COMMAND POOL!");
    cgpu_assert(P->pDxCmdAlloc && "D3D12 ERROR: FREE NULL pDxCmdAlloc!");

    free_transient_command_allocator(P->pDxCmdAlloc);
    cgpu_delete(P);
}

// CMDs
void cgpu_cmd_begin_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    CGPUCommandPool_D3D12* P = (CGPUCommandPool_D3D12*)Cmd->pCmdPool;
    CHECK_HRESULT(Cmd->pDxCmdList->Reset(P->pDxCmdAlloc, NULL));

    if (Cmd->mType != CGPU_QUEUE_TYPE_TRANSFER)
    {
        ID3D12DescriptorHeap* heaps[] = {
            Cmd->pBoundHeaps[0]->pCurrentHeap,
            Cmd->pBoundHeaps[1]->pCurrentHeap,
        };
        Cmd->pDxCmdList->SetDescriptorHeaps(2, heaps);

        Cmd->mBoundHeapStartHandles[0] = Cmd->pBoundHeaps[0]->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
        Cmd->mBoundHeapStartHandles[1] = Cmd->pBoundHeaps[1]->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
    }
    // Reset CPU side data
    Cmd->pBoundRootSignature = NULL;
}

// TODO: https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html#introduction
// Enhanced Barriers is not currently a hardware or driver requirement
void cgpu_cmd_resource_barrier_d3d12(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    const uint32_t barriers_count = desc->buffer_barriers_count + desc->texture_barriers_count;
    D3D12_RESOURCE_BARRIER* barriers = (D3D12_RESOURCE_BARRIER*)alloca(barriers_count * sizeof(D3D12_RESOURCE_BARRIER));
    uint32_t transitionCount = 0;
    for (uint32_t i = 0; i < desc->buffer_barriers_count; ++i)
    {
        const CGPUBufferBarrier* pTransBarrier = &desc->buffer_barriers[i];
        D3D12_RESOURCE_BARRIER* pBarrier = &barriers[transitionCount];
        CGPUBuffer_D3D12* pBuffer = (CGPUBuffer_D3D12*)pTransBarrier->buffer;
        if (pBuffer->super.memory_usage == CGPU_MEM_USAGE_GPU_ONLY ||
            pBuffer->super.memory_usage == CGPU_MEM_USAGE_GPU_TO_CPU ||
            (pBuffer->super.memory_usage == CGPU_MEM_USAGE_CPU_TO_GPU && (pBuffer->super.descriptors & CGPU_RESOURCE_TYPE_RW_BUFFER)))
        {
            if (CGPU_RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->src_state &&
                CGPU_RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->dst_state)
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
                if (pTransBarrier->d3d12.begin_ony)
                {
                    pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
                }
                else if (pTransBarrier->d3d12.end_only)
                {
                    pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                }
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                pBarrier->Transition.pResource = pBuffer->pDxResource;
                pBarrier->Transition.Subresource =
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

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

            pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            if (pTransBarrier->d3d12.begin_ony)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
            }
            else if (pTransBarrier->d3d12.end_only)
            {
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            }
            pBarrier->Transition.pResource = pTexture->pDxResource;
            pBarrier->Transition.Subresource =
            pTransBarrier->subresource_barrier ? CALC_SUBRESOURCE_INDEX(
                                                 pTransBarrier->mip_level, pTransBarrier->array_layer,
                                                 0, pTexture->super.mip_levels,
                                                 pTexture->super.array_size_minus_one + 1) :
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
        Cmd->pDxCmdList->ResourceBarrier(transitionCount, barriers);
    }
}

void cgpu_cmd_begin_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    auto Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    auto pQueryPool = (CGPUQueryPool_D3D12*)pool;
    D3D12_QUERY_TYPE type = pQueryPool->mType;
    switch (type)
    {
        case D3D12_QUERY_TYPE_OCCLUSION:
        case D3D12_QUERY_TYPE_PIPELINE_STATISTICS:
        case D3D12_QUERY_TYPE_BINARY_OCCLUSION:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM1:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM2:
        case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM3:
            break;
        case D3D12_QUERY_TYPE_TIMESTAMP:
            Cmd->pDxCmdList->EndQuery(pQueryPool->pDxQueryHeap, type, desc->index);
            break;
    }
}

void cgpu_cmd_end_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    cgpu_cmd_begin_query(cmd, pool, desc);
}

void cgpu_cmd_reset_query_pool_d3d12(CGPUCommandBufferId, CGPUQueryPoolId, uint32_t, uint32_t) {}

void cgpu_cmd_resolve_query_d3d12(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count)
{
    auto Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    auto pQueryPool = (CGPUQueryPool_D3D12*)pool;
    auto pReadbackBuffer = (CGPUBuffer_D3D12*)readback;
    Cmd->pDxCmdList->ResolveQueryData(
    pQueryPool->pDxQueryHeap, pQueryPool->mType, start_query, query_count,
    pReadbackBuffer->pDxResource, start_query * 8);
}

void cgpu_cmd_end_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    cgpu_assert(Cmd->pDxCmdList);
    CHECK_HRESULT(Cmd->pDxCmdList->Close());
}

// Compute CMDs
CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_d3d12(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc)
{
    // DO NOTHING NOW
    return (CGPUComputePassEncoderId)cmd;
}

void cgpu_compute_encoder_bind_descriptor_set_d3d12(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    const CGPUDescriptorSet_D3D12* Set = (CGPUDescriptorSet_D3D12*)set;
    Cmd->pDxCmdList->SetComputeRootDescriptorTable(set->index, { Cmd->mBoundHeapStartHandles[0].ptr + Set->mCbvSrvUavHandle });
}

bool reset_root_signature(CGPUCommandBuffer_D3D12* pCmd, ECGPUPipelineType type,
ID3D12RootSignature* pRootSignature)
{
    // Set root signature if the current one differs from pRootSignature
    if (pCmd->pBoundRootSignature != pRootSignature)
    {
        pCmd->pBoundRootSignature = pRootSignature;
        if (type == CGPU_PIPELINE_TYPE_GRAPHICS)
            pCmd->pDxCmdList->SetGraphicsRootSignature(pRootSignature);
        else
            pCmd->pDxCmdList->SetComputeRootSignature(pRootSignature);
    }
    return true;
}

void cgpu_compute_encoder_bind_pipeline_d3d12(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPUComputePipeline_D3D12* PPL = (CGPUComputePipeline_D3D12*)pipeline;
    reset_root_signature(Cmd, CGPU_PIPELINE_TYPE_COMPUTE, PPL->pRootSignature);
    Cmd->pDxCmdList->SetPipelineState(PPL->pDxPipelineState);
}

void cgpu_compute_encoder_push_constants_d3d12(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPURootSignature_D3D12* RS = (CGPURootSignature_D3D12*)rs;
    reset_root_signature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, RS->pDxRootSignature);
    if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_GRAPHICS)
    {
        Cmd->pDxCmdList->SetGraphicsRoot32BitConstants(RS->mRootParamIndex,
        RS->mRootConstantParam.Constants.Num32BitValues,
        data, 0);
    }
    else if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_COMPUTE)
    {
        Cmd->pDxCmdList->SetComputeRoot32BitConstants(RS->mRootParamIndex,
        RS->mRootConstantParam.Constants.Num32BitValues,
        data, 0);
    }
}

void cgpu_render_encoder_bind_vertex_buffers_d3d12(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;

    const CGPUBuffer_D3D12** Buffers = (const CGPUBuffer_D3D12**)buffers;
    DECLARE_ZERO(D3D12_VERTEX_BUFFER_VIEW, views[CGPU_MAX_VERTEX_ATTRIBS]);
    for (uint32_t i = 0; i < buffer_count; ++i)
    {
        cgpu_assert(D3D12_GPU_VIRTUAL_ADDRESS_NULL != Buffers[i]->mDxGpuAddress);

        views[i].BufferLocation =
        (Buffers[i]->mDxGpuAddress + (offsets ? offsets[i] : 0));
        views[i].SizeInBytes =
        (UINT)(Buffers[i]->super.size - (offsets ? offsets[i] : 0));
        views[i].StrideInBytes = (UINT)strides[i];
    }

    Cmd->pDxCmdList->IASetVertexBuffers(0, buffer_count, views);
}

void cgpu_render_encoder_bind_index_buffer_d3d12(CGPURenderPassEncoderId encoder, CGPUBufferId buffer,
uint32_t index_stride, uint64_t offset)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    const CGPUBuffer_D3D12* Buffer = (const CGPUBuffer_D3D12*)buffer;
    cgpu_assert(Cmd);
    cgpu_assert(buffer);
    cgpu_assert(CGPU_NULLPTR != Cmd->pDxCmdList);
    cgpu_assert(CGPU_NULLPTR != Buffer->pDxResource);

    DECLARE_ZERO(D3D12_INDEX_BUFFER_VIEW, view);
    view.BufferLocation = Buffer->mDxGpuAddress + offset;
    view.Format =
        (sizeof(uint16_t) == index_stride) ?
        DXGI_FORMAT_R16_UINT :
        ((sizeof(uint8_t) == index_stride) ? DXGI_FORMAT_R8_UINT : DXGI_FORMAT_R32_UINT);
    view.SizeInBytes = (UINT)(Buffer->super.size - offset);
    Cmd->pDxCmdList->IASetIndexBuffer(&view);
}

void cgpu_compute_encoder_dispatch_d3d12(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    Cmd->pDxCmdList->Dispatch(X, Y, Z);
}

void cgpu_cmd_end_compute_pass_d3d12(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder)
{
    // DO NOTHING NOW
}

// Render CMDs
CGPURenderPassEncoderId cgpu_cmd_begin_render_pass_d3d12(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
#ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
    ID3D12GraphicsCommandList4* CmdList4 = (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList;
    DECLARE_ZERO(D3D12_CLEAR_VALUE, clearValues[CGPU_MAX_MRT_COUNT]);
    DECLARE_ZERO(D3D12_CLEAR_VALUE, clearDepth);
    DECLARE_ZERO(D3D12_CLEAR_VALUE, clearStencil);
    DECLARE_ZERO(D3D12_RENDER_PASS_RENDER_TARGET_DESC, renderPassRenderTargetDescs[CGPU_MAX_MRT_COUNT]);
    DECLARE_ZERO(D3D12_RENDER_PASS_DEPTH_STENCIL_DESC, renderPassDepthStencilDesc);
    uint32_t colorTargetCount = 0;
    // color
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        CGPUTextureView_D3D12* TV = (CGPUTextureView_D3D12*)desc->color_attachments[i].view;
        clearValues[i].Format = DXGIUtil_TranslatePixelFormat(TV->super.info.format);
        clearValues[i].Color[0] = desc->color_attachments[i].clear_color.r;
        clearValues[i].Color[1] = desc->color_attachments[i].clear_color.g;
        clearValues[i].Color[2] = desc->color_attachments[i].clear_color.b;
        clearValues[i].Color[3] = desc->color_attachments[i].clear_color.a;
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE beginningAccess = gDx12PassBeginOpTranslator[desc->color_attachments[i].load_action];
        CGPUTextureView_D3D12* TV_Resolve = (CGPUTextureView_D3D12*)desc->color_attachments[i].resolve_view;
        if (desc->sample_count != CGPU_SAMPLE_COUNT_1 && TV_Resolve)
        {
            CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)TV->super.info.texture;
            CGPUTexture_D3D12* T_Resolve = (CGPUTexture_D3D12*)TV_Resolve->super.info.texture;
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE endingAccess = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
            renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = TV->mDxRtvDsvDescriptorHandle;
            renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = { beginningAccess, { clearValues[i] } };
            renderPassRenderTargetDescs[colorTargetCount].EndingAccess = { endingAccess, {} };
            auto& Resolve = renderPassRenderTargetDescs[colorTargetCount].EndingAccess.Resolve;
            Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE; // TODO: int->MODE_MAX
            Resolve.Format = clearValues[i].Format;
            Resolve.pSrcResource = T->pDxResource;
            Resolve.pDstResource = T_Resolve->pDxResource;
            Cmd->mSubResolveResource.SrcRect = { 0, 0, 0, 0 };
            Cmd->mSubResolveResource.DstX = 0;
            Cmd->mSubResolveResource.DstY = 0;
            Cmd->mSubResolveResource.SrcSubresource = 0;
            Cmd->mSubResolveResource.DstSubresource = CALC_SUBRESOURCE_INDEX(
                0, 0, 0,
                T->super.mip_levels, T->super.array_size_minus_one + 1);
            Resolve.PreserveResolveSource = false;
            Resolve.SubresourceCount = 1;
            Resolve.pSubresourceParameters = &Cmd->mSubResolveResource;
        }
        else
        {
            // Load & Store action
            D3D12_RENDER_PASS_ENDING_ACCESS_TYPE endingAccess = gDx12PassEndOpTranslator[desc->color_attachments[i].store_action];
            renderPassRenderTargetDescs[colorTargetCount].cpuDescriptor = TV->mDxRtvDsvDescriptorHandle;
            renderPassRenderTargetDescs[colorTargetCount].BeginningAccess = { beginningAccess, { clearValues[i] } };
            renderPassRenderTargetDescs[colorTargetCount].EndingAccess = { endingAccess, {} };
        }
        colorTargetCount++;
    }
    // depth stencil
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pRenderPassDepthStencilDesc = nullptr;
    if (desc->depth_stencil != nullptr && desc->depth_stencil->view != nullptr)
    {
        CGPUTextureView_D3D12* DTV = (CGPUTextureView_D3D12*)desc->depth_stencil->view;
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE dBeginingAccess = gDx12PassBeginOpTranslator[desc->depth_stencil->depth_load_action];
        D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE sBeginingAccess = gDx12PassBeginOpTranslator[desc->depth_stencil->stencil_load_action];
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE dEndingAccess = gDx12PassEndOpTranslator[desc->depth_stencil->depth_store_action];
        D3D12_RENDER_PASS_ENDING_ACCESS_TYPE sEndingAccess = gDx12PassEndOpTranslator[desc->depth_stencil->stencil_store_action];
        clearDepth.Format = DXGIUtil_TranslatePixelFormat(desc->depth_stencil->view->info.format);
        clearDepth.DepthStencil.Depth = desc->depth_stencil->clear_depth;
        clearStencil.Format = DXGIUtil_TranslatePixelFormat(desc->depth_stencil->view->info.format);
        clearStencil.DepthStencil.Stencil = desc->depth_stencil->clear_stencil;
        renderPassDepthStencilDesc.cpuDescriptor = DTV->mDxRtvDsvDescriptorHandle;
        renderPassDepthStencilDesc.DepthBeginningAccess = { dBeginingAccess, { clearDepth } };
        renderPassDepthStencilDesc.DepthEndingAccess = { dEndingAccess };
        renderPassDepthStencilDesc.StencilBeginningAccess = { sBeginingAccess, { clearStencil } };
        renderPassDepthStencilDesc.StencilEndingAccess = { sEndingAccess };
        pRenderPassDepthStencilDesc = &renderPassDepthStencilDesc;
    }
    D3D12_RENDER_PASS_RENDER_TARGET_DESC* pRenderPassRenderTargetDesc = renderPassRenderTargetDescs;
    CmdList4->BeginRenderPass(colorTargetCount, 
        pRenderPassRenderTargetDesc, pRenderPassDepthStencilDesc,
        D3D12_RENDER_PASS_FLAG_NONE);
    return (CGPURenderPassEncoderId)&Cmd->super;
#endif
    cgpu_info("ID3D12GraphicsCommandList4 is not defined!");
    return (CGPURenderPassEncoderId)&Cmd->super;
}

void cgpu_render_encoder_bind_descriptor_set_d3d12(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    const CGPUDescriptorSet_D3D12* Set = (CGPUDescriptorSet_D3D12*)set;
    CGPURootSignature_D3D12* RS = (CGPURootSignature_D3D12*)Set->super.root_signature;
    SKR_ASSERT(RS);
    reset_root_signature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, RS->pDxRootSignature);
    if (Set->mCbvSrvUavHandle != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
    {
        Cmd->pDxCmdList->SetGraphicsRootDescriptorTable(set->index,
        { Cmd->mBoundHeapStartHandles[0].ptr + Set->mCbvSrvUavHandle });
    }
    else if (Set->mSamplerHandle != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
    {
        Cmd->pDxCmdList->SetGraphicsRootDescriptorTable(set->index,
        { Cmd->mBoundHeapStartHandles[1].ptr + Set->mSamplerHandle });
    }
}

void cgpu_render_encoder_set_viewport_d3d12(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = min_depth;
    viewport.MaxDepth = max_depth;
    Cmd->pDxCmdList->RSSetViewports(1, &viewport);
}

void cgpu_render_encoder_set_scissor_d3d12(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    D3D12_RECT scissor;
    scissor.left = x;
    scissor.top = y;
    scissor.right = x + width;
    scissor.bottom = y + height;
    Cmd->pDxCmdList->RSSetScissorRects(1, &scissor);
}

void cgpu_render_encoder_bind_pipeline_d3d12(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPURenderPipeline_D3D12* PPL = (CGPURenderPipeline_D3D12*)pipeline;
    reset_root_signature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, PPL->pRootSignature);
    Cmd->pDxCmdList->IASetPrimitiveTopology(PPL->mDxPrimitiveTopology);
    Cmd->pDxCmdList->SetPipelineState(PPL->pDxPipelineState);
}

void cgpu_render_encoder_push_constants_d3d12(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPURootSignature_D3D12* RS = (CGPURootSignature_D3D12*)rs;
    reset_root_signature(Cmd, CGPU_PIPELINE_TYPE_GRAPHICS, RS->pDxRootSignature);
    if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_GRAPHICS)
    {
        Cmd->pDxCmdList->SetGraphicsRoot32BitConstants(RS->mRootParamIndex,
        RS->mRootConstantParam.Constants.Num32BitValues,
        data, 0);
    }
    else if (RS->super.pipeline_type == CGPU_PIPELINE_TYPE_COMPUTE)
    {
        Cmd->pDxCmdList->SetComputeRoot32BitConstants(RS->mRootParamIndex,
        RS->mRootConstantParam.Constants.Num32BitValues,
        data, 0);
    }
}

void cgpu_render_encoder_draw_d3d12(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    Cmd->pDxCmdList->DrawInstanced((UINT)vertex_count, (UINT)1, (UINT)first_vertex, (UINT)0);
}

void cgpu_render_encoder_draw_instanced_d3d12(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    Cmd->pDxCmdList->DrawInstanced((UINT)vertex_count, (UINT)instance_count, (UINT)first_vertex, (UINT)first_instance);
}

void cgpu_render_encoder_draw_indexed_d3d12(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    Cmd->pDxCmdList->DrawIndexedInstanced((UINT)index_count, (UINT)1, (UINT)first_index, (UINT)first_vertex, (UINT)0);
}

void cgpu_render_encoder_draw_indexed_instanced_d3d12(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    Cmd->pDxCmdList->DrawIndexedInstanced((UINT)index_count, (UINT)instance_count, (UINT)first_index, (UINT)first_vertex, (UINT)first_instance);
}

void cgpu_cmd_end_render_pass_d3d12(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
#ifdef __ID3D12GraphicsCommandList4_FWD_DEFINED__
    ID3D12GraphicsCommandList4* CmdList4 = (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList;
    CmdList4->EndRenderPass();
    return;
#endif
    cgpu_info("ID3D12GraphicsCommandList4 is not defined!");
}

// SwapChain APIs
CGPUSwapChainId cgpu_create_swapchain_d3d12_impl(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc, CGPUSwapChain_D3D12* old)
{
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)device->adapter->instance;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)old;
    const uint32_t buffer_count = desc->imageCount;
    if (!old)
    {
        void* Memory = cgpu_calloc(1, sizeof(CGPUSwapChain_D3D12) +
                                    sizeof(CGPUTexture_D3D12) * buffer_count +
                                    sizeof(CGPUTextureId) * buffer_count);
        S = cgpu_new_placed<CGPUSwapChain_D3D12>(Memory);
    }
    S->mDxSyncInterval = desc->enable_vsync ? 1 : 0;
    DECLARE_ZERO(DXGI_SWAP_CHAIN_DESC1, chain_desc1)
    chain_desc1.Width = desc->width;
    chain_desc1.Height = desc->height;
    chain_desc1.Format = DXGIUtil_TranslatePixelFormat(desc->format);
    chain_desc1.Stereo = false;
    chain_desc1.SampleDesc.Count = 1; // If multisampling is needed, we'll resolve it later
    chain_desc1.SampleDesc.Quality = 0;
    chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    chain_desc1.BufferCount = desc->imageCount;
    chain_desc1.Scaling = DXGI_SCALING_STRETCH;
    chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // for better performance.
    chain_desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    chain_desc1.Flags = 0;
    BOOL allowTearing = FALSE;
    I->pDXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    chain_desc1.Flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    S->mFlags |= (!desc->enable_vsync && allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    IDXGISwapChain1* swapchain;
    HWND hwnd = (HWND)desc->surface;

    CGPUQueue_D3D12* Q = CGPU_NULLPTR;
    if (desc->present_queues == CGPU_NULLPTR)
    {
        Q = (CGPUQueue_D3D12*)cgpu_get_queue_d3d12(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    }
    else
    {
        Q = (CGPUQueue_D3D12*)desc->present_queues[0];
    }
    auto bCreated =
    SUCCEEDED(I->pDXGIFactory->CreateSwapChainForHwnd(Q->pCommandQueue, hwnd, &chain_desc1, NULL, NULL, &swapchain));
    (void)bCreated;
    cgpu_assert(bCreated && "Failed to Try to Create SwapChain! An existed swapchain might be destroyed!");

    auto bAssociation = SUCCEEDED(I->pDXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    (void)bAssociation;
    cgpu_assert(bAssociation && "Failed to Try to Associate SwapChain With Window!");

    auto bQueryChain3 = SUCCEEDED(swapchain->QueryInterface(IID_PPV_ARGS(&S->pDxSwapChain)));
    (void)bQueryChain3;
    cgpu_assert(bQueryChain3 && "Failed to Query IDXGISwapChain3 from Created SwapChain!");

    SAFE_RELEASE(swapchain);
    // Get swapchain images
    ID3D12Resource** backbuffers = (ID3D12Resource**)alloca(desc->imageCount * sizeof(ID3D12Resource*));
    for (uint32_t i = 0; i < desc->imageCount; ++i)
    {
        CHECK_HRESULT(S->pDxSwapChain->GetBuffer(i, IID_ARGS(&backbuffers[i])));
    }
    CGPUTexture_D3D12* Ts = (CGPUTexture_D3D12*)(S + 1);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Ts[i].pDxResource = backbuffers[i];
        Ts[i].pDxAllocation = nullptr;
        Ts[i].super.is_cube = false;
        Ts[i].super.array_size_minus_one = 0;
        Ts[i].super.device = &D->super;
        Ts[i].super.format = desc->format;
        Ts[i].super.aspect_mask = 1;
        Ts[i].super.depth = 1;
        Ts[i].super.width = desc->width;
        Ts[i].super.height = desc->height;
        Ts[i].super.mip_levels = 1;
        Ts[i].super.node_index = SINGLE_GPU_NODE_INDEX;
        Ts[i].super.owns_image = false;
    }
    CGPUTextureId* Vs = (CGPUTextureId*)(Ts + buffer_count);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Vs[i] = &Ts[i].super;
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
    return cgpu_create_swapchain_d3d12_impl(device, desc, nullptr);
}

uint32_t cgpu_acquire_next_image_d3d12(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc)
{
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)swapchain;
    // On PC AquireNext is always true
    HRESULT hr = S_OK;
    if (FAILED(hr))
    {
        cgpu_error("Failed to acquire next image");
        return UINT32_MAX;
    }
    return S->pDxSwapChain->GetCurrentBackBufferIndex();
}

void cgpu_free_swapchain_d3d12(CGPUSwapChainId swapchain)
{
    CGPUSwapChain_D3D12* S = (CGPUSwapChain_D3D12*)swapchain;
    cgpu_free_swapchain_d3d12_impl(swapchain);
    cgpu_delete_placed(S);
    cgpu_free(S);
}

#include "cgpu/extensions/cgpu_d3d12_exts.h"
// extentions
CGPUDREDSettingsId cgpu_d3d12_enable_DRED()
{
    CGPUDREDSettingsId settings = cgpu_new<CGPUDREDSettings>();
    SUCCEEDED(D3D12GetDebugInterface(__uuidof(settings->pDredSettings), (void**)&(settings->pDredSettings)));
    // Turn on auto-breadcrumbs and page fault reporting.
    settings->pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
    settings->pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
    return settings;
}

void cgpu_d3d12_disable_DRED(CGPUDREDSettingsId settings)
{
    settings->pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_OFF);
    settings->pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_OFF);
    SAFE_RELEASE(settings->pDredSettings);
    cgpu_delete(settings);
}