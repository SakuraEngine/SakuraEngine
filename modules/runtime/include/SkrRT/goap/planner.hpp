#pragma once
#include "SkrContainers/vector.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrRT/goap/dynamic/state.hpp"
#include "SkrRT/goap/action.hpp"

namespace skr::goap
{

template <concepts::WorldState TState, typename TAction = Action<TState>>
    requires(std::is_base_of_v<Action<TState>, TAction> || std::is_same_v<TAction, Action<TState>>)
struct Planner {
    using StateType = TState;
    using ActionType = TAction;
    using IdentifierType = typename StateType::IdentifierType;
    using ActionAndState = std::pair<ActionType, StateType>;

    template <bool> struct PlanTypeSelector;
    template <> struct PlanTypeSelector<true> { using Type = skr::Vector<ActionAndState>; };
    template <> struct PlanTypeSelector<false> { using Type = skr::Vector<ActionType>; };
    template <bool WithState> using PlanType = typename PlanTypeSelector<WithState>::Type;

    template <bool WithState = false>
    SKR_NOINLINE PlanType<WithState> plan(const StateType& start, const StateType& goal, const skr::Vector<ActionType>& actions) SKR_NOEXCEPT;

    // void dumpOpenList() const SKR_NOEXCEPT;
    // void dumpCloseList() const SKR_NOEXCEPT;

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
            , parent_id_(parent_id)
            , g_(g)
            , h_(h)
            , action_(action)
        {
            id_ = ++Global::last_id_;
        }

        // F -- which is simply G+H -- is autocalculated
        auto f() const { return g_ + h_; }

        bool operator<(const Node& rhs) const { return f() < rhs.f(); }
    };
    skr::stl_vector<Node> open_;
    skr::stl_vector<Node> closed_;

    CostType heuristic(const StateType& now, const StateType& goal) const SKR_NOEXCEPT
    {
        return now.distance_to(goal);
    }

    void addToOpenList(Node&& node)
    {
        // insert maintaining sort order
        auto it = std::lower_bound(begin(open_), end(open_), node);
        open_.emplace(it, std::move(node));
    }

    Node& popAndClose()
    {
        SKR_ASSERT(!open_.empty());
        closed_.emplace_back(std::move(open_.front()));
        open_.erase(open_.begin());
        return closed_.back();
    }

    bool memberOfClosed(const StateType& ws) const SKR_NOEXCEPT
    {
        auto&& found = std::find_if(begin(closed_), end(closed_), [&](const Node& n) { return n.ws_ == ws; });
        return found != end(closed_);
    }

    auto memberOfOpen(const StateType& ws) SKR_NOEXCEPT
    {
        return std::find_if(begin(open_), end(open_), [&](const Node& n) { return n.ws_ == ws; });
    }
};

template <concepts::WorldState StateType, typename ActionType>
    requires(std::is_base_of_v<Action<StateType>, ActionType> || std::is_same_v<ActionType, Action<StateType>>)
template <bool WithState>
SKR_NOINLINE auto Planner<StateType, ActionType>::plan(const StateType& start, const StateType& goal, const skr::Vector<ActionType>& actions) SKR_NOEXCEPT -> PlanType<WithState>
{
    using RetType = PlanType<WithState>;
    if (start.meets_goal(goal))
        return RetType();

    // Feasible we'd re-use a planner, so clear out the prior results
    open_.clear();
    closed_.clear();

    Node starting_node(start, 0, heuristic(start, goal), 0, nullptr);

    open_.emplace_back(std::move(starting_node));

    while (open_.size() > 0)
    {
        // Look for Node with the lowest-F-score on the open list. Switch it to closed,
        // and hang onto it -- this is our latest node.
        Node& current(popAndClose());
        // Is our current state the goal state? If so, we've found a path, yay.
        if (current.ws_.meets_goal(goal))
        {
            auto the_plan = RetType();
            do
            {
                if constexpr (WithState)
                    the_plan.emplace(*current.action_, current.ws_);
                else
                    the_plan.emplace(*current.action_);
                
                auto itr = std::find_if(begin(open_), end(open_), [&](const Node& n) { return n.id_ == current.parent_id_; });
                if (itr == end(open_))
                {
                    itr = std::find_if(begin(closed_), end(closed_), [&](const Node& n) { return n.id_ == current.parent_id_; });
                }
                current = *itr;
            } while (current.parent_id_ != 0);
            return the_plan;
        }

        // Check each node REACHABLE from current -- in other words, where can we go from here?
        for (const auto& potential_action : actions)
        {
            if (potential_action.operable_on(current.ws_))
            {
                StateType outcome = potential_action.act_on(current.ws_);

                // Skip if already closed
                if (memberOfClosed(outcome))
                    continue;

                // Look for a Node with this WorldState on the open list.
                auto p_outcome_node = memberOfOpen(outcome);
                if (p_outcome_node == end(open_))
                { // not a member of open list
                    // Make a new node, with current as its parent, recording G & H
                    Node found(outcome, current.g_ + potential_action.cost(), heuristic(outcome, goal), current.id_, &potential_action);
                    // Add it to the open list (maintaining sort-order therein)
                    addToOpenList(std::move(found));
                }
                else
                { // already a member of the open list
                    // check if the current G is better than the recorded G
                    if (current.g_ + potential_action.cost() < p_outcome_node->g_)
                    {
                        // std::cout << "My path to " << p_outcome_node->ws_ << " using " << potential_action.name() << " (combined cost " << current.g_ + potential_action.cost() << ") is better than existing (cost " <<  p_outcome_node->g_ << "\n";
                        p_outcome_node->parent_id_ = current.id_;                          // make current its parent
                        p_outcome_node->g_         = current.g_ + potential_action.cost(); // recalc G & H
                        p_outcome_node->h_         = heuristic(outcome, goal);
                        p_outcome_node->action_    = &potential_action;

                        // resort open list to account for the new F
                        // sorting likely invalidates the p_outcome_node iterator, but we don't need it anymore
                        std::sort(begin(open_), end(open_));
                    }
                }
            }
        }
    }
    start.dump(u8"START", SKR_LOG_LEVEL_ERROR);
    goal.dump(u8"GOAL", SKR_LOG_LEVEL_ERROR);
    SKR_LOG_FATAL(u8"A* planner could not find a path from start to goal");
    return RetType();
}

} // namespace skr::goap