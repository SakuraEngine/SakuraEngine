#include "d3d12_utils.h"
#include <assert.h>
#include <stdio.h>

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
    CGpuInstance_D3D12* result = new CGpuInstance_D3D12();
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
            assert(0 && "The only available GPU has DXGI_ADAPTER_FLAG_SOFTWARE. Early exiting");
            return CGPU_NULLPTR;
        }
    }
    else
    {
        assert("[D3D12 Fatal]: Create DXGIFactory2 Failed!");
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
    delete to_destroy;
}

void cgpu_enum_adapters_d3d12(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    assert(instance != nullptr && "fatal: null instance!");
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
        case ECGpuQueueType_Graphics: return 1;
        case ECGpuQueueType_Compute: return 2;
        case ECGpuQueueType_Transfer: return 2;
        default: return 0;
    }
    */
    return UINT32_MAX;
}

CGpuDeviceId cgpu_create_device_d3d12(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    CGpuAdapter_D3D12* A = (CGpuAdapter_D3D12*)adapter;
    CGpuInstance_D3D12* I = (CGpuInstance_D3D12*)A->super.instance;
    CGpuDevice_D3D12* D = new CGpuDevice_D3D12();
    *const_cast<CGpuAdapterId*>(&D->super.adapter) = adapter;

    if (!SUCCEEDED(D3D12CreateDevice(A->pDxActiveGPU, // default adapter
            A->mFeatureLevel, IID_PPV_ARGS(&D->pDxDevice))))
    {
        assert("[D3D12 Fatal]: Create D3D12Device Failed!");
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
            switch (queueType)
            {
                case ECGpuQueueType_Graphics:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    break;
                case ECGpuQueueType_Compute:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
                case ECGpuQueueType_Transfer:
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;
                default:
                    assert(0 && "[D3D12 Fatal]: Unsupported ECGpuQueueType!");
                    return CGPU_NULLPTR;
            }
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            if (!SUCCEEDED(D->pDxDevice->CreateCommandQueue(
                    &queueDesc, IID_PPV_ARGS(&D->ppCommandQueues[type][j]))))
            {
                assert("[D3D12 Fatal]: Create D3D12CommandQueue Failed!");
            }
        }
    }
    // Create D3D12MA Allocator
    D3D12Util_CreateDMAAllocator(I, A, D);
    assert(D->pResourceAllocator && "DMA Allocator Must be Created!");
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
    for (uint32_t t = 0u; t < ECGpuQueueType_Count; t++)
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
}

// API Objects APIs
CGpuFenceId cgpu_create_fence_d3d12(CGpuDeviceId device)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    // create a Fence and ASSERT that it is valid
    CGpuFence_D3D12* F = new CGpuFence_D3D12();
    assert(F);

    CHECK_HRESULT(D->pDxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ARGS(&F->pDxFence)));
    F->mFenceValue = 1;

    F->pDxWaitIdleFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    return &F->super;
}

void cgpu_free_fence_d3d12(CGpuFenceId fence)
{
    CGpuFence_D3D12* F = (CGpuFence_D3D12*)fence;
    SAFE_RELEASE(F->pDxFence);
    CloseHandle(F->pDxWaitIdleFenceEvent);
    delete F;
}

// Queue APIs
CGpuQueueId cgpu_get_queue_d3d12(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuQueue_D3D12* Q = new CGpuQueue_D3D12();
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

    // ASSERT that given cmd list and given params are valid
    assert(CmdCount > 0);
    assert(Cmds);
    // execute given command list
    assert(Q->pCommandQueue);

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
    assert(queue && "D3D12 ERROR: FREE NULL QUEUE!");
    cgpu_free_fence_d3d12(&Q->pFence->super);
    delete Q;
}

// Command Objects

// allocate_transient_command_allocator
ID3D12CommandAllocator* allocate_transient_command_allocator(CGpuCommandPool_D3D12* E, CGpuQueueId queue)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)queue->device;

    D3D12_COMMAND_LIST_TYPE type =
        queue->type == ECGpuQueueType_Transfer ?
            D3D12_COMMAND_LIST_TYPE_COPY :
        queue->type == ECGpuQueueType_Compute ?
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
    CGpuCommandPool_D3D12* P = new CGpuCommandPool_D3D12();
    P->pDxCmdAlloc = allocate_transient_command_allocator(P, queue);
    return &P->super;
}

