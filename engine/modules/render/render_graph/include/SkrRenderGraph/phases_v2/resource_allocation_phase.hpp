#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "SkrRenderGraph/backend/texture_pool.hpp"
#include "SkrRenderGraph/backend/texture_view_pool.hpp"
#include "SkrRenderGraph/backend/buffer_pool.hpp"
#include "memory_aliasing_phase.hpp"
#include "pass_info_analysis.hpp"
#include "SkrContainersDef/hashmap.hpp"

namespace skr {
namespace RG {

template <typename T>
struct AllocatedReosurce
{
    T v;
    ECGPUResourceState init_state;
};

// 资源分配结果
struct ResourceAllocationResult
{
    // ResourceNode到实际GPU资源的映射
    StackMap<uint32_t, AllocatedReosurce<CGPUTextureId>> bucket_id_to_textures;
    StackMap<uint32_t, AllocatedReosurce<CGPUBufferId>> bucket_id_to_buffers;
    
    // 统计信息
    uint64_t total_allocated_memory = 0;
    uint32_t total_textures_created = 0;
    uint32_t total_buffers_created = 0;
};

// 资源分配配置
struct ResourceAllocationConfig
{
    bool enable_debug_output = false;
    bool defer_resource_creation = false; // 是否延迟创建资源
};

// 资源分配Phase - 负责实际的GPU资源创建和池化
class SKR_RENDER_GRAPH_API ResourceAllocationPhase : public IRenderGraphPhase
{
public:
    ResourceAllocationPhase(
        const MemoryAliasingPhase& aliasing_phase,
        const PassInfoAnalysis& pass_info_analysis,
        const ResourceAllocationConfig& config = {});
    ~ResourceAllocationPhase() override;

    // IRenderGraphPhase 接口
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;

    // 查询接口
    const ResourceAllocationResult& get_result() const { return allocation_result_; }
    
    // 通用资源获取接口（通过resource_redirects查找）
    CGPUTextureId get_resource(TextureNode* texture, ECGPUResourceState* pOutState = nullptr) const;
    CGPUBufferId get_resource(BufferNode* buffer, ECGPUResourceState* pOutState = nullptr) const;
    const MemoryAliasingPhase& get_aliasing_phase() const { return aliasing_phase_; }

    // 统计查询
    uint64_t get_allocated_memory_size() const { return allocation_result_.total_allocated_memory; }
    uint32_t get_texture_count() const { return allocation_result_.total_textures_created; }
    uint32_t get_buffer_count() const { return allocation_result_.total_buffers_created; }

private:
    // 核心分配方法
    void release_resources_to_pool(TexturePool& tpool, BufferPool& bpool) SKR_NOEXCEPT;
    void allocate_pooled_resources(RenderGraph* graph) SKR_NOEXCEPT;
    void create_texture_from_pool(TexturePool& tpool, uint64_t bucket_id, TextureNode* texture) SKR_NOEXCEPT;
    void create_buffer_from_pool(BufferPool& bpool, uint64_t bucket_id, BufferNode* buffer) SKR_NOEXCEPT;

private:
    // 配置
    ResourceAllocationConfig config_;
    
    // 输入Phase引用
    const MemoryAliasingPhase& aliasing_phase_;
    const PassInfoAnalysis& pass_info_analysis_;
    
    // 分配结果
    ResourceAllocationResult allocation_result_;
    
    // 渲染设备
    CGPUDeviceId device_ = nullptr;
    RenderGraphBackend* graph = nullptr;
};

} // namespace RG
} // namespace skr