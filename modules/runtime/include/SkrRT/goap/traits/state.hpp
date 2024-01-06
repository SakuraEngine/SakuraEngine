#pragma once
#include "SkrRT/goap/traits/compare.hpp"

namespace skr::goap
{
namespace concepts
{
template <typename T>
concept StaticState = std::is_same_v<T, bool> || (std::is_enum_v<T> && (sizeof(std::underlying_type_t<T>) <= sizeof(uint32_t)));

template <typename T>
concept IdentifierType = true;

template <typename T>
concept VariableType = goap::concepts::IsComparable<T>;
} // namespace concepts

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
using StateMap = skr::UMap<Identifier, Variable>;

struct StaticWorldState;

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
struct DynamicWorldState;

namespace concepts
{
template <typename T, typename U>
inline constexpr bool IsStaticWorldState = !std::is_base_of_v<U, T>;

template <typename T>
inline constexpr bool IsDynamicWorldState = skr::is_convertible_to_specialization_v<
    T, DynamicWorldState, typename T::IdentifierType, typename T::VariableType>;

template <typename T>
concept DynamicWorldState = IsDynamicWorldState<T>;

template <typename T, typename...Args>
concept StaticWorldState = IsStaticWorldState<T, Args...>;

template <typename T>
concept WorldState = IsDynamicWorldState<T>;
} // namespace concepts
}