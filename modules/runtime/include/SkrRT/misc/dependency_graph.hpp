#pragma once
#include "SkrRT/platform/configure.h"
#include <EASTL/functional.h>

namespace skr
{
typedef uint64_t dag_id_t;
class DependencyGraphEdge;
class SKR_STATIC_API DependencyGraphNode
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
    const dag_id_t get_id() const SKR_NOEXCEPT { return id; }
    uint32_t outgoing_edges() SKR_NOEXCEPT;
    uint32_t incoming_edges() SKR_NOEXCEPT;
    uint32_t foreach_neighbors(eastl::function<void(DependencyGraphNode* neig)>) SKR_NOEXCEPT;
    uint32_t foreach_neighbors(eastl::function<void(const DependencyGraphNode* neig)>) const SKR_NOEXCEPT;
    uint32_t foreach_inv_neighbors(eastl::function<void(DependencyGraphNode* inv_neig)>) SKR_NOEXCEPT;
    uint32_t foreach_inv_neighbors(eastl::function<void(const DependencyGraphNode* inv_neig)>) const SKR_NOEXCEPT;

private:
    class DependencyGraph* graph;
    dag_id_t id;
};

class SKR_STATIC_API DependencyGraphEdge
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
    dag_id_t from_node;
    dag_id_t to_node;
};

class SKR_STATIC_API DependencyGraph
{
public:
    using Node = DependencyGraphNode;
    using Edge = DependencyGraphEdge;
    static DependencyGraph* Create() SKR_NOEXCEPT;
    static void Destroy(DependencyGraph* graph) SKR_NOEXCEPT;
    virtual ~DependencyGraph() SKR_NOEXCEPT = default;
    virtual dag_id_t insert(Node* node) SKR_NOEXCEPT = 0;
    virtual Node* access_node(dag_id_t handle) SKR_NOEXCEPT = 0;
    virtual bool remove(dag_id_t node) SKR_NOEXCEPT = 0;
    virtual bool remove(Node* node) SKR_NOEXCEPT = 0;
    virtual bool clear() SKR_NOEXCEPT = 0;
    virtual bool link(Node* from, Node* to, Edge* edge = nullptr) SKR_NOEXCEPT = 0;
    // virtual Edge* linkage(Node* from, Node* to) SKR_NOEXCEPT = 0;
    // virtual Edge* linkage(dag_id_t from, dag_id_t to) SKR_NOEXCEPT = 0;
    // virtual bool unlink(Node* from, Node* to) SKR_NOEXCEPT = 0;
    // virtual bool unlink(dag_id_t from, dag_id_t to) SKR_NOEXCEPT = 0;
    virtual Node* from_node(Edge* edge) SKR_NOEXCEPT = 0;
    virtual Node* to_node(Edge* edge) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(Node* node, eastl::function<void(Node* neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(dag_id_t node, eastl::function<void(Node* neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(const Node* node, eastl::function<void(const Node* neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_neighbors(const dag_id_t node, eastl::function<void(const Node* neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(Node* node, eastl::function<void(Node* inv_neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(dag_id_t node, eastl::function<void(Node* inv_neig)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(const Node* node, eastl::function<void(const Node* inv_neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_inv_neighbors(const dag_id_t node, eastl::function<void(const Node* inv_neig)>) const SKR_NOEXCEPT = 0;
    virtual uint32_t outgoing_edges(const Node* node) SKR_NOEXCEPT = 0;
    virtual uint32_t outgoing_edges(dag_id_t id) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_outgoing_edges(dag_id_t node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_outgoing_edges(Node* node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t incoming_edges(const Node* node) SKR_NOEXCEPT = 0;
    virtual uint32_t incoming_edges(dag_id_t id) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_incoming_edges(Node* node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_incoming_edges(dag_id_t node,
    eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
    virtual uint32_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)>) SKR_NOEXCEPT = 0;
};

inline DependencyGraphNode* DependencyGraphEdge::from() SKR_NOEXCEPT
{
    return graph->access_node(from_node);
}
inline DependencyGraphNode* DependencyGraphEdge::to() SKR_NOEXCEPT
{
    return graph->access_node(to_node);
}

} // namespace skr