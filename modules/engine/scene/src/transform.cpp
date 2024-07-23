#include "SkrBase/types.h"
#include "SkrBase/math/matrix4x4f.h"
#include "SkrBase/math/vector.h"
#include "SkrBase/math/quat.h"
#include "SkrBase/math/rtm/qvvf.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/storage.hpp"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrScene/scene.h"

struct ParallelEntry
{
    uint32_t threshold = 0;
    uint32_t batch_size = 128;
};
skr::FlatHashMap<sugoi_entity_t, ParallelEntry> parallel_nodes;

rtm::qvvf make_qvv(const skr_rotator_t* r, const skr_float3_t* t, const skr_float3_t* s)
{
    const auto default_translation = rtm::vector_set(0.f, 0.f, 0.f);
    const auto default_scale       = rtm::vector_set(1.f, 1.f, 1.f);
    const auto default_quat        = rtm::quat_set(0.f, 0.f, 0.f, 1.f);
    
    const auto translation = t ? skr::math::load(*t) : default_translation;
    const auto scale       = s ? skr::math::load(*s) : default_scale;
    const auto quat           = r ? skr::math::load(*r) : default_quat;

    return rtm::qvv_set(quat, translation, scale);
}

static void skr_relative_to_world_children(const skr_children_t* children, rtm::qvvf parent, sugoi_storage_t* storage)
{
    auto process = [&](sugoi_chunk_view_t* view) {
        SkrZoneScopedN("CalcTransform(Children)");
        auto transforms = sugoi::get_owned<skr_transform_comp_t, skr_transform_t>(view);
        if (!transforms)
            return;
        auto translations = sugoi::get_owned<const skr_translation_comp_t, const skr_float3_t>(view);
        auto rotations    = sugoi::get_owned<const skr_rotation_comp_t, const skr_rotator_t>(view);
        auto scales       = sugoi::get_owned<const skr_scale_comp_t, const skr_float3_t>(view);
        auto childrens    = sugoi::get_owned<const skr_child_comp_t, const skr_children_t>(view);
        for (EIndex i = 0; i < view->count; ++i)
        {
            auto relative  = make_qvv(
                rotations ? &rotations[i] : nullptr, 
                translations ? &translations[i] : nullptr, 
                scales ? &scales[i] : nullptr
            );
            auto transform = rtm::qvv_mul(relative, parent);
            skr::math::store(transform.translation, transforms[i].translation);
            skr::math::store(transform.rotation, transforms[i].rotation);
            skr::math::store(transform.scale, transforms[i].scale);
            
            if (childrens && childrens->size())
            {
                skr_relative_to_world_children(&childrens[i], transform, storage);
            }
        }
    };
    const auto childrenSize = children->size();
    if (childrenSize > 256) // dispatch recursively
    {
        skr::parallel_for(children->begin(), children->end(), 128,
            [&](const auto begin, const auto end) {
                sugoiS_batch(storage, (sugoi_entity_t*)&*begin, (EIndex)(end - begin), SUGOI_LAMBDA(process));
            });
    }
    else
    {
        sugoiS_batch(storage, (sugoi_entity_t*)children->data(), (EIndex)children->size(), SUGOI_LAMBDA(process));
    }
}

static void skr_relative_to_world_root(void* u, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex)
{
    using namespace skr::math;
    SkrZoneScopedN("CalcTransform");
    auto storage = sugoiQ_get_storage(query);
    auto transforms = sugoi::get_owned_local<skr_transform_t>(view, localTypes[0]);
    auto children = sugoi::get_owned_local<const skr_children_t>(view, localTypes[1]);
    auto translations = sugoi::get_owned_local<const skr_float3_t>(view, localTypes[2]);
    auto rotations = sugoi::get_owned_local<const skr_rotator_t>(view, localTypes[3]);
    auto scales = sugoi::get_owned_local<const skr_float3_t>(view, localTypes[4]);
    for (EIndex i = 0; i < view->count; ++i)
    {
        transforms[i].translation = translations ? translations[i] : skr_float3_t{ 0, 0, 0 };
        transforms[i].rotation    = rotations ? rotations[i] : skr_rotator_t{ 0, 0, 0 };
        transforms[i].scale       = scales ? scales[i] : skr_float3_t{ 1, 1, 1 };
    }
    forloop (i, 0, view->count)
    {
        const auto transform = rtm::qvv_set(
            skr::math::load(transforms[i].rotation), 
            skr::math::load(transforms[i].translation), 
            skr::math::load(transforms[i].scale)
        );
        skr_relative_to_world_children(&children[i], transform, storage);
    }
}

void skr_transform_system_setup(sugoi_storage_t* world, skr_transform_system_t* system)
{
    sugoi::EntitySpawner<skr_scale_comp_t> spawner;
    spawner(world, 1, [&](auto& view){ 
        const auto e = sugoiV_get_entities(view.view)[0];
        system->root_meta = e;
    });

    system->calculateTransformTree = world->new_query()
        .ReadWriteAll<skr_transform_comp_t>()
        .ReadAny<skr_child_comp_t>()
        .ReadAny<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t>()
        .WithMetaEntity(system->root_meta)
        .commit().value();
}

void skr_transform_system_update(skr_transform_system_t* system)
{
    SkrZoneScopedN("CalcTransform");
    sugoiJ_schedule_ecs(system->calculateTransformTree, 0, &skr_relative_to_world_root, nullptr, nullptr, nullptr, nullptr, nullptr);
}
