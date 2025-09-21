#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrGraphics/containers.hpp"
#include "SkrGraphics/backend/d3d12/cgpu_d3d12.h"
#include "SkrGraphics/driver-extensions/cgpu_nvapi.h"
#include "SkrGraphics/driver-extensions/cgpu_ags.h"
#include "d3d12_utils.h"
#include "D3D12MemAlloc.h"
#include <dxcapi.h>
#include <d3d12shader.h>
#include <comutil.h>

#define USE_PIX
#ifdef USE_PIX
    #include "WinPixEventRuntime/pix3.h" // IWYU pragma: keep
#endif

struct CGPUUtil_DXCLoader
{
    static void Load()
    {
        dxcLibrary = LoadLibrary(L"dxcompiler.dll");
        pDxcCreateInstance = (void*)::GetProcAddress((HMODULE)dxcLibrary, "DxcCreateInstance");
    }
    static void Unload()
    {
        pDxcCreateInstance = nullptr;
        ::FreeLibrary(dxcLibrary);
    }
    static DxcCreateInstanceProc Get()
    {
        return (DxcCreateInstanceProc)pDxcCreateInstance;
    }
    static HMODULE dxcLibrary;
    static void* pDxcCreateInstance;
};
void* CGPUUtil_DXCLoader::pDxcCreateInstance = nullptr;
HMODULE CGPUUtil_DXCLoader::dxcLibrary = nullptr;

void D3D12Util_LoadDxcDLL()
{
    CGPUUtil_DXCLoader::Load();
}

void D3D12Util_UnloadDxcDLL()
{
    CGPUUtil_DXCLoader::Unload();
}

DxcCreateInstanceProc D3D12Util_GetDxcCreateInstanceProc()
{
    return CGPUUtil_DXCLoader::Get();
}

size_t D3D12Util_GetShaderBlobSize(struct IDxcBlobEncoding* Blob)
{
    return Blob->GetBufferSize();
}

const void* D3D12Util_GetShaderBlobData(struct IDxcBlobEncoding* Blob)
{
    return Blob->GetBufferPointer();
}

UINT64 encode_color_for_pix(const float* color)
{
    if (color == nullptr)
        return PIX_COLOR(0, 0, 0);

    return PIX_COLOR(
        static_cast<BYTE>(color[0] * 255.f),
        static_cast<BYTE>(color[1] * 255.f),
        static_cast<BYTE>(color[2] * 255.f)
    );
}
void cgpu_cmd_begin_event_d3d12(CGPUCommandBufferId cmd, const CGPUEventInfo* event)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    const auto eventColor = encode_color_for_pix(event->color);
    PIXBeginEvent(Cmd->pDxCmdList, eventColor, (const char*)event->name);
}

void cgpu_cmd_set_marker_d3d12(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    const auto markerColor = encode_color_for_pix(marker->color);
    PIXSetMarker(Cmd->pDxCmdList, markerColor, (const char*)marker->name);
}

void cgpu_cmd_end_event_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    PIXEndEvent(Cmd->pDxCmdList);
}

bool D3D12Util_InitializeEnvironment(struct CGPUInstance* Inst)
{
    Inst->runtime_table = cgpu_create_runtime_table();
    // AGS
    bool AGS_started = false;
    AGS_started = (cgpu_ags_init(Inst) == CGPU_AGS_SUCCESS);
    (void)AGS_started;
    // NVAPI
    bool NVAPI_started = false;
    NVAPI_started = (cgpu_nvapi_init(Inst) == CGPU_NVAPI_OK);
    (void)NVAPI_started;
    return true;
}

void D3D12Util_DeInitializeEnvironment(struct CGPUInstance* Inst)
{
    cgpu_ags_exit(Inst);
    Inst->ags_status = CGPU_AGS_NONE;
    cgpu_nvapi_exit(Inst);
    Inst->nvapi_status = CGPU_NVAPI_NONE;
}

void D3D12Util_Optionalenable_debug_layer(CGPUInstance_D3D12* result, CGPUInstanceDescriptor const* descriptor)
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
        cgpu_warn(u8"D3D12 GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
    }
}

void D3D12Util_QueryAllAdapters(CGPUInstance_D3D12* instance, uint32_t* count, bool* foundSoftwareAdapter)
{
    cgpu_assert(instance->pAdapters == nullptr && "getProperGpuCount should be called only once!");
    cgpu_assert(instance->mAdaptersCount == 0 && "getProperGpuCount should be called only once!");
    IDXGIAdapter* _adapter = NULL;
    cgpu::stl_vector<IDXGIAdapter4*> dxgi_adapters;
    cgpu::stl_vector<D3D_FEATURE_LEVEL> adapter_levels;
    // Find number of usable GPUs
    // Use DXGI6 interface which lets us specify gpu preference so we dont need to use NVOptimus or AMDPowerExpress
    // exports
    for (UINT i = 0;
         instance->pDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter)) != DXGI_ERROR_NOT_FOUND;
         i++)
    {
        SKR_DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc)
        IDXGIAdapter4* adapter = nullptr;
        _adapter->QueryInterface(IID_PPV_ARGS(&adapter));
        adapter->GetDesc3(&desc);
        // Ignore Microsoft Driver
        if (!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
        {
            uint32_t level_c = CGPU_ARRAY_LEN(d3d_feature_levels);
            for (uint32_t level = 0; level < level_c; ++level)
            {
                // Make sure the adapter can support a D3D12 device
                ID3D12Device* pDevice = CGPU_NULLPTR;
                if (SUCCEEDED(D3D12CreateDevice(adapter, d3d_feature_levels[level], __uuidof(ID3D12Device), (void**)&pDevice)))
                {
                    SKR_DECLARE_ZERO(CGPUAdapter_D3D12, cgpuAdapter)
                    HRESULT hres = adapter->QueryInterface(IID_PPV_ARGS(&cgpuAdapter.pDxActiveGPU));
                    if (SUCCEEDED(hres))
                    {
                        SAFE_RELEASE(pDevice);
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
        SAFE_RELEASE(_adapter);
    }
    *count = instance->mAdaptersCount;
    instance->pAdapters = (CGPUAdapter_D3D12*)cgpu_malloc(sizeof(CGPUAdapter_D3D12) * instance->mAdaptersCount);
    for (uint32_t i = 0; i < *count; i++)
    {
        auto& adapter = instance->pAdapters[i];
        // Device Objects
        adapter.pDxActiveGPU = dxgi_adapters[i];
        adapter.mFeatureLevel = adapter_levels[i];
        adapter.super.instance = &instance->super;
        D3D12Util_RecordAdapterDetail(&adapter);
    }
}

void D3D12Util_EnumFormatSupports(CGPUAdapter_D3D12* D3DAdapter, ID3D12Device* pCheckDevice)
{
    CGPUAdapterDetail* adapter_detail = (CGPUAdapterDetail*)&D3DAdapter->adapter_detail;
    for (uint32_t i = 0; i < CGPU_FORMAT_COUNT; ++i)
    {
        adapter_detail->format_supports[i].shader_read = 0;
        adapter_detail->format_supports[i].shader_write = 0;
        adapter_detail->format_supports[i].render_target_write = 0;
        DXGI_FORMAT fmt = DXGIUtil_TranslatePixelFormat((ECGPUFormat)i, false);
        if (fmt == DXGI_FORMAT_UNKNOWN) continue;

        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { fmt, {}, {} };
        pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));

        adapter_detail->format_supports[i].shader_read = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) != 0;
        adapter_detail->format_supports[i].shader_write = (formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0;
        adapter_detail->format_supports[i].render_target_write = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) != 0;
    }
    return;
}

