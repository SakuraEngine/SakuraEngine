#pragma once
#include <type_traits>
#include "serde/binary/blob_fwd.h"
#include "utils/traits.hpp"
#include "platform/debug.h"

// FUCK MSVC COMPILER
RUNTIME_EXTERN_C RUNTIME_API void skr_debug_output(const char* msg);

struct skr_binary_reader_t;

namespace skr
{
namespace binary
{
template <class T, class = void>
struct ReadTrait;

template <class T, class... Args>
int Archive(skr_binary_reader_t* reader, T&& value, Args&&... args)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, value, std::forward<Args>(args)...);
}

template <class T, class... Args>
int ArchiveBlob(skr_binary_reader_t* reader, skr_blob_arena_t& arena, T&& value, Args&&... args)
{
    if constexpr (is_complete_v<BlobTrait<std::decay_t<T>>>)
    {
        if (arena.get_buffer() == nullptr)
        {
            SKR_ASSERT(!arena.get_size());
            return 0;
        }
        return ReadTrait<std::decay_t<T>>::Read(reader, arena, value, std::forward<Args>(args)...);
    }
    else
        return ReadTrait<std::decay_t<T>>::Read(reader, value, std::forward<Args>(args)...);
}
}
}

#ifndef SKR_ARCHIVE
#define SKR_ARCHIVE(...) if(auto ret = skr::binary::Archive(archive, (__VA_ARGS__)); ret != 0) return ret
#endif