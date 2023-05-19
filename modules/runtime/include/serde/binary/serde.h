#pragma once
#include <type_traits>
#include <numeric>
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
}
}