#include "transform.hpp"
#include "ecs/array.hpp"
#include "ecs/dual.h"
#include "ecs/callback.hpp"
#include "ecs/dual_config.h"
#include "ftl/task.h"
#include "math/vector.hpp"
#include "math/vectormath.hpp"
#include "scene.h"
#include "utils/parallel_for.hpp"

template <class T>
static void skr_local_to_x(void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
{
    using namespace skr::math;
    auto translation = (skr_translation_t*)dualV_get_owned_ro_local(view, localTypes[0]);
    auto rotation = (skr_rotation_t*)dualV_get_owned_ro_local(view, localTypes[1]);
    auto scale = (skr_scale_t*)dualV_get_owned_ro_local(view, localTypes[2]);
    auto transform = (T*)dualV_get_owned_ro_local(view, localTypes[3]);
    if (translation)
        forloop (i, 0, view->count)
            transform->matrix = make_transform(translation[i].value, Vector3f::vector_one());
    if (translation && scale)
        forloop (i, 0, view->count)
            transform->matrix = make_transform(translation[i].value, scale[i].value);
    if (translation && rotation)
        forloop (i, 0, view->count)
            transform->matrix = make_transform(translation[i].value, Vector3f::vector_one(), quaternion_from_rotator(rotation[i].euler));
    if (translation && scale && rotation)
        forloop (i, 0, view->count)
            transform->matrix = make_transform(translation[i].value, scale[i].value, quaternion_from_rotator(rotation[i].euler));
    if (scale && rotation)
        forloop (i, 0, view->count)
            transform->matrix = make_transform(Vector3f::vector_zero(), scale[i].value, quaternion_from_rotator(rotation[i].euler));
    if (scale)
        forloop (i, 0, view->count)
            transform->matrix = make_transform(Vector3f::vector_zero(), scale[i].value);
    if (rotation)
        forloop (i, 0, view->count)
            transform->matrix = make_transform(Vector3f::vector_zero(), Vector3f::vector_one(), quaternion_from_rotator(rotation[i].euler));
}

static void skr_relative_to_world_children(skr_children_t* children, skr_l2w_t* parent, dual_storage_t* storage)
{
    using namespace skr::math;
    auto process = [&](skr_child_t child) {
        dual_chunk_view_t view;
        dualS_access(storage, child.entity, &view);
        auto relative = (skr_l2r_t*)dualV_get_owned_ro(&view, dual_id_of<skr_l2r_t>::get());
        if (!relative) return;
        auto transform = (skr_l2w_t*)dualV_get_owned_ro(&view, dual_id_of<skr_l2w_t>::get());
        if (!transform) return;
        transform->matrix = multiply(relative->matrix, parent->matrix);
        auto children = (skr_children_t*)dualV_get_owned_ro(&view, dual_id_of<skr_child_t>::get());
        if (!children) return;
        skr_relative_to_world_children(children, transform, storage);
    };
    if (children->size() > 256) // dispatch recursively
    {
        using iter_t = typename skr_children_t::iterator;
        skr::parallel_for((ftl::TaskScheduler*)dualJ_get_scheduler(), children->begin(), children->end(), 128,
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
    auto transform = (skr_l2w_t*)dualV_get_owned_ro_local(view, localTypes[0]);
    auto children = (skr_children_t*)dualV_get_owned_ro_local(view, localTypes[1]);

    forloop (i, 0, view->count)
        skr_relative_to_world_children(&children[i], &transform[i], storage);
}

void skr_transform_setup(dual_storage_t* world, skr_transform_system* system)
{
    // for root entities, calculate local to world
    system->localToWorld = dualQ_from_literal(world, "[in]|skr_translation_t [in]|skr_rotation_t [in]|skr_scale_t [out]skr_l2w_t !skr_parent_t");

    // for node entities, calculate local to parent
    system->localToRelative = dualQ_from_literal(world, "[in]|skr_translation_t [in]|skr_rotation_t [in]|skr_scale_t [out]skr_l2r_t [has]skr_parent_t");

    // then recursively calculate local to world for node entities
    system->relativeToWorld = dualQ_from_literal(world, "[inout][rand]skr_l2w_t [in][rand]skr_child_t [in][rand]?skr_l2r_t !skr_parent_t");
}

void skr_transform_update(skr_transform_system* query)
{
    dualJ_schedule_ecs(query->localToWorld, 256, &skr_local_to_x<skr_l2w_t>, nullptr, nullptr, nullptr, nullptr);
    dualJ_schedule_ecs(query->localToRelative, 256, &skr_local_to_x<skr_l2r_t>, nullptr, nullptr, nullptr, nullptr);
    dualJ_schedule_ecs(query->relativeToWorld, 128, &skr_relative_to_world_root, nullptr, nullptr, nullptr, nullptr);
}