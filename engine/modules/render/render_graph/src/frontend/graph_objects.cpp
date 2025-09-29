#include "SkrBase/misc/debug.h"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"
#include "SkrContainersDef/hashmap.hpp"
#include "SkrContainersDef/concurrent_queue.hpp"

namespace skr
{
namespace RG
{

static const char* kMemoryPoolName = "RenderGraphFrontentObjects";

struct SKR_RENDER_GRAPH_API NodeAndEdgeFactoryImpl final : public NodeAndEdgeFactory
{
    NodeAndEdgeFactoryImpl() SKR_NOEXCEPT
    {
    }
    ~NodeAndEdgeFactoryImpl() SKR_NOEXCEPT
    {
        for (auto pool : pools)
        {
            SkrDelete(pool.second);
        }
    }

    struct factory_pool_t
    {
        size_t blockSize;
        skr::ConcurrentQueue<void*> blocks;

        factory_pool_t(size_t blockSize, size_t blockCount) SKR_NOEXCEPT
            : blockSize(blockSize),
              blocks(blockCount)
        {
        }
        ~factory_pool_t() SKR_NOEXCEPT
        {
            void* block;
            while (blocks.try_dequeue(block))
                sakura_freeN(block, kMemoryPoolName);
        }
        void* allocate()
        {
            void* block = nullptr;
            if (blocks.try_dequeue(block))
                return block;
            {
                return sakura_callocN(1, blockSize, kMemoryPoolName);
            }
        }
        void free(void* block)
        {
            if (blocks.try_enqueue(block))
                return;
            sakura_freeN(block, kMemoryPoolName);
        }
    };

    bool internalFreeMemory(void* memory, size_t size) override
    {
        auto pool = pools.find(size);
        SKR_ASSERT(pool != pools.end());
        pool->second->free(memory);
        return true;
    }

