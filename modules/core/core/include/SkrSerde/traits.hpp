#pragma once
#include "SkrBase/misc/traits.hpp"

// complete serde
namespace skr
{
template <class T>
struct SerdeCompleteChecker : std::true_type {
};
template <class T>
inline constexpr bool is_complete_serde()
{
    if constexpr (skr::is_complete_v<T>)
    {
        return SerdeCompleteChecker<T>::value;
    }
    else
        return false;
}
template <class T>
constexpr bool is_complete_serde_v = is_complete_serde<T>();
} // namespace skr