#include "cgpu/drivers/cgpu_nvapi.h"
#include "cgpu/drivers/cgpu_ags.h"
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include <dxcapi.h>
#include <d3d12shader.h>
#include "D3D12MemAlloc.h"
#include "utils/make_zeroed.hpp"
#include <EASTL/vector.h>
#include <winsock.h>

#define USE_PIX
#include "cgpu/drivers/WinPixEventRuntime/pix3.h"
#pragma comment(lib, "WinPixEventRuntime.lib")

UINT64 encode_color_for_pix(const float* color)
{
    if (color == nullptr)
        return PIX_COLOR(0, 0, 0);

    return PIX_COLOR(
    static_cast<BYTE>(color[0] * 255.f),
    static_cast<BYTE>(color[1] * 255.f),
    static_cast<BYTE>(color[2] * 255.f));
}
void cgpu_cmd_begin_event_d3d12(CGPUCommandBufferId cmd, const CGPUEventInfo* event)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    const auto eventColor = encode_color_for_pix(event->color);
    PIXBeginEvent(Cmd->pDxCmdList, eventColor, event->name);
}

void cgpu_cmd_set_marker_d3d12(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    const auto markerColor = encode_color_for_pix(marker->color);
    PIXSetMarker(Cmd->pDxCmdList, markerColor, marker->name);
}

void cgpu_cmd_end_event_d3d12(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)cmd;
    PIXEndEvent(Cmd->pDxCmdList);
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
        cgpu_warn("D3D12 GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
    }
}

void D3D12Util_QueryAllAdapters(CGPUInstance_D3D12* instance, uint32_t* count, bool* foundSoftwareAdapter)
{
    cgpu_assert(instance->pAdapters == nullptr && "getProperGpuCount should be called only once!");
    cgpu_assert(instance->mAdaptersCount == 0 && "getProperGpuCount should be called only once!");
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
                    DECLARE_ZERO(CGPUAdapter_D3D12, cgpuAdapter)
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
        DXGI_FORMAT fmt = DXGIUtil_TranslatePixelFormat((ECGPUFormat)i);
        if (fmt == DXGI_FORMAT_UNKNOWN) continue;

        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { fmt };
        pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));

        adapter_detail->format_supports[i].shader_read = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) != 0;
        adapter_detail->format_supports[i].shader_write = (formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0;
        adapter_detail->format_supports[i].render_target_write = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)  != 0;
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
    DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc3)
    adapter.pDxActiveGPU->GetDesc3(&desc3);
    vendor_preset.device_id = desc3.DeviceId;
    vendor_preset.vendor_id = desc3.VendorId;
    _bstr_t b(desc3.Description);
    char* str = b;
    strncpy(adapter_detail.vendor_preset.gpu_name, str, MAX_GPU_VENDOR_STRING_LENGTH);
    // Get Driver Version
    if (I->super.nvapi_status == CGPU_NVAPI_OK)
        vendor_preset.driver_version = cgpu_nvapi_get_driver_version();
    else if (I->super.ags_status == CGPU_AGS_SUCCESS)
        vendor_preset.driver_version = cgpu_ags_get_driver_version();
    else
        vendor_preset.driver_version = 0;
    // Create a device for feature query
    ID3D12Device* pCheckDevice = nullptr;
    if (!SUCCEEDED(D3D12CreateDevice(adapter.pDxActiveGPU, adapter.mFeatureLevel, IID_PPV_ARGS(&pCheckDevice))))
    {
        cgpu_assert("[D3D12 Fatal]: Create D3D12Device Failed When Query Adapter Features!");
    }
    // Architecture features
    DECLARE_ZERO(D3D12_FEATURE_DATA_ARCHITECTURE1, dxgi_feature)
    pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &dxgi_feature, sizeof(dxgi_feature));
    adapter_detail.is_uma = dxgi_feature.UMA;
    adapter_detail.is_cpu = desc3.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
    adapter_detail.is_virtual = false;
    // Constants
    adapter_detail.max_vertex_input_bindings = 32u;
    adapter_detail.uniform_buffer_alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    adapter_detail.upload_buffer_texture_alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
    adapter_detail.upload_buffer_texture_row_alignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
    // Check Format Capacities
    D3D12Util_EnumFormatSupports(D3D12Adapter, pCheckDevice);
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1 = make_zeroed<D3D12_FEATURE_DATA_D3D12_OPTIONS1>();
	if (SUCCEEDED(pCheckDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1))))
    {
        adapter_detail.wave_lane_count = options1.WaveLaneCountMin;
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
#ifdef NVAPI
    if (I->super.nvapi_status == CGPU_NVAPI_OK)
    {
        adapter_detail.host_visible_vram_budget = cgpu_nvapi_d3d12_query_cpu_visible_vram(pCheckDevice);
        adapter_detail.support_host_visible_vram = adapter_detail.host_visible_vram_budget > 0;
    }
#endif
#ifdef AMDAGS
#endif
    SAFE_RELEASE(pCheckDevice);
}