void D3D12Util_RecordAdapterDetail(struct CGPUAdapter_D3D12* D3D12Adapter)
{
    CGPUInstance_D3D12* I = (CGPUInstance_D3D12*)D3D12Adapter->super.instance;
    auto& adapter = *D3D12Adapter;
    auto& adapter_detail = adapter.adapter_detail;
    adapter_detail = make_zeroed<CGPUAdapterDetail>();
    auto& vendor_preset = adapter_detail.vendor_preset;
    // Vendor & Feature Cache
    SKR_DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc3)
    adapter.pDxActiveGPU->GetDesc3(&desc3);
    vendor_preset.device_id = desc3.DeviceId;
    vendor_preset.vendor_id = desc3.VendorId;
    _bstr_t b(desc3.Description);
    char* str = b;
    strncpy(adapter_detail.vendor_preset.gpu_name, str, MAX_GPU_VENDOR_STRING_LENGTH);
    // Get Driver Version
    if (I->super.nvapi_status == CGPU_NVAPI_OK)
        vendor_preset.driver_version = cgpu_nvapi_get_driver_version(&I->super);
    else if (I->super.ags_status == CGPU_AGS_SUCCESS)
        vendor_preset.driver_version = cgpu_ags_get_driver_version(&I->super);
    else
        vendor_preset.driver_version = 0;
    // Create a device for feature query
    ID3D12Device* pCheckDevice = nullptr;
    if (!SUCCEEDED(D3D12CreateDevice(adapter.pDxActiveGPU, adapter.mFeatureLevel, IID_PPV_ARGS(&pCheckDevice))))
    {
        cgpu_assert("[D3D12 Fatal]: Create D3D12Device Failed When Query Adapter Features!");
    }
    // Architecture features
    SKR_DECLARE_ZERO(D3D12_FEATURE_DATA_ARCHITECTURE1, dxgi_feature)
    pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &dxgi_feature, sizeof(dxgi_feature));
    adapter_detail.is_uma = dxgi_feature.UMA;
    adapter_detail.is_cpu = (desc3.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) ? 1 : 0;
    adapter_detail.is_virtual = false;
    // Constants
    adapter_detail.max_vertex_input_bindings = 32u;
    adapter_detail.uniform_buffer_alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    adapter_detail.upload_buffer_texture_alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
    adapter_detail.upload_buffer_texture_row_alignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
    // Check Format Capacities
    D3D12Util_EnumFormatSupports(D3D12Adapter, pCheckDevice);
    D3D12_FEATURE_DATA_SHADER_MODEL SM = make_zeroed<D3D12_FEATURE_DATA_SHADER_MODEL>();
    if (SUCCEEDED(pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SM, sizeof(SM))))
    {
        adapter.mShaderModel = SM.HighestShaderModel;
    }
    D3D12_FEATURE_DATA_D3D12_OPTIONS options = make_zeroed<D3D12_FEATURE_DATA_D3D12_OPTIONS>();
    if (SUCCEEDED(pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
    {
        adapter.mTiledResourceTier = options.TiledResourcesTier;
        adapter.mResourceHeapTier = options.ResourceHeapTier;
        adapter.mStandardSwizzle64KBSupported = options.StandardSwizzle64KBSupported;
        adapter_detail.support_tiled_volume = (options.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_3);
        adapter_detail.support_tiled_texture = (options.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_1);
        adapter_detail.support_tiled_buffer = (options.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_1);
    }
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1 = make_zeroed<D3D12_FEATURE_DATA_D3D12_OPTIONS1>();
    if (SUCCEEDED(pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1))))
    {
        adapter_detail.wave_lane_count = options1.WaveLaneCountMin;
    }
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = make_zeroed<D3D12_FEATURE_DATA_D3D12_OPTIONS5>();
    if (SUCCEEDED(pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
    {
        adapter.mRayTracingTier = options5.RaytracingTier;
        adapter_detail.support_ray_tracing = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }
#ifdef D3D12_HEADER_SUPPORT_VRS
    D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6 = make_zeroed<D3D12_FEATURE_DATA_D3D12_OPTIONS6>();
    if (SUCCEEDED(pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6))))
    {

        if (options6.VariableShadingRateTier == D3D12_VARIABLE_SHADING_RATE_TIER_1)
        {
            adapter_detail.support_shading_rate = true;
            adapter_detail.support_shading_rate_mask = false;
            adapter_detail.support_shading_rate_sv = true;
        }
        else if (options6.VariableShadingRateTier == D3D12_VARIABLE_SHADING_RATE_TIER_2)
        {
            adapter_detail.support_shading_rate = true;
            adapter_detail.support_shading_rate_mask = true;
            adapter_detail.support_shading_rate_sv = true;
        }
        else
        {
            adapter_detail.support_shading_rate = false;
            adapter_detail.support_shading_rate_mask = false;
            adapter_detail.support_shading_rate_sv = false;
        }
    }
#endif
#ifdef CGPU_USE_D3D12_ENHANCED_BARRIERS
    // Enhanced barriers features
    D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
    if (SUCCEEDED(pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12))))
    {
        D3D12Adapter->mEnhancedBarriersSupported = options12.EnhancedBarriersSupported;
    }
#endif
    adapter_detail.host_visible_vram_budget = 0;
    adapter_detail.support_host_visible_vram = false;
#ifdef CGPU_USE_NVAPI
    if (I->super.nvapi_status == CGPU_NVAPI_OK)
    {
        adapter_detail.host_visible_vram_budget = cgpu_nvapi_d3d12_query_cpu_visible_vram(&I->super, pCheckDevice);
        adapter_detail.support_host_visible_vram = adapter_detail.host_visible_vram_budget > 0;
    }
#endif
#ifdef AMDAGS
#endif
    SAFE_RELEASE(pCheckDevice);
}

