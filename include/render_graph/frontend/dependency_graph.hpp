#pragma once
#include "platform/configure.h"
#include <EASTL/functional.h>

namespace sakura
{
class RenderDependencyGraphNode
{
    friend class RenderDependencyGraphImpl;

public:
    RenderDependencyGraphNode() = default;
    using Type = RenderDependencyGraphNode;
    virtual ~RenderDependencyGraphNode() RUNTIME_NOEXCEPT = default;
    // Nodes can't be copied
    RenderDependencyGraphNode(const Type&) RUNTIME_NOEXCEPT = delete;

private:
    class RenderDependencyGraph* graph;
    size_t id;
};

class RenderDependencyGraphEdge
{
    friend class RenderDependencyGraphImpl;

public:
    RenderDependencyGraphEdge() = default;
    using Type = RenderDependencyGraphEdge;
    virtual ~RenderDependencyGraphEdge() RUNTIME_NOEXCEPT = default;
    // Edges can't be copied
    RenderDependencyGraphEdge(const Type&) RUNTIME_NOEXCEPT = delete;

private:
    class RenderDependencyGraph* graph;
};

class RenderDependencyGraph
{
public:
    using Node = RenderDependencyGraphNode;
    using Edge = RenderDependencyGraphEdge;
    static RenderDependencyGraph* Create() RUNTIME_NOEXCEPT;
    virtual ~RenderDependencyGraph() RUNTIME_NOEXCEPT = default;
    virtual size_t insert(Node* Node) = 0;
    virtual bool link(Node* from, Node* to, Edge* edge) = 0;
    virtual Edge* linkage(Node* from, Node* to) = 0;
    virtual Edge* linkage(size_t from, size_t to) = 0;
    virtual bool unlink(Node* from, Node* to) = 0;
    virtual bool unlink(size_t from, size_t to) = 0;
    virtual Node* node_at(size_t ID) = 0;
    virtual size_t outgoing_edges(Node* node) = 0;
    virtual size_t outgoing_edges(size_t id) = 0;
    virtual size_t foreach_outgoing_edges(size_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual size_t foreach_outgoing_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual size_t incoming_edges(Node* node) = 0;
    virtual size_t incoming_edges(size_t id) = 0;
    virtual size_t foreach_incoming_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual size_t foreach_incoming_edges(size_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
    virtual size_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)>) = 0;
};
} // namespace sakura