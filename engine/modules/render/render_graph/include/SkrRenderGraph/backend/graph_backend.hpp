#pragma once
#include "SkrContainersDef/set.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/backend/buffer_pool.hpp"
#include "SkrRenderGraph/backend/buffer_view_pool.hpp"
#include "SkrRenderGraph/backend/texture_pool.hpp"
#include "SkrRenderGraph/backend/texture_view_pool.hpp"
#include "SkrRenderGraph/backend/bind_table_pool.hpp"

namespace skr::RG
{
struct PassInfoAnalysis;
struct QueueSchedule;
struct CrossQueueSyncAnalysis;
struct BarrierGenerationPhase;
struct MemoryAliasingPhase;
struct ResourceLifetimeAnalysis;

class RenderGraphFrameExecutor
{
public:
    friend class RenderGraphBackend;

    RenderGraphFrameExecutor() = default;
    void initialize(CGPUQueueId gfx_queue, CGPUDeviceId device);
    void finalize();

    const struct CGPUXMergedBindTable* merge_tables(const struct CGPUXBindTable** tables, uint32_t count);

    void commit(CGPUQueueId gfx_queue, uint64_t frame_index);
    void reset_begin();

    void write_marker(const char8_t* message);
    void print_error_trace(uint64_t frame_index);

    CGPUCommandPoolId gfx_cmd_pool = nullptr;
    CGPUCommandBufferId gfx_cmd_buf = nullptr;
    CGPUFenceId exec_fence = nullptr;
    uint64_t exec_frame = 0;
    skr::FlatHashMap<CGPURootSignatureId, BindTablePool*> bind_table_pools;

    CGPUBufferId marker_buffer = nullptr;
    uint32_t marker_idx = 0;
    uint32_t valid_marker_val = 1;
    skr::Vector<skr::String> marker_messages;

protected:
    skr::FlatHashMap<CGPURootSignatureId, MergedBindTablePool*> merged_table_pools;
};

// TODO: optimize stack allocation
static constexpr size_t stack_vector_fixed_count = 8;
template <typename T>
using stack_vector = skr::InlineVector<T, stack_vector_fixed_count>;
template <typename T>
using stack_set = skr::InlineSet<T, stack_vector_fixed_count>;

class SKR_RENDER_GRAPH_API RenderGraphBackend : public RenderGraph
{
    friend struct BindablePassContext;
    friend struct PassExecutionPhase;

public:
    void wait_frame(uint64_t frame_index) SKR_NOEXCEPT final;
    bool check_frame(uint64_t frame_index) SKR_NOEXCEPT final;
    virtual uint64_t execute(RenderGraphProfiler* profiler = nullptr) SKR_NOEXCEPT final;
    virtual CGPUDeviceId get_backend_device() SKR_NOEXCEPT final;
    inline virtual CGPUQueueId get_gfx_queue() SKR_NOEXCEPT final { return gfx_queue; }
    inline virtual const skr::Vector<CGPUQueueId>& get_cmpt_queues() SKR_NOEXCEPT final { return cmpt_queues; }
    inline virtual const skr::Vector<CGPUQueueId>& get_cpy_queues() SKR_NOEXCEPT final { return cpy_queues; }
    virtual uint32_t collect_garbage(uint64_t critical_frame,
        uint32_t tex_with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag,
        uint32_t tex_without_tags = 0,
        uint32_t buf_with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag,
        uint32_t buf_without_tags = 0) SKR_NOEXCEPT final;
    virtual uint32_t collect_texture_garbage(uint64_t critical_frame,
        uint32_t with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag,
        uint32_t without_tags = 0) SKR_NOEXCEPT final;
    virtual uint32_t collect_buffer_garbage(uint64_t critical_frame,
        uint32_t with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag,
        uint32_t without_tags = 0) SKR_NOEXCEPT final;

    friend class RenderGraph;

    RenderGraphBackend(const RenderGraphBuilder& builder);
    ~RenderGraphBackend() = default;

    // Public interface for phases
    uint64_t get_latest_finished_frame() SKR_NOEXCEPT;
    
    BufferPool& get_buffer_pool() SKR_NOEXCEPT { return buffer_pool; }
    TexturePool& get_texture_pool() SKR_NOEXCEPT { return texture_pool; }
    TextureViewPool& get_texture_view_pool() SKR_NOEXCEPT { return texture_view_pool; }
    BufferViewPool& get_buffer_view_pool() SKR_NOEXCEPT { return buffer_view_pool; }

protected:
    virtual void initialize() SKR_NOEXCEPT final;
    virtual void finalize() SKR_NOEXCEPT final;

    ECGPUBackend backend;
    CGPUDeviceId device;
    CGPUQueueId gfx_queue;
    skr::Vector<CGPUQueueId> cmpt_queues;
    skr::Vector<CGPUQueueId> cpy_queues;
    RenderGraphFrameExecutor executors[RG_MAX_FRAME_IN_FLIGHT];
    BufferPool buffer_pool;
    BufferViewPool buffer_view_pool;
    TexturePool texture_pool;
    TextureViewPool texture_view_pool;
};
} // namespace skr::RG