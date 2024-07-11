// TODO. ！！！！！！！！！！！ MOVE TO SKR_SERDE
#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"
#include <limits>

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

#ifndef SKR_ARCHIVE
    #define SKR_ARCHIVE(...) \
        if (!skr::binary::Archive(archive, (__VA_ARGS__))) return false
#endif

// FUCK MSVC COMPILER
SKR_EXTERN_C SKR_STATIC_API void skr_debug_output(const char* msg);

struct SBinaryReader;

namespace skr::binary
{
template <class T, class = void>
struct ReadTrait;

template <typename T>
inline static constexpr bool HasReadTrait = requires(SBinaryReader* r, T& t) { ReadTrait<T>::Read(r, t); };

template <class T>
bool Archive(SBinaryReader* reader, T&& value)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, value);
}
} // namespace skr::binary

struct SBinaryWriter;

namespace skr::binary
{
template <class T, class = void>
struct WriteTrait;

template <class T, class... Args>
bool Archive(SBinaryWriter* writer, const T& value, Args&&... args)
{
    return WriteTrait<T>::Write(writer, value, std::forward<Args>(args)...);
}
} // namespace skr::binary