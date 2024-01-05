#pragma once
#include "SkrRT/containers/vector.hpp"
#include "SkrRT/goap/state.hpp"
#include "SkrRT/goap/action.hpp"

namespace skr::goap
{

template <concepts::WorldState StateType>
struct SKR_RUNTIME_API Planner {
    using IdentifierType = typename StateType::IdentifierType;
    using VariableType   = typename StateType::VariableType;
    using ActionType     = Action<StateType>;

    Planner();
    void dumpOpenList() const SKR_NOEXCEPT;
    void dumpCloseList() const SKR_NOEXCEPT;
    skr::Vector<ActionType> plan(const StateType& start, const StateType& goal, const skr::Vector<ActionType>& actions) SKR_NOEXCEPT;

protected:
    struct Node {
        StateType         ws_;
        NodeId            id_        = 0;
        NodeId            parent_id_ = 0;
        CostType          g_         = 0;       // The A* cost from 'start' to 'here'
        CostType          h_         = 0;       // The estimated remaining cost to 'goal' form 'here'
        const ActionType* action_    = nullptr; // The action that got us here (for replay purposes)

        Node() { id_ = ++Global::last_id_; }
        Node(const StateType state, CostType g, CostType h, NodeId parent_id, const ActionType* action)
            : ws_(state)
            , g_(g)
            , h_(h)
            , parent_id_(parent_id)
            , action_(action)
        {
            id_ = ++Global::last_id_;
        }

        // F -- which is simply G+H -- is autocalculated
        auto f() const { return g_ + h_; }

        bool operator<(const Node& rhs) const { return f() < rhs.f(); }
    };
    skr::Vector<Node> open_;
    skr::Vector<Node> close_;

    bool memberOfClosed(const StateType& ws) const SKR_NOEXCEPT;
    Node* memberOfOpen(const StateType& ws);
    Node& popAndClose();
    void addToOpenList(Node&&);
    CostType heuristic(const StateType& from, const StateType& to) const SKR_NOEXCEPT;
};
} // namespace skr::goap