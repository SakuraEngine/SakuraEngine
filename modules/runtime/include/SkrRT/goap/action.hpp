#pragma once
#include "SkrRT/goap/static_state.hpp"
#include "SkrRT/goap/dynamic_state.hpp"
#include "SkrRT/containers/vector.hpp"

namespace skr::goap
{

template <concepts::VariableType T>
inline static bool DoValueCompare(EConditionType t, const T& lhs, const T& rhs);

template <concepts::WorldState StateType>
struct Action {
    using IdentifierType = typename StateType::IdentifierType;
    using ValueStoreType   = typename StateType::ValueStoreType;
    using Predicates     = bool (*)(const StateMap<IdentifierType, ValueStoreType>&);
    struct Condition {
        EConditionType t;
        EVariableFlag f;
        ValueStoreType v;
    };

    Action(const char8_t* name, CostType cost = 0) SKR_NOEXCEPT
        : cost_(cost)
    {
#ifdef SKR_GOAP_SET_NAME
        name_ = name;
#endif
    }

    template <concepts::AtomValue VariableType>
    Action& add_condition(const IdentifierType& id, EVariableFlag flag,
                       const VariableType& value, EConditionType type = EConditionType::Equal) SKR_NOEXCEPT
    {
        conditions_.add_or_assign(id, {type, flag, value});
        return *this;
    }

    /*
    Action& add_condition(Predicates cmp) SKR_NOEXCEPT
    {
        predicates_.add(cmp);
        return *this;
    }
    */

    template <concepts::AtomValue VariableType>
    Action& add_effect(const IdentifierType& id, const VariableType& value) SKR_NOEXCEPT
    {
        effects_.add_or_assign(id, value);
        return *this;
    }

    bool operable_on(const StateType& ws) const SKR_NOEXCEPT
    {
        for (const auto& [k, cond] : conditions_)
        {
            const auto& v = cond.v;
            const auto type = cond.t;
            const auto flag = cond.f;
            
            ValueStoreType value;
            auto found = ws.get_variable(k, value);
            if (!found && (flag == EVariableFlag::Any))
                continue;

            if (found && (flag == EVariableFlag::None))
                return false;
            if (!found && (flag == EVariableFlag::Explicit))
                return false;
            if (!DoValueCompare(type, v, value))
                return false;
        }
        /*
        for (const auto& predicate : predicates_)
        {
            if (!predicate(ws.variables_))
                return false;
        }
        */
        return true;
    }

    StateType act_on(StateType& ws) const SKR_NOEXCEPT
    {
        StateType tmp(ws);
        for (const auto& [k, v] : effects_)
        {
            tmp.set_variable(k, v);
        }
        return tmp;
    }

    CostType cost() const { return cost_; }
#ifdef SKR_GOAP_SET_NAME
    const char8_t* name() const { return name_.u8_str(); }
#else
    const char8_t* name() const { return u8"SET_NAME_NOT_ENABLED"; }
#endif

public:
    template <concepts::AtomValue VariableType>
    Action& exist_and_equal(const IdentifierType& id, const VariableType& value)
    {
        return add_condition(id, EVariableFlag::Explicit, value, EConditionType::Equal);
    }

    template <concepts::AtomValue VariableType>
    Action& exist_and_nequal(const IdentifierType& id, const VariableType& value)
    {
        return add_condition(id, EVariableFlag::Explicit, value, EConditionType::NotEqual);
    }

    template <concepts::AtomValue VariableType>
    Action& none_or_equal(const IdentifierType& id, const VariableType& value)
    {
        return add_condition(id, EVariableFlag::Any, value, EConditionType::Equal);
    }

    template <concepts::AtomValue VariableType>
    Action& none_or_nequal(const IdentifierType& id, const VariableType& value)
    {
        return add_condition(id, EVariableFlag::Any, value, EConditionType::NotEqual);
    }

    template <concepts::AtomValue VariableType>
    Action& none(const IdentifierType& id)
    {
        return add_condition(id, EVariableFlag::None, {}, EConditionType::Equal);
    }

protected:
#ifdef SKR_GOAP_SET_NAME
    skr::String name_;
#endif
    CostType                               cost_ = 0;
    // skr::Vector<Predicates>                predicates_;
    skr::UMap<IdentifierType, Condition> conditions_;
    StateMap<IdentifierType, ValueStoreType> effects_;
};

template <concepts::VariableType T>
inline static bool DoValueCompare(EConditionType t, const T& lhs, const T& rhs)
{
    switch (t)
    {
    case EConditionType::Equal:
        return Compare<T>::Equal(lhs, rhs);
    case EConditionType::NotEqual:
        return Compare<T>::NotEqual(lhs, rhs);
    case EConditionType::Greater:
        return Compare<T>::Greater(lhs, rhs);
    case EConditionType::GreaterEqual:
        return Compare<T>::GreaterEqual(lhs, rhs);
    case EConditionType::Less:
        return Compare<T>::Less(lhs, rhs);
    case EConditionType::LessEqual:
        return Compare<T>::LessEqual(lhs, rhs);
    default:
        return false;
    }
}

} // namespace skr::goap