CGpuCommandBufferId cgpu_create_command_buffer_d3d12(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc)
{
    // initialize to zero
    CGpuCommandBuffer_D3D12* Cmd = new CGpuCommandBuffer_D3D12();
    CGpuCommandPool_D3D12* P = (CGpuCommandPool_D3D12*)pool;
    CGpuQueue_D3D12* Q = (CGpuQueue_D3D12*)P->super.queue;
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)Q->super.device;
    assert(Cmd);

    // set command pool of new command
    Cmd->mNodeIndex = SINGLE_GPU_NODE_MASK;
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
    delete Cmd;
}

void cgpu_free_command_pool_d3d12(CGpuCommandPoolId pool)
{
    CGpuCommandPool_D3D12* P = (CGpuCommandPool_D3D12*)pool;
    assert(pool && "D3D12 ERROR: FREE NULL COMMAND POOL!");
    assert(P->pDxCmdAlloc && "D3D12 ERROR: FREE NULL pDxCmdAlloc!");

    free_transient_command_allocator(P->pDxCmdAlloc);
    delete P;
}

// CMDs
void cgpu_cmd_begin_d3d12(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)cmd;
    CGpuCommandPool_D3D12* P = (CGpuCommandPool_D3D12*)Cmd->pCmdPool;
    CHECK_HRESULT(Cmd->pDxCmdList->Reset(P->pDxCmdAlloc, NULL));

    if (Cmd->mType != ECGpuQueueType_Transfer)
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

void cgpu_cmd_end_d3d12(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_D3D12* Cmd = (CGpuCommandBuffer_D3D12*)cmd;
    assert(Cmd->pDxCmdList);

    CHECK_HRESULT(Cmd->pDxCmdList->Close());
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain_d3d12(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    CGpuInstance_D3D12* I = (CGpuInstance_D3D12*)device->adapter->instance;
    // CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuSwapChain_D3D12* S = new CGpuSwapChain_D3D12();

    S->mDxSyncInterval = desc->enableVsync ? 1 : 0;
    DECLARE_ZERO(DXGI_SWAP_CHAIN_DESC1, desc1)
    desc1.Width = desc->width;
    desc1.Height = desc->height;
    desc1.Format = pf_translate_to_d3d12(desc->format);
    desc1.Stereo = false;
    desc1.SampleDesc.Count = 1; // If multisampling is needed, we'll resolve it later
    desc1.SampleDesc.Quality = 0;
    desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc1.BufferCount = desc->imageCount;
    desc1.Scaling = DXGI_SCALING_STRETCH;
    desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // for better performance.
    desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc1.Flags = 0;
    BOOL allowTearing = FALSE;
    I->pDXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    desc1.Flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    S->mFlags |= (!desc->enableVsync && allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    IDXGISwapChain1* swapchain;
    HWND hwnd = (HWND)desc->surface;

    CGpuQueue_D3D12* Q = CGPU_NULLPTR;
    if (desc->presentQueues == CGPU_NULLPTR)
    {
        Q = (CGpuQueue_D3D12*)cgpu_get_queue_d3d12(device, ECGpuQueueType_Graphics, 0);
    }
    else
    {
        Q = (CGpuQueue_D3D12*)desc->presentQueues[0];
    }
    auto bCreated =
        SUCCEEDED(I->pDXGIFactory->CreateSwapChainForHwnd(Q->pCommandQueue, hwnd, &desc1, NULL, NULL, &swapchain));
    (void)bCreated;
    assert(bCreated && "Failed to Try to Create SwapChain!");

    auto bAssociation = SUCCEEDED(I->pDXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
    (void)bAssociation;
    assert(bAssociation && "Failed to Try to Associate SwapChain With Window!");

    auto bQueryChain3 = SUCCEEDED(swapchain->QueryInterface(IID_PPV_ARGS(&S->pDxSwapChain)));
    (void)bQueryChain3;
    assert(bQueryChain3 && "Failed to Query IDXGISwapChain3 from Created SwapChain!");

    SAFE_RELEASE(swapchain);

    return &S->super;
}

void cgpu_free_swapchain_d3d12(CGpuSwapChainId swapchain)
{
    CGpuSwapChain_D3D12* S = (CGpuSwapChain_D3D12*)swapchain;
    SAFE_RELEASE(S->pDxSwapChain);
    delete S;
}

#include "cgpu/extensions/cgpu_d3d12_exts.h"
// extentions
CGpuDREDSettingsId cgpu_d3d12_enable_DRED()
{
    CGpuDREDSettingsId settings = new CGpuDREDSettings();
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
    delete settings;
}