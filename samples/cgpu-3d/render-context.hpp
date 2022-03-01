#pragma once
#include "render-device.hpp"

// D3D11-CreateContext
class GfxContext
{
    friend class RenderDevice;

public:
    FORCEINLINE RenderDevice* GetRenderDevice() { return device_; }
    void Initialize(RenderDevice* device);
    void Destroy();

    void Begin();
    void ResourceBarrier(const CGpuResourceBarrierDescriptor& barrier_desc);
    void AcquireResources(AsyncRenderTexture* const* textures, uint32_t textures_count,
        AsyncRenderBuffer* const* buffers, uint32_t buffers_count);
    void End();
    void Wait();

protected:
    // fence
    CGpuFenceId exec_fence_;
    // cmds
    CGpuCommandPoolId cmd_pool_;
    CGpuCommandBufferId cmd_buffer_;
    // render device
    RenderDevice* device_;
};

class RenderContext : public GfxContext
{
    friend class RenderDevice;

public:
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

protected:
    // context states
    CGpuRootSignatureId current_root_sig_;
    // encoders
    CGpuRenderPassEncoderId rp_encoder_;
    // cached states
    CGpuRenderPipelineId cached_pipeline = nullptr;
    CGpuDescriptorSetId cached_descset = nullptr;
};