const char* kD3D12MAMemoryName = "D3D12MA";
void D3D12Util_CreateDMAAllocator(CGPUInstance_D3D12* I, CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D)
{
    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    desc.pDevice = D->pDxDevice;
    desc.pAdapter = A->pDxActiveGPU;
    desc.PreferredBlockSize = 0;

    D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
    allocationCallbacks.pAllocate = +[](size_t size, size_t alignment, void*) {
        return cgpu_malloc_alignedN(size, alignment, kD3D12MAMemoryName);
    };
    allocationCallbacks.pFree = +[](void* ptr, void*) {
        cgpu_free_alignedN(ptr, 1, kD3D12MAMemoryName); // TODO: Fix this
    };
    desc.pAllocationCallbacks = &allocationCallbacks;
    desc.Flags |= D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;
    if (!SUCCEEDED(D3D12MA::CreateAllocator(&desc, &D->pResourceAllocator)))
    {
        cgpu_assert(0 && "DMA Allocator Create Failed!");
    }
}

void D3D12Util_FreeDMAAllocator(CGPUDevice_D3D12* D)
{
    if (D->pResourceAllocator)
    {
        SAFE_RELEASE(D->pResourceAllocator);
        D->pResourceAllocator = nullptr;
    }
}

void D3D12Util_SignalFence(CGPUQueue_D3D12* Q, ID3D12Fence* DxF, uint64_t fenceValue)
{
    Q->pCommandQueue->Signal(DxF, fenceValue);
}

// Shader Reflection
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 | (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)
enum DxilFourCC
{
    DFCC_Container = DXIL_FOURCC('D', 'X', 'B', 'C'), // for back-compat with tools that look for DXBC containers
    DFCC_ResourceDef = DXIL_FOURCC('R', 'D', 'E', 'F'),
    DFCC_InputSignature = DXIL_FOURCC('I', 'S', 'G', '1'),
    DFCC_OutputSignature = DXIL_FOURCC('O', 'S', 'G', '1'),
    DFCC_PatchConstantSignature = DXIL_FOURCC('P', 'S', 'G', '1'),
    DFCC_ShaderStatistics = DXIL_FOURCC('S', 'T', 'A', 'T'),
    DFCC_ShaderDebugInfoDXIL = DXIL_FOURCC('I', 'L', 'D', 'B'),
    DFCC_ShaderDebugName = DXIL_FOURCC('I', 'L', 'D', 'N'),
    DFCC_FeatureInfo = DXIL_FOURCC('S', 'F', 'I', '0'),
    DFCC_PrivateData = DXIL_FOURCC('P', 'R', 'I', 'V'),
    DFCC_RootSignature = DXIL_FOURCC('R', 'T', 'S', '0'),
    DFCC_DXIL = DXIL_FOURCC('D', 'X', 'I', 'L'),
    DFCC_PipelineStateValidation = DXIL_FOURCC('P', 'S', 'V', '0'),
    DFCC_RuntimeData = DXIL_FOURCC('R', 'D', 'A', 'T'),
    DFCC_ShaderHash = DXIL_FOURCC('H', 'A', 'S', 'H'),
    DFCC_ShaderSourceInfo = DXIL_FOURCC('S', 'R', 'C', 'I'),
    DFCC_ShaderPDBInfo = DXIL_FOURCC('P', 'D', 'B', 'I'),
    DFCC_CompilerVersion = DXIL_FOURCC('V', 'E', 'R', 'S'),
};
#undef DXIL_FOURCC

const char8_t* D3DShaderEntryName = CGPU_NULLPTR;
static ECGPUResourceType gResourceTypeLUT[] = {
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_CBUFFER
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_TBUFFER
    CGPU_RESOURCE_TYPE2_TEXTURE,                // D3D_SIT_TEXTURE
    CGPU_RESOURCE_TYPE2_SAMPLER,                // D3D_SIT_SAMPLER
    CGPU_RESOURCE_TYPE2_TEXTURE,                // D3D_SIT_UAV_RWTYPED
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_STRUCTURED
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_RWSTRUCTURED
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_BYTEADDRESS
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_UAV_RWBYTEADDRESS
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_UAV_APPEND_STRUCTURED
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_UAV_CONSUME_STRUCTURED
    CGPU_RESOURCE_TYPE2_BUFFER,                 // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
    CGPU_RESOURCE_TYPE2_ACCELERATION_STRUCTURE, // D3D_SIT_RTACCELERATIONSTRUCTURE
};
static CGPUViewUsages gViewUsageLUT[] = {
    CGPU_BUFFER_VIEW_USAGE_CBV,            // D3D_SIT_CBUFFER
    CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL,      // D3D_SIT_TBUFFER
    CGPU_TEXTURE_VIEW_USAGE_SRV,           // D3D_SIT_TEXTURE
    0,                                     // D3D_SIT_SAMPLER
    CGPU_TEXTURE_VIEW_USAGE_UAV,           // D3D_SIT_UAV_RWTYPED
    CGPU_BUFFER_VIEW_USAGE_SRV_STRUCTURED, // D3D_SIT_STRUCTURED
    CGPU_BUFFER_VIEW_USAGE_UAV_STRUCTURED, // D3D_SIT_RWSTRUCTURED
    CGPU_BUFFER_VIEW_USAGE_SRV_RAW,        // D3D_SIT_BYTEADDRESS
    CGPU_BUFFER_VIEW_USAGE_UAV_RAW,        // D3D_SIT_UAV_RWBYTEADDRESS
    CGPU_BUFFER_VIEW_USAGE_UAV_STRUCTURED, // D3D_SIT_UAV_APPEND_STRUCTURED
    CGPU_BUFFER_VIEW_USAGE_UAV_STRUCTURED, // D3D_SIT_UAV_CONSUME_STRUCTURED
    CGPU_BUFFER_VIEW_USAGE_UAV_STRUCTURED, // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
    0,                                     // D3D_SIT_RTACCELERATIONSTRUCTURE
};

static ECGPUTextureDimension gD3D12_TO_RESOURCE_DIM[D3D_SRV_DIMENSION_BUFFEREX + 1] = {
    CGPU_TEXTURE_DIMENSION_UNDEFINED,  // D3D_SRV_DIMENSION_UNKNOWN
    CGPU_TEXTURE_DIMENSION_UNDEFINED,  // D3D_SRV_DIMENSION_BUFFER
    CGPU_TEXTURE_DIMENSION_1D,         // D3D_SRV_DIMENSION_TEXTURE1D
    CGPU_TEXTURE_DIMENSION_1D_ARRAY,   // D3D_SRV_DIMENSION_TEXTURE1DARRAY
    CGPU_TEXTURE_DIMENSION_2D,         // D3D_SRV_DIMENSION_TEXTURE2D
    CGPU_TEXTURE_DIMENSION_2D_ARRAY,   // D3D_SRV_DIMENSION_TEXTURE2DARRAY
    CGPU_TEXTURE_DIMENSION_2DMS,       // D3D_SRV_DIMENSION_TEXTURE2DMS
    CGPU_TEXTURE_DIMENSION_2DMS_ARRAY, // D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
    CGPU_TEXTURE_DIMENSION_3D,         // D3D_SRV_DIMENSION_TEXTURE3D
    CGPU_TEXTURE_DIMENSION_CUBE,       // D3D_SRV_DIMENSION_TEXTURECUBE
    CGPU_TEXTURE_DIMENSION_CUBE_ARRAY, // D3D_SRV_DIMENSION_TEXTURECUBEARRAY
    CGPU_TEXTURE_DIMENSION_UNDEFINED,  // D3D_SRV_DIMENSION_BUFFEREX
};

