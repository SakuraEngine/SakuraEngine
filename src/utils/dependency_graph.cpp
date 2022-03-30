#include "boost/graph/named_graph.hpp"
#include "utils/DAG.boost.hpp"
#include "utils/dependency_graph.hpp"

namespace sakura
{
class DependencyGraphImpl : public DependencyGraph, public DependencyGraphBase
{
    using DAGVertex = DependencyGraphBase::DAGVertex;
    using DAGEdge = DependencyGraphBase::DAGEdge;

public:
    virtual dep_graph_handle_t insert(Node* node) final
    {
        node->id = (dep_graph_handle_t)DAG::add_vertex(node, *this);
        node->graph = this;
        node->on_insert();
        return node->id;
    }
    virtual Node* access_node(dep_graph_handle_t handle) final
    {
        return (*this)[DAGVertex(handle)];
    }
    virtual bool remove(dep_graph_handle_t node) final
    {
        boost::remove_vertex(node, *this);
        (*this)[node]->on_remove();
        return true;
    }
    virtual bool remove(Node* node) final
    {
        return remove(node->id);
    }
    virtual bool clear() final
    {
        DAG::Graph<DependencyGraph::Node*, DependencyGraph::Edge*>::clear();
        return true;
    }
    virtual bool link(Node* from, Node* to, Edge* edge) final
    {
#ifdef _DEBUG
        auto old_edge = boost::edge(get_descriptor(from), get_descriptor(to), *this);
        assert(!old_edge.second && "edge already exists!");
#endif
        auto&& result = DAG::add_edge(get_descriptor(from), get_descriptor(to), edge, *this);
        edge->graph = this;
        edge->from_node = from->get_id();
        edge->to_node = to->get_id();
        edge->on_link();
        return result.second;
    }
    virtual Edge* linkage(Node* from, Node* to) final
    {
        return linkage(from->id, to->id);
    }
    virtual Edge* linkage(dep_graph_handle_t from, dep_graph_handle_t to) final
    {
        auto find_edge = boost::edge((vertex_descriptor)from, (vertex_descriptor)to, *this);
        if (find_edge.second)
        {
            return (*this)[find_edge.first];
        }
        return nullptr;
    }
    virtual bool unlink(Node* from, Node* to) final
    {
        return unlink(from->id, to->id);
    }
    virtual bool unlink(dep_graph_handle_t from, dep_graph_handle_t to) final
    {
        auto find_edge = boost::edge((vertex_descriptor)from, (vertex_descriptor)to, *this);
        if (!find_edge.second) return false;
        (*this)[find_edge.first]->on_unlink();
        boost::remove_edge(find_edge.first, *this);
        return true;
    }
    virtual Node* node_at(dep_graph_handle_t ID) final
    {
        return (*this)[DAGVertex(ID)];
    }
    virtual Node* from_node(Edge* edge) final
    {
        return node_at(edge->from_node);
    }
    virtual Node* to_node(Edge* edge) final
    {
        return node_at(edge->to_node);
    }
    virtual uint32_t foreach_neighbors(Node* node, eastl::function<void(DependencyGraphNode*)> f) final
    {
        return foreach_neighbors(node->get_id(), f);
    }
    virtual uint32_t foreach_neighbors(dep_graph_handle_t node, eastl::function<void(DependencyGraphNode*)> f) final
    {
        DAGVertex vert(node);
        auto neigs = DAG::adjacent_vertices(vert, *this);
        uint32_t count;
        for (auto iter = neigs.first; iter != neigs.second; iter++)
        {
            f((*this)[*iter]);
            count++;
        }
        return count;
    }
    virtual uint32_t foreach_neighbors(const Node* node, eastl::function<void(const DependencyGraphNode*)> f) const final
    {
        return foreach_neighbors(node->get_id(), f);
    }
    virtual uint32_t foreach_neighbors(const dep_graph_handle_t node, eastl::function<void(const DependencyGraphNode*)> f) const final
    {
        DAGVertex vert(node);
        auto neigs = DAG::adjacent_vertices(vert, *this);
        uint32_t count;
        for (auto iter = neigs.first; iter != neigs.second; iter++)
        {
            f((*this)[*iter]);
            count++;
        }
        return count;
    }
    virtual uint32_t foreach_inv_neighbors(Node* node, eastl::function<void(DependencyGraphNode*)> f) final
    {
        return foreach_inv_neighbors(node->get_id(), f);
    }
    virtual uint32_t foreach_inv_neighbors(dep_graph_handle_t node, eastl::function<void(DependencyGraphNode*)> f) final
    {
        DAGVertex vert(node);
        auto neigs = DAG::inv_adjacent_vertices(vert, *this);
        uint32_t count;
        for (auto iter = neigs.first; iter != neigs.second; iter++)
        {
            f((*this)[*iter]);
            count++;
        }
        return count;
    }
    virtual uint32_t foreach_inv_neighbors(const Node* node, eastl::function<void(const DependencyGraphNode*)> f) const final
    {
        return foreach_inv_neighbors(node->get_id(), f);
    }
    virtual uint32_t foreach_inv_neighbors(const dep_graph_handle_t node, eastl::function<void(const DependencyGraphNode*)> f) const final
    {
        DAGVertex vert(node);
        auto neigs = DAG::inv_adjacent_vertices(vert, *this);
        uint32_t count;
        for (auto iter = neigs.first; iter != neigs.second; iter++)
        {
            f((*this)[*iter]);
            count++;
        }
        return count;
    }
    virtual uint32_t outgoing_edges(const Node* node) final
    {
        return outgoing_edges(node->id);
    }
    virtual uint32_t outgoing_edges(dep_graph_handle_t id) final
    {
        auto oedges = DAG::out_edges((vertex_descriptor)id, *this);
        uint32_t count = 0;
        for (auto iter = oedges.first; iter != oedges.second; iter++)
        {
            count++;
        }
        return count;
    }
    virtual uint32_t foreach_outgoing_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        return foreach_outgoing_edges(node->id, func);
    }
    virtual uint32_t foreach_outgoing_edges(dep_graph_handle_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        auto oedges = DAG::out_edges((vertex_descriptor)node, *this);
        uint32_t count = 0;
        for (auto iter = oedges.first; iter != oedges.second; iter++)
        {
            func(node_at(iter->m_source), node_at(iter->m_target), (*this)[*iter]);
            count++;
        }
        return count;
    }
    virtual uint32_t incoming_edges(const Node* node) final
    {
        return incoming_edges(node->id);
    }
    virtual uint32_t incoming_edges(dep_graph_handle_t id) final
    {
        auto iedges = DAG::in_edges((vertex_descriptor)id, *this);
        uint32_t count = 0;
        for (auto iter = iedges.first; iter != iedges.second; iter++)
        {
            count++;
        }
        return count;
    }
    virtual uint32_t foreach_incoming_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        return foreach_incoming_edges(node->id, func);
    }
    virtual uint32_t foreach_incoming_edges(dep_graph_handle_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        auto oedges = DAG::in_edges((vertex_descriptor)node, *this);
        uint32_t count = 0;
        for (auto iter = oedges.first; iter != oedges.second; iter++)
        {
            func(node_at(iter->m_source), node_at(iter->m_target), (*this)[*iter]);
            count++;
        }
        return count;
    }
    virtual uint32_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        auto edges = boost::edges(*this);
        uint32_t count = 0;
        for (auto iter = edges.first; iter != edges.second; iter++)
        {
            func(node_at(iter->m_source), node_at(iter->m_target), (*this)[*iter]);
            count++;
        }
        return count;
    }

