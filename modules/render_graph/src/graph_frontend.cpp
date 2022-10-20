#include "render_graph/frontend/render_graph.hpp"
#include <EASTL/vector_map.h>
#include "render_graph/backend/texture_view_pool.hpp"

#include "tracy/Tracy.hpp"

namespace skr
{
namespace render_graph
{
RenderGraph::RenderGraph(const RenderGraphBuilder& builder) SKR_NOEXCEPT
    : aliasing_enabled(builder.memory_aliasing)
{
}

const ResourceNode::LifeSpan ResourceNode::lifespan() const SKR_NOEXCEPT
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

inline bool aliasing_capacity(TextureNode* aliased, TextureNode* aliasing) SKR_NOEXCEPT
{
    return !aliased->is_imported() &&
           !aliased->get_desc().is_dedicated &&
           aliased->get_size() >= aliasing->get_size() &&
           aliased->get_sample_count() == aliasing->get_sample_count();
}
bool RenderGraph::compile() SKR_NOEXCEPT
{
    ZoneScopedN("RenderGraphCompile");
    {
        ZoneScopedN("Cull");
        // 1.cull
        resources.erase(
        eastl::remove_if(resources.begin(), resources.end(),
        [this](ResourceNode* resource) {
            ZoneScopedN("Resource");
            const bool lone = !(resource->incoming_edges() + resource->outgoing_edges());
            {
                ZoneScopedN("RecordDealloc");
                if (lone) culled_resources.emplace_back(resource);
            }
            return lone;
        }),
        resources.end());
        passes.erase(
        eastl::remove_if(passes.begin(), passes.end(),
        [this](PassNode* pass) {
            ZoneScopedN("Pass");
            const bool lone = !(pass->incoming_edges() + pass->outgoing_edges());
            {
                ZoneScopedN("RecordDealloc");
                if (lone) culled_passes.emplace_back(pass);
            }
            return lone;
        }),
        passes.end());
    }
    if (aliasing_enabled)
    {
        ZoneScopedN("CalculateAliasing");
        // 2.calc aliasing
        // - 先在aliasing chain里找一圈，如果有不重合的，直接把它加入到aliasing chain里
        // - 如果没找到，在所有resource中找一个合适的加入到aliasing chain
        eastl::vector_map<TextureNode*, TextureNode::LifeSpan> alliasing_lifespans;
        foreach_textures([&](TextureNode* texture) SKR_NOEXCEPT {
            if (texture->imported) return;
            for (auto&& [aliased, aliaed_span] : alliasing_lifespans)
            {
                if (aliasing_capacity(aliased, texture) &&
                    aliaed_span.to < texture->lifespan().from)
                {
                    if (!texture->frame_aliasing_source ||
                        texture->frame_aliasing_source->get_size() > aliased->get_size() // always choose smallest block
                    )
                    {
                        texture->descriptor.is_aliasing = true;
                        texture->frame_aliasing_source = aliased;
                        aliaed_span.to = texture->lifespan().to;
                    }
                }
            }
            if (texture->frame_aliasing_source) return;
            foreach_textures([&](TextureNode* aliased) SKR_NOEXCEPT {
                if (aliasing_capacity(aliased, texture) &&
                    aliased->lifespan().to < texture->lifespan().from)
                {
                    if (!texture->frame_aliasing_source ||
                        texture->frame_aliasing_source->get_size() > aliased->get_size() // always choose smallest block
                    )
                    {
                        texture->descriptor.is_aliasing = true;
                        texture->frame_aliasing_source = aliased;
                        alliasing_lifespans[aliased].from = aliased->lifespan().from;
                        alliasing_lifespans[aliased].to = aliased->lifespan().from;
                    }
                }
            });
        });
    }
    return true;
}

uint32_t RenderGraph::foreach_textures(eastl::function<void(TextureNode*)> f) SKR_NOEXCEPT
{
    uint32_t num = 0;
    for (auto&& resource : resources)
    {
        if (resource->type == EObjectType::Texture)
        {
            f((TextureNode*)resource);
            num++;
        }
    }
    return num;
}

uint32_t RenderGraph::foreach_writer_passes(TextureHandle texture,
eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const SKR_NOEXCEPT
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
eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const SKR_NOEXCEPT
{
    return graph->foreach_outgoing_edges(
    texture,
    [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) SKR_NOEXCEPT {
        PassNode* pass = static_cast<PassNode*>(to);
        TextureNode* texture = static_cast<TextureNode*>(from);
        RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
        f(pass, texture, edge);
    });
}

uint32_t RenderGraph::foreach_writer_passes(BufferHandle buffer,
eastl::function<void(PassNode*, BufferNode*, RenderGraphEdge*)> f) const SKR_NOEXCEPT
{
    return graph->foreach_incoming_edges(
    buffer,
    [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) SKR_NOEXCEPT {
        PassNode* pass = static_cast<PassNode*>(from);
        BufferNode* buffer = static_cast<BufferNode*>(to);
        RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
        f(pass, buffer, edge);
    });
}

uint32_t RenderGraph::foreach_reader_passes(BufferHandle buffer,
eastl::function<void(PassNode*, BufferNode*, RenderGraphEdge*)> f) const SKR_NOEXCEPT
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

const ECGPUResourceState RenderGraph::get_lastest_state(const TextureNode* texture, const PassNode* pending_pass) const SKR_NOEXCEPT
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

const ECGPUResourceState RenderGraph::get_lastest_state(const BufferNode* buffer, const PassNode* pending_pass) const SKR_NOEXCEPT
{
    ZoneScopedN("CaclulateLatestState-Buffer");

    if (passes[0] == pending_pass)
        return buffer->init_state;
    PassNode* pass_iter = nullptr;
    auto result = buffer->init_state;
    foreach_writer_passes(buffer->get_handle(),
    [&](PassNode* pass, BufferNode* buffer, RenderGraphEdge* edge) SKR_NOEXCEPT {
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
    [&](PassNode* pass, BufferNode* buffer, RenderGraphEdge* edge) SKR_NOEXCEPT {
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

uint64_t RenderGraph::execute(RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    graph->clear();
    return frame_index++;
}

void RenderGraph::initialize() SKR_NOEXCEPT
{
    graph = DependencyGraph::Create();
}

void RenderGraph::finalize() SKR_NOEXCEPT
{
    DependencyGraph::Destroy(graph);
}
} // namespace render_graph
} // namespace skr