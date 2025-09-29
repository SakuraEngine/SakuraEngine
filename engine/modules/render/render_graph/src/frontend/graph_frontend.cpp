#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"

#include "SkrProfile/profile.h"

#include <SkrContainers/string.hpp>
#include <SkrContainers/hashmap.hpp>

namespace skr::RG
{
struct BlackboardImpl final : public Blackboard
{
public:
    void clear() SKR_NOEXCEPT override
    {
        named_passes.clear();
        named_textures.clear();
        named_buffers.clear();
        named_acceleration_structures.clear();
    }

    PassNode* pass(const char8_t* name) SKR_NOEXCEPT final override
    {
        auto it = named_passes.find(name);
        if (it != named_passes.end())
        {
            return it->second;
        }
        return nullptr;
    }

    TextureNode* texture(const char8_t* name) SKR_NOEXCEPT final override
    {
        auto it = named_textures.find(name);
        if (it != named_textures.end())
        {
            return it->second;
        }
        return nullptr;
    }

    BufferNode* buffer(const char8_t* name) SKR_NOEXCEPT final override
    {
        auto it = named_buffers.find(name);
        if (it != named_buffers.end())
        {
            return it->second;
        }
        return nullptr;
    }

    AccelerationStructureNode* acceleration_structure(const char8_t* name) SKR_NOEXCEPT final override
    {
        auto it = named_acceleration_structures.find(name);
        if (it != named_acceleration_structures.end())
        {
            return it->second;
        }
        return nullptr;
    }

    bool value(const char8_t* name, double& v) SKR_NOEXCEPT final override
    {
        auto it = named_values.find(name);
        if (it != named_values.end())
        {
            v = it->second;
            return true;
        }
        return false;
    }

    bool set_value(const char8_t* name, double v) SKR_NOEXCEPT final override
    {
        named_values[name] = v;
        return true;
    }

    bool add_pass(const char8_t* name, class PassNode* pass) SKR_NOEXCEPT final override
    {
        auto it = named_passes.find(name);
        if (it != named_passes.end())
        {
            return false;
        }
        named_passes.emplace(pass->get_name(), pass);
        return true;
    }

    bool add_texture(const char8_t* name, class TextureNode* texture) SKR_NOEXCEPT final override
    {
        auto it = named_textures.find(name);
        if (it != named_textures.end())
        {
            return false;
        }
        named_textures.emplace(texture->get_name(), texture);
        return true;
    }

    bool add_buffer(const char8_t* name, class BufferNode* buffer) SKR_NOEXCEPT final override
    {
        auto it = named_buffers.find(name);
        if (it != named_buffers.end())
        {
            return false;
        }
        named_buffers.emplace(buffer->get_name(), buffer);
        return true;
    }

    bool add_acceleration_structure(const char8_t* name, class AccelerationStructureNode* acceleration_structure) SKR_NOEXCEPT final override
    {
        auto it = named_acceleration_structures.find(name);
        if (it != named_acceleration_structures.end())
        {
            return false;
        }
        named_acceleration_structures.emplace(acceleration_structure->get_name(), acceleration_structure);
        return true;
    }

    void override_pass(const char8_t* name, class PassNode* pass) SKR_NOEXCEPT final override
    {
        auto it = named_passes.find(name);
        if (it != named_passes.end())
        {
            named_passes[name] = pass;
        }
        named_passes.emplace(name, pass);
    }

    void override_texture(const char8_t* name, class TextureNode* texture) SKR_NOEXCEPT final override
    {
        auto it = named_textures.find(name);
        if (it != named_textures.end())
        {
            named_textures[name] = texture;
        }
        named_textures.emplace(name, texture);
    }

    void override_buffer(const char8_t* name, class BufferNode* buffer) SKR_NOEXCEPT final override
    {
        auto it = named_buffers.find(name);
        if (it != named_buffers.end())
        {
            named_buffers[name] = buffer;
        }
        named_buffers.emplace(name, buffer);
    }

    void override_acceleration_structure(const char8_t* name, class AccelerationStructureNode* acceleration_structure) SKR_NOEXCEPT final override
    {
        auto it = named_acceleration_structures.find(name);
        if (it != named_acceleration_structures.end())
        {
            named_acceleration_structures[name] = acceleration_structure;
        }
        named_acceleration_structures.emplace(name, acceleration_structure);
    }

protected:
    template <typename T>
    using FlatStringMap = skr::FlatHashMap<skr::StringView, T, skr::Hash<skr::StringView>>;

