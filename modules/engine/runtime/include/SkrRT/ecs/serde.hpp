#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrSerde/bin_serde.hpp"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

namespace sugoi
{
template <class C>
void SetSerdeCallback(sugoi_type_description_t& desc)
{
    if constexpr (skr::HasBinWrite<C>)
    {
        desc.callback.serialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryWriter* writer) {
            bin_write<C>(writer, *(C*)data);
        };
    }
    if constexpr (skr::HasBinRead<C>)
    {
        desc.callback.deserialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryReader* reader) {
            bin_read<C>(reader, *(C*)data);
        };
    }
    if constexpr (skr::json::HasWriteTrait<C>)
    {
        desc.callback.serialize_text = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonWriter* writer) {
            skr::json::Write<C>(writer, *(C*)data);
        };
    }
    if constexpr (skr::json::HasReadTrait<C>)
    {
        desc.callback.deserialize_text = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr::archive::JsonReader* reader) {
            skr::json::Read(reader, *(C*)data);
        };
    }
}
} // namespace sugoi