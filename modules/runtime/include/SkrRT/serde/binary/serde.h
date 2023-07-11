#pragma once
#include "SkrRT/misc/traits.hpp"
#include "SkrRT/serde/binary/serde.h"
#include <limits>

namespace skr
{

namespace binary
{
template<class T>
struct FloatingPackConfig {
    using type = T;
    bool asIntegers = false;
    //scale before convert to integer
    T scale = 1.0f;
    using Integer = std::conditional_t<std::is_same_v<T, float>, int32_t, int64_t>;
    T max = std::numeric_limits<Integer>::max();
};

template<class T>
struct IntegerPackConfig {
    using type = T;
    T min = std::numeric_limits<T>::min();
    T max = std::numeric_limits<T>::max();
};

template<class T>
struct VectorPackConfig {
    using type = T;
    float scale = 1.0f;
};

template<class T, class E>
struct ContainerConfig {
    using type = T;
    using element = E;
};

struct ArrayCheckConfig {
    uint64_t max = std::numeric_limits<uint64_t>::max();
    uint64_t min = std::numeric_limits<uint64_t>::min();
};
enum class ErrorCode
{
    UnknownError = -1,
    Success = 0,
    OutOfRange = -2,
};

} // namespace binary

template<class T>
struct SerdeCompleteChecker : std::true_type {};

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