#pragma once
#include "SkrRuntime/sugoi/sugoi.h"
#include <SkrCore/serialize/serialize_traits.hpp>

namespace sugoi
{
template <class T>
void SetSerdeCallback(sugoi_type_description_t& desc)
{
    // serialize
    if constexpr (skr::concepts::HasSerdeRead<T>)
    {
        desc.callback.serialize = +[](sugoi_type_index_t, const void* data, EIndex count, skr::ArchiveWrite* writer) -> bool {
            auto* typed_data = (const T*)data;

            skr::Archive::ArrayScope arr_scope(*writer);
            SKR_FAST_CHECK(arr_scope.is_success(), false);

            for (uint32_t i = 0; i < count; i++)
            {
                SKR_FAST_CHECK(writer->value(typed_data[i]), false);
            }

            return true;
        };
    }
    if constexpr (skr::concepts::HasSerdeWrite<T>)
    {
        desc.callback.deserialize = +[](sugoi_type_index_t, void* data, EIndex count, skr::ArchiveRead* reader) -> bool {
            auto* typed_data = (T*)data;

            skr::Archive::ArrayScope arr_scope(*reader);
            SKR_FAST_CHECK(arr_scope.is_success(), false);

            // adjust array size when use structured archive
            uint64_t adjusted_count = count;
            if (reader->is_structured())
            {
                uint64_t arr_count;
                SKR_FAST_CHECK(reader->array_size_structured(arr_count), false);
                SKR_FAST_CHECK(
                    reader->adjust_array_size(
                        arr_count,
                        count,
                        adjusted_count
                    ),
                    false
                );
            }

            for (uint64_t i = 0; i < adjusted_count; i++)
            {
                SKR_FAST_CHECK(reader->value(typed_data[i]), false);
            }

            return true;
        };
    }
}
} // namespace sugoi