static ECGPUFormat gD3D12_TO_VERTEX_FORMAT[] = {
    CGPU_FORMAT_UNDEFINED,  // 0
    CGPU_FORMAT_R32_UINT,   // 1 D3D_REGISTER_COMPONENT_UINT32
    CGPU_FORMAT_R32_SINT,   // 2 D3D_REGISTER_COMPONENT_SINT32
    CGPU_FORMAT_R32_SFLOAT, // 3 D3D_REGISTER_COMPONENT_FLOAT32

    CGPU_FORMAT_R32G32_UINT,   // 1 + 3 * 1
    CGPU_FORMAT_R32G32_SINT,   // 2 * 3 * 1
    CGPU_FORMAT_R32G32_SFLOAT, // 2 * 3 * 1

    CGPU_FORMAT_R32G32B32_UINT,   // 1 + 3 * 2
    CGPU_FORMAT_R32G32B32_SINT,   // 2 * 3 * 2
    CGPU_FORMAT_R32G32B32_SFLOAT, // 3 * 3 * 2

    CGPU_FORMAT_R32G32B32A32_UINT,  // 1 + 3 * 3
    CGPU_FORMAT_R32G32B32A32_SINT,  // 2 * 3 * 3
    CGPU_FORMAT_R32G32B32A32_SFLOAT // 3 * 3 * 3
};

template <typename D3D12_SHADER_DESC_T>
inline static ECGPUShaderStage D3D12Util_GetShaderStageFromDesc(const D3D12_SHADER_DESC_T& shaderDesc)
{
    UINT shaderType = D3D12_SHVER_GET_TYPE(shaderDesc.Version);
    switch (shaderType)
    {
    case D3D12_SHVER_VERTEX_SHADER:
        return CGPU_SHADER_STAGE_VERT;
    case D3D12_SHVER_PIXEL_SHADER:
        return CGPU_SHADER_STAGE_FRAG;
    case D3D12_SHVER_GEOMETRY_SHADER:
        return CGPU_SHADER_STAGE_GEOM;
    case D3D12_SHVER_HULL_SHADER:
        return CGPU_SHADER_STAGE_TESC;
    case D3D12_SHVER_DOMAIN_SHADER:
        return CGPU_SHADER_STAGE_TESE;
    case D3D12_SHVER_COMPUTE_SHADER:
        return CGPU_SHADER_STAGE_COMPUTE;
    case D3D12_SHVER_RAY_GENERATION_SHADER:
        return CGPU_SHADER_STAGE_RAYTRACING;
    case D3D12_SHVER_INTERSECTION_SHADER:
        return CGPU_SHADER_STAGE_RAYTRACING;
    case D3D12_SHVER_ANY_HIT_SHADER:
        return CGPU_SHADER_STAGE_RAYTRACING;
    case D3D12_SHVER_CLOSEST_HIT_SHADER:
        return CGPU_SHADER_STAGE_RAYTRACING;
    case D3D12_SHVER_MISS_SHADER:
        return CGPU_SHADER_STAGE_RAYTRACING;
    /*
        case D3D12_SHVER_LIBRARY:
            return CGPU_SHADER_STAGE_LIBRARY;
        case D3D12_SHVER_CALLABLE_SHADER:
            return CGPU_SHADER_STAGE_CALLABLE;
        case D3D12_SHVER_MESH_SHADER:
            return CGPU_SHADER_STAGE_MESH;
        case D3D12_SHVER_AMPLIFICATION_SHADER:
            return CGPU_SHADER_STAGE_AMPLIFICATION;
        */
    default:
        return CGPU_SHADER_STAGE_NONE;
    }
}

