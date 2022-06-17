#pragma once
#include "render_graph/frontend/render_graph.hpp"
#include "texture_pool.hpp"
#include "buffer_pool.hpp"
#include "texture_view_pool.hpp"
#include "desc_set_heap.hpp"

namespace skr
{
namespace render_graph
{
class RenderGraphFrameExecutor
{
public:
    friend class RenderGraphBackend;

    RenderGraphFrameExecutor() = default;
    void initialize(CGPUQueueId gfx_queue, CGPUDeviceId device)
    {
        CGPUCommandPoolDescriptor pool_desc = {};
        gfx_cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
        CGPUCommandBufferDescriptor cmd_desc = {};
        cmd_desc.is_secondary = false;
        gfx_cmd_buf = cgpu_create_command_buffer(gfx_cmd_pool, &cmd_desc);
        exec_fence = cgpu_create_fence(device);
    }
    void commit(CGPUQueueId gfx_queue)
    {
        CGPUQueueSubmitDescriptor submit_desc = {};
        submit_desc.cmds = &gfx_cmd_buf;
        submit_desc.cmds_count = 1;
        submit_desc.signal_fence = exec_fence;
        cgpu_submit_queue(gfx_queue, &submit_desc);
    }
    void reset_begin(TextureViewPool& texture_view_pool)
    {
        for (auto desc_heap : desc_set_pool)
        {
            desc_heap.second->reset();
        }
        for (auto aliasing_texture : aliasing_textures)
        {
            texture_view_pool.erase(aliasing_texture);
            cgpu_free_texture(aliasing_texture);
        }
        aliasing_textures.clear();
        cgpu_reset_command_pool(gfx_cmd_pool);
        cgpu_cmd_begin(gfx_cmd_buf);
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
    CGPUCommandPoolId gfx_cmd_pool = nullptr;
    CGPUCommandBufferId gfx_cmd_buf = nullptr;
    CGPUFenceId exec_fence = nullptr;
    eastl::vector<CGPUTextureId> aliasing_textures;
    eastl::unordered_map<CGPURootSignatureId, DescSetHeap*> desc_set_pool;
};

class RenderGraphBackend : public RenderGraph
{
public:
    void devirtualize(TextureNode* node);
    void devirtualize(PassNode* node);

    virtual uint64_t execute(RenderGraphProfiler* profiler = nullptr) SKR_NOEXCEPT final;
    virtual CGPUDeviceId get_backend_device() SKR_NOEXCEPT final;
    inline virtual CGPUQueueId get_gfx_queue() SKR_NOEXCEPT final { return gfx_queue; }
    virtual uint32_t collect_garbage(uint64_t critical_frame) SKR_NOEXCEPT final;
    virtual uint32_t collect_texture_garbage(uint64_t critical_frame) SKR_NOEXCEPT final;
    virtual uint32_t collect_buffer_garbage(uint64_t critical_frame) SKR_NOEXCEPT final;

    friend class RenderGraph;

protected:
    RenderGraphBackend(const RenderGraphBuilder& builder);
    virtual void initialize() SKR_NOEXCEPT final;
    virtual void finalize() SKR_NOEXCEPT final;

    CGPUTextureId resolve(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT;
    CGPUTextureId try_aliasing_allocate(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT;
    CGPUBufferId resolve(RenderGraphFrameExecutor& executor, const BufferNode& node) SKR_NOEXCEPT;

    void calculate_barriers(RenderGraphFrameExecutor& executor, PassNode* pass,
    eastl::vector<CGPUTextureBarrier>& tex_barriers, eastl::vector<eastl::pair<TextureHandle, CGPUTextureId>>& resolved_textures,
    eastl::vector<CGPUBufferBarrier>& buf_barriers, eastl::vector<eastl::pair<BufferHandle, CGPUBufferId>>& resolved_buffers) SKR_NOEXCEPT;
    gsl::span<CGPUDescriptorSetId> alloc_update_pass_descsets(RenderGraphFrameExecutor& executor, PassNode* pass) SKR_NOEXCEPT;
    void deallocate_resources(PassNode* pass) SKR_NOEXCEPT;

    void execute_compute_pass(RenderGraphFrameExecutor& executor, ComputePassNode* pass) SKR_NOEXCEPT;
    void execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass) SKR_NOEXCEPT;
    void execute_copy_pass(RenderGraphFrameExecutor& executor, CopyPassNode* pass) SKR_NOEXCEPT;
    void execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass) SKR_NOEXCEPT;

    CGPUQueueId gfx_queue;
    CGPUDeviceId device;
    ECGPUBackend backend;
    RenderGraphFrameExecutor executors[RG_MAX_FRAME_IN_FLIGHT];
    TexturePool texture_pool;
    BufferPool buffer_pool;
    TextureViewPool texture_view_pool;
};
} // namespace render_graph
} // namespace skr