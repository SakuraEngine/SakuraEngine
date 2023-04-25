#include "ecs/array.hpp"
#include "ecs/dual.h"

#include "ecs/dual_config.h"
#include "utils/parallel_for.hpp"
#include "SkrScene/scene.h"
#include "math/matrix4x4f.h"
#include "math/vector.h"
#include "math/quat.h"
#include "rtm/qvvf.h"

rtm::qvvf make_qvv(skr_rotator_t* r, skr_float3_t* t, skr_float3_t* s)
{
    const auto default_translation = rtm::vector_set(0.f, 0.f, 0.f);
    const auto default_scale = rtm::vector_set(1.f, 1.f, 1.f);
    const auto default_quat = rtm::quat_set(0.f, 0.f, 0.f, 1.f);
    if (r && t && s)
        return rtm::qvv_set(skr::math::load(*r), skr::math::load(*t), skr::math::load(*s));
    else if (r && s)
        return rtm::qvv_set(skr::math::load(*r), default_translation, skr::math::load(*s));
    else if (r && t)
        return rtm::qvv_set(skr::math::load(*r), skr::math::load(*t), default_scale);
    else if (t && s)
        return rtm::qvv_set(default_quat, skr::math::load(*t), skr::math::load(*s));
    else if (t)
        return rtm::qvv_set(default_quat, skr::math::load(*t), default_scale);
    else if (r)
        return rtm::qvv_set(skr::math::load(*r), default_translation, default_scale);
    else if (s)
        return rtm::qvv_set(default_quat, default_translation, skr::math::load(*s));
    else
        return rtm::qvv_set(default_quat, default_translation, default_scale);
}

static void skr_relative_to_world_children(skr_children_t* children, rtm::qvvf parent, dual_storage_t* storage)
{
    auto process = [&](dual_chunk_view_t* view) {
        auto transforms = (skr_transform_t*)dualV_get_owned_ro(view, dual_id_of<skr_transform_comp_t>::get());
        if (!transforms) 
            return;
        auto translations = (skr_float3_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_comp_t>::get());
        auto rotations = (skr_rotator_t*)dualV_get_owned_ro(view, dual_id_of<skr_rotation_comp_t>::get());
        auto scales = (skr_float3_t*)dualV_get_owned_ro(view, dual_id_of<skr_scale_comp_t>::get());
        auto childrens = (skr_children_t*)dualV_get_owned_ro(view, dual_id_of<skr_child_comp_t>::get());
        for(EIndex i = 0; i < view->count; ++i)
        {
            auto relative = make_qvv(rotations ? &rotations[i] : nullptr, translations ? &translations[i] : nullptr, scales ? &scales[i] : nullptr);
            auto transform = rtm::qvv_mul(relative, parent);
            skr::math::store(transform.translation, transforms[i].translation);
            skr::math::store(transform.rotation, transforms[i].rotation);
            skr::math::store(transform.scale, transforms[i].scale);
            if (!childrens) 
                continue;
            skr_relative_to_world_children(children, transform, storage);
        }
    };
    if (children->size() > 256) // dispatch recursively
    {
        using iter_t = typename skr_children_t::iterator;
        skr::parallel_for(children->begin(), children->end(), 128,
        [&](iter_t begin, iter_t end) {
            dualS_batch(storage, (dual_entity_t*)&*begin, (EIndex)(end-begin), DUAL_LAMBDA(process));
        });
    }
    else
    {
        dualS_batch(storage, (dual_entity_t*)children->data(), (EIndex)children->size(), DUAL_LAMBDA(process));
    }
}

static void skr_relative_to_world_root(void* u, dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
{
    using namespace skr::math;
    auto transforms = (skr_transform_t*)dualV_get_owned_ro_local(view, localTypes[0]);
    auto children = (skr_children_t*)dualV_get_owned_ro_local(view, localTypes[1]);
    auto translations = (skr_float3_t*)dualV_get_owned_ro_local(view, localTypes[2]);
    auto rotations = (skr_rotator_t*)dualV_get_owned_ro_local(view, localTypes[3]);
    auto scales = (skr_float3_t*)dualV_get_owned_ro_local(view, localTypes[4]);
    for(EIndex i = 0; i < view->count; ++i)
    {
        transforms[i].translation = translations ? translations[i] : skr_float3_t{0,0,0};
        transforms[i].rotation = rotations ? rotations[i] : skr_rotator_t{0,0,0};
        transforms[i].scale = scales ? scales[i] : skr_float3_t{1,1,1};
    }
    auto storage = dualQ_get_storage(query);
    forloop (i, 0, view->count)
    {
        auto transform = rtm::qvv_set(skr::math::load(transforms[i].rotation), skr::math::load(transforms[i].translation), skr::math::load(transforms[i].scale));
        skr_relative_to_world_children(&children[i], transform, storage);
    }
}

void skr_transform_setup(dual_storage_t* world, skr_transform_system_t* system)
{
    // then recursively calculate local to world for node entities
    system->relativeToWorld = dualQ_from_literal(world, "[inout]<seq>skr_transform_comp_t,[in]<seq>skr_child_comp_t,!skr_parent_comp_t,[in]<seq>?skr_translation_comp_t,[in]<seq>?skr_rotation_comp_t,[in]<seq>?skr_scale_comp_t");
}

void skr_transform_update(skr_transform_system_t* query)
{
    dualJ_schedule_ecs(query->relativeToWorld, 128, &skr_relative_to_world_root, nullptr, nullptr, nullptr, nullptr, nullptr);
}
