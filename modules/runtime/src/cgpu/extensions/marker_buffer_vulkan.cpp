#include "cgpu/extensions/cgpu_marker_buffer.h"
#include "cgpu/extensions/cgpu_vulkan_exts.h"

void cgpu_marker_buffer_write_vulkan(CGPUCommandBufferId cmd, CGPUMarkerBufferId buffer, uint32_t index, uint32_t value)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUBuffer_Vulkan* Buffer = (CGPUBuffer_Vulkan*)buffer->cgpu_buffer;
    CGPUDevice_Vulkan* Device = (CGPUDevice_Vulkan*)buffer->cgpu_buffer->device;
    auto write_fn = Device->mVkDeviceTable.vkCmdFillBuffer;
    if (write_fn)
    {
        write_fn(Cmd->pVkCmdBuf, Buffer->pVkBuffer, index * sizeof(uint32_t), sizeof(uint32_t), value);
    }
}