template <typename ID3D12ReflectionT, typename D3D12_SHADER_DESC_T>
void reflectionRecordShaderResources(ID3D12ReflectionT* d3d12reflection, ECGPUShaderStage stage, const D3D12_SHADER_DESC_T& shaderDesc, CGPUShaderLibrary_D3D12* S, uint32_t entryIndex)
{
    // Get the number of bound resources
    CGPUShaderReflection* Reflection = S->super.entry_reflections + entryIndex;
    if constexpr (std::is_same_v<ID3D12ReflectionT, ID3D12FunctionReflection>)
    {
        ID3D12FunctionReflection* dx12FuncRefl = d3d12reflection;
        const D3D12_FUNCTION_DESC& dx12FuncDesc = shaderDesc;
        skr::String FullString = skr::String::FromRaw(dx12FuncDesc.Name + 2);
        uint64_t SymbolEnd = FullString.find(u8"@@").index();
        skr::StringView FunctionSymbol = FullString.subview(0, SymbolEnd);
        auto newString = (char8_t*)cgpu_malloc(sizeof(char8_t) * (1 + FunctionSymbol.size()));
        ::memcpy(newString, FunctionSymbol.data(), FunctionSymbol.size());
        newString[FunctionSymbol.size()] = u8'\0';
        Reflection->entry_name = newString;
    }
    else
    {
        Reflection->entry_name = D3DShaderEntryName;
    }
    Reflection->shader_resources_count = shaderDesc.BoundResources;
    Reflection->shader_resources = (CGPUShaderResource*)cgpu_calloc(shaderDesc.BoundResources, sizeof(CGPUShaderResource));

    // Count string sizes of the bound resources for the name pool
    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc;
        d3d12reflection->GetResourceBindingDesc(i, &bindDesc);
        const size_t source_len = strlen(bindDesc.Name);
        Reflection->shader_resources[i].name = (char8_t*)cgpu_malloc(sizeof(char8_t) * (source_len + 1));
        Reflection->shader_resources[i].name_hash = skr_hash_of(bindDesc.Name, strlen(bindDesc.Name));
        // We are very sure it's windows platform
        strcpy_s((char*)Reflection->shader_resources[i].name, source_len + 1, bindDesc.Name);
        Reflection->shader_resources[i].type = gResourceTypeLUT[bindDesc.Type];
        Reflection->shader_resources[i].view_usages = gViewUsageLUT[bindDesc.Type];
        Reflection->shader_resources[i].set = bindDesc.Space;
        Reflection->shader_resources[i].binding = bindDesc.BindPoint;
        Reflection->shader_resources[i].size = bindDesc.BindCount;
        Reflection->shader_resources[i].stages = stage;
        Reflection->shader_resources[i].dim = gD3D12_TO_RESOURCE_DIM[bindDesc.Dimension];
        if (shaderDesc.ConstantBuffers && bindDesc.Type == D3D_SIT_CBUFFER)
        {
            ID3D12ShaderReflectionConstantBuffer* buffer = d3d12reflection->GetConstantBufferByName(bindDesc.Name);
            cgpu_assert(buffer && "D3D12 reflection fatal: CBV not found!");
            D3D12_SHADER_BUFFER_DESC bufferDesc;
            buffer->GetDesc(&bufferDesc);
            Reflection->shader_resources[i].size = bufferDesc.Size;
        }
        // RWTyped is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for RWBuffer here
        if (bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
        {
            Reflection->shader_resources[i].type = CGPU_RESOURCE_TYPE2_BUFFER;
            Reflection->shader_resources[i].view_usages = CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL;
        }
        // Buffer<> is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for Buffer<> here
        if (bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
        {
            Reflection->shader_resources[i].type = CGPU_RESOURCE_TYPE2_BUFFER;
            Reflection->shader_resources[i].view_usages = CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL;
        }
    }
}

template <typename ID3D12ReflectionT, typename D3D12_SHADER_DESC_T>
inline static void D3D12Util_CollectShaderReflectionData(ID3D12ReflectionT* pReflection, CGPUShaderLibrary_D3D12* S, uint32_t entryIndex)
{
    // Get a description of this shader
    D3D12_SHADER_DESC_T _shaderDesc;
    pReflection->GetDesc(&_shaderDesc);
    ECGPUShaderStage stage = D3D12Util_GetShaderStageFromDesc(_shaderDesc);
    reflectionRecordShaderResources(pReflection, stage, _shaderDesc, S, entryIndex);
    CGPUShaderReflection* Reflection = S->super.entry_reflections + entryIndex;
    Reflection->stage = stage;
    // Collect vertex inputs
    if (stage == CGPU_SHADER_STAGE_VERT)
    {
        if constexpr (std::is_same_v<D3D12_SHADER_DESC_T, D3D12_SHADER_DESC>)
        {
            ID3D12ShaderReflection* pShaderReflection = (ID3D12ShaderReflection*)pReflection;
            const D3D12_SHADER_DESC& shaderDesc = _shaderDesc;
            Reflection->vertex_inputs_count = shaderDesc.InputParameters;
            Reflection->vertex_inputs = (CGPUVertexInput*)cgpu_calloc(Reflection->vertex_inputs_count, sizeof(CGPUVertexInput));
            // Count the string sizes of the vertex inputs for the name pool
            for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
            {
                D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
                pShaderReflection->GetInputParameterDesc(i, &paramDesc);
                // Get the length of the semantic name
                bool hasParamIndex = paramDesc.SemanticIndex > 0 || !strcmp(paramDesc.SemanticName, "TEXCOORD");
                uint32_t source_len = (uint32_t)strlen(paramDesc.SemanticName) + (hasParamIndex ? 1 : 0);

                Reflection->vertex_inputs[i].name = (char8_t*)cgpu_malloc(sizeof(char8_t) * (source_len + 1));
                if (hasParamIndex)
                    sprintf((char*)Reflection->vertex_inputs[i].name, "%s%u", paramDesc.SemanticName, paramDesc.SemanticIndex);
                else
                    sprintf((char*)Reflection->vertex_inputs[i].name, "%s", paramDesc.SemanticName);
                const uint32_t Comps = (uint32_t)log2(paramDesc.Mask);
                Reflection->vertex_inputs[i].format = gD3D12_TO_VERTEX_FORMAT[paramDesc.ComponentType + 3 * Comps];
            }
        }
        else if constexpr (std::is_same_v<D3D12_SHADER_DESC_T, D3D12_FUNCTION_DESC>)
        {
            cgpu_assert(false && "D3D12 reflection fatal: Lib shader is not supported now!");
        }
        else
        {
            cgpu_assert(false && "D3D12 reflection fatal: Unknown shader desc type!");
        }
    }
    else if (stage == CGPU_SHADER_STAGE_COMPUTE)
    {
        if constexpr (std::is_same_v<D3D12_SHADER_DESC_T, D3D12_SHADER_DESC>)
        {
            pReflection->GetThreadGroupSize(
                &Reflection->thread_group_sizes[0],
                &Reflection->thread_group_sizes[1],
                &Reflection->thread_group_sizes[2]
            );
        }
        else if constexpr (std::is_same_v<D3D12_SHADER_DESC_T, D3D12_FUNCTION_DESC>)
        {
            cgpu_assert(false && "D3D12 reflection fatal: Lib shader is not supported now!");
        }
        else
        {
            cgpu_assert(false && "D3D12 reflection fatal: Unknown shader desc type!");
        }
    }
}

void D3D12Util_InitializeShaderReflection(CGPUDevice_D3D12* D, CGPUShaderLibrary_D3D12* S, const struct CGPUShaderLibraryDescriptor* desc)
{
    S->super.device = &D->super;

    IDxcContainerReflection* pReflection;
    auto procDxcCreateInstnace = D3D12Util_GetDxcCreateInstanceProc();
    SKR_ASSERT(procDxcCreateInstnace && "Failed to get dxc proc!");
    procDxcCreateInstnace(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection));
    pReflection->Load(S->pShaderBlob);

    UINT32 rdatIdx = 0;
    bool isLibrary = SUCCEEDED(pReflection->FindFirstPartKind(DFCC_RuntimeData, &rdatIdx));

    if (isLibrary)
    {
        UINT32 shaderIdx;
        bool hasDXIL = SUCCEEDED(pReflection->FindFirstPartKind(DFCC_ShaderStatistics, &shaderIdx));

        ID3D12LibraryReflection* pLibReflection = nullptr;
        CHECK_HRESULT(pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&pLibReflection)));

        D3D12_LIBRARY_DESC libDesc;
        CHECK_HRESULT(pLibReflection->GetDesc(&libDesc));

        S->super.entrys_count = libDesc.FunctionCount;
        S->super.entry_reflections = (CGPUShaderReflection*)cgpu_calloc(S->super.entrys_count, sizeof(CGPUShaderReflection));
        for (uint32_t i = 0; i < libDesc.FunctionCount; i++)
        {
            auto Entry = pLibReflection->GetFunctionByIndex(i);
            D3D12Util_CollectShaderReflectionData<ID3D12FunctionReflection, D3D12_FUNCTION_DESC>(Entry, S, i);
        }

        pLibReflection->Release();
    }
    else
    {
        UINT32 shaderIdx;
        bool hasDXIL = SUCCEEDED(pReflection->FindFirstPartKind(DFCC_ShaderStatistics, &shaderIdx));

        ID3D12ShaderReflection* pShaderReflection = nullptr;
        CHECK_HRESULT(pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&pShaderReflection)));

        S->super.entrys_count = 1;
        S->super.entry_reflections = (CGPUShaderReflection*)cgpu_calloc(S->super.entrys_count, sizeof(CGPUShaderReflection));
        D3D12Util_CollectShaderReflectionData<ID3D12ShaderReflection, D3D12_SHADER_DESC>(pShaderReflection, S, 0);

        pShaderReflection->Release();
    }

    pReflection->Release();
}

