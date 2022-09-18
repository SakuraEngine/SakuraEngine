#include "cgpu/extensions/cgpu_marker_buffer.h"
#include "cgpu/extensions/cgpu_d3d12_exts.h"
#include <d3d12.h>

void cgpu_marker_buffer_write_d3d12(CGPUCommandBufferId cmd, CGPUMarkerBufferId buffer, uint32_t index, uint32_t value)
{
    ID3D12GraphicsCommandList2* pCommandList2;
    ID3D12GraphicsCommandList* pCommandList = cgpu_d3d12_get_command_list(cmd);
    pCommandList->QueryInterface(IID_PPV_ARGS(&pCommandList2));
    ID3D12Resource* pBuffer = cgpu_d3d12_get_buffer(buffer->cgpu_buffer);

    D3D12_WRITEBUFFERIMMEDIATE_PARAMETER param = { pBuffer->GetGPUVirtualAddress() + index * sizeof(UINT), value };
    D3D12_WRITEBUFFERIMMEDIATE_MODE mode = D3D12_WRITEBUFFERIMMEDIATE_MODE_MARKER_OUT;
    pCommandList2->WriteBufferImmediate(1, &param, &mode);
    pCommandList2->Release();
}