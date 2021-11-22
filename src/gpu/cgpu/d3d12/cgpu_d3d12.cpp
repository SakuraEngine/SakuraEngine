#define DLL_IMPLEMENTATION

#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/d3d12_bridge.h"
#include <dxcapi.h>

#include <assert.h>
#include <stdlib.h>
#ifdef CGPU_USE_D3D12
    //
    // C++ is the only language supported by D3D12:
    //   https://msdn.microsoft.com/en-us/library/windows/desktop/dn899120(v=vs.85).aspx
    //
    #if !defined(__cplusplus)
        #error "D3D12 requires C++! Sorry!"
    #endif
    #include <stdio.h>
    #include <vector>

    #if !defined(XBOX)
        #pragma comment(lib, "d3d12.lib")
        #pragma comment(lib, "dxgi.lib")
        #pragma comment(lib, "dxguid.lib")
        #pragma comment(lib, "dxcompiler.lib")
    #endif

    #ifndef SAFE_RELEASE
        #define SAFE_RELEASE(p_var) \
            if (p_var)              \
            {                       \
                p_var->Release();   \
                p_var = NULL;       \
            }
    #endif

D3D_FEATURE_LEVEL feature_levels[] = {
    //
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0
    //
};

void optionalEnableDebugLayer(CGpuInstance_D3D12* result, CGpuInstanceDescriptor const* descriptor)
{
    if (descriptor->enableDebugLayer)
    {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&result->pDXDebug))))
        {
            result->pDXDebug->EnableDebugLayer();
            if (descriptor->enableGpuBasedValidation)
            {
                ID3D12Debug1* pDebug1 = NULL;
                if (SUCCEEDED(result->pDXDebug->QueryInterface(IID_PPV_ARGS(&pDebug1))))
                {
                    pDebug1->SetEnableGPUBasedValidation(descriptor->enableGpuBasedValidation);
                    pDebug1->Release();
                }
            }
        }
    }
    else if (descriptor->enableGpuBasedValidation)
    {
        printf("[D3D12 Warning]: GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
    }
}

    #include <comdef.h>

// Call this only once.
void getProperGpuCount(CGpuInstance_D3D12* instance, uint32_t* count, bool* foundSoftwareAdapter)
{
    assert(instance->pAdapters == nullptr && "getProperGpuCount should be called only once!");
    assert(instance->mAdaptersCount == 0 && "getProperGpuCount should be called only once!");
    IDXGIAdapter4* adapter = NULL;
    std::vector<IDXGIAdapter4*> adapters;
    std::vector<D3D_FEATURE_LEVEL> adapter_levels;
    // Find number of usable GPUs
    // Use DXGI6 interface which lets us specify gpu preference so we dont need to use NVOptimus or AMDPowerExpress
    // exports
    for (UINT i = 0; DXGI_ERROR_NOT_FOUND != instance->pDXGIFactory->EnumAdapterByGpuPreference(
                                                 i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
         ++i)
    {
        DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc)
        adapter->GetDesc3(&desc);
        // Ignore Microsoft Driver
        if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
        {
            uint32_t level_c = CGPU_ARRAY_LEN(feature_levels);
            for (uint32_t level = 0; level < level_c; ++level)
            {
                // Make sure the adapter can support a D3D12 device
                if (SUCCEEDED(D3D12CreateDevice(adapter, feature_levels[level], __uuidof(ID3D12Device), NULL)))
                {
                    DECLARE_ZERO(CGpuAdapter_D3D12, cgpuAdapter)
                    HRESULT hres = adapter->QueryInterface(IID_PPV_ARGS(&cgpuAdapter.pDxActiveGPU));
                    if (SUCCEEDED(hres))
                    {
                        SAFE_RELEASE(cgpuAdapter.pDxActiveGPU);
                        instance->mAdaptersCount++;
                        // Add ref
                        {
                            adapters.push_back(adapter);
                            adapter_levels.push_back(feature_levels[level]);
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            *foundSoftwareAdapter = true;
            adapter->Release();
        }
    }
    *count = instance->mAdaptersCount;
    instance->pAdapters = (CGpuAdapter_D3D12*)malloc(sizeof(CGpuAdapter_D3D12) * instance->mAdaptersCount);
    for (uint32_t i = 0; i < *count; i++)
    {
        instance->pAdapters[i].pDxActiveGPU = adapters[i];
        instance->pAdapters[i].mFeatureLevel = adapter_levels[i];
        DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc)
        adapters[i]->GetDesc3(&desc);

        instance->pAdapters[i].adapter_detail.deviceId = desc.DeviceId;
        instance->pAdapters[i].adapter_detail.vendorId = desc.VendorId;
        _bstr_t b(desc.Description);
        char* str = b;
        memcpy(instance->pAdapters[i].mDescription, str, b.length());
        instance->pAdapters[i].mDescription[b.length()] = '\0';
        instance->pAdapters[i].adapter_detail.name = instance->pAdapters[i].mDescription;

        instance->pAdapters[i].super.instance = &instance->super;
    }
}

CGpuInstanceId cgpu_create_instance_d3d12(CGpuInstanceDescriptor const* descriptor)
{
    CGpuInstance_D3D12* result = new CGpuInstance_D3D12();
    optionalEnableDebugLayer(result, descriptor);

    UINT flags = 0;
    if (descriptor->enableDebugLayer) flags = DXGI_CREATE_FACTORY_DEBUG;
    #if defined(XBOX)
    #else
    if (SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&result->pDXGIFactory))))
    {
        uint32_t gpuCount = 0;
        bool foundSoftwareAdapter = false;
        getProperGpuCount(result, &gpuCount, &foundSoftwareAdapter);
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
    CGpuInstance_D3D12* result = (CGpuInstance_D3D12*)instance;
    if (result->mAdaptersCount > 0)
    {
        for (uint32_t i = 0; i < result->mAdaptersCount; i++)
        {
            result->pAdapters[i].pDxActiveGPU->Release();
        }
    }
    result->pDXGIFactory->Release();
    if (result->pDXDebug)
    {
        result->pDXDebug->Release();
    }
    #ifdef _DEBUG
    {
        IDXGIDebug1* dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(
                DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
        dxgiDebug->Release();
    }
    #endif
    delete result;
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
    const CGpuAdapter_D3D12* A = (CGpuAdapter_D3D12*)adapter;
    CGpuDevice_D3D12* cgpuD3D12Device = new CGpuDevice_D3D12();
    *const_cast<CGpuAdapterId*>(&cgpuD3D12Device->super.adapter) = adapter;

    if (!SUCCEEDED(D3D12CreateDevice(A->pDxActiveGPU, // default adapter
            A->mFeatureLevel, IID_PPV_ARGS(&cgpuD3D12Device->pDxDevice))))
    {
        assert("[D3D12 Fatal]: Create D3D12Device Failed!");
    }

    // Create Requested Queues.
    for (uint32_t i = 0u; i < desc->queueGroupCount; i++)
    {
        const auto& queueGroup = desc->queueGroups[i];
        const auto queueType = queueGroup.queueType;

        *const_cast<uint32_t*>(&cgpuD3D12Device->pCommandQueueCounts[i]) = queueGroup.queueCount;
        *const_cast<ID3D12CommandQueue***>(&cgpuD3D12Device->ppCommandQueues[queueType]) =
            (ID3D12CommandQueue**)malloc(sizeof(ID3D12CommandQueue*) * queueGroup.queueCount);

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
            if (!SUCCEEDED(cgpuD3D12Device->pDxDevice->CreateCommandQueue(
                    &queueDesc, IID_PPV_ARGS(&cgpuD3D12Device->ppCommandQueues[queueType][j]))))
            {
                assert("[D3D12 Fatal]: Create D3D12CommandQueue Failed!");
            }
        }
    }
    return &cgpuD3D12Device->super;
}

void cgpu_free_device_d3d12(CGpuDeviceId device)
{
    CGpuDevice_D3D12* cgpuD3D12Device = (CGpuDevice_D3D12*)device;
    for (uint32_t t = 0u; t < ECGpuQueueType_Count; t++)
    {
        for (uint32_t i = 0; i < cgpuD3D12Device->pCommandQueueCounts[t]; i++)
        {
            cgpuD3D12Device->ppCommandQueues[t][i]->Release();
        }
    }
    cgpuD3D12Device->pDxDevice->Release();
}

CGpuQueueId cgpu_get_queue_d3d12(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)device;
    CGpuQueue_D3D12* Q = new CGpuQueue_D3D12();
    Q->pCommandQueue = D->ppCommandQueues[type][index];
    return &Q->super;
}

