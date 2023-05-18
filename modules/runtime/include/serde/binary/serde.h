#pragma once
#include <type_traits>
#include <numeric>
namespace skr
{
namespace binary
{
template<class T>
struct FloatingSerdeConfig {
    using type = T;
    bool asIntegers = false;
    //scale before convert to integer
    T scale = 1.0f;
    using Integer = std::conditional_t<std::is_same_v<T, float>, int32_t, int64_t>;
    T max = std::numeric_limits<Integer>::max();
};

template<class T>
struct IntegerSerdeConfig {
    using type = T;
    T min = std::numeric_limits<T>::min();
    T max = std::numeric_limits<T>::max();
};

template<class T>
struct VectorSerdeConfig {
    using type = T;
    float scale = 1.0f;
};
}
}