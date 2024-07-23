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

namespace skr
{
struct skr::TransformSystem::Impl
{
    struct ParallelEntry
    {
        uint32_t threshold = 0;
        uint32_t batch_size = 128;
    };
    sugoi_query_t* calculateTransformTree;
    sugoi_entity_t root_meta;
    skr::FlatHashMap<sugoi_entity_t, ParallelEntry> parallel_nodes;
};

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

static void skr_relative_to_world_children(const skr::ChildrenArray* children, rtm::qvvf parent, sugoi_storage_t* storage)
{
    auto task = [&](sugoi_chunk_view_t* view) {
        SkrZoneScopedN("CalcTransform(Children)");
        if (auto transforms = sugoi::get_owned<skr::TransformComponent, skr_transform_t>(view))
        {
            auto translations = sugoi::get_owned<const skr::TranslationComponent, const skr_float3_t>(view);
            auto rotations    = sugoi::get_owned<const skr::RotationComponent, const skr_rotator_t>(view);
            auto scales       = sugoi::get_owned<const skr::ScaleComponent, const skr_float3_t>(view);
            auto childrens    = sugoi::get_owned<const skr::ChildrenComponent, const skr::ChildrenArray>(view);
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
        }
    };
    const auto childrenSize = children->size();
    if (childrenSize > 256) // dispatch recursively
    {
        skr::parallel_for(children->begin(), children->end(), 128,
            [&](const auto begin, const auto end) {
                sugoiS_batch(storage, (sugoi_entity_t*)&*begin, (EIndex)(end - begin), SUGOI_LAMBDA(task));
            });
    }
    else
    {
        sugoiS_batch(storage, (sugoi_entity_t*)children->data(), (EIndex)children->size(), SUGOI_LAMBDA(task));
    }
}

static void skr_relative_to_world_root(void* u, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex)
{
    using namespace skr::math;
    SkrZoneScopedN("CalcTransform");
    auto storage = sugoiQ_get_storage(query);
    auto transforms = sugoi::get_owned_local<skr_transform_t>(view, localTypes[0]);
    auto children = sugoi::get_owned_local<const skr::ChildrenArray>(view, localTypes[1]);
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

TransformSystem* TransformSystem::Create(sugoi_storage_t* world) SKR_NOEXCEPT
{
    SkrZoneScopedN("CreateTransformSystem");
    auto memory = (uint8_t*)sakura_calloc(1, sizeof(TransformSystem) + sizeof(TransformSystem::Impl));
    auto system = new(memory) TransformSystem();
    system->impl = new(memory + sizeof(TransformSystem)) TransformSystem::Impl();

    sugoi::EntitySpawner<skr::ScaleComponent> spawner;
    spawner(world, 1, [&](auto& view){ 
        const auto e = sugoiV_get_entities(view.view)[0];
        system->impl->root_meta = e;
    });

    system->impl->calculateTransformTree = world->new_query()
        .ReadWriteAll<skr::TransformComponent>()
        .ReadAny<skr::ChildrenComponent>()
        .ReadAny<skr::TranslationComponent, skr::RotationComponent, skr::ScaleComponent>()
        .WithMetaEntity(system->impl->root_meta)
        .commit().value();

    return system;
}

void TransformSystem::Destroy(TransformSystem *system) SKR_NOEXCEPT
{
    SkrZoneScopedN("FinalizeTransformSystem");
    system->impl->~Impl();
    system->~TransformSystem();
    sakura_free(system);
}

void TransformSystem::update() SKR_NOEXCEPT
{
    SkrZoneScopedN("CalcTransform");
    sugoiJ_schedule_ecs(impl->calculateTransformTree, 0, &skr_relative_to_world_root, nullptr, nullptr, nullptr, nullptr, nullptr);
}

sugoi_entity_t TransformSystem::root_mark() const SKR_NOEXCEPT
{
    return impl->root_meta;
}

void TransformSystem::set_parallel_entry(sugoi_entity_t entity) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

} // namespace skr

skr::TransformSystem* skr_transform_system_create(sugoi_storage_t* world)
{
    return skr::TransformSystem::Create(world);
}

void skr_transform_system_destroy(skr::TransformSystem* system)
{
    skr::TransformSystem::Destroy(system);
}

void skr_transform_system_set_parallel_entry(skr::TransformSystem* system, sugoi_entity_t entity)
{
    system->set_parallel_entry(entity);
}

void skr_transform_system_update(skr::TransformSystem* system)
{
    system->update();
}

void skr_propagate_transform(sugoi_storage_t* world, sugoi_entity_t* entities, uint32_t count)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}