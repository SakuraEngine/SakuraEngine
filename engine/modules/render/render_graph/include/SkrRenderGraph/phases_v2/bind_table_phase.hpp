#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "pass_info_analysis.hpp"
#include "resource_allocation_phase.hpp"

namespace skr {
namespace RG {

// Bind table information for a single pass
struct PassBindTableInfo {
    PassNode* pass = nullptr;
    CGPURootSignatureId root_signature = nullptr;
    CGPUXBindTableId bind_table = nullptr;
    uint32_t bound_texture_count = 0;
    uint32_t bound_buffer_count = 0;
    uint32_t bound_acceleration_structure_count = 0;
};

// Bind table phase result
struct BindTableResult {
    StackMap<PassNode*, PassBindTableInfo> pass_bind_tables;
    uint32_t total_bind_tables_created = 0;
    uint32_t total_texture_views_created = 0;
    uint32_t total_buffer_views_created = 0;
};

// Bind table phase for managing bind tables for each Pass
class SKR_RENDER_GRAPH_API BindTablePhase : public IRenderGraphPhase {
public:
    BindTablePhase(
        const PassInfoAnalysis& pass_info_analysis,
        const ResourceAllocationPhase& resource_allocation_phase);
    ~BindTablePhase() override;
    
    // IRenderGraphPhase interface
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;
    
    // Results access
    const BindTableResult& get_result() const { return bind_table_result_; }
    const PassBindTableInfo* get_pass_bind_table_info(PassNode* pass) const;
    CGPUXBindTableId get_pass_bind_table(PassNode* pass) const;
    
private:
    CGPUXBindTableId create_bind_table_for_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, PassNode* pass, CGPURootSignatureId root_sig) SKR_NOEXCEPT;
    
    // Input phase references
    const PassInfoAnalysis& pass_info_analysis_;
    const ResourceAllocationPhase& resource_allocation_phase_;
    
    BindTableResult bind_table_result_;
};

} // namespace RG
} // namespace skr