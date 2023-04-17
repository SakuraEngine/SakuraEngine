#include "SkrScene/scene.h"
#include "ecs/callback.hpp"
#include "containers/vector.hpp"
#include <execution>
#include "utils/parallel_for.hpp"
#include "platform/guid.hpp"
#include "json/writer.h"
#include "utils/format.hpp"

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
    std::sort(std::execution::par_unseq, indices.begin(), indices.end(), [&](dual_entity_t a, dual_entity_t b)
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
        for(int i=0; i<view->count; ++i)
        {
            auto guidStr = skr::format("{}", cguids[i]);
            writer->Key(guidStr.c_str(), guidStr.size());
            writer->StartObject();
            for(int j=0; j<type.type.length; ++j)
            {
                auto index = dualG_get_stable_order(group, j);
                auto component = type.type.data[index];
                auto desc = dualT_get_desc(component);
                auto data = dualV_get_owned_ro_local(view, index);
                auto type = skr_get_type(&desc->guid);
                if(!type)
                    continue;
                writer->Key(desc->name);
                type->SerializeText((char*)data + i * desc->size, writer);
            }
            writer->EndObject();
        }
    };
    dualS_batch(world, sortedEntities.data(), sortedEntities.size(), DUAL_LAMBDA(saveEntity));
    writer->EndObject();
}

void skr_load_scene(dual_storage_t* world, skr_json_reader_t* reader)
{

}