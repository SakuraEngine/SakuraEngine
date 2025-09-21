#pragma once
#include <concepts>

namespace skr
{
inline namespace math
{
inline constexpr float kPi  = 3.14159265358979323846f;
inline constexpr float kPi2 = 6.28318530717958647692f; // 2 * pi

enum class EAxis3
{
    X = 0,
    Y = 1,
    Z = 2,
};
enum class EAxis4
{
    X = 0,
    Y = 1,
    Z = 2,
    W = 3,
};
enum class ECoordinateSystemHand
{
    LeftHand  = 0,
    RightHand = 1,
};

struct MathNoInitType
{
};
inline constexpr MathNoInitType kMathNoInit = MathNoInitType{};

// template maker
template <typename T, size_t kDimensions>
struct MathVectorMaker
{
    // using Type = float3; // make vector from type and dimension
};
template <typename T, size_t kDimensions>
struct MathMatrixMaker
{
    // using Type = float3x3; // make matrix from type and dimension
};
template <typename T, size_t kDimensions>
using vec_t = typename MathVectorMaker<T, kDimensions>::Type;
template <typename T, size_t kDimensions>
using matrix_t = typename MathMatrixMaker<T, kDimensions>::Type;

// traits
template <typename T>
struct MathVectorTraits
{
    // inline static constexpr size_t kDimensions = 3; // get vector dimension
    // using ComponentType = float; // get vector component type
    // using ImplementationFlag = void;
};
template <typename T>
struct MathMatrixTraits
{
    // inline static constexpr size_t kDimensions = 3; // get matrix dimension
    // using ComponentType = float; // get matrix component type
    // using ImplementationFlag = void;
};
template <typename T>
concept MathVector = requires() {
    typename MathVectorTraits<T>::ImplementationFlag;
};
template <typename T>
concept MathMatrix = requires() {
    typename MathMatrixTraits<T>::ImplementationFlag;
};

} // namespace math
} // namespace skr