#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "SkrRenderGraph/stack_allocator.hpp"

// Forward declare types from dependency analysis
namespace skr
{
namespace render_graph
{

// Resource access type
enum class EResourceAccessType : uint32_t
{
    Read,
    Write,
    ReadWrite
};

// Resource access info - detailed for dependency analysis
struct ResourceAccessInfo {
    PassNode* pass = nullptr;
    ResourceNode* resource = nullptr;
    EResourceAccessType access_type;
    ECGPUResourceState resource_state = CGPU_RESOURCE_STATE_UNDEFINED;
    // Texture subresource range (optional)
    uint32_t mip_base = 0;
    uint32_t mip_count = 0;
    uint32_t array_base = 0;
    uint32_t array_count = 0;
    // Buffer range (optional)
    uint64_t buffer_from = 0;
    uint64_t buffer_to = 0;
};

// Resource info - direct extraction with detailed access info
struct PassResourceInfo {
    StackVector<ResourceAccessInfo> resource_accesses; // For dependency analysis
    uint32_t total_resource_count = 0;
};

// Performance info - from hints only
struct PassPerformanceInfo {
    bool is_compute_intensive = false;    // ComputeIntensive flag
    bool is_bandwidth_intensive = false;  // BandwidthIntensive flag
    bool is_vertex_bound = false;         // VertexBoundIntensive flag
    bool is_pixel_bound = false;          // PixelBoundIntensive flag
    bool has_small_working_set = false;   // SmallWorkingSet flag
    bool has_large_working_set = false;   // LargeWorkingSet flag
    bool has_random_access = false;       // RandomAccess flag
    bool has_streaming_access = false;    // StreamingAccess flag
    bool prefers_async_compute = false;   // PreferAsyncCompute flag
    bool separate_command_buffer = false; // SeperateFromCommandBuffer flag
};

// Pass info container
struct PassInfo {
    PassNode* pass = nullptr;
    EPassType pass_type = EPassType::Render;
    PassResourceInfo resource_info;
    PassPerformanceInfo performance_info;
};

struct ResourceInfo {
    ResourceNode* resource = nullptr;
    uint64_t memory_size = 0;
    skr::InlineSet<ECGPUQueueType, 3> access_queues;
    StackMap<PassNode*, ECGPUResourceState> used_states;
};

// Analysis phase - runs before DependencyAnalysis
class SKR_RENDER_GRAPH_API PassInfoAnalysis : public IRenderGraphPhase
{
public:
    PassInfoAnalysis() = default;
    ~PassInfoAnalysis() override = default;

    // IRenderGraphPhase interface
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;

    // Query interface
    const PassInfo* get_pass_info(PassNode* pass) const;
    const ResourceInfo* get_resource_info(ResourceNode* resource) const;
    const PassResourceInfo* get_resource_info(PassNode* pass) const;
    const PassPerformanceInfo* get_performance_info(PassNode* pass) const;

    // For dependency analysis - avoid recomputation
    EResourceAccessType get_resource_access_type(PassNode* pass, ResourceNode* resource) const;
    ECGPUResourceState get_resource_state(PassNode* pass, ResourceNode* resource) const;
    
    // 资源状态重路由查询接口
    bool resource_needs_rerouting(ResourceNode* resource) const;
    
private:
    void extract_pass_info(PassNode* pass);
    void extract_resource_info(PassNode* pass, PassResourceInfo& info);
    void extract_performance_info(PassNode* pass, PassPerformanceInfo& info);

private:
    StackHashMap<PassNode*, PassInfo> pass_infos;
    StackHashMap<ResourceNode*, ResourceInfo> resource_infos; // For dependency analysis
};

} // namespace render_graph
} // namespace skr