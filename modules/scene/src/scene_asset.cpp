#include "SkrScene/scene.h"
#include "SkrRT/async/parallel_for.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRT/serde/json/writer.h"
#include "SkrRT/serde/json/reader.h"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrBase/misc/make_zeroed.hpp"

#include "SkrContainers/vector.hpp"
#include "SkrContainers/string.hpp"

#include <numeric> // std::iota
#include <execution>

void skr_save_scene(sugoi_storage_t* world, skr_json_writer_t* writer)
{
    skr::Vector<sugoi_entity_t> entities;
    skr::Vector<skr_guid_t> guids;
    auto entityCount = sugoiS_count(world, true, false);
    entities.reserve(entityCount);
    guids.reserve(entityCount);
    auto accumulate = [&](sugoi_chunk_view_t* view)
    {
        auto centities = sugoiV_get_entities(view);
        auto cguids = (skr_guid_t*)sugoiV_get_owned_ro(view, SUGOI_COMPONENT_GUID);
        if(!cguids)
            return;
        //append to vector
        entities.append({centities, view->count});
        guids.append({cguids, view->count});
        
    };
    sugoiS_all(world, true, false, SUGOI_LAMBDA(accumulate));
    skr::Vector<sugoi_entity_t> indices;
    indices.resize_default(guids.size());
    skr::parallel_for(indices.begin(), indices.end(), 2048, [&](auto&& begin, auto&& end)
    {
        std::iota(begin, end, begin - indices.begin());
    });
    //sort by guid to ensure deterministic order
    std::sort(
#if __cpp_lib_execution >= 201603L
    std::execution::par_unseq, 
#endif
    indices.begin(), indices.end(), [&](sugoi_entity_t a, sugoi_entity_t b)
    {
        return std::lexicographical_compare(&guids[a].Storage0, &guids[a].Storage3, &guids[b].Storage0, &guids[b].Storage3);
    });
    skr::Vector<sugoi_entity_t> sortedEntities;
    sortedEntities.resize_default(guids.size());
    skr::parallel_for(indices.begin(), indices.end(), 2048, [&](auto&& begin, auto&& end)
    {
        for (auto it = begin; it != end; ++it)
        {
            sortedEntities[*it] = entities[it - indices.begin()];
        }
    });
    writer->StartObject();
    auto saveEntity = [&](sugoi_chunk_view_t* view)
    {
        auto cguids = (skr_guid_t*)sugoiV_get_owned_ro(view, SUGOI_COMPONENT_GUID);
        auto group = sugoiC_get_group(view->chunk);
        sugoi_entity_type_t type;
        sugoiG_get_type(group, &type);
        for (EIndex i = 0; i < view->count; ++i)
        {
            auto guidStr = skr::format(u8"{}", cguids[i]);
            writer->Key(guidStr.u8_str(), guidStr.size());
            writer->StartObject();
            for (EIndex j = 0; j < type.type.length; ++j)
            {
                auto index = sugoiG_get_stable_order(group, j);
                auto component = type.type.data[index];
                auto desc = sugoiT_get_desc(component);
                auto data = sugoiV_get_owned_ro_local(view, index);
                auto serde = desc->callback.serialize_text;
                if(!serde)
                    continue;
                writer->Key(desc->name);
                serde(view->chunk, view->start + i, (char*)data, 1, writer);
            }
            writer->EndObject();
        }
    };
    sugoiS_batch(world, sortedEntities.data(), (EIndex)sortedEntities.size(), SUGOI_LAMBDA(saveEntity));
    writer->EndObject();
}

void skr_load_scene(sugoi_storage_t* world, skr_json_reader_t* reader)
{
    skr::json::value_t value = std::move(*(skr::json::value_t*)reader);
    auto root = value.get_object();
    #define ERR(var) var.error() != simdjson::error_code::SUCCESS
    if (ERR(root))
        return;
    for (auto field : root.value_unsafe())
    {
        auto key = field.unescaped_key();
        if(ERR(key))
            continue;
        skr_guid_t guid;
        auto keyStr = key.value_unsafe();
        if(!skr::guid::make_guid(skr::StringView((const char8_t*)keyStr.data(), (int32_t)keyStr.length()), guid))
            continue;
        auto entity = field.value().get_object();
        if(entity.error() != simdjson::error_code::SUCCESS)
            continue;
        sugoi::type_builder_t entityType;
        entityType.with(SUGOI_COMPONENT_GUID);
        for (auto component : entity.value_unsafe())
        {
            auto key = component.unescaped_key();
            if(ERR(key))
                continue;
            auto keyStr = key.value_unsafe();
            auto typeId = sugoiT_get_type_by_name((const char8_t*)keyStr.data());
            if(typeId == sugoi::kInvalidTypeIndex)
                continue;
            auto desc = sugoiT_get_desc(typeId);
            if(!desc->callback.deserialize_text || !desc->callback.serialize_text)
                continue;
            entityType.with(typeId);
        }
        sugoi_entity_type_t type = make_zeroed<sugoi_entity_type_t>();
        type.type = entityType.build();
        auto setup = [&, entity](sugoi_chunk_view_t* view) mutable
        {
            auto cguids = (skr_guid_t*)sugoiV_get_owned_rw(view, SUGOI_COMPONENT_GUID);
            cguids[0] = guid;
            for(auto component : entity.value_unsafe())
            {
                auto componentValue = component.value();
                if(ERR(componentValue))
                    continue;
                auto type = sugoiT_get_type_by_name((const char8_t*)component.unescaped_key().value_unsafe().data());
                auto desc = sugoiT_get_desc(type);
                auto serde = desc->callback.deserialize_text;
                if(!serde)
                    continue;
                auto data = sugoiV_get_owned_ro(view, type);
                serde(view->chunk, view->start, (char*)data, 1, &componentValue);
            }
        };
        sugoiS_allocate_type(world, &type, 1, SUGOI_LAMBDA(setup));
    }
}