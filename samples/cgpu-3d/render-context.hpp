#pragma once
#include "render-device.hpp"

// D3D11-CreateContext
class RenderContext
{
    friend class RenderDevice;

public:
    void Initialize(RenderDevice* device);
    void Destroy();
    FORCEINLINE RenderDevice* GetRenderDevice() { return device_; }
    void Begin();
    void ResourceBarrier(const CGpuResourceBarrierDescriptor& barrier_desc);

    void BeginRenderPass(const CGpuRenderPassDescriptor& rp_desc);
    void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth);
    void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void BindPipeline(CGpuRenderPipelineId pipeline);
    void BindVertexBuffers(uint32_t buffer_count, const CGpuBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
    void BindIndexBuffer(CGpuBufferId buffer, uint32_t index_stride, uint64_t offset);
    void BindDescriptorSet(CGpuDescriptorSetId desc_set);
    void PushConstants(CGpuRootSignatureId rs, const char8_t* name, const void* data);
    void DrawIndexedInstanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
    void EndRenderPass();
    void End();

    void Wait();

protected:
    // context states
    CGpuRootSignatureId current_root_sig_;
    // cmds
    CGpuCommandPoolId cmd_pool_;
    CGpuCommandBufferId cmd_buffer_;
    CGpuFenceId exec_fence_;
    // encoders
    CGpuRenderPassEncoderId rp_encoder_;
    // render device
    RenderDevice* device_;
};
