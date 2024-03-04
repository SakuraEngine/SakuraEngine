#pragma once
#include "SkrRT/goap/traits.hpp"
#include "SkrCore/log.hpp"

namespace skr::goap
{
template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
struct CondType {
    using IdentifierType = Identifier;
    using ValueStoreType = Variable;

    struct Condition {
        EConditionType type;
        EVariableFlag  flag;
        ValueStoreType value;
    };

    void add(const IdentifierType& id, EVariableFlag flag,
             const ValueStoreType& value, EConditionType type) SKR_NOEXCEPT
    {
        conditions_.add(id, { type, flag, value });
    }

    template <typename F>
    bool foreachOperand(F&& func) const
    {
        for (const auto& [k, v] : conditions_)
        {
            bool fail = !func(k, v.flag, v.value, v.type);
            if (fail)
                return false;
        }
        return true;
    }

private:
    skr::Map<IdentifierType, Condition> conditions_;
};
} // namespace skr::goap