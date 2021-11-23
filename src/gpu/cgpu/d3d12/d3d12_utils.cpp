#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/d3d12_bridge.h"
#include "d3d12_utils.h"
#include "D3D12MemAlloc.h"
#include <EASTL/vector.h>

void D3D12Util_Optionalenable_debug_layer(CGpuInstance_D3D12* result, CGpuInstanceDescriptor const* descriptor)
{
    if (descriptor->enable_debug_layer)
    {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&result->pDXDebug))))
        {
            result->pDXDebug->EnableDebugLayer();
            if (descriptor->enable_gpu_based_validation)
            {
                ID3D12Debug1* pDebug1 = NULL;
                if (SUCCEEDED(result->pDXDebug->QueryInterface(IID_PPV_ARGS(&pDebug1))))
                {
                    pDebug1->SetEnableGPUBasedValidation(descriptor->enable_gpu_based_validation);
                    pDebug1->Release();
                }
            }
        }
    }
    else if (descriptor->enable_gpu_based_validation)
    {
        printf("[D3D12 Warning]: GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
    }
}

void D3D12Util_QueryAllAdapters(CGpuInstance_D3D12* instance, uint32_t* count, bool* foundSoftwareAdapter)
{
    assert(instance->pAdapters == nullptr && "getProperGpuCount should be called only once!");
    assert(instance->mAdaptersCount == 0 && "getProperGpuCount should be called only once!");
    IDXGIAdapter4* adapter = NULL;
    eastl::vector<IDXGIAdapter4*> dxgi_adapters;
    eastl::vector<D3D_FEATURE_LEVEL> adapter_levels;
    // Find number of usable GPUs
    // Use DXGI6 interface which lets us specify gpu preference so we dont need to use NVOptimus or AMDPowerExpress
    // exports
    for (UINT i = 0;
         instance->pDXGIFactory->EnumAdapterByGpuPreference(i,
             DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
             IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
         i++)
    {
        DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc)
        adapter->GetDesc3(&desc);
        // Ignore Microsoft Driver
        if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
        {
            uint32_t level_c = CGPU_ARRAY_LEN(d3d_feature_levels);
            for (uint32_t level = 0; level < level_c; ++level)
            {
                // Make sure the adapter can support a D3D12 device
                if (SUCCEEDED(D3D12CreateDevice(adapter, d3d_feature_levels[level], __uuidof(ID3D12Device), NULL)))
                {
                    DECLARE_ZERO(CGpuAdapter_D3D12, cgpuAdapter)
                    HRESULT hres = adapter->QueryInterface(IID_PPV_ARGS(&cgpuAdapter.pDxActiveGPU));
                    if (SUCCEEDED(hres))
                    {
                        SAFE_RELEASE(cgpuAdapter.pDxActiveGPU);
                        instance->mAdaptersCount++;
                        // Add ref
                        {
                            dxgi_adapters.push_back(adapter);
                            adapter_levels.push_back(d3d_feature_levels[level]);
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            *foundSoftwareAdapter = true;
            SAFE_RELEASE(adapter);
        }
    }
    *count = instance->mAdaptersCount;
    instance->pAdapters = (CGpuAdapter_D3D12*)cgpu_malloc(sizeof(CGpuAdapter_D3D12) * instance->mAdaptersCount);
    for (uint32_t i = 0; i < *count; i++)
    {
        auto& adapter = instance->pAdapters[i];
        // Device Objects
        adapter.pDxActiveGPU = dxgi_adapters[i];
        adapter.mFeatureLevel = adapter_levels[i];
        D3D12Util_RecordAdapterDetail(&adapter);
    }
}

void D3D12Util_RecordAdapterDetail(struct CGpuAdapter_D3D12* D3D12Adapter)
{
    auto& adapter = *D3D12Adapter;
    auto& adapter_detail = adapter.adapter_detail;
    auto& vendor_preset = adapter_detail.vendor_preset;
    // Vendor & Feature Cache
    DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc3)
    adapter.pDxActiveGPU->GetDesc3(&desc3);
    vendor_preset.device_id = desc3.DeviceId;
    vendor_preset.vendor_id = desc3.VendorId;
    _bstr_t b(desc3.Description);
    char* str = b;
    strncpy(adapter_detail.vendor_preset.gpu_name, str, MAX_GPU_VENDOR_STRING_LENGTH);
    // TODO: Driver Version
    vendor_preset.driver_version = 0;
    // Create a device for feature query
    DECLARE_ZERO(D3D12_FEATURE_DATA_ARCHITECTURE1, dxgi_feature)
    ID3D12Device* pCheckDevice = nullptr;
    if (!SUCCEEDED(D3D12CreateDevice(adapter.pDxActiveGPU, adapter.mFeatureLevel, IID_PPV_ARGS(&pCheckDevice))))
    {
        assert("[D3D12 Fatal]: Create D3D12Device Failed When Query Adapter Features!");
    }
    pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &dxgi_feature, sizeof(dxgi_feature));
    adapter_detail.is_uma = dxgi_feature.UMA;
    adapter_detail.is_cpu = desc3.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
    adapter_detail.is_virtual = false;
    SAFE_RELEASE(pCheckDevice);
}

void D3D12Util_CreateDMAAllocator(CGpuInstance_D3D12* I, CGpuAdapter_D3D12* A, CGpuDevice_D3D12* D)
{
    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    desc.pDevice = D->pDxDevice;
    desc.pAdapter = A->pDxActiveGPU;

    D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
    allocationCallbacks.pAllocate = [](size_t size, size_t alignment, void*) {
        return cgpu_memalign(size, alignment);
    };
    allocationCallbacks.pFree = [](void* ptr, void*) { cgpu_free(ptr); };
    desc.pAllocationCallbacks = &allocationCallbacks;
    if (!SUCCEEDED(D3D12MA::CreateAllocator(&desc, &D->pResourceAllocator)))
    {
        assert(0 && "DMA Allocator Create Failed!");
    }
}