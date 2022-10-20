#pragma once
#include "platform/configure.h"
#include <EASTL/functional.h>

namespace skr
{
typedef uint64_t dep_graph_handle_t;
class DependencyGraphEdge;
class DependencyGraphNode
{
    friend class DependencyGraphImpl;
public:
    DependencyGraphNode() = default;
    using Type = DependencyGraphNode;
    virtual ~DependencyGraphNode() SKR_NOEXCEPT = default;
    // Nodes can't be copied
    DependencyGraphNode(const Type&) SKR_NOEXCEPT = delete;
    virtual void on_insert() SKR_NOEXCEPT {}
    virtual void on_remove() SKR_NOEXCEPT {}
    const dep_graph_handle_t get_id() const SKR_NOEXCEPT { return id; }
    uint32_t outgoing_edges() SKR_NOEXCEPT;
    uint32_t incoming_edges() SKR_NOEXCEPT;
    uint32_t foreach_neighbors(eastl::function<void(DependencyGraphNode* neig)>) SKR_NOEXCEPT;
    uint32_t foreach_neighbors(eastl::function<void(const DependencyGraphNode* neig)>) const SKR_NOEXCEPT;
    uint32_t foreach_inv_neighbors(eastl::function<void(DependencyGraphNode* inv_neig)>) SKR_NOEXCEPT;
    uint32_t foreach_inv_neighbors(eastl::function<void(const DependencyGraphNode* inv_neig)>) const SKR_NOEXCEPT;

private:
    class DependencyGraph* graph;
    dep_graph_handle_t id;
};

class DependencyGraphEdge
{
    friend class DependencyGraphImpl;

public:
    DependencyGraphEdge() = default;
    using Type = DependencyGraphEdge;
    virtual ~DependencyGraphEdge() SKR_NOEXCEPT = default;
    // Edges can't be copied
    DependencyGraphEdge(const Type&) SKR_NOEXCEPT = delete;
    virtual void on_link() SKR_NOEXCEPT {}
    virtual void on_unlink() SKR_NOEXCEPT {}
    DependencyGraphNode* from() SKR_NOEXCEPT;
    DependencyGraphNode* to() SKR_NOEXCEPT;

protected:
    class DependencyGraph* graph;
    dep_graph_handle_t from_node;
    dep_graph_handle_t to_node;
};

class DependencyGraph
{
public:
    using Node = DependencyGraphNode;
    using Edge = DependencyGraphEdge;
    static DependencyGraph* Create() SKR_NOEXCEPT;
    static void Destroy(DependencyGraph* graph) SKR_NOEXCEPT;
    virtual ~DependencyGraph() SKR_NOEXCEPT = default;
    virtual dep_graph_handle_t insert(Node* node) SKR_NOEXCEPT = 0;
    virtual Node* access_node(dep_graph_handle_t handle) SKR_NOEXCEPT = 0;
    virtual bool remove(dep_graph_handle_t node) SKR_NOEXCEPT = 0;
    virtual bool remove(Node* node) SKR_NOEXCEPT = 0;
    virtual bool clear() SKR_NOEXCEPT = 0;
    virtual bool link(Node* from, Node* to, Edge* edge = nullptr) SKR_NOEXCEPT = 0;
    virtual Edge* linkage(Node* from, Node* to) SKR_NOEXCEPT = 0;
    virtual Edge* linkage(dep_graph_handle_t from, dep_graph_handle_t to) SKR_NOEXCEPT = 0;
    virtual bool unlink(Node* from, Node* to) SKR_NOEXCEPT = 0;
    virtual bool unlink(dep_graph_handle_t from, dep_graph_handle_t to) SKR_NOEXCEPT = 0;
    virtual Node* node_at(dep_graph_handle_t ID) SKR_NOEXCEPT = 0;
    virtual Node* from_node(Edge* edge) SKR_NOEXCEPT = 0;
    virtual Node* to_node(Edge* edge) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(Node* node, eastl::function<void(Node* neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(dep_graph_handle_t node, eastl::function<void(Node* neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(const Node* node, eastl::function<void(const Node* neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(const dep_graph_handle_t node, eastl::function<void(const Node* neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(Node* node, eastl::function<void(Node* inv_neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(dep_graph_handle_t node, eastl::function<void(Node* inv_neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(const Node* node, eastl::function<void(const Node* inv_neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(const dep_graph_handle_t node, eastl::function<void(const Node* inv_neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t outgoing_edges(const Node* node) SKR_NOEXCEPT = 0;
    virtual uint32_t outgoing_edges(dep_graph_handle_t id) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_outgoing_edges(dep_graph_handle_t node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_outgoing_edges(Node* node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t incoming_edges(const Node* node) SKR_NOEXCEPT = 0;
    virtual uint32_t incoming_edges(dep_graph_handle_t id) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_incoming_edges(Node* node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_incoming_edges(dep_graph_handle_t node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
};

inline DependencyGraphNode* DependencyGraphEdge::from() SKR_NOEXCEPT
{
    return graph->node_at(from_node);
}
inline DependencyGraphNode* DependencyGraphEdge::to() SKR_NOEXCEPT
{
    return graph->node_at(to_node);
}

} // namespace skr