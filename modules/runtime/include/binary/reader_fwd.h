#pragma once
#include <type_traits>
#include "platform/configure.h"
#include "binary/blob_fwd.h"
#include "utils/traits.hpp"
#ifdef _DEBUG
#include "platform/debug.h"
#endif

struct skr_binary_reader_t;

namespace skr
{
namespace binary
{
template <class T, class = void>
struct ReadTrait;

template <class T>
int Archive(skr_binary_reader_t* reader, T&& value)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, value);
}

template <class T>
int Archive(skr_binary_reader_t* reader, skr_blob_arena_t& arena, T&& value)
{
    if constexpr (is_complete_v<BlobTrait<std::decay_t<T>>>)
    {
        if (arena.get_buffer() == nullptr)
        {
#ifdef _DEBUG
            SKR_ASSERT(!arena.get_size());
#endif
            return 0;
        }
        return ReadTrait<std::decay_t<T>>::Read(reader, arena, value);
    }
    else
        return ReadTrait<std::decay_t<T>>::Read(reader, value);
}
}
}

#ifndef SKR_ARCHIVE
#define SKR_ARCHIVE(...) if(auto ret = skr::binary::Archive(archive, (__VA_ARGS__)); ret != 0) return ret
#endif