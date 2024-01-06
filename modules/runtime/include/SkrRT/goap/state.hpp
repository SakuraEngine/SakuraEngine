#pragma once
#include "SkrRT/goap/config.hpp"
#include "SkrRT/misc/log.hpp"

namespace skr::goap
{

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
struct WorldState {
    using IdentifierType = Identifier;
    using VariableType   = Variable;

    virtual ~WorldState() = default;

    WorldState& set_variable(const Identifier& id, const VariableType& value) SKR_NOEXCEPT
    {
        variables_.add_or_assign(id, value);
        return *this;
    }

    WorldState& assign_variable(const Identifier& id, const VariableType& value) SKR_NOEXCEPT
    {
        auto found = variables_.find(id);
        if (!found) 
            return *this;
        found->value = value;
        return *this;
    }

    bool get_variable(const Identifier& id, VariableType& value) SKR_NOEXCEPT
    {
        auto found = variables_.find(id);
        if (!found) return false;
        value = found->value;
        return true;
    }

    bool meets_goal(const WorldState& goal) const SKR_NOEXCEPT
    {
        for (const auto& [k, v] : goal.variables_)
        {
            auto found = variables_.find(k);
            if (!found)
                return false;
            if (Compare<VariableType>::NotEqual(v, found->value)) 
                return false;
        }
        return true;
    }

    uint64_t distance_to(const WorldState& goal) const SKR_NOEXCEPT
    {
        uint64_t distance = 0;
        for (const auto& [k, v] : goal.variables_)
        {
            auto found = variables_.find(k);
            if (!found || Compare<VariableType>::NotEqual(v, found->value))
                distance += 1;
        }
        return distance;
    }

    bool operator==(const WorldState& other) const SKR_NOEXCEPT
    {
        for (const auto& [k, v] : other.variables_)
        {
            auto found = variables_.find(k);
            if (!found || Compare<VariableType>::NotEqual(v, found->value))
                return false;
        }
        return true;
    }

    void dump(const char8_t* what, int level = SKR_LOG_LEVEL_INFO) const
    {
        SKR_LOG_FMT_WITH_LEVEL(level, u8"{} WorldState: {}", what, name_);
        for (const auto& [k, v] : variables_)
        {
            SKR_LOG_FMT_WITH_LEVEL(level, u8"    {} = {}", k, v);
        }
    }

    PriorityType                        priority_ = 0.f;
    skr::String                         name_     = u8"";
    skr::UMap<Identifier, VariableType> variables_;
};

} // namespace skr::goap