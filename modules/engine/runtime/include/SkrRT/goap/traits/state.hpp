#pragma once
#include "SkrRT/goap/traits/compare.hpp"

namespace skr::goap
{
namespace concepts
{
template <typename T>
concept AtomValue = std::is_same_v<T, bool> ||
                    std::is_same_v<T, uint32_t> ||
                    (std::is_enum_v<T> && (sizeof(std::underlying_type_t<T>) <= sizeof(uint32_t)));

template <typename T>
inline constexpr bool IsStaticState = true; /*TODO*/

template <typename T>
concept StaticState = IsStaticState<T>;

template <typename T>
concept IdentifierType = true;

template <typename T>
concept VariableType = goap::concepts::IsComparable<T>;
} // namespace concepts

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
using StateMap = skr::Map<Identifier, Variable>;

template <concepts::StaticState T, StringLiteral Literal>
struct StaticWorldState;

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
struct DynamicWorldState;

namespace concepts
{
template <typename T>
inline constexpr bool IsStaticWorldState = false;

template <concepts::StaticState T, StringLiteral Literal>
inline constexpr bool IsStaticWorldState<StaticWorldState<T, Literal>> = true;

template <typename T>
inline constexpr bool IsDynamicWorldState = false;

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
inline constexpr bool IsDynamicWorldState<DynamicWorldState<Identifier, Variable>> = true;

template <typename T>
concept DynamicWorldState = IsDynamicWorldState<T>;

template <typename T>
concept StaticWorldState = IsStaticWorldState<T>;

template <typename T>
concept WorldState = IsStaticWorldState<T> || IsDynamicWorldState<T>;
} // namespace concepts

template <concepts::WorldState StateType>
struct Action;
} // namespace skr::goap