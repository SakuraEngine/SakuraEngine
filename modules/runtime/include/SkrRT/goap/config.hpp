#pragma once
#include "SkrBase/concepts/concepts.hpp"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/containers/umap.hpp"

#ifndef SKR_GOAP_SET_NAME
    #ifdef _DEBUG
        #define SKR_GOAP_SET_NAME
    #endif
#endif

namespace skr::goap
{
using CostType     = int64_t;
using PriorityType = float;
using NodeId       = uint64_t;
using NameType     = skr::String;

template <typename Identifier, typename Variable>
using MapType = skr::UMap<Identifier, Variable>;

template <typename T>
struct Compare;

template <typename T> requires(skr::concepts::IsComparable<T>)
struct Compare<T> {
    static bool Equal(const T& a, const T& b) SKR_NOEXCEPT { return a == b; }
    static bool NotEqual(const T& a, const T& b) SKR_NOEXCEPT { return a != b; }
};

namespace concepts
{
template <typename T>
inline constexpr bool IsComparable = requires(const T& a, const T& b) {
    {
        Compare<T>::Equal(a, b)
    } -> std::convertible_to<bool>;
    {
        Compare<T>::NotEqual(a, b)
    } -> std::convertible_to<bool>;
};

template <typename T>
concept IdentifierType = true;
template <typename T>
concept VariableType = goap::concepts::IsComparable<T>;

} // namespace concepts

template <concepts::IdentifierType Identifier, concepts::VariableType Variable>
struct WorldState;

namespace concepts
{
template <typename T>
inline constexpr bool IsWorldState = skr::is_convertible_to_specialization_v<
    T, WorldState, typename T::IdentifierType, typename T::VariableType>;

template <typename T>
concept WorldState = IsWorldState<T>;
} // namespace concepts

struct SKR_RUNTIME_API Global {
    static NodeId last_id_;
};

} // namespace skr::goap