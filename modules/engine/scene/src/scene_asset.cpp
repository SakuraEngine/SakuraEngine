#include "SkrScene/scene.h"
#include "SkrTask/parallel_for.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/json/reader.h"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrBase/misc/make_zeroed.hpp"

#include "SkrContainers/vector.hpp"
#include "SkrContainers/string.hpp"

#include <numeric> // std::iota
#include <execution>

void skr_save_scene(sugoi_storage_t* world, skr::archive::JsonWriter* writer)
{
    skr::Vector<sugoi_entity_t> entities;
    skr::Vector<skr_guid_t>     guids;
    auto                        entityCount = sugoiS_count(world, true, false);
    entities.reserve(entityCount);
    guids.reserve(entityCount);
    auto accumulate = [&](sugoi_chunk_view_t* view) {
        auto centities = sugoiV_get_entities(view);
        auto cguids    = (skr_guid_t*)sugoiV_get_owned_ro(view, SUGOI_COMPONENT_GUID);
        if (!cguids)
            return;
        // append to vector
        entities.append({ centities, view->count });
        guids.append({ cguids, view->count });
    };
    sugoiS_all(world, true, false, SUGOI_LAMBDA(accumulate));
    skr::Vector<sugoi_entity_t> indices;
    indices.resize_default(guids.size());
    skr::parallel_for(indices.begin(), indices.end(), 2048, [&](auto&& begin, auto&& end) {
        std::iota(begin, end, begin - indices.begin());
    });
    // sort by guid to ensure deterministic order
    std::sort(
#if __cpp_lib_execution >= 201603L
    std::execution::par_unseq,
#endif
    indices.begin(), indices.end(), [&](sugoi_entity_t a, sugoi_entity_t b) {
        return std::lexicographical_compare(&guids[a].storage0, &guids[a].storage3, &guids[b].storage0, &guids[b].storage3);
    });
    skr::Vector<sugoi_entity_t> sortedEntities;
    sortedEntities.resize_default(guids.size());
    skr::parallel_for(indices.begin(), indices.end(), 2048, [&](auto&& begin, auto&& end) {
        for (auto it = begin; it != end; ++it)
        {
            sortedEntities[*it] = entities[it - indices.begin()];
        }
    });
    writer->StartObject();
    auto saveEntity = [&](sugoi_chunk_view_t* view) {
        auto                cguids = (skr_guid_t*)sugoiV_get_owned_ro(view, SUGOI_COMPONENT_GUID);
        auto                group  = sugoiC_get_group(view->chunk);
        sugoi_entity_type_t type;
        sugoiG_get_type(group, &type);
        for (EIndex i = 0; i < view->count; ++i)
        {
            auto guidStr = skr::format(u8"{}", cguids[i]);
            writer->Key(guidStr.u8_str());
            writer->StartObject();
            for (EIndex j = 0; j < type.type.length; ++j)
            {
                auto index     = sugoiG_get_stable_order(group, j);
                auto component = type.type.data[index];
                auto desc      = sugoiT_get_desc(component);
                auto data      = sugoiV_get_owned_ro_local(view, index);
                auto serde     = desc->callback.serialize_text;
                if (!serde)
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

void skr_load_scene(sugoi_storage_t* world, skr::archive::JsonReader* reader)
{
}