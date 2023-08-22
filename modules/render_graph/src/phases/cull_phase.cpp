#include "SkrRenderGraph/phases/cull_phase.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"

#include "SkrProfile/profile.h"

namespace skr {
namespace render_graph {

void CullPhase::on_compile(RenderGraph* graph) SKR_NOEXCEPT
{
    SkrZoneScopedN("RenderGraphCull");
    auto& resources = get_resources(graph);
    auto& passes = get_passes(graph);

    resources.erase(
    eastl::remove_if(resources.begin(), resources.end(),
    [this](ResourceNode* resource) {
        SKR_UNUSED const auto name = resource->get_name_view();
        SkrZoneScopedC(tracy::Color::SteelBlue);
        ZoneName((const char*)name.raw().data(), name.size());

        const bool lone = !(resource->incoming_edges() + resource->outgoing_edges());
        {
            SkrZoneScopedN("RecordDealloc");
            if (lone) culled_resources.emplace_back(resource);
        }
        return lone;
    }),
    resources.end());

    passes.erase(
    eastl::remove_if(passes.begin(), passes.end(),
    [this](PassNode* pass) {
        SKR_UNUSED const auto name = pass->get_name_view();
        SkrZoneScopedC(tracy::Color::SteelBlue);
        ZoneName((const char*)name.raw().data(), name.size());

        const bool lone = !(pass->incoming_edges() + pass->outgoing_edges());
        const bool can_be_lone = pass->get_can_be_lone();
        const bool culled = lone && !can_be_lone;
        {
            SkrZoneScopedN("RecordDealloc");
            if (culled) culled_passes.emplace_back(pass);
        }
        return culled;
    }),
    passes.end());
}

void CullPhase::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    auto node_factory = graph->get_node_factory();
    // 1.dealloc culled resources
    for (auto culled_resource : culled_resources)
    {
        node_factory->Dealloc(culled_resource);
    }
    culled_resources.clear();
    // 2.dealloc culled passes 
    for (auto culled_pass : culled_passes)
    {
        node_factory->Dealloc(culled_pass);
    }
    culled_passes.clear();
}

} // namespace render_graph
} // namespace skr