void cgpu_free_queue_d3d12(CGpuQueueId queue)
{
    CGpuQueue_D3D12* Q = (CGpuQueue_D3D12*)queue;
    assert(queue && "D3D12 ERROR: FREE NULL QUEUE!");
    delete Q;
}

// allocate_transient_command_allocator
ID3D12CommandAllocator* allocate_transient_command_allocator(CGpuCommandPool_D3D12* E, CGpuQueueId queue)
{
    CGpuDevice_D3D12* D = (CGpuDevice_D3D12*)queue->device;
    D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    bool res = SUCCEEDED(D->pDxDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&E->pCommandAllocator)));
    if (res)
    {
        return E->pCommandAllocator;
    }
    return CGPU_NULLPTR;
}

void free_transient_command_allocator(ID3D12CommandAllocator* allocator) { allocator->Release(); }

CGpuCommandPoolId cgpu_create_command_pool_d3d12(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc)
{
    CGpuCommandPool_D3D12* E = new CGpuCommandPool_D3D12();
    E->pCommandAllocator = allocate_transient_command_allocator(E, queue);
    return &E->super;
}

void cgpu_free_command_pool_d3d12(CGpuCommandPoolId encoder)
{
    CGpuCommandPool_D3D12* E = (CGpuCommandPool_D3D12*)encoder;
    assert(encoder && "D3D12 ERROR: FREE NULL COMMAND ENCODER!");
    assert(E->pCommandAllocator && "D3D12 ERROR: FREE NULL pCommandAllocator!");

    free_transient_command_allocator(E->pCommandAllocator);
    delete E;
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
    pUtils->CreateBlobWithEncodingOnHeapCopy(desc->code, desc->code_size, DXC_CP_ACP, &S->pShaderBlob);
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

    swapchain->Release();

    return &S->super;
}

void cgpu_free_swapchain_d3d12(CGpuSwapChainId swapchain)
{
    CGpuSwapChain_D3D12* S = (CGpuSwapChain_D3D12*)swapchain;
    S->pDxSwapChain->Release();
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
    settings->pDredSettings->Release();
    delete settings;
}

#endif
