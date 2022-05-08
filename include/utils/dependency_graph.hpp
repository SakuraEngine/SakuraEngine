#pragma once
#include "platform/configure.h"
#include <EASTL/functional.h>

namespace skr
{
typedef uint64_t dep_graph_handle_t;
class DependencyGraphEdge;
class RUNTIME_API DependencyGraphNode
{
    friend class DependencyGraphImpl;

public:
    DependencyGraphNode() = default;
    using Type = DependencyGraphNode;
    virtual ~DependencyGraphNode() RUNTIME_NOEXCEPT = default;
    // Nodes can't be copied
    DependencyGraphNode(const Type&) RUNTIME_NOEXCEPT = delete;
    virtual void on_insert() {}
    virtual void on_remove() {}
    const dep_graph_handle_t get_id() const { return id; }
    uint32_t outgoing_edges();
    uint32_t incoming_edges();
    uint32_t foreach_neighbors(eastl::function<void(DependencyGraphNode* neig)>);
    uint32_t foreach_neighbors(eastl::function<void(const DependencyGraphNode* neig)>) const;
    uint32_t foreach_inv_neighbors(eastl::function<void(DependencyGraphNode* inv_neig)>);
    uint32_t foreach_inv_neighbors(eastl::function<void(const DependencyGraphNode* inv_neig)>) const;

private:
    class DependencyGraph* graph;
    dep_graph_handle_t id;
};

class RUNTIME_API DependencyGraphEdge
{
    friend class DependencyGraphImpl;

public:
    DependencyGraphEdge() = default;
    using Type = DependencyGraphEdge;
    virtual ~DependencyGraphEdge() RUNTIME_NOEXCEPT = default;
    // Edges can't be copied
    DependencyGraphEdge(const Type&) RUNTIME_NOEXCEPT = delete;
    virtual void on_link() {}
    virtual void on_unlink() {}
    DependencyGraphNode* from();
    DependencyGraphNode* to();

protected:
    class DependencyGraph* graph;
    dep_graph_handle_t from_node;
    dep_graph_handle_t to_node;
};

class RUNTIME_API DependencyGraph
{
public:
    using Node = DependencyGraphNode;
    using Edge = DependencyGraphEdge;
    static DependencyGraph* Create() RUNTIME_NOEXCEPT;
    virtual ~DependencyGraph() RUNTIME_NOEXCEPT = default;
    virtual dep_graph_handle_t insert(Node* node) = 0;
    virtual Node* access_node(dep_graph_handle_t handle) = 0;
    virtual bool remove(dep_graph_handle_t node) = 0;
    virtual bool remove(Node* node) = 0;
    virtual bool clear() = 0;
    virtual bool link(Node* from, Node* to, Edge* edge = nullptr) = 0;
    virtual Edge* linkage(Node* from, Node* to) = 0;
    virtual Edge* linkage(dep_graph_handle_t from, dep_graph_handle_t to) = 0;
    virtual bool unlink(Node* from, Node* to) = 0;
    virtual bool unlink(dep_graph_handle_t from, dep_graph_handle_t to) = 0;
    virtual Node* node_at(dep_graph_handle_t ID) = 0;
    virtual Node* from_node(Edge* edge) = 0;
    virtual Node* to_node(Edge* edge) = 0;
    virtual uint32_t foreach_neighbors(Node* node, eastl::function<void(Node* neig)>) = 0;
    virtual uint32_t foreach_neighbors(dep_graph_handle_t node, eastl::function<void(Node* neig)>) = 0;
    virtual uint32_t foreach_neighbors(const Node* node, eastl::function<void(const Node* neig)>) const = 0;
    virtual uint32_t foreach_neighbors(const dep_graph_handle_t node, eastl::function<void(const Node* neig)>) const = 0;
    virtual uint32_t foreach_inv_neighbors(Node* node, eastl::function<void(Node* inv_neig)>) = 0;
    virtual uint32_t foreach_inv_neighbors(dep_graph_handle_t node, eastl::function<void(Node* inv_neig)>) = 0;
    virtual uint32_t foreach_inv_neighbors(const Node* node, eastl::function<void(const Node* inv_neig)>) const = 0;
    virtual uint32_t foreach_inv_neighbors(const dep_graph_handle_t node, eastl::function<void(const Node* inv_neig)>) const = 0;
    virtual uint32_t outgoing_edges(const Node* node) = 0;
    virtual uint32_t outgoing_edges(dep_graph_handle_t id) = 0;
    virtual uint32_t foreach_outgoing_edges(dep_graph_handle_t node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t foreach_outgoing_edges(Node* node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t incoming_edges(const Node* node) = 0;
    virtual uint32_t incoming_edges(dep_graph_handle_t id) = 0;
    virtual uint32_t foreach_incoming_edges(Node* node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t foreach_incoming_edges(dep_graph_handle_t node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
};

inline DependencyGraphNode* DependencyGraphEdge::from()
{
    return graph->node_at(from_node);
}
inline DependencyGraphNode* DependencyGraphEdge::to()
{
    return graph->node_at(to_node);
}

} // namespace skr