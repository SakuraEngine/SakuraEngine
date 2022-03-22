#pragma once
#include "platform/configure.h"
#include <EASTL/functional.h>

namespace sakura
{
typedef uint64_t dep_graph_handle_t;
class DependencyGraphNode
{
    friend class DependencyGraphImpl;

public:
    DependencyGraphNode() = default;
    using Type = DependencyGraphNode;
    virtual ~DependencyGraphNode() RUNTIME_NOEXCEPT = default;
    // Nodes can't be copied
    DependencyGraphNode(const Type&) RUNTIME_NOEXCEPT = delete;

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
    virtual ~DependencyGraphEdge() RUNTIME_NOEXCEPT = default;
    // Edges can't be copied
    DependencyGraphEdge(const Type&) RUNTIME_NOEXCEPT = delete;

private:
    class DependencyGraph* graph;
};

class DependencyGraph
{
public:
    using Node = DependencyGraphNode;
    using Edge = DependencyGraphEdge;
    static DependencyGraph* Create() RUNTIME_NOEXCEPT;
    virtual ~DependencyGraph() RUNTIME_NOEXCEPT = default;
    virtual dep_graph_handle_t insert(Node* Node) = 0;
    virtual bool link(Node* from, Node* to, Edge* edge = nullptr) = 0;
    virtual Edge* linkage(Node* from, Node* to) = 0;
    virtual Edge* linkage(dep_graph_handle_t from, dep_graph_handle_t to) = 0;
    virtual bool unlink(Node* from, Node* to) = 0;
    virtual bool unlink(dep_graph_handle_t from, dep_graph_handle_t to) = 0;
    virtual Node* node_at(dep_graph_handle_t ID) = 0;
    virtual uint32_t outgoing_edges(Node* node) = 0;
    virtual uint32_t outgoing_edges(dep_graph_handle_t id) = 0;
    virtual uint32_t foreach_outgoing_edges(dep_graph_handle_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t foreach_outgoing_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t incoming_edges(Node* node) = 0;
    virtual uint32_t incoming_edges(dep_graph_handle_t id) = 0;
    virtual uint32_t foreach_incoming_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t foreach_incoming_edges(dep_graph_handle_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual uint32_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
};
} // namespace sakura