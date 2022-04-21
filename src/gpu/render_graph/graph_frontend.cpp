#include "render_graph/frontend/render_graph.hpp"
#include "../cgpu/common/common_utils.h"
#include <EASTL/vector_map.h>
#include "tracy/Tracy.hpp"

namespace sakura
{
namespace render_graph
{
const ResourceNode::LifeSpan ResourceNode::lifespan() const
{
    if (frame_lifespan.from != UINT32_MAX && frame_lifespan.to != UINT32_MAX)
    {
        return frame_lifespan;
    }
    uint32_t from = UINT32_MAX, to = 0;
    foreach_neighbors([&](const DependencyGraphNode* node) {
        auto rg_node = static_cast<const RenderGraphNode*>(node);
        if (rg_node->type == EObjectType::Pass)
        {
            auto pass_node = static_cast<const RenderPassNode*>(node);
            from = (from <= pass_node->order) ? from : pass_node->order;
            to = (to >= pass_node->order) ? to : pass_node->order;
        }
    });
    foreach_inv_neighbors([&](const DependencyGraphNode* node) {
        auto rg_node = static_cast<const RenderGraphNode*>(node);
        if (rg_node->type == EObjectType::Pass)
        {
            auto pass_node = static_cast<const RenderPassNode*>(node);
            from = (from <= pass_node->order) ? from : pass_node->order;
            to = (to >= pass_node->order) ? to : pass_node->order;
        }
    });
    frame_lifespan = { from, to };
    return frame_lifespan;
}

inline bool aliasing_capacity(TextureNode* aliased, TextureNode* aliasing)
{
    return !aliased->is_imported() &&
           aliased->get_size() >= aliasing->get_size() &&
           aliased->get_sample_count() == aliasing->get_sample_count();
}
bool RenderGraph::compile()
{
    ZoneScopedN("RenderGraphCompile");
    {
        ZoneScopedN("ull");
        // 1.cull
        resources.erase(
            eastl::remove_if(resources.begin(), resources.end(),
                [this](ResourceNode* resource) {
                    const bool lone = !(resource->incoming_edges() + resource->outgoing_edges());
                    if (lone) culled_resources.emplace_back(resource);
                    return lone;
                }),
            resources.end());
        passes.erase(
            eastl::remove_if(passes.begin(), passes.end(),
                [this](PassNode* pass) {
                    const bool lone = !(pass->incoming_edges() + pass->outgoing_edges());
                    if (lone) culled_passes.emplace_back(pass);
                    return lone;
                }),
            passes.end());
    }
    {
        ZoneScopedN("CalculateAliasing");
        // 2.calc aliasing
        // - 先在aliasing chain里找一圈，如果有不重合的，直接把它加入到aliasing chain里
        // - 如果没找到，在所有resource中找一个合适的加入到aliasing chain
        eastl::vector_map<TextureNode*, TextureNode::LifeSpan> alliasing_lifespans;
        for (auto& resource : resources)
        {
            if (resource->type == EObjectType::Texture)
            {
                TextureNode* texture = static_cast<TextureNode*>(resource);
                if (texture->imported) continue;
                for (auto&& aliasing_lifespan : alliasing_lifespans)
                {
                    auto&& owner_span = aliasing_lifespan.second;
                    auto&& owner = aliasing_lifespan.first;
                    if (aliasing_capacity(owner, texture) &&
                        owner_span.to < texture->lifespan().from)
                    {
                        texture->descriptor.is_aliasing = true;
                        texture->frame_aliasing_source = owner;
                        owner_span.to = texture->lifespan().to;
                        break;
                    }
                }
                for (auto target_resource : resources)
                {
                    if (target_resource->type == EObjectType::Texture)
                    {
                        TextureNode* target_texture = static_cast<TextureNode*>(target_resource);
                        if (aliasing_capacity(target_texture, texture) &&
                            target_texture->lifespan().to < texture->lifespan().from) // allocation capacity
                        {
                            target_texture->descriptor.aliasing_capacity = true;
                            texture->descriptor.is_aliasing = true;
                            texture->frame_aliasing_source = target_texture;
                            alliasing_lifespans[target_texture].from = target_texture->lifespan().from;
                            alliasing_lifespans[target_texture].to = texture->lifespan().from;
                        }
                    }
                }
            }
        }
    }
    return true;
}

uint32_t RenderGraph::foreach_writer_passes(TextureHandle texture,
    eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_incoming_edges(
        texture,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(from);
            TextureNode* texture = static_cast<TextureNode*>(to);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
}

uint32_t RenderGraph::foreach_reader_passes(TextureHandle texture,
    eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_outgoing_edges(
        texture,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(to);
            TextureNode* texture = static_cast<TextureNode*>(from);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
}

uint32_t RenderGraph::foreach_writer_passes(BufferHandle buffer,
    eastl::function<void(PassNode*, BufferNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_incoming_edges(
        buffer,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(from);
            BufferNode* buffer = static_cast<BufferNode*>(to);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, buffer, edge);
        });
}

uint32_t RenderGraph::foreach_reader_passes(BufferHandle buffer,
    eastl::function<void(PassNode*, BufferNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_outgoing_edges(
        buffer,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(to);
            BufferNode* buffer = static_cast<BufferNode*>(from);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, buffer, edge);
        });
}

const ECGpuResourceState RenderGraph::get_lastest_state(
    const TextureNode* texture, const PassNode* pending_pass) const
{
    ZoneScopedN("CaclulateLatestState-Texture");

    if (passes[0] == pending_pass)
        return texture->init_state;
    PassNode* pass_iter = nullptr;
    auto result = texture->init_state;
    foreach_writer_passes(texture->get_handle(),
        [&](PassNode* pass, TextureNode* texture, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::TextureWrite)
            {
                auto write_edge = static_cast<TextureRenderEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = write_edge->requested_state;
                }
            }
            else if (edge->type == ERelationshipType::TextureReadWrite)
            {
                auto rw_edge = static_cast<TextureReadWriteEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = rw_edge->requested_state;
                }
            }
        });
    foreach_reader_passes(texture->get_handle(),
        [&](PassNode* pass, TextureNode* texture, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::TextureRead)
            {
                auto read_edge = static_cast<TextureRenderEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = read_edge->requested_state;
                }
            }
        });
    return result;
}

const ECGpuResourceState RenderGraph::get_lastest_state(
    const BufferNode* buffer, const PassNode* pending_pass) const
{
    ZoneScopedN("CaclulateLatestState-Buffer");

    if (passes[0] == pending_pass)
        return buffer->init_state;
    PassNode* pass_iter = nullptr;
    auto result = buffer->init_state;
    foreach_writer_passes(buffer->get_handle(),
        [&](PassNode* pass, BufferNode* buffer, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::BufferReadWrite)
            {
                auto write_edge = static_cast<BufferReadWriteEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = write_edge->requested_state;
                }
            }
        });
    foreach_reader_passes(buffer->get_handle(),
        [&](PassNode* pass, BufferNode* buffer, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::BufferRead)
            {
                auto read_edge = static_cast<BufferReadEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = read_edge->requested_state;
                }
            }
            else if (edge->type == ERelationshipType::PipelineBuffer)
            {
                auto ppl_edge = static_cast<PipelineBufferEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = ppl_edge->requested_state;
                }
            }
        });
    return result;
}

uint64_t RenderGraph::execute()
{
    graph->clear();
    return frame_index++;
}

void RenderGraph::initialize()
{
}

void RenderGraph::finalize()
{
}
} // namespace render_graph
} // namespace sakura