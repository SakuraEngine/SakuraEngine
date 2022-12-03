#pragma once
#include <type_traits>
#include "platform/configure.h"
#include "binary/blob_fwd.h"

struct skr_binary_reader_t;

namespace skr
{
namespace binary
{
template <class T, class = void>
struct ReadTrait;

template <class T>
int Archive(skr_binary_reader_t* writer, T&& value)
{
    return ReadTrait<std::decay_t<T>>::Read(writer, value);
}

template <class T>
int Archive(skr_binary_reader_t* reader, skr_blob_arena_t& arena, T&& value)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, arena, value);
}
}
}

#ifndef SKR_ARCHIVE
#define SKR_ARCHIVE(...) if(auto ret = skr::binary::Archive(archive, (__VA_ARGS__)); ret != 0) return ret
#endif