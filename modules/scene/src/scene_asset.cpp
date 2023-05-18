#include "SkrScene/scene.h"
#include "utils/parallel_for.hpp"
#include "platform/guid.hpp"
#include "json/writer.h"
#include "json/reader.h"
#include "ecs/type_builder.hpp"
#include "utils/make_zeroed.hpp"

#include "containers/vector.hpp"
#include "containers/string.hpp"

#include <execution>

void skr_save_scene(dual_storage_t* world, skr_json_writer_t* writer)
{
    skr::vector<dual_entity_t> entities;
    skr::vector<skr_guid_t> guids;
    auto entityCount = dualS_count(world, true, false);
    entities.reserve(entityCount);
    guids.reserve(entityCount);
    auto accumulate = [&](dual_chunk_view_t* view)
    {
        auto centities = dualV_get_entities(view);
        auto cguids = (skr_guid_t*)dualV_get_owned_ro(view, DUAL_COMPONENT_GUID);
        if(!cguids)
            return;
        //append to vector
        entities.insert(entities.end(), centities, centities + view->count);
        guids.insert(guids.end(), cguids, cguids + view->count);
        
    };
    dualS_all(world, true, false, DUAL_LAMBDA(accumulate));
    skr::vector<dual_entity_t> indices;
    indices.resize(guids.size());
    using iter = skr::vector<dual_entity_t>::iterator;
    skr::parallel_for(indices.begin(), indices.end(), 2048, [&](iter begin, iter end)
    {
        std::iota(begin, end, begin - indices.begin());
    });
    //sort by guid to ensure deterministic order
    std::sort(
#if __cpp_lib_execution >= 201603L
    std::execution::par_unseq, 
#endif
    indices.begin(), indices.end(), [&](dual_entity_t a, dual_entity_t b)
    {
        return std::lexicographical_compare(&guids[a].Storage0, &guids[a].Storage3, &guids[b].Storage0, &guids[b].Storage3);
    });
    skr::vector<dual_entity_t> sortedEntities;
    sortedEntities.resize(guids.size());
    skr::parallel_for(indices.begin(), indices.end(), 2048, [&](iter begin, iter end)
    {
        for(auto it = begin; it != end; ++it)
        {
            sortedEntities[*it] = entities[it - indices.begin()];
        }
    });
    writer->StartObject();
    auto saveEntity = [&](dual_chunk_view_t* view)
    {
        auto cguids = (skr_guid_t*)dualV_get_owned_ro(view, DUAL_COMPONENT_GUID);
        auto group = dualC_get_group(view->chunk);
        dual_entity_type_t type;
        dualG_get_type(group, &type);
        for(EIndex i = 0; i < view->count; ++i)
        {
            auto guidStr = skr::format(u8"{}", cguids[i]);
            writer->Key(guidStr.u8_str(), guidStr.size());
            writer->StartObject();
            for(EIndex j = 0; j < type.type.length; ++j)
            {
                auto index = dualG_get_stable_order(group, j);
                auto component = type.type.data[index];
                auto desc = dualT_get_desc(component);
                auto data = dualV_get_owned_ro_local(view, index);
                auto serde = desc->callback.serialize_text;
                if(!serde)
                    continue;
                writer->Key(desc->name);
                serde(view->chunk, view->start + i, (char*)data, 1, writer);
            }
            writer->EndObject();
        }
    };
    dualS_batch(world, sortedEntities.data(), (EIndex)sortedEntities.size(), DUAL_LAMBDA(saveEntity));
    writer->EndObject();
}

void skr_load_scene(dual_storage_t* world, skr_json_reader_t* reader)
{
    skr::json::value_t value = std::move(*(skr::json::value_t*)reader);
    auto root = value.get_object();
    #define ERR(var) var.error() != simdjson::error_code::SUCCESS
    if(ERR(root))
        return;
    for(auto field : root.value_unsafe())
    {
        auto key = field.unescaped_key();
        if(ERR(key))
            continue;
        skr_guid_t guid;
        auto keyStr = key.value_unsafe();
        if(!skr::guid::make_guid(skr::string_view((const char8_t*)keyStr.data(), keyStr.length()), guid))
            continue;
        auto entity = field.value().get_object();
        if(entity.error() != simdjson::error_code::SUCCESS)
            continue;
        dual::type_builder_t entityType;
        entityType.with(DUAL_COMPONENT_GUID);
        for(auto component : entity.value_unsafe())
        {
            auto key = component.unescaped_key();
            if(ERR(key))
                continue;
            auto keyStr = key.value_unsafe();
            auto typeId = dualT_get_type_by_name((const char8_t*)keyStr.data());
            if(typeId == dual::kInvalidTypeIndex)
                continue;
            auto desc = dualT_get_desc(typeId);
            if(!desc->callback.deserialize_text || !desc->callback.serialize_text)
                continue;
            entityType.with(typeId);
        }
        dual_entity_type_t type = make_zeroed<dual_entity_type_t>();
        type.type = entityType.build();
        auto setup = [&, entity](dual_chunk_view_t* view) mutable
        {
            auto cguids = (skr_guid_t*)dualV_get_owned_rw(view, DUAL_COMPONENT_GUID);
            cguids[0] = guid;
            for(auto component : entity.value_unsafe())
            {
                auto componentValue = component.value();
                if(ERR(componentValue))
                    continue;
                auto type = dualT_get_type_by_name((const char8_t*)component.unescaped_key().value_unsafe().data());
                auto desc = dualT_get_desc(type);
                auto serde = desc->callback.deserialize_text;
                if(!serde)
                    continue;
                auto data = dualV_get_owned_ro(view, type);
                serde(view->chunk, view->start, (char*)data, 1, &componentValue);
            }
        };
        dualS_allocate_type(world, &type, 1, DUAL_LAMBDA(setup));
    }
}