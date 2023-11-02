#pragma once
#include <type_traits>
#include "SkrRT/serde/binary/blob_fwd.h"
#include "SkrRT/misc/traits.hpp"
#include "SkrRT/platform/debug.h"

// FUCK MSVC COMPILER
SKR_EXTERN_C SKR_RUNTIME_API void skr_debug_output(const char* msg);

struct skr_binary_writer_t;

namespace skr::binary
{
template <class T, class = void>
struct WriteTrait;

template <class T, class... Args>
int Archive(skr_binary_writer_t* writer, const T& value, Args&&... args)
{
    return WriteTrait<T>::Write(writer, value, std::forward<Args>(args)...);
}

template <class T, class... Args>
int ArchiveBlob(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value, Args&&... args)
{
    if constexpr (is_complete_v<BlobTrait<T>>)
    {
        if (arena.get_buffer() == nullptr)
        {
            SKR_ASSERT(!arena.get_size());
            return 0;
        }
        return WriteTrait<T>::Write(writer, arena, value, std::forward<Args>(args)...);
    }
    else
        return WriteTrait<T>::Write(writer, value, std::forward<Args>(args)...);
}
} // namespace skr::binary

#ifndef SKR_ARCHIVE
    #define SKR_ARCHIVE(...) \
        if (auto ret = skr::binary::Archive(archive, (__VA_ARGS__)); ret != 0) return ret
#endif