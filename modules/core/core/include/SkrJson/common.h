#pragma once
#include "SkrBase/types.h"

typedef enum ESkrJsonType
{
    SKR_JSONTYPE_BOOL,
    SKR_JSONTYPE_NUMBER,
    SKR_JSONTYPE_STRING,
    SKR_JSONTYPE_OBJECT,
    SKR_JSONTYPE_ARRAY,
} ESkrJsonType;

typedef struct SJsonDocument SJsoDocument;
typedef struct SJsonMutableDocument SJsonMutableDocument;
typedef struct SJsonValue SJsonValue;
typedef struct SJsonMutableValue SJsonMutableValue;

#if defined(__cplusplus)
#include "SkrContainers/string.hpp"

template <typename T>
inline static constexpr bool IsJsonPrimitiveReadableType =
    std::is_same_v<T, bool> || std::is_integral_v<T> || std::is_floating_point_v<T> ||
    std::is_same_v<T, SJsonCharType*> || 
    std::is_same_v<T, skr::String>;

template <typename T>
inline static constexpr bool IsJsonPrimitiveWritableType =
    IsJsonPrimitiveReadableType<T> || std::is_same_v<T, skr::StringView>;

template <typename T>
concept JsonPrimitiveWritableType = IsJsonPrimitiveWritableType<T>;

template <typename T>
concept JsonPrimitiveReadableType = IsJsonPrimitiveReadableType<T>;

#endif