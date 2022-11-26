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
        };

        template<class T>
        struct BlobHelper<skr::span<T>> {
            static void BuildArena(skr_blob_arena_builder_t& arena, skr::span<T>& dst, const typename BlobOwnedType<skr::span<T>>::type& src)
            {
                auto buffer = arena.allocate(src.size() * sizeof(T), alignof(T));
                memcpy(buffer, src.data(), src.size() * sizeof(T));
                dst = skr::span<T>(static_cast<T*>(buffer), src.size());
                
                if constexpr(is_complete_v<BlobHelper<T>>)
                {
                    for(int i = 0; i < src.size(); ++i)
                    {
                        BlobHelper<T>::BuildArena(arena, dst[i], src[i]);
                    }
                }
            }
        };
    }
}