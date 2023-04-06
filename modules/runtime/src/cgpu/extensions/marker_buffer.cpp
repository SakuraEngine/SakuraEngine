#include "platform/memory.h"
#include "cgpu/extensions/cgpu_marker_buffer.h"

#ifdef CGPU_USE_D3D12
extern void cgpu_marker_buffer_write_d3d12(CGPUCommandBufferId cmd, CGPUMarkerBufferId buffer, uint32_t index, uint32_t value);
#endif
#ifdef CGPU_USE_VULKAN
extern void cgpu_marker_buffer_write_vulkan(CGPUCommandBufferId cmd, CGPUMarkerBufferId buffer, uint32_t index, uint32_t value);
#endif

bool cgpu_device_support_marker_buffer(CGPUDeviceId device)
{
    // AMD    ID3D12GraphicsCommandList2 & vkCmdWriteBufferMarkerAMD
    // NVIDIA ID3D12GraphicsCommandList2 & vkCmdFillBuffer
    // Intel  ID3D12GraphicsCommandList2 & vkCmdFillBuffer
    const auto amd = cgpux_adapter_is_amd(device->adapter);(void)amd;
    const auto backend = device->adapter->instance->backend;
    return (backend == CGPU_BACKEND_D3D12) || (backend == CGPU_BACKEND_VULKAN);
}

CGPUMarkerBufferId cgpu_create_marker_buffer(CGPUDeviceId device, CGPUMarkerBufferDescriptor const* descriptor)
{   
    CGPUMarkerBuffer* buffer = SkrNew<CGPUMarkerBuffer>();
    CGPUBufferDescriptor buf_desc = {};
    buf_desc.name = u8"MarkerBuffer";
    buf_desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
    buf_desc.memory_usage = CGPU_MEM_USAGE_GPU_TO_CPU;
    buf_desc.size = (descriptor->marker_count + 3) / 4 * 4 * sizeof(uint32_t);
    buffer->cgpu_buffer = cgpu_create_buffer(device, &buf_desc);
    buffer->marker_count = descriptor->marker_count;
    buffer->used_count = 0;
    return buffer;
}

void cgpu_marker_buffer_write(CGPUCommandBufferId cmd, CGPUMarkerBufferId buffer, uint32_t index, uint32_t value)
{
    if (!buffer) return;
    const auto backend = buffer->cgpu_buffer->device->adapter->instance->backend;
#ifdef CGPU_USE_D3D12
    if (backend == CGPU_BACKEND_D3D12)
    {
        cgpu_marker_buffer_write_d3d12(cmd, buffer, index, value);
    }
#endif   
#ifdef CGPU_USE_VULKAN
    if (backend == CGPU_BACKEND_VULKAN)
    {
        cgpu_marker_buffer_write_vulkan(cmd, buffer, index, value);
    }
#endif    
}

void cgpu_free_marker_buffer(CGPUMarkerBufferId buffer)
{
    cgpu_free_buffer(buffer->cgpu_buffer);
    SkrDelete(buffer);
}