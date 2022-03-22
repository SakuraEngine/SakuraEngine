#include "utils/DAG.hpp"
#include "render_graph/frontend/dependency_graph.hpp"

namespace sakura
{
class RenderDependencyGraphImpl
    : public RenderDependencyGraph,
      public DAG::Graph<RenderDependencyGraph::Node*, RenderDependencyGraph::Edge*>
{
    using DAGVertex = DAG::GraphVertex<RenderDependencyGraph::Node*, RenderDependencyGraph::Edge*>;
    using DAGEdge = DAG::GraphEdge<RenderDependencyGraph::Node*, RenderDependencyGraph::Edge*>;

public:
    virtual size_t insert(Node* Node) final
    {
        Node->id = (size_t)DAG::add_vertex(Node, *this);
        Node->graph = this;
        return Node->id;
    }
    virtual bool link(Node* from, Node* to, Edge* edge) final
    {
#ifdef _DEBUG
        auto old_edge = boost::edge(get_descriptor(from), get_descriptor(to), *this);
        assert(!old_edge.second && "edge already exists!");
#endif
        auto&& result = DAG::add_edge(get_descriptor(from), get_descriptor(to), *this);
        (*this)[result.first] = edge;
        edge->graph = this;
        return result.second;
    }
    virtual Edge* linkage(Node* from, Node* to) final
    {
        return linkage(from->id, to->id);
    }
    virtual Edge* linkage(size_t from, size_t to) final
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
    virtual bool unlink(size_t from, size_t to) final
    {
        auto find_edge = boost::edge((vertex_descriptor)from, (vertex_descriptor)to, *this);
        if (!find_edge.second) return false;
        boost::remove_edge(find_edge.first, *this);
        return true;
    }
    virtual Node* node_at(size_t ID) final
    {
        return (*this)[DAGVertex(ID)];
    }
    virtual size_t outgoing_edges(Node* node) final
    {
        return outgoing_edges(node->id);
    }
    virtual size_t outgoing_edges(size_t id) final
    {
        auto oedges = DAG::out_edges((vertex_descriptor)id, *this);
        size_t count = 0;
        for (auto iter = oedges.first; iter != oedges.second; iter++)
        {
            count++;
        }
        return count;
    }
    virtual size_t foreach_outgoing_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        return foreach_outgoing_edges(node->id, func);
    }
    virtual size_t foreach_outgoing_edges(size_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        auto oedges = DAG::out_edges((vertex_descriptor)node, *this);
        size_t count = 0;
        for (auto iter = oedges.first; iter != oedges.second; iter++)
        {
            func(node_at(iter->m_source), node_at(iter->m_target), (Edge*)iter->get_property());
            count++;
        }
        return count;
    }
    virtual size_t incoming_edges(Node* node) final
    {
        return incoming_edges(node->id);
    }
    virtual size_t incoming_edges(size_t id) final
    {
        auto iedges = DAG::in_edges((vertex_descriptor)id, *this);
        size_t count = 0;
        for (auto iter = iedges.first; iter != iedges.second; iter++)
        {
            count++;
        }
        return count;
    }
    virtual size_t foreach_incoming_edges(Node* node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        return foreach_incoming_edges(node->id, func);
    }
    virtual size_t foreach_incoming_edges(size_t node,
        eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        auto oedges = DAG::in_edges((vertex_descriptor)node, *this);
        size_t count = 0;
        for (auto iter = oedges.first; iter != oedges.second; iter++)
        {
            func(node_at(iter->m_source), node_at(iter->m_target), (Edge*)iter->get_property());
            count++;
        }
        return count;
    }
    virtual size_t foreach_edges(eastl::function<void(Node* from, Node* to, Edge* edge)> func) final
    {
        auto edges = boost::edges(*this);
        size_t count = 0;
        for (auto iter = edges.first; iter != edges.second; iter++)
        {
            func(node_at(iter->m_source), node_at(iter->m_target), (Edge*)iter->get_property());
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
} // namespace sakura

namespace sakura
{
RenderDependencyGraph* RenderDependencyGraph::Create() RUNTIME_NOEXCEPT
{
    return new RenderDependencyGraphImpl();
}
} // namespace sakura