void D3D12Util_FreeShaderReflection(CGPUShaderLibrary_D3D12* S)
{
    if (S->super.entry_reflections)
    {
        for (uint32_t i = 0; i < S->super.entrys_count; i++)
        {
            CGPUShaderReflection* reflection = S->super.entry_reflections + i;
            if (reflection->entry_name != D3DShaderEntryName)
            {
                cgpu_free((void*)reflection->entry_name);
            }
            if (reflection->vertex_inputs)
            {
                for (uint32_t j = 0; j < reflection->vertex_inputs_count; j++)
                {
                    cgpu_free((void*)reflection->vertex_inputs[j].name);
                }
                cgpu_free(reflection->vertex_inputs);
            }
            if (reflection->shader_resources)
            {
                for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
                {
                    cgpu_free((void*)reflection->shader_resources[j].name);
                }
                cgpu_free(reflection->shader_resources);
            }
        }
    }
    cgpu_free(S->super.entry_reflections);
}

// Descriptor Heap
// Descriptor handles are stored in blocks, and occupancy is kept track of using the bits of a 32-bit unsigned integer.
// This means that each block contains 32 descriptor handles, one for each bit.
#define DESCRIPTOR_HEAP_BLOCK_SIZE 32

typedef struct D3D12Util_DescriptorHeap
{
    /// DX Heap
    ID3D12DescriptorHeap* pCurrentHeap;
    /// Lock for multi-threaded descriptor allocations
    SMutex* pMutex;
    ID3D12Device* pDevice;
    /// Start position in the heap
    D3D12_CPU_DESCRIPTOR_HANDLE mStartCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE mStartGpuHandle;
    // Bitmask to track free regions (set bit means occupied)
    uint32_t* pFlags;
    /// Description
    D3D12_DESCRIPTOR_HEAP_TYPE mType;
    uint32_t mNumDescriptors;
    /// Descriptor Increment Size
    uint32_t mDescriptorSize;
    // Usage
    uint32_t mUsedDescriptors;
} D3D12Util_DescriptorHeap;

void D3D12Util_CreateDescriptorHeap(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, struct D3D12Util_DescriptorHeap** ppDescHeap)
{
    uint32_t numDescriptors = pDesc->NumDescriptors;

    // Keep 32 aligned for easy remove
    numDescriptors = cgpu_round_up(numDescriptors, DESCRIPTOR_HEAP_BLOCK_SIZE);

    const size_t sizeInBytes = (numDescriptors / DESCRIPTOR_HEAP_BLOCK_SIZE) * sizeof(uint32_t);

    D3D12Util_DescriptorHeap* pHeap = (D3D12Util_DescriptorHeap*)cgpu_calloc(1, sizeof(*pHeap) + sizeInBytes);
    pHeap->pFlags = (uint32_t*)(pHeap + 1);
    pHeap->pDevice = pDevice;

#ifdef CGPU_THREAD_SAFETY
    pHeap->pMutex = (SMutex*)cgpu_calloc(1, sizeof(SMutex));
    skr_init_mutex(pHeap->pMutex);
#endif

    D3D12_DESCRIPTOR_HEAP_DESC desc = *pDesc;
    desc.NumDescriptors = numDescriptors;

    CHECK_HRESULT(pDevice->CreateDescriptorHeap(&desc, IID_ARGS(&pHeap->pCurrentHeap)));

    pHeap->mStartCpuHandle = pHeap->pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
    cgpu_assert(pHeap->mStartCpuHandle.ptr);
    if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        pHeap->mStartGpuHandle = pHeap->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
        cgpu_assert(pHeap->mStartGpuHandle.ptr);
    }
    pHeap->mNumDescriptors = desc.NumDescriptors;
    pHeap->mType = desc.Type;
    pHeap->mDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(pHeap->mType);

    *ppDescHeap = pHeap;
}

void D3D12Util_ResetDescriptorHeap(struct D3D12Util_DescriptorHeap* pHeap)
{
    memset(pHeap->pFlags, 0, (pHeap->mNumDescriptors / DESCRIPTOR_HEAP_BLOCK_SIZE) * sizeof(uint32_t));
    pHeap->mUsedDescriptors = 0;
}

void D3D12Util_FreeDescriptorHeap(D3D12Util_DescriptorHeap* pHeap)
{
    if (pHeap == nullptr) return;
    SAFE_RELEASE(pHeap->pCurrentHeap);

#ifdef CGPU_THREAD_SAFETY
    if (pHeap->pMutex)
    {
        skr_destroy_mutex(pHeap->pMutex);
        cgpu_free(pHeap->pMutex);
    }
#endif

    cgpu_free(pHeap);
}

// Internal helper for unlocked return
static void D3D12Util_ReturnDescriptorHandlesUnlocked(D3D12Util_DescriptorHeap* pHeap, DxDescriptorId handle, uint32_t count)
{
    if (handle == UINT32_MAX || !count)
    {
        return;
    }

    for (uint32_t id = handle; id < handle + count; ++id)
    {
        const uint32_t i = id / DESCRIPTOR_HEAP_BLOCK_SIZE;
        const uint32_t mask = ~(1 << (id % DESCRIPTOR_HEAP_BLOCK_SIZE));
        pHeap->pFlags[i] &= mask;
    }

    pHeap->mUsedDescriptors -= count;
}