void D3D12Util_CreateDMAAllocator(CGPUInstance_D3D12* I, CGPUAdapter_D3D12* A, CGPUDevice_D3D12* D)
{
    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    desc.pDevice = D->pDxDevice;
    desc.pAdapter = A->pDxActiveGPU;

    D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
    allocationCallbacks.pAllocate = +[](size_t size, size_t alignment, void*) {
        return cgpu_memalign(size, alignment);
    };
    allocationCallbacks.pFree = +[](void* ptr, void*) { cgpu_free_aligned(ptr); };
    desc.pAllocationCallbacks = &allocationCallbacks;
    desc.Flags |= D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;
    if (!SUCCEEDED(D3D12MA::CreateAllocator(&desc, &D->pResourceAllocator)))
    {
        cgpu_assert(0 && "DMA Allocator Create Failed!");
    }
}

void D3D12Util_SignalFence(CGPUQueue_D3D12* Q, ID3D12Fence* DxF, uint64_t fenceValue)
{
    Q->pCommandQueue->Signal(DxF, fenceValue);
}

// Shader Reflection
const char8_t* D3DShaderEntryName = "FuckD3D";
static ECGPUResourceType gD3D12_TO_DESCRIPTOR[] = {
    CGPU_RESOURCE_TYPE_UNIFORM_BUFFER, // D3D_SIT_CBUFFER
    CGPU_RESOURCE_TYPE_BUFFER,         // D3D_SIT_TBUFFER
    CGPU_RESOURCE_TYPE_TEXTURE,        // D3D_SIT_TEXTURE
    CGPU_RESOURCE_TYPE_SAMPLER,        // D3D_SIT_SAMPLER
    CGPU_RESOURCE_TYPE_RW_TEXTURE,     // D3D_SIT_UAV_RWTYPED
    CGPU_RESOURCE_TYPE_BUFFER,         // D3D_SIT_STRUCTURED
    CGPU_RESOURCE_TYPE_RW_BUFFER,      // D3D_SIT_RWSTRUCTURED
    CGPU_RESOURCE_TYPE_BUFFER,         // D3D_SIT_BYTEADDRESS
    CGPU_RESOURCE_TYPE_RW_BUFFER,      // D3D_SIT_UAV_RWBYTEADDRESS
    CGPU_RESOURCE_TYPE_RW_BUFFER,      // D3D_SIT_UAV_APPEND_STRUCTURED
    CGPU_RESOURCE_TYPE_RW_BUFFER,      // D3D_SIT_UAV_CONSUME_STRUCTURED
    CGPU_RESOURCE_TYPE_RW_BUFFER,      // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
    CGPU_RESOURCE_TYPE_RAY_TRACING,    // D3D_SIT_RTACCELERATIONSTRUCTURE
};

static ECGPUTextureDimension gD3D12_TO_RESOURCE_DIM[D3D_SRV_DIMENSION_BUFFEREX + 1] = {
    CGPU_TEX_DIMENSION_UNDEFINED,  // D3D_SRV_DIMENSION_UNKNOWN
    CGPU_TEX_DIMENSION_UNDEFINED,  // D3D_SRV_DIMENSION_BUFFER
    CGPU_TEX_DIMENSION_1D,         // D3D_SRV_DIMENSION_TEXTURE1D
    CGPU_TEX_DIMENSION_1D_ARRAY,   // D3D_SRV_DIMENSION_TEXTURE1DARRAY
    CGPU_TEX_DIMENSION_2D,         // D3D_SRV_DIMENSION_TEXTURE2D
    CGPU_TEX_DIMENSION_2D_ARRAY,   // D3D_SRV_DIMENSION_TEXTURE2DARRAY
    CGPU_TEX_DIMENSION_2DMS,       // D3D_SRV_DIMENSION_TEXTURE2DMS
    CGPU_TEX_DIMENSION_2DMS_ARRAY, // D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
    CGPU_TEX_DIMENSION_3D,         // D3D_SRV_DIMENSION_TEXTURE3D
    CGPU_TEX_DIMENSION_CUBE,       // D3D_SRV_DIMENSION_TEXTURECUBE
    CGPU_TEX_DIMENSION_CUBE_ARRAY, // D3D_SRV_DIMENSION_TEXTURECUBEARRAY
    CGPU_TEX_DIMENSION_UNDEFINED,  // D3D_SRV_DIMENSION_BUFFEREX
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

template <typename ID3D12ReflectionT, typename D3D12_SHADER_DESC_T>
void reflectionRecordShaderResources(ID3D12ReflectionT* d3d12reflection, ECGPUShaderStage stage, const D3D12_SHADER_DESC_T& shaderDesc, CGPUShaderLibrary_D3D12* S)
{
    // Get the number of bound resources
    S->super.entrys_count = 1;
    S->super.entry_reflections = (CGPUShaderReflection*)cgpu_calloc(S->super.entrys_count, sizeof(CGPUShaderReflection));
    CGPUShaderReflection* Reflection = S->super.entry_reflections;
    Reflection->entry_name = D3DShaderEntryName;
    Reflection->shader_resources_count = shaderDesc.BoundResources;
    Reflection->shader_resources = (CGPUShaderResource*)cgpu_calloc(shaderDesc.BoundResources, sizeof(CGPUShaderResource));

    // Count string sizes of the bound resources for the name pool
    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc;
        d3d12reflection->GetResourceBindingDesc(i, &bindDesc);
        const size_t source_len = strlen(bindDesc.Name);
        Reflection->shader_resources[i].name = (char8_t*)cgpu_malloc(sizeof(char8_t) * (source_len + 1));
        Reflection->shader_resources[i].name_hash = cgpu_hash(bindDesc.Name, strlen(bindDesc.Name), *(size_t*)&S->super.device);
        // We are very sure it's windows platform
        strcpy_s((char8_t*)Reflection->shader_resources[i].name, source_len + 1, bindDesc.Name);
        Reflection->shader_resources[i].type = gD3D12_TO_DESCRIPTOR[bindDesc.Type];
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
            Reflection->shader_resources[i].type = CGPU_RESOURCE_TYPE_RW_BUFFER;
        }
        // Buffer<> is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for Buffer<> here
        if (bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
        {
            Reflection->shader_resources[i].type = CGPU_RESOURCE_TYPE_BUFFER;
        }
    }
}

