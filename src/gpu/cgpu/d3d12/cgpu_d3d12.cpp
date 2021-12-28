#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include "../common/common_utils.h"
#include <dxcapi.h>

#if !defined(XBOX)
    #pragma comment(lib, "d3d12.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "dxguid.lib")
    #pragma comment(lib, "dxcompiler.lib")
#endif

#include <comdef.h>

// Call this only once.

CGpuInstanceId cgpu_create_instance_d3d12(CGpuInstanceDescriptor const* descriptor)
{
    CGpuInstance_D3D12* result = cgpu_new<CGpuInstance_D3D12>();
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

void cgpu_query_instance_features_d3d12(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
    CGpuInstance_D3D12* I = (CGpuInstance_D3D12*)instance;
    (void)I;
    features->specialization_constant = false;
}

void cgpu_free_instance_d3d12(CGpuInstanceId instance)
{
    CGpuInstance_D3D12* to_destroy = (CGpuInstance_D3D12*)instance;
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
    cgpu_delete(to_destroy);
}

void cgpu_enum_adapters_d3d12(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    cgpu_assert(instance != nullptr && "fatal: null instance!");
    CGpuInstance_D3D12* I = (CGpuInstance_D3D12*)instance;
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

const CGpuAdapterDetail* cgpu_query_adapter_detail_d3d12(const CGpuAdapterId adapter)
{
    const CGpuAdapter_D3D12* A = (CGpuAdapter_D3D12*)adapter;
    return &A->adapter_detail;
}

uint32_t cgpu_query_queue_count_d3d12(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    // queues are virtual in d3d12.
    /*
    switch(type)
    {
        case QUEUE_TYPE_GRAPHICS: return 1;
        case QUEUE_TYPE_COMPUTE: return 2;
        case QUEUE_TYPE_TRANSFER: return 2;
        default: return 0;
    }
    */
    return UINT32_MAX;
}

CGpuDeviceId cgpu_create_device_d3d12(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    CGpuAdapter_D3D12* A = (CGpuAdapter_D3D12*)adapter;
    CGpuInstance_D3D12* I = (CGpuInstance_D3D12*)A->super.instance;
    CGpuDevice_D3D12* D = cgpu_new<CGpuDevice_D3D12>();
    *const_cast<CGpuAdapterId*>(&D->super.adapter) = adapter;

    if (!SUCCEEDED(D3D12CreateDevice(A->pDxActiveGPU, // default adapter
            A->mFeatureLevel, IID_PPV_ARGS(&D->pDxDevice))))
    {
        cgpu_assert("[D3D12 Fatal]: Create D3D12Device Failed!");
    }

    // Create Requested Queues.
    for (uint32_t i = 0u; i < desc->queueGroupCount; i++)
    {
        const auto& queueGroup = desc->queueGroups[i];
        const auto type = queueGroup.queueType;

        *const_cast<uint32_t*>(&D->pCommandQueueCounts[type]) = queueGroup.queueCount;
        *const_cast<ID3D12CommandQueue***>(&D->ppCommandQueues[type]) =
            (ID3D12CommandQueue**)cgpu_malloc(sizeof(ID3D12CommandQueue*) * queueGroup.queueCount);

        for (uint32_t j = 0u; j < queueGroup.queueCount; j++)
        {
            DECLARE_ZERO(D3D12_COMMAND_QUEUE_DESC, queueDesc)
            switch (type)
            {
                case QUEUE_TYPE_GRAPHICS:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    break;
                case QUEUE_TYPE_COMPUTE:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
                case QUEUE_TYPE_TRANSFER:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;
                default:
                    cgpu_assert(0 && "[D3D12 Fatal]: Unsupported ECGpuQueueType!");
                    return CGPU_NULLPTR;
            }
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            if (!SUCCEEDED(D->pDxDevice->CreateCommandQueue(
                    &queueDesc, IID_PPV_ARGS(&D->ppCommandQueues[type][j]))))
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
    return &D->super;
}

void cgpu_free_device_d3d12(CGpuDeviceId device)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    for (uint32_t t = 0u; t < QUEUE_TYPE_COUNT; t++)
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
    cgpu_delete(D);
}

// API Objects APIs
CGpuFenceId cgpu_create_fence_d3d12(CGpuDeviceId device)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    // create a Fence and cgpu_assert that it is valid
    CGpuFence_D3D12* F = cgpu_new<CGpuFence_D3D12>();
    cgpu_assert(F);

    CHECK_HRESULT(D->pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ARGS(&F->pDxFence)));
    F->mFenceValue = 1;

    F->pDxWaitIdleFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    return &F->super;
}

void cgpu_wait_fences_d3d12(const CGpuFenceId* fences, uint32_t fence_count)
{
    const CGpuFence_D3D12** Fences = (const CGpuFence_D3D12**)fences;
    for (uint32_t i = 0; i < fence_count; ++i)
    {
        ECGpuFenceStatus fenceStatus = cgpu_query_fence_status(fences[i]);
        uint64_t fenceValue = Fences[i]->mFenceValue - 1;
        if (fenceStatus == FENCE_STATUS_INCOMPLETE)
        {
            Fences[i]->pDxFence->SetEventOnCompletion(
                fenceValue, Fences[i]->pDxWaitIdleFenceEvent);
            WaitForSingleObject(Fences[i]->pDxWaitIdleFenceEvent, INFINITE);
        }
    }
}

ECGpuFenceStatus cgpu_query_fence_status_d3d12(CGpuFenceId fence)
{
    ECGpuFenceStatus status = FENCE_STATUS_COMPLETE;
    CGpuFence_D3D12* F = (CGpuFence_D3D12*)fence;
    if (F->pDxFence->GetCompletedValue() < F->mFenceValue - 1)
        status = FENCE_STATUS_INCOMPLETE;
    else
        status = FENCE_STATUS_COMPLETE;
    return status;
}

void cgpu_free_fence_d3d12(CGpuFenceId fence)
{
    CGpuFence_D3D12* F = (CGpuFence_D3D12*)fence;
    SAFE_RELEASE(F->pDxFence);
    CloseHandle(F->pDxWaitIdleFenceEvent);
    cgpu_delete(F);
}

CGpuRootSignatureId cgpu_create_root_signature_d3d12(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuRootSignature_D3D12* RS = cgpu_new<CGpuRootSignature_D3D12>();
    // Pick root parameters from desc data
    CGpuShaderStages shaderStages = 0;
    for (uint32_t i = 0; i < desc->shaders_count; i++)
    {
        CGpuPipelineShaderDescriptor* shader_desc = desc->shaders + i;
        shaderStages |= shader_desc->stage;
    }
    // Pick shader reflection data
    DECLARE_ZERO(CGpuUtil_RSBlackboard, bb)
    CGpuUtil_InitRSBlackboardAndParamTables((CGpuRootSignature*)RS, &bb, desc);
    // Fill resource slots
    // Only support descriptor tables now
    // TODO: Support root CBVs
    //       Add backend sort for better performance
    const UINT tableCount = bb.set_count;
    UINT descRangeCount = 0;
    for (uint32_t i = 0; i < bb.set_count; i++)
    {
        descRangeCount += bb.valid_bindings[i];
    }
    D3D12_ROOT_PARAMETER1* rootParams = (D3D12_ROOT_PARAMETER1*)cgpu_calloc(tableCount, sizeof(D3D12_ROOT_PARAMETER1));
    D3D12_DESCRIPTOR_RANGE1* cbvSrvUavRanges = (D3D12_DESCRIPTOR_RANGE1*)cgpu_calloc(descRangeCount, sizeof(D3D12_DESCRIPTOR_RANGE1));
    uint32_t i_table = 0;
    uint32_t i_range = 0;
    // Create descriptor tables
    for (uint32_t i_set = 0; i_set < bb.set_count; i_set++)
    {
        D3D12_ROOT_PARAMETER1* rootParam = &rootParams[i_table];
        rootParam->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        CGpuShaderStages visStages = SHADER_STAGE_NONE;
        const D3D12_DESCRIPTOR_RANGE1* descRangeCursor = &cbvSrvUavRanges[i_range];
        for (uint32_t i_binding = 0; i_binding < bb.valid_bindings[i_set]; i_binding++)
        {
            D3D12_DESCRIPTOR_RANGE1* descRange = &cbvSrvUavRanges[i_range];
            CGpuShaderResource* reflSlot = &bb.sig_reflections[i_set * i_binding + i_binding];
            descRange->RegisterSpace = reflSlot->set;
            descRange->BaseShaderRegister = reflSlot->binding;
            descRange->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
            descRange->NumDescriptors = reflSlot->size;
            descRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            descRange->RangeType = D3D12Util_ResourceTypeToDescriptorRangeType(reflSlot->type);
            visStages |= reflSlot->stages;
            i_range++;
        }
        rootParam->ShaderVisibility = D3D12Util_TranslateShaderStages(visStages);
        rootParam->DescriptorTable.NumDescriptorRanges = bb.valid_bindings[i_set];
        rootParam->DescriptorTable.pDescriptorRanges = descRangeCursor;
        i_table++;
    }
    // End create descriptor tables
    // TODO: Support static samplers
    UINT staticSamplerCount = 0;
    D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs = CGPU_NULLPTR;
    bool useInputLayout = shaderStages & SHADER_STAGE_VERT; // VertexStage uses input layout
    // Fill RS flags
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    if (useInputLayout)
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    if (!(shaderStages & SHADER_STAGE_VERT))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    if (!(shaderStages & SHADER_STAGE_ALL_HULL))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if (!(shaderStages & SHADER_STAGE_ALL_DOMAIN))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if (!(shaderStages & SHADER_STAGE_GEOM))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    if (!(shaderStages & SHADER_STAGE_FRAG))
        rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    // Serialize versioned RS
    const UINT paramCount = tableCount - staticSamplerCount;
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
    CGpuUtil_FreeRSBlackboard(&bb);
    cgpu_free(rootParams);
    cgpu_free(cbvSrvUavRanges);
    return &RS->super;
}

void cgpu_free_root_signature_d3d12(CGpuRootSignatureId signature)
{
    CGpuRootSignature_D3D12* RS = (CGpuRootSignature_D3D12*)signature;
    SAFE_RELEASE(RS->pDxRootSignature);
    cgpu_delete(RS);
}

CGpuDescriptorSetId cgpu_create_descriptor_set_d3d12(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    const CGpuRootSignature_D3D12* RS = (const CGpuRootSignature_D3D12*)desc->root_signature;
    CGpuDescriptorSet_D3D12* Set = cgpu_new<CGpuDescriptorSet_D3D12>();
    const uint32_t nodeIndex = SINGLE_GPU_NODE_INDEX;
    struct D3D12Util_DescriptorHeap* pCbvSrvUavHeap = D->pCbvSrvUavHeaps[nodeIndex];
    struct D3D12Util_DescriptorHeap* pSamplerHeap = D->pSamplerHeaps[nodeIndex];
    (void)pSamplerHeap;
    CGpuParameterTable* param_table = &RS->super.tables[desc->set_index];
    // CBV/SRV/UAV
    auto StartHandle = D3D12Util_ConsumeDescriptorHandles(pCbvSrvUavHeap, param_table->resources_count);
    Set->mCbvSrvUavHandle = StartHandle.mGpu.ptr - pCbvSrvUavHeap->mStartHandle.mGpu.ptr;
    Set->mCbvSrvUavStride = param_table->resources_count * pCbvSrvUavHeap->mDescriptorSize;
    // TODO: Static samplers
    // TODO: Support root descriptors
    return &Set->super;
}

void cgpu_update_descriptor_set_d3d12(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count)
{
    CGpuDescriptorSet_D3D12* Set = (CGpuDescriptorSet_D3D12*)set;
    const CGpuRootSignature_D3D12* RS = (const CGpuRootSignature_D3D12*)set->root_signature;
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)set->root_signature->device;
    CGpuParameterTable* ParamTable = &RS->super.tables[set->index];
    const uint32_t nodeIndex = SINGLE_GPU_NODE_INDEX;
    struct D3D12Util_DescriptorHeap* pCbvSrvUavHeap = D->pCbvSrvUavHeaps[nodeIndex];
    for (uint32_t i = 0; i < count; i++)
    {
        // Descriptor Info
        const CGpuDescriptorData* ArgData = datas + i;
        CGpuShaderResource* ResData = CGPU_NULLPTR;
        size_t argNameHash = cgpu_hash(ArgData->name, strlen(ArgData->name), *(size_t*)&D);
        for (uint32_t p = 0; p < ParamTable->resources_count; p++)
        {
            if (ParamTable->resources[p].name_hash == argNameHash)
            {
                ResData = ParamTable->resources + p;
            }
        }
        // Update Info
        const CGpuDescriptorData* pParam = datas + i;
        const uint32_t arrayCount = cgpu_max(1U, pParam->count);
        switch (ResData->type)
        {
            case RT_BUFFER:
            case RT_BUFFER_RAW: {
            }
            case RT_RW_BUFFER:
            case RT_RW_BUFFER_RAW: {
                cgpu_assert(ArgData->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                CGpuBuffer_D3D12** Buffers = (CGpuBuffer_D3D12**)ArgData->buffers;
                for (uint32_t arr = 0; arr < arrayCount; ++arr)
                {
                    cgpu_assert(ArgData->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    D3D12Util_CopyDescriptorHandle(pCbvSrvUavHeap,
                        { Buffers[arr]->mDxDescriptorHandles.ptr + Buffers[arr]->mDxUavOffset },
                        Set->mCbvSrvUavHandle, arr);
                }
                break;
            }
            default:
                break;
        }
    }
}

void cgpu_free_descriptor_set_d3d12(CGpuDescriptorSetId set)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)set->root_signature->device;
    CGpuDescriptorSet_D3D12* Set = (CGpuDescriptorSet_D3D12*)set;
    (void)D; // TODO: recycle of descriptor set heap handles
    cgpu_delete(Set);
}

