#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/serde/binary/reader.h"
#include "SkrRT/serde/binary/writer.h"
#include "SkrRT/serde/json/reader.h"
#include "SkrRT/serde/json/writer.h"
namespace sugoi
{
template <class C>
void SetSerdeCallback(sugoi_type_description_t& desc)
{
    if constexpr (skr::is_complete_serde_v<skr::binary::WriteTrait<C>>)
        desc.callback.serialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* writer) {
            skr::binary::Write<C>(writer, *(C*)data);
        };
    if constexpr (skr::is_complete_serde_v<skr::binary::ReadTrait<C>>)
        desc.callback.deserialize = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* reader) {
            skr::binary::Read(reader, *(C*)data);
        };
    if constexpr (skr::is_complete_serde_v<skr::json::WriteTrait<C>>)
        desc.callback.serialize_text = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_json_writer_t* writer) {
            skr::json::Write<C>(writer, *(C*)data);
        };
    if constexpr (skr::is_complete_serde_v<skr::json::ReadTrait<C>>)
        desc.callback.deserialize_text = +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, void* reader) {
            skr::json::Read(std::move(*(simdjson::ondemand::value*)reader), *(C*)data);
        };
}
} // namespace sugoi