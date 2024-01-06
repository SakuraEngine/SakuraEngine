#pragma once
#include "SkrRT/goap/static/state.hpp"
#include "SkrRT/goap/dynamic/state.hpp"
#include "SkrRT/containers/vector.hpp"

namespace skr::goap
{

template <concepts::VariableType T>
inline static bool DoValueCompare(EConditionType t, const T& lhs, const T& rhs);

template <concepts::WorldState StateType>
struct Action {
    using IdentifierType = typename StateType::IdentifierType;
    using ValueStoreType = typename StateType::ValueStoreType;
    using Predicates     = bool (*)(const StateMap<IdentifierType, ValueStoreType>&);
    struct Condition {
        EConditionType t;
        EVariableFlag  f;
        ValueStoreType v;
    };

    Action(const char8_t* name, CostType cost = 0) SKR_NOEXCEPT
        : cost_(cost)
    {
#ifdef SKR_GOAP_SET_NAME
        name_ = name;
#endif
    }

    template <concepts::AtomValue ValueType>
    Action& add_condition(const IdentifierType& id, EVariableFlag flag,
                          const ValueType& value, EConditionType type = EConditionType::Equal) SKR_NOEXCEPT
    {
        conditions_.add_or_assign(id, { type, flag, static_cast<ValueStoreType>(value) });
        return *this;
    }

    template <auto Member> 
    requires( concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member> )
    Action& add_condition(EVariableFlag flag, const AtomValueType<typename MemberInfo<Member>::Type>& value, EConditionType type = EConditionType::Equal) SKR_NOEXCEPT
    {
        return add_condition(atom_id<Member>, flag, value, type);
    }

    template <concepts::AtomValue ValueType>
    Action& add_effect(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        effects_.set(id, value);
        return *this;
    }

    template <auto Member> 
    requires( concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member> )
    Action& add_effect(const AtomValueType<typename MemberInfo<Member>::Type>& value) SKR_NOEXCEPT
    {
        return add_effect(atom_id<Member>, value);
    }

    bool operable_on(const StateType& ws) const SKR_NOEXCEPT
    {
        for (const auto& [k, cond] : conditions_)
        {
            const auto& v    = cond.v;
            const auto  type = cond.t;
            const auto  flag = cond.f;

            ValueStoreType value;
            auto           found = ws.get_variable(k, value);
            if (!found && (flag == EVariableFlag::Optional))
                continue;

            if (found && (flag == EVariableFlag::None))
                return false;
            if (!found && (flag == EVariableFlag::Explicit))
                return false;
            if (!DoValueCompare(type, v, value))
                return false;
        }
        return true;
    }

    StateType act_on(StateType& ws) const SKR_NOEXCEPT
    {
        StateType tmp(ws);
        effects_.foreach_variable([&](const auto& k, const auto& v) { tmp.set(k, v); });
        return tmp;
    }

    CostType cost() const { return cost_; }
#ifdef SKR_GOAP_SET_NAME
    const char8_t* name() const { return name_.u8_str(); }
#else
    const char8_t* name() const { return u8"SET_NAME_NOT_ENABLED"; }
#endif

public:
    template <concepts::AtomValue ValueType>
    Action& exist_and_equal(const IdentifierType& id, const ValueType& value)
    {
        return add_condition(id, EVariableFlag::Explicit, value, EConditionType::Equal);
    }

    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& exist_and_equal(const AtomValueType<typename MemberInfo<Member>::Type>& value)
    {
        return exist_and_equal(atom_id<Member>, value);
    }

    template <concepts::AtomValue ValueType>
    Action& exist_and_nequal(const IdentifierType& id, const ValueType& value)
    {
        return add_condition(id, EVariableFlag::Explicit, value, EConditionType::NotEqual);
    }

    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& exist_and_nequal(const AtomValueType<typename MemberInfo<Member>::Type>& value)
    {
        return exist_and_nequal(atom_id<Member>, value);
    }

    template <concepts::AtomValue ValueType>
    Action& none_or_equal(const IdentifierType& id, const ValueType& value)
    {
        return add_condition(id, EVariableFlag::Optional, value, EConditionType::Equal);
    }

    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& none_or_equal(const AtomValueType<typename MemberInfo<Member>::Type>& value)
    {
        return none_or_equal(atom_id<Member>, value);
    }

    template <concepts::AtomValue ValueType>
    Action& none_or_nequal(const IdentifierType& id, const ValueType& value)
    {
        return add_condition(id, EVariableFlag::Optional, value, EConditionType::NotEqual);
    }

    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& none_or_nequal(const AtomValueType<typename MemberInfo<Member>::Type>& value)
    {
        return none_or_nequal(atom_id<Member>, value);
    }

    Action& none(const IdentifierType& id)
    {
        return add_condition(id, EVariableFlag::None, {}, EConditionType::Equal);
    }

    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& none()
    {
        return none(atom_id<Member>);
    }

protected:
#ifdef SKR_GOAP_SET_NAME
    skr::String name_;
#endif
    CostType cost_ = 0;
    skr::UMap<IdentifierType, Condition>     conditions_;
    StateType effects_;
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
