#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrSerde/bin_serde.hpp"
#include "SkrSerde/json_serde.hpp"

namespace sugoi
{
template <class C>
void SetSerdeCallback(sugoi_type_description_t& desc)
{
    if constexpr (skr::concepts::HasBinWrite<C>)
    {
        desc.callback.serialize = +[](sugoi_type_index_t, sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryWriter* writer) {
            ::skr::bin_write<C>(writer, *(C*)data);
        };
    }
    if constexpr (skr::concepts::HasBinRead<C>)
    {
        desc.callback.deserialize = +[](sugoi_type_index_t, sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryReader* reader) {
            ::skr::bin_read<C>(reader, *(C*)data);
        };
    }
    if constexpr (skr::concepts::HasJsonWrite<C>)
    {
        desc.callback.serialize_text = +[](sugoi_type_index_t, sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonWriter* writer) {
            ::skr::json_write<C>(writer, *(C*)data);
        };
    }
    if constexpr (skr::concepts::HasJsonRead<C>)
    {
        desc.callback.deserialize_text = +[](sugoi_type_index_t, sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonReader* reader) {
            ::skr::json_read(reader, *(C*)data);
        };
    }
}
} // namespace sugoi