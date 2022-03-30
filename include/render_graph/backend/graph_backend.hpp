#pragma once
#include "render_graph/frontend/render_graph.hpp"
#include "texture_pool.hpp"
#include "texture_view_pool.hpp"

#define MAX_FRAME_IN_FLIGHT 3

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
    void initialize(CGpuQueueId gfx_queue, CGpuDeviceId deivce)
    {
        CGpuCommandPoolDescriptor pool_desc = {};
        gfx_cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
        CGpuCommandBufferDescriptor cmd_desc = {};
        cmd_desc.is_secondary = false;
        gfx_cmd_buf = cgpu_create_command_buffer(gfx_cmd_pool, &cmd_desc);
    }
    void finalize()
    {
        if (gfx_cmd_buf) cgpu_free_command_buffer(gfx_cmd_buf);
        if (gfx_cmd_pool) cgpu_free_command_pool(gfx_cmd_pool);
        gfx_cmd_buf = nullptr;
        gfx_cmd_pool = nullptr;
    }
    CGpuCommandPoolId gfx_cmd_pool = nullptr;
    CGpuCommandBufferId gfx_cmd_buf = nullptr;
};

class RenderGraphBackend : public RenderGraph
{
public:
    void devirtualize(TextureNode* node);
    void devirtualize(PassNode* node);

    virtual uint64_t execute() final;

    friend class RenderGraph;

protected:
    RenderGraphBackend(CGpuQueueId gfx_queue, CGpuDeviceId device)
        : gfx_queue(gfx_queue)
        , device(device)
    {
    }
    virtual void initialize() final;
    virtual void finalize() final;

    CGpuTextureId resolve(const TextureNode& node);

    void execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass);
    void execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass);

    CGpuQueueId gfx_queue;
    CGpuDeviceId device;
    ECGpuBackend backend;
    RenderGraphFrameExecutor executors[MAX_FRAME_IN_FLIGHT];
    TexturePool texture_pool;
    TextureViewPool texture_view_pool;
};
} // namespace render_graph
} // namespace sakura