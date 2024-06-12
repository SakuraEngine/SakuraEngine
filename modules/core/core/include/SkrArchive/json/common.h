#pragma once
#include "SkrBase/types.h"

typedef struct SJsonDocument SJsoDocument;
typedef struct SJsonMutableDocument SJsonMutableDocument;
typedef struct SJsonValue SJsonValue;
typedef struct SJsonMutableValue SJsonMutableValue;

#if defined(__cplusplus)
#include "SkrContainers/string.hpp"

namespace skr::archive
{

enum class JsonErrorCode : uint32_t 
{
    UnknownError,       // RW
    NoOpenScope,        // RW
    ScopeTypeMismatch,  // RW

    EmptyObjectFieldKey,// RW
    ArrayElementWithKey,// RW
    RootObjectWithKey,  // RW

    KeyNotFound,        // R
    UnknownTypeToRead,  // R

    PresetKeyNotConsumedYet, // RW
    PresetKeyIsEmpty         // RW
};
using JsonResult = skr::Expected<JsonErrorCode>;

template <typename T>
inline static constexpr bool IsJsonPrimitiveReadableType =
    std::is_same_v<T, bool> || std::is_integral_v<T> || std::is_floating_point_v<T> ||
    std::is_same_v<T, char8_t*> || 
    std::is_same_v<T, skr::String>;

template <typename T>
inline static constexpr bool IsJsonPrimitiveWritableType =
    IsJsonPrimitiveReadableType<T> || std::is_same_v<T, skr::StringView>;

template <typename T>
concept JsonPrimitiveWritableType = IsJsonPrimitiveWritableType<T>;

template <typename T>
concept JsonPrimitiveReadableType = IsJsonPrimitiveReadableType<T>;

}
#endif