    void* internalAllocateMemory(size_t size) override
    {
        auto pool = pools.find(size);
        if (pool == pools.end())
        {
            pool = pools.emplace(size, SkrNew<factory_pool_t>(size, 2048u)).first;
        }
        return pool->second->allocate();
    }
    skr::FlatHashMap<size_t, factory_pool_t*> pools;
};

NodeAndEdgeFactory* NodeAndEdgeFactory::Create()
{
    return SkrNew<NodeAndEdgeFactoryImpl>();
}

void NodeAndEdgeFactory::Destroy(NodeAndEdgeFactory* factory)
{
    SkrDelete(factory);
}

// 0.node
RenderGraphNode::RenderGraphNode(EObjectType type)
    : type(type)
{
}

void RenderGraphNode::set_name(const char8_t* n)
{
    name = n;
}

const char8_t* RenderGraphNode::get_name() const
{
    return (const char8_t*)name.c_str();
}

const skr::StringView RenderGraphNode::get_name_view() const
{
    return name.view();
}

RenderGraphEdge::RenderGraphEdge(ERelationshipType type)
    : type(type)
{
}

// 1.resource nodes

ResourceNode::ResourceNode(EObjectType type, uint64_t frame_index) SKR_NOEXCEPT
    : RenderGraphNode(type),
      imported(false),
      frame_index(frame_index)
{
    
}

TextureNode::TextureNode(uint64_t frame_index) SKR_NOEXCEPT
    : ResourceNode(EObjectType::Texture, frame_index)
{
    descriptor.sample_count = CGPU_SAMPLE_COUNT_1;
}

BufferNode::BufferNode(uint64_t frame_index) SKR_NOEXCEPT
    : ResourceNode(EObjectType::Buffer, frame_index)
{
}

AccelerationStructureNode::AccelerationStructureNode(uint64_t frame_index) SKR_NOEXCEPT
    : ResourceNode(EObjectType::AccelerationStructure, frame_index)
{
}

// 2.pass nodes

PassNode::PassNode(EPassType pass_type, uint32_t order, uint64_t frame_index)
    : RenderGraphNode(EObjectType::Pass)
    , frame_index(frame_index)
    , pass_type(pass_type)
    , order(order)

{
}

const PassHandle PassNode::get_handle() const
{
    return PassHandle(get_id(), frame_index);
}

const bool PassNode::before(const PassNode* other) const
{
    if (other == nullptr) return false;
    const bool _ = order < other->order;
    return _;
}

const bool PassNode::after(const PassNode* other) const
{
    if (other == nullptr) return true;
    const bool _ = order > other->order;
    return _;
}

skr::Span<TextureReadEdge*> PassNode::tex_read_edges()
{
    return skr::Span<TextureReadEdge*>(in_texture_edges.data(), in_texture_edges.size());
}

skr::Span<TextureRenderEdge*> PassNode::tex_write_edges()
{
    return skr::Span<TextureRenderEdge*>(out_texture_edges.data(), out_texture_edges.size());
}

skr::Span<TextureReadWriteEdge*> PassNode::tex_readwrite_edges()
{
    return skr::Span<TextureReadWriteEdge*>(inout_texture_edges.data(), inout_texture_edges.size());
}

void PassNode::foreach_textures(skr::stl_function<void(TextureNode*, TextureEdge*)> f)
{
    for (auto&& e : tex_read_edges())
        f(e->get_texture_node(), e);
    for (auto&& e : tex_write_edges())
        f(e->get_texture_node(), e);
    for (auto&& e : tex_readwrite_edges())
        f(e->get_texture_node(), e);
}

skr::Span<BufferReadEdge*> PassNode::buf_read_edges()
{
    return skr::Span<BufferReadEdge*>(in_buffer_edges.data(), in_buffer_edges.size());
}

skr::Span<BufferReadWriteEdge*> PassNode::buf_readwrite_edges()
{
    return skr::Span<BufferReadWriteEdge*>(out_buffer_edges.data(), out_buffer_edges.size());
}

skr::Span<PipelineBufferEdge*> PassNode::buf_ppl_edges()
{
    return skr::Span<PipelineBufferEdge*>(ppl_buffer_edges.data(), ppl_buffer_edges.size());
}

void PassNode::foreach_buffers(skr::stl_function<void(BufferNode*, BufferEdge*)> f)
{
    for (auto&& e : buf_read_edges())
        f(e->get_buffer_node(), e);
    for (auto&& e : buf_readwrite_edges())
        f(e->get_buffer_node(), e);
    for (auto&& e : buf_ppl_edges())
        f(e->get_buffer_node(), e);
}

skr::Span<AccelerationStructureReadEdge*> PassNode::acceleration_structure_read_edges()
{
    return skr::Span<AccelerationStructureReadEdge*>(in_acceleration_structure_edges.data(), in_acceleration_structure_edges.size());
}

void PassNode::foreach_acceleration_structures(skr::stl_function<void(AccelerationStructureNode*, AccelerationStructureEdge*)> f)
{
    for (auto&& e : acceleration_structure_read_edges())
        f(e->get_acceleration_structure_node(), e);
}

RenderPassNode::RenderPassNode(uint32_t order, uint64_t frame_index)
    : PassNode(EPassType::Render, order, frame_index)
{
}

ComputePassNode::ComputePassNode(uint32_t order, uint64_t frame_index)
    : PassNode(EPassType::Compute, order, frame_index)
{
}

CopyPassNode::CopyPassNode(uint32_t order, uint64_t frame_index)
    : PassNode(EPassType::Copy, order, frame_index)
{
}

PresentPassNode::PresentPassNode(uint32_t order, uint64_t frame_index)
    : PassNode(EPassType::Present, order, frame_index)
{
}

// 3.edges

TextureEdge::TextureEdge(ERelationshipType type, ECGPUResourceState requested_state) SKR_NOEXCEPT
    : RenderGraphEdge(type),
      requested_state(requested_state)
{
}

// 3.1 Texture Read Edge

TextureReadEdge::TextureReadEdge(const skr::StringView name, TextureSRVHandle handle, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureRead, state)
    , name_hash(skr_hash_of(name.data(), name.size()))
    , name(name)
    , handle(handle)
{
}

TextureNode* TextureReadEdge::get_texture_node()
{
    return static_cast<TextureNode*>(from());
}

PassNode* TextureReadEdge::get_pass_node()
{
    return (PassNode*)to();
}

// 3.2 Texture RTV Edge

TextureRenderEdge::TextureRenderEdge(uint32_t mrt_index, TextureRTVHandle handle, CGPUClearValue clear_value, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureWrite, state)
    , mrt_index(mrt_index)
    , handle(handle)
    , clear_value(clear_value)
{
}

TextureNode* TextureRenderEdge::get_texture_node()
{
    return static_cast<TextureNode*>(to());
}

PassNode* TextureRenderEdge::get_pass_node()
{
    return (PassNode*)from();
}

// 3.3 Texture UAV edge

TextureReadWriteEdge::TextureReadWriteEdge(const skr::StringView name, TextureUAVHandle handle, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureReadWrite, state)
    , name_hash(skr_hash_of(name.data(), name.size()))
    , name(name)
    , handle(handle)

{
}

TextureNode* TextureReadWriteEdge::get_texture_node()
{
    return static_cast<TextureNode*>(to());
}

PassNode* TextureReadWriteEdge::get_pass_node()
{
    return (PassNode*)from();
}

// 3.4 Pipeline buffer edge

PipelineBufferEdge::PipelineBufferEdge(PipelineBufferHandle handle, ECGPUResourceState state)
    : BufferEdge(ERelationshipType::PipelineBuffer, state)
    , handle(handle)
{
}

BufferNode* PipelineBufferEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(from());
}

PassNode* PipelineBufferEdge::get_pass_node()
{
    return (PassNode*)to();
}

// 3.5 Buffer read edge

BufferReadEdge::BufferReadEdge(const skr::StringView name, BufferRangeHandle handle, ECGPUResourceState state)
    : BufferEdge(ERelationshipType::BufferRead, state)
    , name_hash(skr_hash_of(name.data(), name.size()))
    , name(name)
    , handle(handle)
{
}

BufferNode* BufferReadEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(from());
}

PassNode* BufferReadEdge::get_pass_node()
{
    return (PassNode*)to();
}

// 3.6 Buffer UAV edge

BufferReadWriteEdge::BufferReadWriteEdge(const skr::StringView name, BufferRangeHandle handle, ECGPUResourceState state)
    : BufferEdge(ERelationshipType::BufferReadWrite, state)
    , name(name)
    , name_hash(skr_hash_of(name.data(), name.size()))
    , handle(handle)
{
}

BufferNode* BufferReadWriteEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(to());
}

PassNode* BufferReadWriteEdge::get_pass_node()
{
    return (PassNode*)from();
}

// 3.7 Acceleration Structure Read Edge

AccelerationStructureReadEdge::AccelerationStructureReadEdge(const skr::StringView name, AccelerationStructureSRVHandle handle)
    : AccelerationStructureEdge(ERelationshipType::AccelerationStructureRead, CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_READ)
    , name(name)
    , name_hash(skr_hash_of(name.data(), name.size()))
    , handle(handle)
{
}

AccelerationStructureNode* AccelerationStructureReadEdge::get_acceleration_structure_node()
{
    return static_cast<AccelerationStructureNode*>(from());
}

PassNode* AccelerationStructureReadEdge::get_pass_node()
{
    return (PassNode*)to();
}

} // namespace RG
} // namespace skr