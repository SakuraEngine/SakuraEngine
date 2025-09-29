#include <fstream>
#include <sstream>
#include <iomanip>

#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/phases_v2/pass_info_analysis.hpp"
#include "SkrRenderGraph/phases_v2/pass_dependency_analysis.hpp"
#include "SkrRenderGraph/phases_v2/queue_schedule.hpp"
#include "SkrRenderGraph/phases_v2/resource_lifetime_analysis.hpp"
#include "SkrRenderGraph/phases_v2/cross_queue_sync_analysis.hpp"
#include "SkrRenderGraph/phases_v2/memory_aliasing_phase.hpp"
#include "SkrRenderGraph/phases_v2/barrier_generation_phase.hpp"

namespace skr::RG
{

void GraphViz::generate_graphviz_visualization(
    skr::RG::RenderGraph* graph,
    const skr::RG::PassInfoAnalysis& info_analysis,
    const skr::RG::QueueSchedule& queue_schedule,
    const skr::RG::CrossQueueSyncAnalysis& ssis_phase,
    const skr::RG::BarrierGenerationPhase& barrier_phase,
    const skr::RG::MemoryAliasingPhase& aliasing_phase,
    const skr::RG::ResourceLifetimeAnalysis& lifetime_analysis)
{
    using namespace skr::RG;

    std::stringstream dot;

    // Graphviz 头部
    dot << "digraph RenderGraphExecution {\n";
    dot << "  rankdir=TB;\n";
    dot << "  compound=true;\n";
    dot << "  fontname=\"Arial\";\n";
    dot << "  node [shape=box, style=\"rounded,filled\", fontname=\"Arial\"];\n";
    dot << "  edge [fontname=\"Arial\", fontsize=10];\n\n";

    const auto& schedule_result = queue_schedule.get_schedule_result();
    const auto& aliasing_result = aliasing_phase.get_result();
    const auto& lifetime_result = lifetime_analysis.get_result();

    // 定义通用的 find_pass_position lambda
    auto find_pass_position = [&](PassNode* pass) -> std::pair<uint32_t, uint32_t> {
        for (uint32_t q = 0; q < schedule_result.queue_schedules.size(); ++q)
        {
            const auto& queue_schedule = schedule_result.queue_schedules[q];
            for (uint32_t i = 0; i < queue_schedule.size(); ++i)
            {
                if (queue_schedule[i] == pass)
                    return { q, i };
            }
        }
        return { UINT32_MAX, UINT32_MAX };
    };

    // 队列颜色定义
    const char* queue_colors[] = { "#FFE6E6", "#E6F3FF", "#E6FFE6" };
    const char* queue_names[] = { "Graphics Queue", "Compute Queue", "Copy Queue" };

    // 1. 生成队列和Pass节点
    for (uint32_t q = 0; q < schedule_result.queue_schedules.size(); ++q)
    {
        const auto& queue_schedule = schedule_result.queue_schedules[q];
        if (queue_schedule.is_empty()) continue;

        // 队列子图
        dot << "  subgraph cluster_queue_" << q << " {\n";
        dot << "    label=\"" << queue_names[std::min(q, 2u)] << "\";\n";
        dot << "    style=filled;\n";
        dot << "    fillcolor=\"" << queue_colors[std::min(q, 2u)] << "\";\n";
        dot << "    node [fillcolor=white];\n\n";

        // Pass节点
        for (uint32_t i = 0; i < queue_schedule.size(); ++i)
        {
            auto* pass = queue_schedule[i];

            // 根据Pass类型选择不同的节点样式
            const char* shape = "box";
            const char* color = "white";
            if (pass->pass_type == EPassType::Compute)
            {
                shape = "ellipse";
                color = "#E8F5E9"; // Light green for compute
            }
            else if (pass->pass_type == EPassType::Copy)
            {
                shape = "octagon";
                color = "#FFF3E0"; // Light orange for copy
            }
            else if (pass->pass_type == EPassType::Present)
            {
                shape = "diamond";
                color = "#F3E5F5"; // Light purple for present
            }

            dot << "    pass_q" << q << "_" << i
                << " [label=\"" << (const char*)pass->get_name()
                << "\\nOrder: " << i
                << "\", shape=" << shape
                << ", fillcolor=\"" << color << "\"];\n";
        }

        // 队列内执行顺序边
        for (uint32_t i = 1; i < queue_schedule.size(); ++i)
        {
            dot << "    pass_q" << q << "_" << (i - 1)
                << " -> pass_q" << q << "_" << i
                << " [style=solid, color=gray];\n";
        }

        dot << "  }\n\n";
    }

    // 2. 生成内存别名转换边
    dot << "  // Memory aliasing transitions\n";
    for (const auto& transition : aliasing_result.alias_transitions)
    {
        if (!transition.to_resource || !transition.transition_pass)
            continue;

        // 找到旧资源的最后使用Pass
        PassNode* from_pass = nullptr;
        if (transition.from_resource)
        {
            auto resource_info = info_analysis.get_resource_info(transition.from_resource);
            from_pass = resource_info->used_states.at_last().key;
        }

        auto [to_q, to_i] = find_pass_position(transition.transition_pass);

        if (from_pass && to_q != UINT32_MAX)
        {
            auto [from_q, from_i] = find_pass_position(from_pass);
            if (from_q != UINT32_MAX)
            {
                std::string label = "Alias: ";
                label += (const char*)transition.from_resource->get_name();
                label += " → ";
                label += (const char*)transition.to_resource->get_name();
                label += "\\nBucket " + std::to_string(transition.bucket_index);

                dot << "  pass_q" << from_q << "_" << from_i
                    << " -> pass_q" << to_q << "_" << to_i
                    << " [style=dashed, color=purple, penwidth=2, label=\"" << label
                    << "\", fontcolor=purple];\n";
            }
        }
    }

    // 3. 生成跨队列同步边
    dot << "\n  // Cross-queue synchronization\n";
    const auto& ssis_result = ssis_phase.get_ssis_result();
    for (const auto& sync : ssis_result.optimized_sync_points)
    {
        if (!sync.producer_pass || !sync.consumer_pass)
            continue;

        auto [prod_q, prod_i] = find_pass_position(sync.producer_pass);
        auto [cons_q, cons_i] = find_pass_position(sync.consumer_pass);

        if (prod_q != UINT32_MAX && cons_q != UINT32_MAX)
        {
            std::string label = "Sync";
            if (sync.resource)
            {
                label += ": ";
                label += (const char*)sync.resource->get_name();
            }

            dot << "  pass_q" << prod_q << "_" << prod_i
                << " -> pass_q" << cons_q << "_" << cons_i
                << " [style=dashed, color=red, penwidth=2, label=\"" << label
                << "\", fontcolor=red];\n";
        }
    }

    // 4. 生成资源状态转换边 (ResourceTransition barriers)
    dot << "\n  // Resource state transitions\n";
    const auto& barrier_result = barrier_phase.get_result();

    // Track processed transitions to avoid duplicates
    skr::FlatHashSet<std::string> processed_transitions;

    // Debug: Count barriers
    int resource_transition_count = 0;
    int skipped_null_pass = 0;
    int skipped_duplicate = 0;
    int skipped_invalid_position = 0;
    int generated_edges = 0;

    // Iterate through pass barrier batches instead of removed all_barriers
    for (const auto& [pass, batches] : barrier_result.pass_barrier_batches)
    {
        for (const auto& batch : batches)
        {
            for (const auto& barrier : batch.barriers)
            {
                if (barrier.type != EBarrierType::ResourceTransition)
                    continue;

                resource_transition_count++;

                if (!barrier.source_pass || !barrier.target_pass || !barrier.resource)
                {
                    skipped_null_pass++;
                    SKR_LOG_DEBUG(u8"Skipping ResourceTransition: source_pass=%p, target_pass=%p, resource=%p",
                        barrier.source_pass,
                        barrier.target_pass,
                        barrier.resource);
                    continue;
                }

                // Create unique key for this transition, include Begin/End suffix for split barriers
                std::string transition_key = std::to_string(reinterpret_cast<uintptr_t>(barrier.source_pass)) + "_" +
                    std::to_string(reinterpret_cast<uintptr_t>(barrier.target_pass)) + "_" +
                    std::to_string(reinterpret_cast<uintptr_t>(barrier.resource));

                // Add suffix for Begin/End barriers to avoid duplicate key conflicts
                if (barrier.transition.is_begin)
                    transition_key += "_begin";
                else if (barrier.transition.is_end)
                    transition_key += "_end";

                if (processed_transitions.contains(transition_key))
                {
                    skipped_duplicate++;
                    continue;
                }
                processed_transitions.insert(transition_key);

                auto [src_q, src_i] = find_pass_position(barrier.source_pass);
                auto [tgt_q, tgt_i] = find_pass_position(barrier.target_pass);

                if (src_q == UINT32_MAX || tgt_q == UINT32_MAX)
                {
                    skipped_invalid_position++;
                    SKR_LOG_DEBUG(u8"Invalid position for passes: %s -> %s (q:%u,%u i:%u,%u)",
                        barrier.source_pass ? barrier.source_pass->get_name() : u8"null",
                        barrier.target_pass ? barrier.target_pass->get_name() : u8"null",
                        src_q,
                        tgt_q,
                        src_i,
                        tgt_i);
                    continue;
                }

                if (src_q != UINT32_MAX && tgt_q != UINT32_MAX)
                {
                    generated_edges++;
                    // Format state names
                    auto format_state = [](ECGPUResourceState state) -> const char* {
                        switch (state)
                        {
                        case CGPU_RESOURCE_STATE_UNDEFINED:
                            return "Undefined";
                        case CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER:
                            return "VB/CB";
                        case CGPU_RESOURCE_STATE_INDEX_BUFFER:
                            return "IB";
                        case CGPU_RESOURCE_STATE_RENDER_TARGET:
                            return "RT";
                        case CGPU_RESOURCE_STATE_UNORDERED_ACCESS:
                            return "UAV";
                        case CGPU_RESOURCE_STATE_DEPTH_WRITE:
                            return "DepthW";
                        case CGPU_RESOURCE_STATE_DEPTH_READ:
                            return "DepthR";
                        case CGPU_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
                            return "SRV(VS)";
                        case CGPU_RESOURCE_STATE_SHADER_RESOURCE:
                            return "SRV";
                        case CGPU_RESOURCE_STATE_STREAM_OUT:
                            return "StreamOut";
                        case CGPU_RESOURCE_STATE_INDIRECT_ARGUMENT:
                            return "Indirect";
                        case CGPU_RESOURCE_STATE_COPY_DEST:
                            return "CopyDst";
                        case CGPU_RESOURCE_STATE_COPY_SOURCE:
                            return "CopySrc";
                        case CGPU_RESOURCE_STATE_GENERIC_READ:
                            return "GenericR";
                        case CGPU_RESOURCE_STATE_PRESENT:
                            return "Present";
                        case CGPU_RESOURCE_STATE_COMMON:
                            return "Common";
                        case CGPU_RESOURCE_STATE_SHADING_RATE_SOURCE:
                            return "SRS";
                        default:
                            return "Unknown";
                        }
                    };

                    std::string label = (const char*)barrier.resource->get_name();
                    label += "\\n";
                    label += format_state(barrier.transition.before_state);
                    label += " → ";
                    label += format_state(barrier.transition.after_state);

                    // Choose color and style based on barrier type and Begin/End
                    const char* color = (src_q == tgt_q) ? "blue" : "orange";
                    const char* style = (src_q == tgt_q) ? "dotted" : "dashed";

                    // Special handling for Begin/End barriers - create portal nodes
                    if (barrier.transition.is_begin || barrier.transition.is_end)
                    {
                        std::string portal_id = "portal_" + transition_key + (barrier.transition.is_begin ? "_begin" : "_end");
                        std::string portal_label = barrier.transition.is_begin ? "BEGIN" : "END";
                        const char* portal_color = barrier.transition.is_begin ? "#FFE4B5" : "#E0E4CC"; // Moccasin for begin, light gray for end
                        const char* portal_shape = "diamond";

                        // Create portal node
                        dot << "  " << portal_id
                            << " [label=\"" << portal_label << "\\n"
                            << (const char*)barrier.resource->get_name()
                            << "\", shape=" << portal_shape
                            << ", style=\"filled,dashed\", fillcolor=\"" << portal_color
                            << "\", fontsize=8, width=0.5, height=0.5];\n";

                        if (barrier.transition.is_begin)
                        {
                            // Begin: Source Pass -> Portal
                            dot << "  pass_q" << src_q << "_" << src_i
                                << " -> " << portal_id
                                << " [style=dashed, color=green, penwidth=2, label=\"BEGIN\", fontcolor=green, fontsize=8];\n";
                        }
                        else
                        {
                            // End: Portal -> Target Pass
                            dot << "  " << portal_id
                                << " -> pass_q" << tgt_q << "_" << tgt_i
                                << " [style=dashed, color=red, penwidth=2, label=\"END\", fontcolor=red, fontsize=8];\n";
                        }
                    }
                    else
                    {
                        // Normal resource transition
                        dot << "  pass_q" << src_q << "_" << src_i
                            << " -> pass_q" << tgt_q << "_" << tgt_i
                            << " [style=" << style << ", color=" << color
                            << ", penwidth=1.5, label=\"" << label
                            << "\", fontcolor=" << color << ", fontsize=9];\n";
                    }
                }
            }
        }
    }

    // Debug output
    SKR_LOG_INFO(u8"ResourceTransition debug: total=%d, null_pass=%d, duplicate=%d, invalid_pos=%d, generated=%d",
        resource_transition_count,
        skipped_null_pass,
        skipped_duplicate,
        skipped_invalid_position,
        generated_edges);

    // 5. 生成内存桶信息
    dot << "\n  // Memory buckets\n";
    dot << "  subgraph cluster_memory {\n";
    dot << "    label=\"Memory Aliasing Buckets\";\n";
    dot << "    style=filled;\n";
    dot << "    fillcolor=\"#FFFACD\";\n";
    dot << "    node [shape=record, fillcolor=white];\n\n";

    for (uint32_t i = 0; i < aliasing_result.memory_buckets.size(); ++i)
    {
        const auto& bucket = aliasing_result.memory_buckets[i];

        std::stringstream label;
        label << "{Bucket " << i;
        label << "|Size: " << (bucket.total_size / 1024) << " KB";
        label << "|Compression: " << int(bucket.compression_ratio * 100) << "%";
        label << "|Resources:";

        for (auto* resource : bucket.aliased_resources)
        {
            label << "\\n• " << (const char*)resource->get_name();

            if (auto lifetime_it = lifetime_result.resource_lifetimes.find(resource))
            {
                label << " [L" << lifetime_it.value().start_dependency_level
                      << "-" << lifetime_it.value().end_dependency_level << "]";
            }
        }

        if (bucket.aliased_resources.size() > 1)
        {
            label << "\\n\\n→ Memory reused";
        }
        label << "}";

        dot << "    bucket_" << i << " [label=\"" << label.str() << "\"];\n";
    }

    dot << "  }\n\n";

    // 6. 生成统计信息
    dot << "  // Statistics\n";
    dot << "  subgraph cluster_stats {\n";
    dot << "    label=\"Barrier Statistics\";\n";
    dot << "    style=filled;\n";
    dot << "    fillcolor=\"#E8EAF6\";\n";
    dot << "    node [shape=record, fillcolor=white];\n\n";

    const auto& barrier_stats = barrier_phase.get_result();

    std::stringstream stats_label;
    stats_label << "{Statistics";
    stats_label << "|Total Barriers: " << barrier_phase.get_total_barriers();
    stats_label << "|Resource Transitions: " << barrier_stats.total_resource_barriers;
    stats_label << "|Cross-Queue Syncs: " << barrier_stats.total_sync_barriers;
    stats_label << "|Memory Aliasing: " << barrier_stats.total_aliasing_barriers;
    stats_label << "|Execution Dependencies: " << barrier_stats.total_execution_barriers;
    stats_label << "|Optimized Away: " << barrier_stats.optimized_away_barriers;
    stats_label << "|Estimated Cost: " << std::fixed << std::setprecision(2) << barrier_stats.estimated_barrier_cost;
    stats_label << "}";

    dot << "    stats_node [label=\"" << stats_label.str() << "\"];\n";
    dot << "  }\n\n";

    // 7. 生成图例
    dot << "  // Legend\n";
    dot << "  subgraph cluster_legend {\n";
    dot << "    label=\"Legend\";\n";
    dot << "    style=filled;\n";
    dot << "    fillcolor=lightgray;\n";
    dot << "    node [shape=plaintext];\n";
    dot << "    legend [label=<\n";
    dot << "      <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n";
    dot << "        <TR><TD COLSPAN=\"2\"><B>Edge Types</B></TD></TR>\n";
    dot << "        <TR><TD>Solid Gray</TD><TD>Execution Order</TD></TR>\n";
    dot << "        <TR><TD>Dashed Red</TD><TD>Cross-Queue Sync</TD></TR>\n";
    dot << "        <TR><TD>Dashed Purple</TD><TD>Memory Alias</TD></TR>\n";
    dot << "        <TR><TD>Dotted Blue</TD><TD>Resource Transition (Same Queue)</TD></TR>\n";
    dot << "        <TR><TD>Dashed Orange</TD><TD>Resource Transition (Cross Queue)</TD></TR>\n";
    dot << "        <TR><TD>Green Diamond (BEGIN)</TD><TD>Split Barrier Begin Portal</TD></TR>\n";
    dot << "        <TR><TD>Red Diamond (END)</TD><TD>Split Barrier End Portal</TD></TR>\n";
    dot << "        <TR><TD COLSPAN=\"2\"><B>Pass Types</B></TD></TR>\n";
    dot << "        <TR><TD>Box</TD><TD>Render Pass</TD></TR>\n";
    dot << "        <TR><TD>Ellipse</TD><TD>Compute Pass</TD></TR>\n";
    dot << "        <TR><TD>Octagon</TD><TD>Copy Pass</TD></TR>\n";
    dot << "        <TR><TD>Diamond</TD><TD>Present Pass</TD></TR>\n";
    dot << "      </TABLE>\n";
    dot << "    >];\n";
    dot << "  }\n";

    dot << "}\n";

    // 保存到文件
    std::ofstream file("render_graph_execution.dot");
    file << dot.str();
    file.close();

    SKR_LOG_INFO(u8"📊 Graphviz visualization saved to render_graph_execution.dot");
    SKR_LOG_INFO(u8"   Run: dot -Tpng render_graph_execution.dot -o render_graph_execution.png");
}

} // namespace skr::RG
