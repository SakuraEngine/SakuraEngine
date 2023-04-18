#include "ecs/array.hpp"
#include "ecs/dual.h"

#include "ecs/dual_config.h"
#include "utils/parallel_for.hpp"
#include "SkrScene/scene.h"
#include "math/matrix4x4f.h"
#include "math/vector.h"
#include "math/quat.h"
#include "rtm/qvvf.h"

skr_float4x4_t make_transform(skr_float3_t t, skr_float3_t s, skr_rotator_t r)
{
    const auto quat = skr::math::load(r);
    const rtm::vector4f translation = skr::math::load(t);
    const rtm::vector4f scale = skr::math::load(s);
    const rtm::qvvf transform = rtm::qvv_set(quat, translation, scale);
    const rtm::matrix4x4f matrix = rtm::matrix_cast(rtm::matrix_from_qvv(transform));
    skr_float4x4_t result;
    skr::math::store(matrix, result);
    return result;
}

template <class T>
static void skr_local_to_x(void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
{
    static_assert(alignof(decltype(T::matrix)) == 16, "Alignment of matrix must be 16");
    static_assert(sizeof(T::matrix) == sizeof(skr_float4x4_t), "Size of matrix must equal to skr_float4x4_t");
    auto translation = (skr_float3_t*)dualV_get_owned_ro_local(view, localTypes[0]);
    auto rotation = (skr_rotator_t*)dualV_get_owned_ro_local(view, localTypes[1]);
    auto scale = (skr_float3_t*)dualV_get_owned_ro_local(view, localTypes[2]);
    auto transform = (skr_float4x4_t*)dualV_get_owned_ro_local(view, localTypes[3]);
    const auto default_translation = skr_float3_t {0.f, 0.f, 0.f};
    const auto default_scale = skr_float3_t {1.f, 1.f, 1.f};
    const auto default_rotation = skr_rotator_t {0.f, 0.f, 0.f};
    if (translation)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], default_scale, default_rotation);
    if (translation && scale)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], scale[i], default_rotation);
    if (translation && rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], default_scale, rotation[i]);
    if (translation && scale && rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(translation[i], scale[i], rotation[i]);
    if (scale && rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(default_translation, scale[i], rotation[i]);
    if (scale)
        forloop (i, 0, view->count)
            *transform = make_transform(default_translation, scale[i], default_rotation);
    if (rotation)
        forloop (i, 0, view->count)
            *transform = make_transform(default_translation, default_scale, rotation[i]);
}

static void skr_relative_to_world_children(skr_children_t* children, skr_l2w_comp_t* parent, dual_storage_t* storage)
{
    auto process = [&](skr_child_comp_t child) {
        dual_chunk_view_t view;
        //TODO: consider dualS_batch?
        dualS_access(storage, child.entity, &view);
        auto relative = (skr_float4x4_t*)dualV_get_owned_ro(&view, dual_id_of<skr_l2r_comp_t>::get());
        if (!relative) 
            return;
        auto transform = (skr_float4x4_t*)dualV_get_owned_ro(&view, dual_id_of<skr_l2w_comp_t>::get());
        if (!transform) 
            return;
        skr::math::store(rtm::matrix_mul(skr::math::load(*relative), skr::math::load(parent->matrix)), *transform);
        auto children = (skr_children_t*)dualV_get_owned_ro(&view, dual_id_of<skr_child_comp_t>::get());
        if (!children) 
            return;
        skr_relative_to_world_children(children, (skr_l2w_comp_t*)transform, storage);
    };
    if (children->size() > 256) // dispatch recursively
    {
        using iter_t = typename skr_children_t::iterator;
        skr::parallel_for(children->begin(), children->end(), 128,
        [&](iter_t begin, iter_t end) {
            for (auto i = begin; i != end; ++i)
                process(*i);
        });
    }
    else
        for (auto child : *children)
            process(child);
}

static void skr_relative_to_world_root(void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
{
    using namespace skr::math;
    auto transform = (skr_l2w_comp_t*)dualV_get_owned_ro_local(view, localTypes[0]);
    auto children = (skr_children_t*)dualV_get_owned_ro_local(view, localTypes[1]);

    forloop (i, 0, view->count)
        skr_relative_to_world_children(&children[i], &transform[i], storage);
}

void skr_transform_setup(dual_storage_t* world, skr_transform_system_t* system)
{
    // for root entities, calculate local to world
    system->localToWorld = dualQ_from_literal(world, "[in]|skr_translation_comp_t,[in]|skr_rotation_comp_t, [in]|skr_scale_comp_t,[out]skr_l2w_comp_t,!skr_parent_comp_t");

    // for node entities, calculate local to parent
    system->localToRelative = dualQ_from_literal(world, "[in]|skr_translation_comp_t,[in]|skr_rotation_comp_t,[in]|skr_scale_comp_t,[out]skr_l2r_comp_t,[has]skr_parent_comp_t");

    // then recursively calculate local to world for node entities
    system->relativeToWorld = dualQ_from_literal(world, "[inout]<seq>skr_l2w_comp_t,[in]<seq>skr_child_comp_t,[in]<seq>?skr_l2r_comp_t,!skr_parent_comp_t");
}

void skr_transform_update(skr_transform_system_t* query)
{
    dualJ_schedule_ecs(query->localToWorld, 256, &skr_local_to_x<skr_l2w_comp_t>, nullptr, nullptr, nullptr, nullptr, nullptr);
    dualJ_schedule_ecs(query->localToRelative, 256, &skr_local_to_x<skr_l2r_comp_t>, nullptr, nullptr, nullptr, nullptr, nullptr);
    dualJ_schedule_ecs(query->relativeToWorld, 128, &skr_relative_to_world_root, nullptr, nullptr, nullptr, nullptr, nullptr);
}
