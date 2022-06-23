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
    void ResourceBarrier(const CGPUResourceBarrierDescriptor& barrier_desc);
    void AcquireResources(AsyncRenderTexture* const* textures, uint32_t textures_count,
    AsyncRenderBuffer* const* buffers, uint32_t buffers_count);
    void End();
    void Wait();

protected:
    // fence
    CGPUFenceId exec_fence_;
    // cmds
    CGPUCommandPoolId cmd_pool_;
    CGPUCommandBufferId cmd_buffer_;
    // render device
    RenderDevice* device_;
};

class RenderContext : public GfxContext
{
    friend class RenderDevice;

public:
    void BeginRenderPass(const CGPURenderPassDescriptor& rp_desc);
    void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth);
    void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void BindPipeline(CGPURenderPipelineId pipeline);
    void BindVertexBuffers(uint32_t buffer_count, const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets);
    void BindIndexBuffer(CGPUBufferId buffer, uint32_t index_stride, uint64_t offset);
    void BindDescriptorSet(CGPUDescriptorSetId desc_set);
    void PushConstants(CGPURootSignatureId rs, const char8_t* name, const void* data);
    void DrawIndexedInstanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex);
    void EndRenderPass();

protected:
    // context states
    CGPURootSignatureId current_root_sig_;
    // encoders
    CGPURenderPassEncoderId rp_encoder_;
    // cached states
    CGPURenderPipelineId cached_pipeline = nullptr;
    CGPUDescriptorSetId cached_descset = nullptr;
};