FORCEINLINE void D3D12Util_CollectShaderReflectionData(ID3D12ShaderReflection* d3d12reflection, ECGPUShaderStage stage, CGPUShaderLibrary_D3D12* S)
{
    // Get a description of this shader
    D3D12_SHADER_DESC shaderDesc;
    d3d12reflection->GetDesc(&shaderDesc);
    reflectionRecordShaderResources(d3d12reflection, stage, shaderDesc, S);
    CGPUShaderReflection* Reflection = S->super.entry_reflections;
    Reflection->stage = stage;
    // Collect vertex inputs
    if (stage == CGPU_SHADER_STAGE_VERT)
    {
        Reflection->vertex_inputs_count = shaderDesc.InputParameters;
        Reflection->vertex_inputs = (CGPUVertexInput*)cgpu_calloc(Reflection->vertex_inputs_count, sizeof(CGPUVertexInput));
        // Count the string sizes of the vertex inputs for the name pool
        for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
        {
            D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
            d3d12reflection->GetInputParameterDesc(i, &paramDesc);
            // Get the length of the semantic name
            bool hasParamIndex = paramDesc.SemanticIndex > 0 || !strcmp(paramDesc.SemanticName, "TEXCOORD");
            uint32_t source_len = (uint32_t)strlen(paramDesc.SemanticName) + (hasParamIndex ? 1 : 0);

            Reflection->vertex_inputs[i].name = (char8_t*)cgpu_malloc(sizeof(char8_t) * (source_len + 1));
            if (hasParamIndex)
                sprintf((char8_t*)Reflection->vertex_inputs[i].name, "%s%u", paramDesc.SemanticName, paramDesc.SemanticIndex);
            else
                sprintf((char8_t*)Reflection->vertex_inputs[i].name, "%s", paramDesc.SemanticName);
            const uint32_t Comps = (uint32_t)log2(paramDesc.Mask);
            Reflection->vertex_inputs[i].format = gD3D12_TO_VERTEX_FORMAT[paramDesc.ComponentType + 3 * Comps];
        }
    }
    else if (stage == CGPU_SHADER_STAGE_COMPUTE)
    {
        d3d12reflection->GetThreadGroupSize(
        &Reflection->thread_group_sizes[0],
        &Reflection->thread_group_sizes[1],
        &Reflection->thread_group_sizes[2]);
    }
}

void D3D12Util_InitializeShaderReflection(CGPUDevice_D3D12* D, CGPUShaderLibrary_D3D12* S, const struct CGPUShaderLibraryDescriptor* desc)
{
    S->super.device = &D->super;
    ID3D12ShaderReflection* d3d12reflection = nullptr;
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) \
    ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 | (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)

    IDxcContainerReflection* pReflection;
    UINT32 shaderIdx;
    DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection));
    pReflection->Load(S->pShaderBlob);
    (pReflection->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'), &shaderIdx));

    // TODO: Support RTX Shaders
    CHECK_HRESULT(pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&d3d12reflection)));
    if (d3d12reflection)
        D3D12Util_CollectShaderReflectionData(d3d12reflection, desc->stage, S);

    pReflection->Release();
    d3d12reflection->Release();
}

void D3D12Util_FreeShaderReflection(CGPUShaderLibrary_D3D12* S)
{
    if (S->super.entry_reflections)
    {
        for (uint32_t i = 0; i < S->super.entrys_count; i++)
        {
            CGPUShaderReflection* reflection = S->super.entry_reflections + i;
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
