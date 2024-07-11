#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

namespace sugoi
{
template <class C>
void SetSerdeCallback(sugoi_type_description_t& desc)
{
    if constexpr (skr::binary::HasWriteTrait<C>)
    {
        desc.callback.serialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryWriter* writer) {
            skr::binary::Write<C>(writer, *(C*)data);
        };
    }
    if constexpr (skr::binary::HasReadTrait<C>)
    {
        desc.callback.deserialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, SBinaryReader* reader) {
            skr::binary::Read(reader, *(C*)data);
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