#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrSerde/bin_serde.hpp"
#include "SkrSerde/json_serde.hpp"

namespace sugoi
{
template <class C>
void SetSerdeCallback(sugoi_type_description_t& desc)
{
    if constexpr (skr::HasBinWrite<C>)
    {
        desc.callback.serialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryWriter* writer) {
            ::skr::bin_write<C>(writer, *(C*)data);
        };
    }
    if constexpr (skr::HasBinRead<C>)
    {
        desc.callback.deserialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryReader* reader) {
            ::skr::bin_read<C>(reader, *(C*)data);
        };
    }
    if constexpr (skr::HasJsonWrite<C>)
    {
        desc.callback.serialize_text = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonWriter* writer) {
            ::skr::json_write<C>(writer, *(C*)data);
        };
    }
    if constexpr (skr::HasJsonRead<C>)
    {
        desc.callback.deserialize_text = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonReader* reader) {
            ::skr::json_read(reader, *(C*)data);
        };
    }
}
} // namespace sugoi