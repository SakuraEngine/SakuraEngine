#pragma once
#include "SkrRT/goap/traits.hpp"
#include "SkrRT/misc/log.hpp"

namespace skr::goap
{

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
struct DynamicWorldState {
    using IdentifierType = Identifier;
    using ValueStoreType   = Variable;

    virtual ~DynamicWorldState() = default;

    DynamicWorldState& set_variable(const Identifier& id, const ValueStoreType& value) SKR_NOEXCEPT
    {
        variables_.add_or_assign(id, value);
        return *this;
    }

    DynamicWorldState& assign_variable(const Identifier& id, const ValueStoreType& value) SKR_NOEXCEPT
    {
        auto found = variables_.find(id);
        if (!found) 
            return *this;
        found->value = value;
        return *this;
    }

    bool get_variable(const Identifier& id, ValueStoreType& value) const SKR_NOEXCEPT
    {
        auto found = variables_.find(id);
        if (!found) return false;
        value = found->value;
        return true;
    }

    bool meets_goal(const DynamicWorldState& goal) const SKR_NOEXCEPT
    {
        for (const auto& [k, v] : goal.variables_)
        {
            auto found = variables_.find(k);
            if (!found)
                return false;
            if (Compare<ValueStoreType>::NotEqual(v, found->value)) 
                return false;
        }
        return true;
    }

    uint64_t distance_to(const DynamicWorldState& goal) const SKR_NOEXCEPT
    {
        uint64_t distance = 0;
        for (const auto& [k, v] : goal.variables_)
        {
            auto found = variables_.find(k);
            if (!found || Compare<ValueStoreType>::NotEqual(v, found->value))
                distance += 1;
        }
        return distance;
    }

    bool operator==(const DynamicWorldState& other) const SKR_NOEXCEPT
    {
        for (const auto& [k, v] : other.variables_)
        {
            auto found = variables_.find(k);
            if (!found || Compare<ValueStoreType>::NotEqual(v, found->value))
                return false;
        }
        return true;
    }

    void dump(const char8_t* what, int level = SKR_LOG_LEVEL_INFO) const
    {
        SKR_LOG_FMT_WITH_LEVEL(level, u8"{} DynamicWorldState: {}", what, name_);
        for (const auto& [k, v] : variables_)
        {
            SKR_LOG_FMT_WITH_LEVEL(level, u8"    {} = {}", k, v);
        }
    }

    PriorityType                        priority_ = 0.f;
    skr::String                         name_     = u8"";
    skr::UMap<Identifier, ValueStoreType> variables_;
};

} // namespace skr::goap