    FlatStringMap<class PassNode*> named_passes;
    FlatStringMap<class TextureNode*> named_textures;
    FlatStringMap<class BufferNode*> named_buffers;
    FlatStringMap<class AccelerationStructureNode*> named_acceleration_structures;
    FlatStringMap<double> named_values;
};

Blackboard* Blackboard::Create() SKR_NOEXCEPT
{
    return SkrNew<BlackboardImpl>();
}

void Blackboard::Destroy(Blackboard* blackboard) SKR_NOEXCEPT
{
    SkrDelete(blackboard);
}

EObjectType TextureNode::get_type() const SKR_NOEXCEPT
{
    return EObjectType::Texture;
}

EObjectType BufferNode::get_type() const SKR_NOEXCEPT
{
    return EObjectType::Buffer;
}

EObjectType AccelerationStructureNode::get_type() const SKR_NOEXCEPT
{
    return EObjectType::AccelerationStructure;
}

RenderGraph::RenderGraph(const RenderGraphBuilder& builder) SKR_NOEXCEPT
    : aliasing_enabled(builder.memory_aliasing)
{
}

BufferNode* RenderGraph::resolve(BufferHandle hdl) SKR_NOEXCEPT
{
    if (hdl.handle.frame_index() != frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: buffer is not alive in the current frame");
    return static_cast<BufferNode*>(graph->access_node(hdl.handle.id()));
}

TextureNode* RenderGraph::resolve(TextureHandle hdl) SKR_NOEXCEPT
{
    if (hdl.handle.frame_index() != frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: texture is not alive in the current frame");
    return static_cast<TextureNode*>(graph->access_node(hdl.handle.id()));
}

PassNode* RenderGraph::resolve(PassHandle hdl) SKR_NOEXCEPT
{
    if (hdl.handle.frame_index() != frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: pass is not alive in the current frame");
    return static_cast<PassNode*>(graph->access_node(hdl.handle.id()));
}

const CGPUBufferDescriptor* RenderGraph::resolve_descriptor(BufferHandle hdl) SKR_NOEXCEPT
{
    if (const auto node = resolve(hdl))
    {
        return &node->descriptor;
    }
    return nullptr;
}

const CGPUTextureDescriptor* RenderGraph::resolve_descriptor(TextureHandle hdl) SKR_NOEXCEPT
{
    if (const auto node = resolve(hdl))
    {
        return &node->descriptor;
    }
    return nullptr;
}

uint32_t RenderGraph::foreach_textures(skr::stl_function<void(TextureNode*)> f) SKR_NOEXCEPT
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

uint32_t RenderGraph::foreach_passes(TextureHandle texture,
    skr::stl_function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const SKR_NOEXCEPT
{
    uint32_t n = 0;
    n += graph->foreach_incoming_edges(
        texture.handle.id(),
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(from);
            TextureNode* texture = static_cast<TextureNode*>(to);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
    n += graph->foreach_outgoing_edges(
        texture.handle.id(),
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(to);
            TextureNode* texture = static_cast<TextureNode*>(from);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
    return n;
}

uint32_t RenderGraph::foreach_passes(BufferHandle buffer,
    skr::stl_function<void(PassNode*, BufferNode*, RenderGraphEdge*)> f) const SKR_NOEXCEPT
{
    uint32_t n = 0;
    n += graph->foreach_incoming_edges(
        buffer.handle.id(),
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) SKR_NOEXCEPT {
            PassNode* pass = static_cast<PassNode*>(from);
            BufferNode* buffer = static_cast<BufferNode*>(to);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, buffer, edge);
        });
    n += graph->foreach_outgoing_edges(
        buffer.handle.id(),
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) SKR_NOEXCEPT {
            PassNode* pass = static_cast<PassNode*>(to);
            BufferNode* buffer = static_cast<BufferNode*>(from);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, buffer, edge);
        });
    return n;
}

const ECGPUResourceState RenderGraph::get_lastest_state(const TextureNode* texture, const PassNode* pending_pass) const SKR_NOEXCEPT
{
    SkrZoneScopedN("CaclulateLatestState-Texture");

    if (passes[0] == pending_pass)
        return texture->init_state;
    PassNode* pass_iter = nullptr;
    auto result = texture->init_state;
    foreach_passes(texture->get_handle(),
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
            else if (edge->type == ERelationshipType::TextureRead)
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
    SkrZoneScopedN("CaclulateLatestState-Buffer");

    if (passes[0] == pending_pass)
        return buffer->init_state;
    PassNode* pass_iter = nullptr;
    auto result = buffer->init_state;
    foreach_passes(buffer->get_handle(),
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
            else if (edge->type == ERelationshipType::BufferRead)
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

void RenderGraph::add_before_execute_callback(const BeforeExecuteCallback& callback)
{
    exec_callbacks.add(callback);
}

void RenderGraph::wait_frame(uint64_t frame_index) SKR_NOEXCEPT
{
}

bool RenderGraph::check_frame(uint64_t frame_index) SKR_NOEXCEPT
{
    return true;
}

uint64_t RenderGraph::execute(RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    for (auto callback : exec_callbacks)
        callback(*this);
    exec_callbacks.clear();

    graph->clear();
    return frame_index++;
}

void RenderGraph::initialize() SKR_NOEXCEPT
{
    graph = DependencyGraph::Create();
    node_factory = NodeAndEdgeFactory::Create();
    blackboard = Blackboard::Create();
}

void RenderGraph::finalize() SKR_NOEXCEPT
{
    Blackboard::Destroy(blackboard);
    NodeAndEdgeFactory::Destroy(node_factory);
    DependencyGraph::Destroy(graph);
}
} // namespace skr::RG