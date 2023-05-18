#pragma once
#include "ecs/dual.h"
#include "serde/binary/reader.h"
#include "serde/binary/writer.h"
#include "serde/json/reader.h"
#include "serde/json/writer.h"
namespace dual
{
    template<class C>
    void SetSerdeCallback(dual_type_description_t& desc)
    {
        if constexpr(skr::is_complete_serde_v<skr::binary::WriteTrait<const C&>>)
            desc.callback.serialize = +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* writer) {
                skr::binary::Write<const C&>(writer, *(C*)data);
            };
        if constexpr(skr::is_complete_serde_v<skr::binary::ReadTrait<C>>)
            desc.callback.deserialize = +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* reader) {
                skr::binary::Read(reader, *(C*)data);
            };
        if constexpr(skr::is_complete_serde_v<skr::json::WriteTrait<const C&>>)
            desc.callback.serialize_text = +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_json_writer_t* writer) {
                skr::json::Write<const C&>(writer, *(C*)data);
            };
        if constexpr(skr::is_complete_serde_v<skr::json::ReadTrait<C>>)
            desc.callback.deserialize_text = +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, void* reader) {
                skr::json::Read(std::move(*(simdjson::ondemand::value*)reader), *(C*)data);
            };
    }
}