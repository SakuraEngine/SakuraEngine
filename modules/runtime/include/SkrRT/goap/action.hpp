#pragma once
#include "SkrRT/goap/static/state.hpp"
#include "SkrRT/goap/dynamic/state.hpp"
#include "SkrRT/containers/vector.hpp"

namespace skr::goap
{

template <concepts::VariableType T>
inline static bool DoValueCompare(EConditionType cond, const T& lhs, const T& rhs);

template <concepts::WorldState StateType>
struct Action {
    using IdentifierType = typename StateType::IdentifierType;
    using ValueStoreType = typename StateType::ValueStoreType;
    using CondType       = typename StateType::CondType;

    Action(const char8_t* name, CostType cost = 0) SKR_NOEXCEPT
        : cost_(cost)
    {
#ifdef SKR_GOAP_SET_NAME
        name_ = name;
#endif
    }

    template <concepts::AtomValue ValueType>
    Action& add_condition(const IdentifierType& id, EVariableFlag flag,
                          const ValueType& value, EConditionType cond = EConditionType::Equal) SKR_NOEXCEPT
    {
        cond_.add(id, flag, static_cast<ValueStoreType>(value), cond);
        return *this;
    }

    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& add_condition(EVariableFlag flag, const AtomValueType<typename MemberInfo<Member>::Type>& value, EConditionType cond = EConditionType::Equal) SKR_NOEXCEPT
    {
        return add_condition(atom_id<Member>, flag, value, cond);
    }

    template <concepts::AtomValue ValueType>
    Action& add_effect(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        assign_effect_.set(id, value);
        return *this;
    }

    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& add_effect(const AtomValueType<typename MemberInfo<Member>::Type>& value) SKR_NOEXCEPT
    {
        return add_effect(atom_id<Member>, value);
    }

    bool operable_on(const StateType& ws) const SKR_NOEXCEPT
    {
        return cond_.foreachOperand([&](const auto& k, const auto& flag, const auto& expect, const auto& cond) {
            ValueStoreType value;
            auto           found = ws.get_variable(k, value);
            if (!found && (flag != EVariableFlag::Explicit))
                return true;

            if (found && (flag == EVariableFlag::None))
                return false;
            if (!found && (flag == EVariableFlag::Explicit))
                return false;
            return DoValueCompare(cond, expect, value);
        });
    }

    StateType act_on(StateType& ws) const SKR_NOEXCEPT
    {
        StateType tmp(ws);
        assign_effect_.foreachAtomValue([&](const auto& k, const auto& v) { tmp.set(k, v); return true; });
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
        return add_condition(id, EVariableFlag::None, ValueStoreType{}, EConditionType::Equal);
    }
    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& none()
    {
        return none(atom_id<Member>);
    }

    Action& exist(const IdentifierType& id)
    {
        return add_condition(id, EVariableFlag::Explicit, ValueStoreType{}, EConditionType::Exist);
    }
    template <auto Member>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& exist()
    {
        return exist(atom_id<Member>);
    }

    template <typename FlagType>
    Action& with_flag(const IdentifierType& id, const FlagType& value)
    {
        return add_condition(id, EVariableFlag::Explicit, static_cast<ValueStoreType>(value), EConditionType::And);
    }
    template <auto Member, typename FlagType>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& with_flag(const FlagType& value)
    {
        return with_flag(atom_id<Member>, value);
    }

    template <typename FlagType>
    Action& without_flag(const IdentifierType& id, const FlagType& value)
    {
        return add_condition(id, EVariableFlag::Optional, static_cast<ValueStoreType>(value), EConditionType::Nand);
    }
    template <auto Member, typename FlagType>
    requires(concepts::IsStaticWorldState<StateType> && concepts::IsAtomMember<Member>)
    Action& without_flag(const FlagType& value)
    {
        return without_flag(atom_id<Member>, value);
    }
protected:
#ifdef SKR_GOAP_SET_NAME
    skr::String name_;
#endif
    CostType  cost_ = 0;
    CondType  cond_;
    StateType assign_effect_;
};

template <concepts::VariableType T>
inline static bool DoValueCompare(EConditionType cond, const T& lhs, const T& rhs)
{
    switch (cond)
    {
        case EConditionType::Exist:
            return true;
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
        case EConditionType::And:
            return Compare<T>::And(lhs, rhs);
        case EConditionType::Nand:
            return !Compare<T>::And(lhs, rhs);
        default:
            SKR_ASSERT(false && "unknown condition type");
            return false;
    }
}

} // namespace skr::goap
