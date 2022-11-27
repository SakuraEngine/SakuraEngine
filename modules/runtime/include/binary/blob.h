#pragma once
#include "blob_fwd.h"
#include "containers/span.hpp"
#include "containers/string.hpp"
#include "type/type_helper.hpp"
#include "EASTL/vector.h"

namespace skr
{
    namespace binary
    {
        template <>
        struct RUNTIME_API BlobHelper<skr::string_view> {
            static void BuildArena(skr_blob_arena_builder_t& arena, skr::string_view& dst, const skr::string& src);
            static void FillView(skr_blob_arena_builder_t& arena, skr::string_view& dst);
        };

        template<class T>
        struct BlobHelper<skr::span<T>> {
            static void BuildArena(skr_blob_arena_builder_t& arena, skr::span<T>& dst, const typename BlobBuilderType<skr::span<T>>::type& src)
            {
                size_t offset = arena.allocate(src.size() * sizeof(T), alignof(T));
                
                if constexpr(is_complete_v<BlobHelper<T>>)
                {
                    for(int i = 0; i < src.size(); ++i)
                    {   
                        T dstV{};
                        BlobHelper<T>::BuildArena(arena, dstV, src[i]);
                        skr::span<T> span = {(T*)((char*)arena.get_buffer() + offset), src.size()};
                        span[i] = dstV;
                    }
                }
                else 
                {
                    auto buffer = (char*)arena.get_buffer() + offset;
                    memcpy(buffer, src.data(), src.size() * sizeof(T));
                }
                //store offset inplace
                dst = skr::span<T>((T*)(offset), src.size());
            }
            static void FillView(skr_blob_arena_builder_t& arena, skr::span<T>& dst)
            {
                dst = skr::span<T>((T*)((char*)arena.get_buffer() + (uint32_t)dst.data()), dst.size());

                if constexpr(is_complete_v<BlobHelper<T>>)
                {
                    for(auto& dstV : dst)
                    {
                        BlobHelper<T>::FillView(arena, dstV);
                    }
                }
            }
        };
    }
}