protected:
    vertex_descriptor get_descriptor(Node* node)
    {
        return (vertex_descriptor)node->id;
    }
};

uint32_t DependencyGraphNode::outgoing_edges()
{
    return graph->outgoing_edges(this);
}

uint32_t DependencyGraphNode::incoming_edges()
{
    return graph->incoming_edges(this);
}

uint32_t DependencyGraphNode::foreach_neighbors(eastl::function<void(DependencyGraphNode* neig)> f)
{
    return graph->foreach_neighbors(this, f);
}

uint32_t DependencyGraphNode::foreach_neighbors(eastl::function<void(const DependencyGraphNode* neig)> f) const
{
    return graph->foreach_neighbors(this, f);
}

uint32_t DependencyGraphNode::foreach_inv_neighbors(eastl::function<void(DependencyGraphNode* inv_neig)> f)
{
    return graph->foreach_inv_neighbors(this, f);
}

uint32_t DependencyGraphNode::foreach_inv_neighbors(eastl::function<void(const DependencyGraphNode* inv_neig)> f) const
{
    return graph->foreach_inv_neighbors(this, f);
}
} // namespace sakura

namespace sakura
{
DependencyGraph* DependencyGraph::Create() RUNTIME_NOEXCEPT
{
    return new DependencyGraphImpl();
}

DependencyGraphBase* DependencyGraphBase::as(DependencyGraph* interface)
{
    return (DependencyGraphImpl*)interface;
}
} // namespace sakura