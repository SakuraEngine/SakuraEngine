#pragma once
#include "SkrRT/goap/state.hpp"

namespace skr::goap
{

template <concepts::WorldState StateType>
struct SKR_RUNTIME_API Action {
    using IdentifierType = typename StateType::IdentifierType;
    using VariableType   = typename StateType::VariableType;

    Action(const char8_t* name, CostType cost) SKR_NOEXCEPT
        : cost_(cost)
    {
#ifdef SKR_GOAP_SET_NAME
        name_ = name;
#endif
    }

    void add_condition(const IdentifierType& id, const VariableType& value) SKR_NOEXCEPT
    {
        conditions_.add_or_assign(id, value);
    }

    void add_effect(const IdentifierType& id, const VariableType& value) SKR_NOEXCEPT
    {
        effects_.add_or_assign(id, value);
    }

    bool operate_on(const StateType& ws) const SKR_NOEXCEPT
    {
        for (const auto& precond : conditions_)
        {
            auto found = ws.variables_.find(precond.first);
            if (found->data)
            if (ws.variables_.at(precond.first) != precond.second)
            {
                return false;
            }
        }
        return true;
    }

    bool act_on(StateType& ws) const SKR_NOEXCEPT
    {
        StateType tmp(ws);
        for (const auto& effect : effects_)
        {
            tmp.set_variable(effect.first, effect.second);
        }
        return tmp;
    }

    CostType cost() const { return cost_; }
#ifdef SKR_GOAP_SET_NAME
    const char8_t* name() const { return name_.u8_str(); }
#else
    const char8_t* name() const { return u8"SET_NAME_NOT_ENABLED"; }
#endif

protected:
#ifdef SKR_GOAP_SET_NAME
    skr::String name_;
#endif
    CostType                                cost_ = 0;
    skr::UMap<IdentifierType, VariableType> conditions_;
    skr::UMap<IdentifierType, VariableType> effects_;
};

} // namespace skr::goap
