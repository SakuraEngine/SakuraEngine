#pragma once
#include <type_traits>
#include "platform/configure.h"
#include "binary/blob_fwd.h"
#include "utils/traits.hpp"

struct skr_binary_writer_t;

namespace skr::binary
{
template <class T, class = void>
struct WriteTrait;

template <class T>
int Archive(skr_binary_writer_t* writer, const T& value)
{
    return WriteTrait<const T&>::Write(writer, value);
}

template <class T>
int Archive(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value)
{
    if constexpr (is_complete_v<BlobTrait<T>>)
        return WriteTrait<const T&>::Write(writer, arena, value);
    else
        return WriteTrait<const T&>::Write(writer, value);
}
}

#ifndef SKR_ARCHIVE
#define SKR_ARCHIVE(...) if(auto ret = skr::binary::Archive(archive, (__VA_ARGS__)); ret != 0) return ret
#endif