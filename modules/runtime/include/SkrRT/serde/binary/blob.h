#pragma once
#include "SkrContainers/span.hpp"
#include "SkrContainers/string.hpp"

namespace skr
{
namespace binary
{
template <>
struct SKR_STATIC_API BlobTrait<skr::StringView> {
    static void BuildArena(skr_blob_arena_builder_t& arena, skr::StringView& dst, const skr::String& src);
    static void Remap(skr_blob_arena_t& arena, skr::StringView& dst);
};

template <class T>
struct BlobTrait<skr::span<T>> {
    static void BuildArena(skr_blob_arena_builder_t& arena, skr::span<T>& dst, const typename BlobBuilderType<skr::span<T>>::type& src)
    {
        size_t offset = arena.allocate(src.size() * sizeof(T), alignof(T));

        if constexpr (is_complete_v<BlobTrait<T>>)
        {
            for (int i = 0; i < src.size(); ++i)
            {
                T dstV{};
                BlobTrait<T>::BuildArena(arena, dstV, src[i]);
                skr::span<T> _span = { (T*)((char*)arena.get_buffer() + offset), src.size() };
                _span[i]           = dstV;
            }
        }
        else
        {
            auto buffer = (char*)arena.get_buffer() + offset;
            memcpy(buffer, src.data(), src.size() * sizeof(T));
        }
        // store offset inplace
        dst = skr::span<T>((T*)(offset), src.size());
    }
    static void Remap(skr_blob_arena_t& arena, skr::span<T>& dst)
    {
        dst = skr::span<T>((T*)((char*)arena.get_buffer() + ((size_t)dst.data() - arena.base())), dst.size());

        if constexpr (is_complete_v<BlobTrait<T>>)
        {
            for (auto& dstV : dst)
            {
                BlobTrait<T>::Remap(arena, dstV);
            }
        }
    }
};

template <class T>
void BuildArena(skr_blob_arena_builder_t& arena, T& dst, const typename BlobBuilderType<T>::type& src)
{
    if constexpr (is_complete_v<BlobTrait<T>>)
    {
        BlobTrait<T>::BuildArena(arena, dst, src);
    }
    else // fallback to copy
    {
        dst = src;
    }
}

template <class T>
void Remap(skr_blob_arena_t& arena, T& dst)
{
    if constexpr (is_complete_v<BlobTrait<T>>)
    {
        BlobTrait<T>::Remap(arena, dst);
    }
}
} // namespace binary
} // namespace skr