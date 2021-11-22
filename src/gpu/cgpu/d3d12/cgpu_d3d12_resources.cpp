#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/d3d12_bridge.h"
#include <dxcapi.h>

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
    pUtils->CreateBlobWithEncodingOnHeapCopy(desc->code, (uint32_t)desc->code_size, DXC_CP_ACP, &S->pShaderBlob);
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

// Buffer APIs
CGpuBufferId cgpu_create_buffer_d3d12(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc)
{
    CGpuBuffer_D3D12* B = new CGpuBuffer_D3D12();

    return &B->super;
}

void cgpu_free_buffer_d3d12(CGpuBufferId buffer)
{
    CGpuBuffer_D3D12* B = (CGpuBuffer_D3D12*)buffer;

    delete B;
}