CGpuComputePipelineId cgpu_create_compute_pipeline_d3d12(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuComputePipeline_D3D12* PPL = cgpu_new<CGpuComputePipeline_D3D12>();
    CGpuRootSignature_D3D12* RS = (CGpuRootSignature_D3D12*)desc->root_signature;
    CGpuShaderLibrary_D3D12* SL = (CGpuShaderLibrary_D3D12*)desc->compute_shader->library;
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
    // TODO: Pipeline cache
    HRESULT result = E_FAIL;
    ID3D12PipelineLibrary* psoCache = CGPU_NULLPTR;
    (void)psoCache;
    if (!SUCCEEDED(result))
    {
        // TODO: Support PSO extensions
        CHECK_HRESULT(D->pDxDevice->CreateComputePipelineState(&pipeline_state_desc, IID_PPV_ARGS(&PPL->pDxPipelineState)));
    }
    return &PPL->super;
}

void cgpu_free_compute_pipeline_d3d12(CGpuComputePipelineId pipeline)
{
    CGpuComputePipeline_D3D12* PPL = (CGpuComputePipeline_D3D12*)pipeline;
    SAFE_RELEASE(PPL->pDxPipelineState);
    cgpu_delete(PPL);
}

D3D12_DEPTH_STENCIL_DESC gDefaultDepthDesc = {};
D3D12_BLEND_DESC gDefaultBlendDesc = {};
D3D12_RASTERIZER_DESC gDefaultRasterizerDesc = {};
CGpuRenderPipelineId cgpu_create_render_pipeline_d3d12(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuRenderPipeline_D3D12* PPL = cgpu_new<CGpuRenderPipeline_D3D12>();
    CGpuRootSignature_D3D12* RS = (CGpuRootSignature_D3D12*)desc->root_signature;
    // TODO: Vertex input state
    uint32_t input_elementCount = 0;
    DECLARE_ZERO(D3D12_INPUT_ELEMENT_DESC, input_elements[MAX_VERTEX_ATTRIBS]);
    if (desc->vertex_layout != nullptr)
    {
        for (uint32_t attrib_index = 0; attrib_index < desc->vertex_layout->attribute_count; ++attrib_index)
        {
            const CGpuVertexAttribute* attrib = &(desc->vertex_layout->attributes[attrib_index]);
            (void)attrib;
        }
    }
    DECLARE_ZERO(D3D12_INPUT_LAYOUT_DESC, input_layout_desc);
    input_layout_desc.pInputElementDescs = input_elementCount ? input_elements : NULL;
    input_layout_desc.NumElements = input_elementCount;
    // Shader stages
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, VS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, PS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, DS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, HS);
    DECLARE_ZERO(D3D12_SHADER_BYTECODE, GS);
    for (uint32_t i = 0; i < 5; ++i)
    {
        ECGpuShaderStage stage_mask = (ECGpuShaderStage)(1 << i);
        switch (stage_mask)
        {
            case SHADER_STAGE_VERT: {
                if (desc->vertex_shader)
                {
                    CGpuShaderLibrary_D3D12* VertLib = (CGpuShaderLibrary_D3D12*)desc->vertex_shader->library;
                    VS.BytecodeLength = VertLib->pShaderBlob->GetBufferSize();
                    VS.pShaderBytecode = VertLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case SHADER_STAGE_TESC: {
                if (desc->tesc_shader)
                {
                    CGpuShaderLibrary_D3D12* TescLib = (CGpuShaderLibrary_D3D12*)desc->tesc_shader->library;
                    HS.BytecodeLength = TescLib->pShaderBlob->GetBufferSize();
                    HS.pShaderBytecode = TescLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case SHADER_STAGE_TESE: {
                if (desc->tese_shader)
                {
                    CGpuShaderLibrary_D3D12* TeseLib = (CGpuShaderLibrary_D3D12*)desc->tese_shader->library;
                    DS.BytecodeLength = TeseLib->pShaderBlob->GetBufferSize();
                    DS.pShaderBytecode = TeseLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case SHADER_STAGE_GEOM: {
                if (desc->geom_shader)
                {
                    CGpuShaderLibrary_D3D12* GeomLib = (CGpuShaderLibrary_D3D12*)desc->geom_shader->library;
                    GS.BytecodeLength = GeomLib->pShaderBlob->GetBufferSize();
                    GS.pShaderBytecode = GeomLib->pShaderBlob->GetBufferPointer();
                }
            }
            break;
            case SHADER_STAGE_FRAG: {
                if (desc->fragment_shader)
                {
                    CGpuShaderLibrary_D3D12* FragLib = (CGpuShaderLibrary_D3D12*)desc->fragment_shader->library;
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
    // Depth stencil
    DECLARE_ZERO(D3D12_DEPTH_STENCILOP_DESC, depth_stencilop_desc);
    depth_stencilop_desc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencilop_desc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencilop_desc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depth_stencilop_desc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    // Sample
    DECLARE_ZERO(DXGI_SAMPLE_DESC, sample_desc);
    sample_desc.Count = (UINT)(desc->sample_count);
    sample_desc.Quality = (UINT)(desc->sample_quality);
    // TODO: Pipeline cache
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
    pipeline_state_desc.DepthStencilState =
        desc->depth_state ? D3D12Util_TranslateDephStencilState(desc->depth_state) : gDefaultDepthDesc;
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
    // TODO: Pipeline cache
    if (!SUCCEEDED(result))
    {
        CHECK_HRESULT(D->pDxDevice->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&PPL->pDxPipelineState)));
    }
    D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    switch (desc->prim_topology)
    {
        case PRIM_TOPO_POINT_LIST:
            topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case PRIM_TOPO_LINE_LIST:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case PRIM_TOPO_LINE_STRIP:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case PRIM_TOPO_TRI_LIST:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case PRIM_TOPO_TRI_STRIP:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        case PRIM_TOPO_PATCH_LIST: {
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

void cgpu_free_render_pipeline_d3d12(CGpuRenderPipelineId pipeline)
{
    CGpuRenderPipeline_D3D12* PPL = (CGpuRenderPipeline_D3D12*)pipeline;
    SAFE_RELEASE(PPL->pDxPipelineState);
    cgpu_delete(PPL);
}

// Queue APIs
CGpuQueueId cgpu_get_queue_d3d12(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuQueue_D3D12* Q = cgpu_new<CGpuQueue_D3D12>();
    Q->pCommandQueue = D->ppCommandQueues[type][index];
    Q->pFence = (CGpuFence_D3D12*)cgpu_create_fence_d3d12(device);
    return &Q->super;
}

void cgpu_submit_queue_d3d12(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc)
{
    // TODO: Add Necessary Semaphores
    uint32_t CmdCount = desc->cmds_count;
    CGpuCommandBuffer_D3D12** Cmds = (CGpuCommandBuffer_D3D12**)desc->cmds;
    CGpuQueue_D3D12* Q = (CGpuQueue_D3D12*)queue;
    CGpuFence_D3D12* F = (CGpuFence_D3D12*)desc->signal_fence;

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
    Q->pCommandQueue->ExecuteCommandLists(CmdCount, cmds);

    if (F)
        D3D12Util_SignalFence(Q, F->pDxFence, F->mFenceValue++);
}

void cgpu_wait_queue_idle_d3d12(CGpuQueueId queue)
{
    CGpuQueue_D3D12* Q = (CGpuQueue_D3D12*)queue;
    D3D12Util_SignalFence(Q, Q->pFence->pDxFence, Q->pFence->mFenceValue++);

    uint64_t fenceValue = Q->pFence->mFenceValue - 1;
    if (Q->pFence->pDxFence->GetCompletedValue() < Q->pFence->mFenceValue - 1)
    {
        Q->pFence->pDxFence->SetEventOnCompletion(fenceValue, Q->pFence->pDxWaitIdleFenceEvent);
        WaitForSingleObject(Q->pFence->pDxWaitIdleFenceEvent, INFINITE);
    }
}

void cgpu_free_queue_d3d12(CGpuQueueId queue)
{
    CGpuQueue_D3D12* Q = (CGpuQueue_D3D12*)queue;
    cgpu_assert(queue && "D3D12 ERROR: FREE NULL QUEUE!");
    cgpu_free_fence_d3d12(&Q->pFence->super);
    cgpu_delete(Q);
}

// Command Objects

// allocate_transient_command_allocator
ID3D12CommandAllocator* allocate_transient_command_allocator(CGpuCommandPool_D3D12* E, CGpuQueueId queue)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)queue->device;

    D3D12_COMMAND_LIST_TYPE type =
        queue->type == QUEUE_TYPE_TRANSFER ?
            D3D12_COMMAND_LIST_TYPE_COPY :
        queue->type == QUEUE_TYPE_COMPUTE ?
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

CGpuCommandPoolId cgpu_create_command_pool_d3d12(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc)
{
    CGpuCommandPool_D3D12* P = cgpu_new<CGpuCommandPool_D3D12>();
    P->pDxCmdAlloc = allocate_transient_command_allocator(P, queue);
    return &P->super;
}

CGpuCommandBufferId cgpu_create_command_buffer_d3d12(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc)
{
    // initialize to zero
    CGpuCommandBuffer_D3D12* Cmd = cgpu_new<CGpuCommandBuffer_D3D12>();
    CGpuCommandPool_D3D12* P = (CGpuCommandPool_D3D12*)pool;
    CGpuQueue_D3D12* Q = (CGpuQueue_D3D12*)P->super.queue;
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)Q->super.device;
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

void cgpu_free_command_buffer_d3d12(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)cmd;
    SAFE_RELEASE(Cmd->pDxCmdList);
    cgpu_delete(Cmd);
}

void cgpu_free_command_pool_d3d12(CGpuCommandPoolId pool)
{
    CGpuCommandPool_D3D12* P = (CGpuCommandPool_D3D12*)pool;
    cgpu_assert(pool && "D3D12 ERROR: FREE NULL COMMAND POOL!");
    cgpu_assert(P->pDxCmdAlloc && "D3D12 ERROR: FREE NULL pDxCmdAlloc!");

    free_transient_command_allocator(P->pDxCmdAlloc);
    cgpu_delete(P);
}

// CMDs
void cgpu_cmd_begin_d3d12(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)cmd;
    CGpuCommandPool_D3D12* P = (CGpuCommandPool_D3D12*)Cmd->pCmdPool;
    CHECK_HRESULT(Cmd->pDxCmdList->Reset(P->pDxCmdAlloc, NULL));

    if (Cmd->mType != QUEUE_TYPE_TRANSFER)
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
void cgpu_cmd_resource_barrier_d3d12(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)cmd;
    const uint32_t barriers_count = desc->buffer_barriers_count;
    D3D12_RESOURCE_BARRIER* barriers = (D3D12_RESOURCE_BARRIER*)alloca(barriers_count * sizeof(D3D12_RESOURCE_BARRIER));
    uint32_t transitionCount = 0;
    for (uint32_t i = 0; i < desc->buffer_barriers_count; ++i)
    {
        const CGpuBufferBarrier* pTransBarrier = &desc->buffer_barriers[i];
        D3D12_RESOURCE_BARRIER* pBarrier = &barriers[transitionCount];
        CGpuBuffer_D3D12* pBuffer = (CGpuBuffer_D3D12*)pTransBarrier->buffer;
        if (pBuffer->super.memory_usage == MEM_USAGE_GPU_ONLY ||
            pBuffer->super.memory_usage == MEM_USAGE_GPU_TO_CPU ||
            (pBuffer->super.memory_usage == MEM_USAGE_CPU_TO_GPU && (pBuffer->super.descriptors & RT_RW_BUFFER)))
        {
            if (RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->src_state &&
                RESOURCE_STATE_UNORDERED_ACCESS == pTransBarrier->dst_state)
            {
                pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                pBarrier->UAV.pResource = pBuffer->pDxResource;
                ++transitionCount;
            }
            else
            {
                pBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                // TODO: BeginOnly/EndOnly
                pBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                pBarrier->Transition.pResource = pBuffer->pDxResource;
                pBarrier->Transition.Subresource =
                    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                pBarrier->Transition.StateBefore = D3D12Util_TranslateResourceState(pTransBarrier->src_state);
                pBarrier->Transition.StateAfter = D3D12Util_TranslateResourceState(pTransBarrier->dst_state);
                ++transitionCount;
            }
        }
    }
    if (transitionCount)
        Cmd->pDxCmdList->ResourceBarrier(transitionCount, barriers);
}

void cgpu_cmd_end_d3d12(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)cmd;
    cgpu_assert(Cmd->pDxCmdList);

    CHECK_HRESULT(Cmd->pDxCmdList->Close());
}

// Compute CMDs
CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass_d3d12(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc)
{
    // DO NOTHING NOW
    return (CGpuComputePassEncoderId)cmd;
}

void cgpu_compute_encoder_bind_descriptor_set_d3d12(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId set)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)encoder;
    const CGpuDescriptorSet_D3D12* Set = (CGpuDescriptorSet_D3D12*)set;
    Cmd->pDxCmdList->SetComputeRootDescriptorTable(set->index, { Cmd->mBoundHeapStartHandles[0].ptr + Set->mCbvSrvUavHandle });
}

bool reset_root_signature(CGpuCommandBuffer_D3D12* pCmd, ECGpuPipelineType type,
    ID3D12RootSignature* pRootSignature)
{
    // Set root signature if the current one differs from pRootSignature
    if (pCmd->pBoundRootSignature != pRootSignature)
    {
        pCmd->pBoundRootSignature = pRootSignature;
        if (type == PIPELINE_TYPE_GRAPHICS)
            pCmd->pDxCmdList->SetGraphicsRootSignature(pRootSignature);
        else
            pCmd->pDxCmdList->SetComputeRootSignature(pRootSignature);
    }
    return false;
}

void cgpu_compute_encoder_bind_pipeline_d3d12(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)encoder;
    CGpuComputePipeline_D3D12* PPL = (CGpuComputePipeline_D3D12*)pipeline;
    reset_root_signature(Cmd, PIPELINE_TYPE_COMPUTE, PPL->pRootSignature);
    Cmd->pDxCmdList->SetPipelineState(PPL->pDxPipelineState);
}

void cgpu_compute_encoder_dispatch_d3d12(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)encoder;
    Cmd->pDxCmdList->Dispatch(X, Y, Z);
}

void cgpu_cmd_end_compute_pass_d3d12(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder)
{
    // DO NOTHING NOW
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain_d3d12(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    CGpuInstance_D3D12* I = (CGpuInstance_D3D12*)device->adapter->instance;
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    const uint32_t buffer_count = desc->imageCount;
    void* Memory = cgpu_calloc(1, sizeof(CGpuSwapChain_D3D12) +
                                      sizeof(CGpuTexture_D3D12) * buffer_count +
                                      sizeof(CGpuTextureId) * buffer_count);
    CGpuSwapChain_D3D12* S = cgpu_new_placed<CGpuSwapChain_D3D12>(Memory);

    S->mDxSyncInterval = desc->enableVsync ? 1 : 0;
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
    S->mFlags |= (!desc->enableVsync && allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    IDXGISwapChain1* swapchain;
    HWND hwnd = (HWND)desc->surface;

    CGpuQueue_D3D12* Q = CGPU_NULLPTR;
    if (desc->presentQueues == CGPU_NULLPTR)
    {
        Q = (CGpuQueue_D3D12*)cgpu_get_queue_d3d12(device, QUEUE_TYPE_GRAPHICS, 0);
    }
    else
    {
        Q = (CGpuQueue_D3D12*)desc->presentQueues[0];
    }
    auto bCreated =
        SUCCEEDED(I->pDXGIFactory->CreateSwapChainForHwnd(Q->pCommandQueue, hwnd, &chain_desc1, NULL, NULL, &swapchain));
    (void)bCreated;
    cgpu_assert(bCreated && "Failed to Try to Create SwapChain!");

    auto bAssociation = SUCCEEDED(I->pDXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    (void)bAssociation;
    cgpu_assert(bAssociation && "Failed to Try to Associate SwapChain With Window!");

    auto bQueryChain3 = SUCCEEDED(swapchain->QueryInterface(IID_PPV_ARGS(&S->pDxSwapChain)));
    (void)bQueryChain3;
    cgpu_assert(bQueryChain3 && "Failed to Query IDXGISwapChain3 from Created SwapChain!");

    SAFE_RELEASE(swapchain);
    // Get swapchain images
    ID3D12Resource** backbuffers =
        (ID3D12Resource**)alloca(desc->imageCount * sizeof(ID3D12Resource*));
    for (uint32_t i = 0; i < desc->imageCount; ++i)
    {
        CHECK_HRESULT(S->pDxSwapChain->GetBuffer(i, IID_ARGS(&backbuffers[i])));
    }
    CGpuTexture_D3D12* Ts = (CGpuTexture_D3D12*)(S + 1);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Ts[i].pDxResource = backbuffers[i];
        Ts[i].pDxAllocation = nullptr;
        Ts[i].super.is_cube = false;
        Ts[i].super.array_size_minus_one = 0;
        Ts[i].super.device = &D->super;
        Ts[i].super.format = desc->format;
        Ts[i].super.aspect_mask = 1; // TODO: aspect mask
        Ts[i].super.depth = 1;
        Ts[i].super.width = desc->width;
        Ts[i].super.height = desc->height;
        Ts[i].super.mip_levels = 1;
        Ts[i].super.node_index = SINGLE_GPU_NODE_INDEX;
        Ts[i].super.owns_image = false;
    }
    CGpuTextureId* Vs = (CGpuTextureId*)(Ts + buffer_count);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Vs[i] = &Ts[i].super;
    }
    S->super.back_buffers = Vs;
    S->super.buffer_count = buffer_count;
    return &S->super;
}

void cgpu_free_swapchain_d3d12(CGpuSwapChainId swapchain)
{
    CGpuSwapChain_D3D12* S = (CGpuSwapChain_D3D12*)swapchain;
    SAFE_RELEASE(S->pDxSwapChain);
    cgpu_delete_placed(S);
    cgpu_free(S);
}

#include "cgpu/extensions/cgpu_d3d12_exts.h"
// extentions
CGpuDREDSettingsId cgpu_d3d12_enable_DRED()
{
    CGpuDREDSettingsId settings = cgpu_new<CGpuDREDSettings>();
    SUCCEEDED(D3D12GetDebugInterface(__uuidof(settings->pDredSettings), (void**)&(settings->pDredSettings)));
    // Turn on auto-breadcrumbs and page fault reporting.
    settings->pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
    settings->pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
    return settings;
}

void cgpu_d3d12_disable_DRED(CGpuDREDSettingsId settings)
{
    settings->pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_OFF);
    settings->pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_OFF);
    SAFE_RELEASE(settings->pDredSettings);
    cgpu_delete(settings);
}