#pragma once
#include <type_traits>
#include "platform/configure.h"
#include "binary/blob_fwd.h"

struct skr_binary_writer_t;

namespace skr::binary
{
template <class T, class = void>
struct WriteHelper;

template <class T>
int Archive(skr_binary_writer_t* writer, const T& value)
{
    return WriteHelper<const T&>::Write(writer, value);
}

template <class T>
int Archive(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value)
{
    return WriteHelper<const T&>::Write(writer, arena, value);
}
}

#ifndef SKR_ARCHIVE
#define SKR_ARCHIVE(...) if(auto ret = skr::binary::Archive(archive, (__VA_ARGS__)); ret != 0) return ret
#endif