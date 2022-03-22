#include "utils/DAG.boost.hpp"
#include "utils/dependency_graph.hpp"

namespace sakura
{
class DependencyGraphImpl
    : public DependencyGraph,
      public DAG::Graph<DependencyGraph::Node*, DependencyGraph::Edge*>
{
    using DAGVertex = DAG::GraphVertex<DependencyGraph::Node*, DependencyGraph::Edge*>;
    using DAGEdge = DAG::GraphEdge<DependencyGraph::Node*, DependencyGraph::Edge*>;

public:
    virtual dep_graph_handle_t insert(Node* Node) final
    {
        Node->id = (dep_graph_handle_t)DAG::add_vertex(Node, *this);
        Node->graph = this;
        return Node->id;
    }
    virtual bool link(Node* from, Node* to, Edge* edge) final
    {
#ifdef _DEBUG
        auto old_edge = boost::edge(get_descriptor(from), get_descriptor(to), *this);
        assert(!old_edge.second && "edge already exists!");
#endif
        auto&& result = DAG::add_edge(get_descriptor(from), get_descriptor(to), edge, *this);
        edge->graph = this;
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
        boost::remove_edge(find_edge.first, *this);
        return true;
    }
    virtual Node* node_at(dep_graph_handle_t ID) final
    {
        return (*this)[DAGVertex(ID)];
    }
    virtual uint32_t outgoing_edges(Node* node) final
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
            func(node_at(iter->m_source), node_at(iter->m_target), (Edge*)iter->get_property());
            count++;
        }
        return count;
    }
    virtual uint32_t incoming_edges(Node* node) final
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
            func(node_at(iter->m_source), node_at(iter->m_target), (Edge*)iter->get_property());
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
DependencyGraph* DependencyGraph::Create() RUNTIME_NOEXCEPT
{
    return new DependencyGraphImpl();
}
} // namespace sakura