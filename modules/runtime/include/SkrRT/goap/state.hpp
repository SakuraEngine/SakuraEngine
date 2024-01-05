#pragma once
#include "SkrRT/goap/config.hpp"

namespace skr::goap {

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
struct SKR_RUNTIME_API WorldState {
    using IdentifierType = Identifier;
    using VariableType = Variable;
    
    virtual ~WorldState() = default;

    bool set_variable(const Identifier& id, const VariableType& value) SKR_NOEXCEPT;
    bool get_variable(const Identifier& id, VariableType& value) SKR_NOEXCEPT;
    bool meets_goal(const WorldState& goal) const SKR_NOEXCEPT;
    CostType distance_to(const WorldState& goal) const SKR_NOEXCEPT;
    bool operator==(const WorldState& other) const SKR_NOEXCEPT;

protected:
    PriorityType priority_ = 0.f;
    skr::String name_ = u8"";
    skr::UMap<Identifier, VariableType> variables_;
};

}