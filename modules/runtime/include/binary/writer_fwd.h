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

template <class T, class ...Args>
int Archive(skr_binary_writer_t* writer, const T& value, Args&&... args)
{
    return WriteTrait<const T&>::Write(writer, value, std::forward<Args>(args)...);
}

template <class T, class ...Args>
int ArchiveBlob(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value, Args&&... args)
{
    if constexpr (is_complete_v<BlobTrait<T>>)
    {
        if (arena.get_buffer() == nullptr)
        {
#ifdef _DEBUG
            SKR_ASSERT(!arena.get_size());
#endif
            return 0;
        }
        return WriteTrait<const T&>::Write(writer, arena, value, std::forward<Args>(args)...);
    }
    else
        return WriteTrait<const T&>::Write(writer, value, std::forward<Args>(args)...);
}
}

#ifndef SKR_ARCHIVE
#define SKR_ARCHIVE(...) if(auto ret = skr::binary::Archive(archive, (__VA_ARGS__)); ret != 0) return ret
#endif