DxDescriptorId D3D12Util_ConsumeDescriptorHandles(D3D12Util_DescriptorHeap* pHeap, uint32_t descriptorCount)
{
    if (!descriptorCount)
    {
        return D3D12_DESCRIPTOR_ID_NONE;
    }

#ifdef CGPU_THREAD_SAFETY
    SMutexLock lock(*pHeap->pMutex);
#endif

    uint32_t result = UINT32_MAX;
    uint32_t firstResult = UINT32_MAX;
    uint32_t foundCount = 0;

    // Scan for block with `descriptorCount` contiguous free descriptor handles
    for (uint32_t i = 0; i < pHeap->mNumDescriptors / DESCRIPTOR_HEAP_BLOCK_SIZE; ++i)
    {
        const uint32_t flag = pHeap->pFlags[i];
        if (UINT32_MAX == flag)
        {
            D3D12Util_ReturnDescriptorHandlesUnlocked(pHeap, firstResult, foundCount);
            foundCount = 0;
            result = UINT32_MAX;
            firstResult = UINT32_MAX;
            continue;
        }

        for (int32_t j = 0, mask = 1; j < DESCRIPTOR_HEAP_BLOCK_SIZE; ++j, mask <<= 1)
        {
            if (!(flag & mask))
            {
                pHeap->pFlags[i] |= mask;
                result = i * DESCRIPTOR_HEAP_BLOCK_SIZE + j;

                cgpu_assert(result != UINT32_MAX && "Out of descriptors");

                if (UINT32_MAX == firstResult)
                {
                    firstResult = result;
                }

                ++foundCount;
                ++pHeap->mUsedDescriptors;

                if (foundCount == descriptorCount)
                {
                    return firstResult;
                }
            }
            // Non contiguous. Start scanning again from this point
            else if (foundCount)
            {
                D3D12Util_ReturnDescriptorHandlesUnlocked(pHeap, firstResult, foundCount);
                foundCount = 0;
                result = UINT32_MAX;
                firstResult = UINT32_MAX;
            }
        }
    }

    cgpu_assert(result != UINT32_MAX && "Out of descriptors");
    return firstResult;
}

void D3D12Util_ReturnDescriptorHandles(struct D3D12Util_DescriptorHeap* pHeap, DxDescriptorId handle, uint32_t count)
{
#ifdef CGPU_THREAD_SAFETY
    SMutexLock lock(*pHeap->pMutex);
#endif
    D3D12Util_ReturnDescriptorHandlesUnlocked(pHeap, handle, count);
}

void D3D12Util_CopyDescriptorHandle(D3D12Util_DescriptorHeap* pSrcHeap, DxDescriptorId srcId, D3D12Util_DescriptorHeap* pDstHeap, uint32_t dstId)
{
    cgpu_assert(pSrcHeap->mType == pDstHeap->mType);
    // Use the helper functions to correctly convert descriptor IDs to CPU handles
    D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = D3D12Util_DescriptorIdToCpuHandle(pSrcHeap, srcId);
    D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = D3D12Util_DescriptorIdToCpuHandle(pDstHeap, dstId);
    pSrcHeap->pDevice->CopyDescriptorsSimple(1, dstHandle, srcHandle, pSrcHeap->mType);
}

ID3D12DescriptorHeap* D3D12Util_GetUnderlyingHeap(const D3D12Util_DescriptorHeap* pHeap)
{
    return pHeap->pCurrentHeap;
}

size_t D3D12Util_GetDescriptorSize(const struct D3D12Util_DescriptorHeap* pHeap)
{
    return pHeap->mDescriptorSize;
}

void D3D12Util_ConsumeSRV(CGPUDevice_D3D12* D, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, DxDescriptorId* pId)
{
    *pId = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1);
    D->pDxDevice->CreateShaderResourceView(pResource, pSrvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], *pId));
}

void D3D12Util_ConsumeUAV(CGPUDevice_D3D12* D, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc, DxDescriptorId* pId)
{
    *pId = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1);
    D->pDxDevice->CreateUnorderedAccessView(pResource, pCounterResource, pSrvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], *pId));
}

void D3D12Util_ConsumeCBV(CGPUDevice_D3D12* D, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pSrvDesc, DxDescriptorId* pId)
{
    *pId = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1);
    D->pDxDevice->CreateConstantBufferView(pSrvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], *pId));
}

void D3D12Util_ConsumeRTV(CGPUDevice_D3D12* D, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, DxDescriptorId* pId)
{
    *pId = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], 1);
    D->pDxDevice->CreateRenderTargetView(pResource, pRtvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], *pId));
}

void D3D12Util_ConsumeDSV(CGPUDevice_D3D12* D, ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, DxDescriptorId* pId)
{
    *pId = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], 1);
    D->pDxDevice->CreateDepthStencilView(pResource, pDsvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], *pId));
}

void D3D12Util_CreateCBVForBufferView(DxDescriptorId ID, const CGPUBufferViewDescriptor* desc)
{
    const CGPUBuffer_D3D12* B = (const CGPUBuffer_D3D12*)desc->buffer;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)B->super.device;
    CGPUBuffer_D3D12* buffer_res = (CGPUBuffer_D3D12*)desc->buffer;
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = buffer_res->mDxGpuAddress + desc->offset;
    cbvDesc.SizeInBytes = cgpu_round_up((uint32_t)desc->size, 256);
    D->pDxDevice->CreateConstantBufferView(&cbvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], ID));
}

void D3D12Util_CreateSRVForBufferView(DxDescriptorId ID, ECGPUViewUsage view_usage, const CGPUBufferViewDescriptor* desc)
{
    const CGPUBuffer_D3D12* B = (const CGPUBuffer_D3D12*)desc->buffer;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)B->super.device;
    const auto BufferViewSize = desc->size ? desc->size : (B->super.info->size - desc->offset);
    const auto BufferOffset = desc->offset;

    UINT ElementStride;
    D3D12_BUFFER_SRV_FLAGS Flags;
    DXGI_FORMAT Format;
    if (view_usage == CGPU_BUFFER_VIEW_USAGE_SRV_RAW)
    {
        ElementStride = sizeof(uint32_t);
        Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        Format = DXGI_FORMAT_R32_TYPELESS;
    }
    else if (view_usage == CGPU_BUFFER_VIEW_USAGE_SRV_STRUCTURED)
    {
        ElementStride = desc->structure.element_stride;
        Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        Format = DXGI_FORMAT_UNKNOWN;
    }
    else if (view_usage == CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL)
    {
        ElementStride = FormatUtil_BitSizeOfBlock(desc->texel.format) / 8u;
        Format = DXGIUtil_TranslatePixelFormat(desc->texel.format, false);
        Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        cgpu_assert(Format != DXGI_FORMAT_UNKNOWN);
    }

    const auto FirstElement = BufferOffset / ElementStride;
    const auto ElementCount = BufferViewSize / ElementStride;
    const auto Mod = BufferOffset % ElementStride;

    cgpu_assert((Mod == 0) && "Offset for structured view must aligns element_stride!");
    SKR_DECLARE_ZERO(D3D12_SHADER_RESOURCE_VIEW_DESC, srvDesc);
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = Format;

    srvDesc.Buffer.Flags = Flags;
    srvDesc.Buffer.FirstElement = FirstElement;
    srvDesc.Buffer.NumElements = (UINT)ElementCount;

    if (Format == DXGI_FORMAT_UNKNOWN)
        srvDesc.Buffer.StructureByteStride = ElementStride;
    else
        srvDesc.Buffer.StructureByteStride = 0;

    D->pDxDevice->CreateShaderResourceView(B->pDxResource, &srvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], ID));
}

