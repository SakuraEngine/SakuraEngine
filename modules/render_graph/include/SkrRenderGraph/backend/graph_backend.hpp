#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "texture_pool.hpp"
#include "buffer_pool.hpp"
#include "texture_view_pool.hpp"
#include "bind_table_pool.hpp"

#include "cgpu/extensions/cgpu_marker_buffer.h"

namespace skr
{
namespace render_graph
{
class RenderGraphFrameExecutor
{
public:
    friend class RenderGraphBackend;

    RenderGraphFrameExecutor() = default;
    void initialize(CGPUQueueId gfx_queue, CGPUDeviceId device);
    void finalize();

    const struct CGPUXMergedBindTable* merge_tables(const struct CGPUXBindTable **tables, uint32_t count);

    void commit(CGPUQueueId gfx_queue, uint64_t frame_index);
    void reset_begin(TextureViewPool& texture_view_pool);

    void write_marker(const char* message);
    void print_error_trace(uint64_t frame_index);

    CGPUCommandPoolId gfx_cmd_pool = nullptr;
    CGPUCommandBufferId gfx_cmd_buf = nullptr;
    CGPUFenceId exec_fence = nullptr;
    uint64_t exec_frame = 0;
    eastl::vector<CGPUTextureId> aliasing_textures;
    skr::flat_hash_map<CGPURootSignatureId, BindTablePool*> bind_table_pools;

    CGPUMarkerBufferId marker_buffer = nullptr;
    uint32_t marker_idx = 0;
    uint32_t valid_marker_val = 1;
    eastl::vector<skr::string> marker_messages;
protected:
    skr::flat_hash_map<CGPURootSignatureId, MergedBindTablePool*> merged_table_pools;
};

// TODO: optimize stack allocation
// static constexpr size_t stack_vector_fixed_count = 8;
template <typename T>
using stack_vector = eastl::vector<T>;

class RenderGraphBackend : public RenderGraph
{
public:
    void devirtualize(TextureNode* node);
    void devirtualize(PassNode* node);

    virtual uint64_t execute(RenderGraphProfiler* profiler = nullptr) SKR_NOEXCEPT final;
    virtual CGPUDeviceId get_backend_device() SKR_NOEXCEPT final;
    inline virtual CGPUQueueId get_gfx_queue() SKR_NOEXCEPT final { return gfx_queue; }
    virtual uint32_t collect_garbage(uint64_t critical_frame,
        uint32_t tex_with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t tex_without_tags = 0,
        uint32_t buf_with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t buf_without_tags = 0) SKR_NOEXCEPT final;
    virtual uint32_t collect_texture_garbage(uint64_t critical_frame,
        uint32_t with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t without_tags = 0) SKR_NOEXCEPT final;
    virtual uint32_t collect_buffer_garbage(uint64_t critical_frame,
        uint32_t with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t without_tags = 0) SKR_NOEXCEPT final;

    friend class RenderGraph;

    RenderGraphBackend(const RenderGraphBuilder& builder);
    ~RenderGraphBackend() = default;
protected:
    virtual void initialize() SKR_NOEXCEPT final;
    virtual void finalize() SKR_NOEXCEPT final;

    CGPUTextureId resolve(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT;
    CGPUTextureId try_aliasing_allocate(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT;
    CGPUBufferId resolve(RenderGraphFrameExecutor& executor, const BufferNode& node) SKR_NOEXCEPT;

    void calculate_barriers(RenderGraphFrameExecutor& executor, PassNode* pass,
        stack_vector<CGPUTextureBarrier>& tex_barriers, stack_vector<eastl::pair<TextureHandle, CGPUTextureId>>& resolved_textures,
        stack_vector<CGPUBufferBarrier>& buf_barriers, stack_vector<eastl::pair<BufferHandle, CGPUBufferId>>& resolved_buffers) SKR_NOEXCEPT;
    CGPUXBindTableId alloc_update_pass_bind_table(RenderGraphFrameExecutor& executor, PassNode* pass) SKR_NOEXCEPT;
    void deallocate_resources(PassNode* pass) SKR_NOEXCEPT;

    void execute_compute_pass(RenderGraphFrameExecutor& executor, ComputePassNode* pass) SKR_NOEXCEPT;
    void execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass) SKR_NOEXCEPT;
    void execute_copy_pass(RenderGraphFrameExecutor& executor, CopyPassNode* pass) SKR_NOEXCEPT;
    void execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass) SKR_NOEXCEPT;

    uint64_t get_latest_finished_frame() SKR_NOEXCEPT;

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