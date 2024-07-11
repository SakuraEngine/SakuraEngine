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

struct SBinaryReader;
struct SBinaryWriter;

namespace skr::binary
{
// read
template <class T, class = void>
struct ReadTrait;
template <class T>
bool Read(SBinaryReader* reader, T&& value)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, value);
}
template <typename T>
inline static constexpr bool HasReadTrait = requires(SBinaryReader* r, T& t) { ReadTrait<T>::Read(r, t); };

// write
template <class T, class = void>
struct WriteTrait;
template <class T>
bool Write(SBinaryWriter* writer, const T& value)
{
    return WriteTrait<T>::Write(writer, value);
}

} // namespace skr::binary