void D3D12Util_CreateUAVForBufferView(DxDescriptorId ID, ECGPUViewUsage view_usage, const CGPUBufferViewDescriptor* desc)
{
    const CGPUBuffer_D3D12* B = (const CGPUBuffer_D3D12*)desc->buffer;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)B->super.device;
    const auto BufferViewSize = desc->size ? desc->size : (B->super.info->size - desc->offset);
    const auto BufferOffset = desc->offset;

    UINT ElementStride;
    D3D12_BUFFER_UAV_FLAGS Flags;
    DXGI_FORMAT Format;
    if (view_usage == CGPU_BUFFER_VIEW_USAGE_UAV_RAW)
    {
        ElementStride = sizeof(uint32_t);
        Flags = D3D12_BUFFER_UAV_FLAG_RAW;
        Format = DXGI_FORMAT_R32_TYPELESS;
    }
    else if (view_usage == CGPU_BUFFER_VIEW_USAGE_UAV_STRUCTURED)
    {
        ElementStride = desc->structure.element_stride;
        Flags = D3D12_BUFFER_UAV_FLAG_NONE;
        Format = DXGI_FORMAT_UNKNOWN;
    }
    else if (view_usage == CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL)
    {
        ElementStride = FormatUtil_BitSizeOfBlock(desc->texel.format) / 8u;
        Format = DXGIUtil_TranslatePixelFormat(desc->texel.format, false);
        Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    }

    const auto FirstElement = BufferOffset / ElementStride;
    const auto ElementCount = BufferViewSize / ElementStride;
    const auto Mod = BufferOffset % ElementStride;

    cgpu_assert((Mod == 0) && "Offset for structured view must aligns element_stride!");
    SKR_DECLARE_ZERO(D3D12_UNORDERED_ACCESS_VIEW_DESC, uavDesc);
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = Format;

    uavDesc.Buffer.Flags = Flags;
    uavDesc.Buffer.FirstElement = FirstElement;
    uavDesc.Buffer.NumElements = (UINT)ElementCount;

    if (Format == DXGI_FORMAT_UNKNOWN)
        uavDesc.Buffer.StructureByteStride = ElementStride;
    else
        uavDesc.Buffer.StructureByteStride = 0;

    D->pDxDevice->CreateUnorderedAccessView(B->pDxResource, CGPU_NULLPTR, &uavDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], ID));
}

void D3D12Util_CreateSRVForTextureView(DxDescriptorId ID, const CGPUTextureViewDescriptor* desc)
{
    CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)desc->texture;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)T->super.device;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, true);
    // TODO: SUPPORT RGBA COMPONENT VIEW MAPPING SWIZZLE
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    switch (desc->dims)
    {
    case CGPU_TEXTURE_DIMENSION_1D: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        srvDesc.Texture1D.MipLevels = desc->mip_level_count;
        srvDesc.Texture1D.MostDetailedMip = desc->base_mip_level;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_1D_ARRAY: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        srvDesc.Texture1DArray.MipLevels = desc->mip_level_count;
        srvDesc.Texture1DArray.MostDetailedMip = desc->base_mip_level;
        srvDesc.Texture1DArray.FirstArraySlice = desc->base_array_layer;
        srvDesc.Texture1DArray.ArraySize = desc->array_layer_count;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_2DMS: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_2D: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc->mip_level_count;
        srvDesc.Texture2D.MostDetailedMip = desc->base_mip_level;
        srvDesc.Texture2D.PlaneSlice = 0;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_2DMS_ARRAY: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        srvDesc.Texture2DMSArray.ArraySize = desc->array_layer_count;
        srvDesc.Texture2DMSArray.FirstArraySlice = desc->base_array_layer;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_2D_ARRAY: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MipLevels = desc->mip_level_count;
        srvDesc.Texture2DArray.MostDetailedMip = desc->base_mip_level;
        srvDesc.Texture2DArray.PlaneSlice = 0;
        srvDesc.Texture2DArray.FirstArraySlice = desc->base_array_layer;
        srvDesc.Texture2DArray.ArraySize = desc->array_layer_count;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_3D: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = desc->mip_level_count;
        srvDesc.Texture3D.MostDetailedMip = desc->base_mip_level;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_CUBE: {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = desc->mip_level_count;
        srvDesc.TextureCube.MostDetailedMip = desc->base_mip_level;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_CUBE_ARRAY: {
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

    D->pDxDevice->CreateShaderResourceView(T->pDxResource, &srvDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], ID));
}

void D3D12Util_CreateUAVForTextureView(DxDescriptorId ID, const CGPUTextureViewDescriptor* desc)
{
    CGPUTexture_D3D12* T = (CGPUTexture_D3D12*)desc->texture;
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)T->super.device;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = (DXGI_FORMAT)DXGIUtil_TranslatePixelFormat(desc->format, true);
    cgpu_assert(desc->mip_level_count <= 1 && "UAV must be created with non-multi mip slices!");
    switch (desc->dims)
    {
    case CGPU_TEXTURE_DIMENSION_1D: {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        uavDesc.Texture1D.MipSlice = desc->base_mip_level;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_1D_ARRAY: {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        uavDesc.Texture1DArray.MipSlice = desc->base_mip_level;
        uavDesc.Texture1DArray.FirstArraySlice = desc->base_array_layer;
        uavDesc.Texture1DArray.ArraySize = desc->array_layer_count;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_2D: {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = desc->base_mip_level;
        uavDesc.Texture2D.PlaneSlice = 0;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_2D_ARRAY: {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice = desc->base_mip_level;
        uavDesc.Texture2DArray.PlaneSlice = 0;
        uavDesc.Texture2DArray.FirstArraySlice = desc->base_array_layer;
        uavDesc.Texture2DArray.ArraySize = desc->array_layer_count;
    }
    break;
    case CGPU_TEXTURE_DIMENSION_3D: {
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

    D->pDxDevice->CreateUnorderedAccessView(T->pDxResource, CGPU_NULLPTR, &uavDesc, D3D12Util_DescriptorIdToCpuHandle(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], ID));
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Util_DescriptorIdToCpuHandle(D3D12Util_DescriptorHeap* pHeap, DxDescriptorId index)
{
    cgpu_assert(index < pHeap->mNumDescriptors);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = pHeap->mStartCpuHandle;
    handle.ptr += (SIZE_T)index * pHeap->mDescriptorSize;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12Util_DescriptorIdToGpuHandle(D3D12Util_DescriptorHeap* pHeap, DxDescriptorId index)
{
    cgpu_assert(index < pHeap->mNumDescriptors);
    D3D12_GPU_DESCRIPTOR_HANDLE handle = pHeap->mStartGpuHandle;
    handle.ptr += index * pHeap->mDescriptorSize;
    return handle;
}