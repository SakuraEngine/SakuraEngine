#pragma once
#include "SkrBase/types.h"

#if defined(__cplusplus)
    #include "SkrContainersDef/string.hpp"

namespace skr::archive
{

enum class BinaryErrorCode : uint32_t
{
    UnknownError,      // RW
    NoOpenScope,       // RW
    ScopeTypeMismatch, // RW

    EmptyObjectFieldKey, // RW
    ArrayElementWithKey, // RW
    RootObjectWithKey,   // RW

    KeyNotFound,       // R
    UnknownTypeToRead, // R

    PresetKeyNotConsumedYet, // RW
    PresetKeyIsEmpty         // RW
};
using BinaryResult = skr::Expected<BinaryErrorCode>;

template <typename T>
inline static constexpr bool IsBinaryPrimitiveReadableType =
std::is_same_v<T, bool> || std::is_integral_v<T> || std::is_floating_point_v<T> ||
std::is_same_v<T, char8_t*> ||
std::is_same_v<T, skr::String>;

template <typename T>
inline static constexpr bool IsBinaryPrimitiveWritableType =
IsBinaryPrimitiveReadableType<T> || std::is_same_v<T, skr::StringView>;

template <typename T>
concept BinaryPrimitiveWritableType = IsBinaryPrimitiveWritableType<T>;

template <typename T>
concept BinaryPrimitiveReadableType = IsBinaryPrimitiveReadableType<T>;

} // namespace skr::archive
#endif