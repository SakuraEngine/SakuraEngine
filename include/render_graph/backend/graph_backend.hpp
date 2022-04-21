#pragma once
#include "render_graph/frontend/render_graph.hpp"
#include "texture_pool.hpp"
#include "buffer_pool.hpp"
#include "texture_view_pool.hpp"
#include "desc_set_heap.hpp"

namespace sakura
{
namespace render_graph
{
class RenderGraphFrameExecutor
{
public:
    friend class RenderGraphBackend;

protected:
    RenderGraphFrameExecutor() = default;
    void initialize(CGpuQueueId gfx_queue, CGpuDeviceId device)
    {
        CGpuCommandPoolDescriptor pool_desc = {};
        gfx_cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
        CGpuCommandBufferDescriptor cmd_desc = {};
        cmd_desc.is_secondary = false;
        gfx_cmd_buf = cgpu_create_command_buffer(gfx_cmd_pool, &cmd_desc);
        exec_fence = cgpu_create_fence(device);
    }
    void finalize()
    {
        if (gfx_cmd_buf) cgpu_free_command_buffer(gfx_cmd_buf);
        if (gfx_cmd_pool) cgpu_free_command_pool(gfx_cmd_pool);
        if (exec_fence) cgpu_free_fence(exec_fence);
        gfx_cmd_buf = nullptr;
        gfx_cmd_pool = nullptr;
        exec_fence = nullptr;
        for (auto desc_set_heap : desc_set_pool)
        {
            desc_set_heap.second->destroy();
        }
        for (auto aliasing_tex : aliasing_textures)
        {
            cgpu_free_texture(aliasing_tex);
        }
    }
    CGpuCommandPoolId gfx_cmd_pool = nullptr;
    CGpuCommandBufferId gfx_cmd_buf = nullptr;
    CGpuFenceId exec_fence = nullptr;
    eastl::vector<CGpuTextureId> aliasing_textures;
    eastl::unordered_map<CGpuRootSignatureId, DescSetHeap*> desc_set_pool;
};

class RenderGraphBackend : public RenderGraph
{
public:
    void devirtualize(TextureNode* node);
    void devirtualize(PassNode* node);

    virtual uint64_t execute() final;
    virtual CGpuDeviceId get_backend_device() final;
    inline virtual CGpuQueueId get_gfx_queue() final { return gfx_queue; }
    virtual uint32_t collect_garbage(uint64_t critical_frame) final;
    virtual uint32_t collect_texture_garbage(uint64_t critical_frame) final;
    virtual uint32_t collect_buffer_garbage(uint64_t critical_frame) final;

    friend class RenderGraph;

protected:
    RenderGraphBackend(CGpuQueueId gfx_queue, CGpuDeviceId device)
        : gfx_queue(gfx_queue)
        , device(device)
    {
    }
    virtual void initialize() final;
    virtual void finalize() final;

    CGpuTextureId resolve(RenderGraphFrameExecutor& executor, const TextureNode& node);
    CGpuTextureId try_aliasing_allocate(RenderGraphFrameExecutor& executor, const TextureNode& node);
    CGpuBufferId resolve(RenderGraphFrameExecutor& executor, const BufferNode& node);

    void calculate_barriers(RenderGraphFrameExecutor& executor, PassNode* pass,
        eastl::vector<CGpuTextureBarrier>& tex_barriers, eastl::vector<eastl::pair<TextureHandle, CGpuTextureId>>& resolved_textures,
        eastl::vector<CGpuBufferBarrier>& buf_barriers, eastl::vector<eastl::pair<BufferHandle, CGpuBufferId>>& resolved_buffers);
    gsl::span<CGpuDescriptorSetId> alloc_update_pass_descsets(RenderGraphFrameExecutor& executor, PassNode* pass);
    void deallocate_resources(PassNode* pass);

    void execute_compute_pass(RenderGraphFrameExecutor& executor, ComputePassNode* pass);
    void execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass);
    void execute_copy_pass(RenderGraphFrameExecutor& executor, CopyPassNode* pass);
    void execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass);

    CGpuQueueId gfx_queue;
    CGpuDeviceId device;
    ECGpuBackend backend;
    RenderGraphFrameExecutor executors[RG_MAX_FRAME_IN_FLIGHT];
    TexturePool texture_pool;
    BufferPool buffer_pool;
    TextureViewPool texture_view_pool;
};
} // namespace render_graph
} // namespace sakura