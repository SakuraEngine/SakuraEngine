#include "render_world.h"

#include "MPShared/components.h"
#include "SkrRenderer/render_effect.h"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/ecs/set.hpp"
#include "SkrBase/math/vector.h"
#include "SkrBase/math/quat.h"

#include "SkrProfile/profile.h"

void MPRenderWorld::Initialize(MPClientWorld* gameWorld)
{
    this->storage = sugoiS_create();
    this->gameWorld = gameWorld;
    skr_init_hires_timer(&renderTimer);
    renderGhostsQuery = sugoiQ_from_literal(storage, u8"[in]CGhost");
    gameGhostsQuery = sugoiQ_from_literal(gameWorld->storage, u8"[in]CGhost, [has]CPrefab");
    transformQuery = sugoiQ_from_literal(storage, u8"[in]skr_rotation_comp_t, [in]skr_translation_comp_t, [in]skr_scale_comp_t, [in]CGhost");
    cameraQuery = sugoiQ_from_literal(storage, u8"[inout]skr_translation_comp_t, [inout]skr_camera_comp_t");
    sugoiJ_bind_storage(storage);
}

void MPRenderWorld::Shutdown()
{
    sugoiJ_unbind_storage(storage);
    sugoiQ_release(renderGhostsQuery);
    sugoiQ_release(gameGhostsQuery);
    sugoiQ_release(cameraQuery);
    sugoiS_release(storage);
}

void MPRenderWorld::LoadScene()
{
    {
        auto setup = [&](sugoi_chunk_view_t* view) {
            auto translations = sugoi::get_owned_rw<skr_translation_comp_t>(view);
            auto rotations = sugoi::get_owned_rw<skr_rotation_comp_t>(view);
            auto scales = sugoi::get_owned_rw<skr_scale_comp_t>(view);
            
            for (uint32_t i = 0; i < view->count; i++)
            {
                translations[i].value = { 0.f, -100.f, 0.f };
                rotations[i].euler = { 0.f, 0.f, 0.f };
                scales[i].value = { 8.f, 8.f, 8.f };
            }
        };          
        // allocate 1 player entity
        auto playerT_builder = make_zeroed<sugoi::TypeSetBuilder>();
        playerT_builder
            .with<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t>()
            .with<skr_camera_comp_t>();
        auto playerT = make_zeroed<sugoi_entity_type_t>();
        playerT.type = playerT_builder.build();
        sugoiS_allocate_type(storage, &playerT, 1, SUGOI_LAMBDA(setup));
    }
}


sugoi::TypeSetBuilder MPRenderWorld::GetRenderEntityType(skr_resource_handle_t prefab, bool controller)
{
    sugoi::TypeSetBuilder builder;
    builder.with<CGhost, skr_render_effect_t>();
    builder.with<skr_rotation_comp_t, skr_translation_comp_t, skr_scale_comp_t>();
    if (controller)
    {
        builder.with<CController>();
    }
    return builder;
}

void MPRenderWorld::UpdateStructuralChanges()
{
    SkrZoneScopedN("MPRenderWorld::UpdateStructuralChanges");
    gameToRenderEntityMap.clear();
    toDeleteRenderEntities.clear();
    newGameEntities.clear();
    auto buildMap = [&](sugoi_chunk_view_t* view)
    {
        auto entities = sugoiV_get_entities(view);
        auto ghosts = (const CGhost*)sugoiV_get_owned_ro(view, sugoi_id_of<CGhost>::get());
        for (int i = 0; i < view->count; ++i)
        {
            if(ghosts[i].mappedEntity != SUGOI_NULL_ENTITY)
            {
                gameToRenderEntityMap[entities[i]] = ghosts[i].mappedEntity;
            }
            else 
            {
                newGameEntities.add(entities[i]);
            }
        }
    };
    sugoiQ_get_views(gameGhostsQuery, SUGOI_LAMBDA(buildMap));
    for(auto& pair : renderToGameEntityMap)
    {
        if(gameToRenderEntityMap.find(pair.second) == gameToRenderEntityMap.end())
        {
            toDeleteRenderEntities.add(pair.first);
        }
    }
    auto deleteRenderEntity = [&](sugoi_chunk_view_t* view)
    {
        auto entities = (sugoi_entity_t*)sugoiV_get_entities(view);
        for (int i = 0; i < view->count; ++i)
        {
            renderToGameEntityMap.erase(entities[i]);
        }
        auto modelFree = [=](sugoi_chunk_view_t* view) {
        };
        skr_render_effect_access(renderer, view, u8"ForwardEffect", SUGOI_LAMBDA(modelFree));
        skr_render_effect_detach(renderer, view, u8"ForwardEffect");
    };
    sugoiS_batch(storage, toDeleteRenderEntities.data(), toDeleteRenderEntities.size(), SUGOI_LAMBDA(deleteRenderEntity));
    sugoiS_destroy_entities(storage, toDeleteRenderEntities.data(), (uint32_t)toDeleteRenderEntities.size());
    auto createRenderEntity = [&](sugoi_chunk_view_t* view)
    {
        auto entities = sugoiV_get_entities(view);
        auto ghosts = (CGhost*)sugoiV_get_owned_rw(view, sugoi_id_of<CGhost>::get());
        auto prefabs = (CPrefab*)sugoiV_get_owned_ro(view, sugoi_id_of<CPrefab>::get());
        auto controllers = (CController*)sugoiV_get_owned_ro(view, sugoi_id_of<CController>::get());
        auto translations = sugoi::get_owned_ro<skr_translation_comp_t>(view);
        auto rotations = sugoi::get_owned_ro<skr_rotation_comp_t>(view);
        auto scale = sugoi::get_owned_ro<skr_scale_comp_t>(view);
        
        sugoi_entity_type_t batchedType = make_zeroed<sugoi_entity_type_t>();
        auto batchedBuilder = GetRenderEntityType(prefabs[0].prefab, controllers != nullptr);
        batchedType.type = batchedBuilder.build();
        EIndex start = 0;
        EIndex count = 1;
        auto create = [&]()
        {
            EIndex g_id = start;
            auto initialize = [&](sugoi_chunk_view_t* view) {
                auto renderGhosts = (CGhost*)sugoiV_get_owned_rw(view, sugoi_id_of<CGhost>::get());
                auto renderEntities = sugoiV_get_entities(view);
                auto renderControllers = (CController*)sugoiV_get_owned_rw(view, sugoi_id_of<CController>::get());
                auto renderTranslations = sugoi::get_owned_rw<skr_translation_comp_t>(view);
                auto renderRotations = sugoi::get_owned_rw<skr_rotation_comp_t>(view);
                auto renderScale = sugoi::get_owned_rw<skr_scale_comp_t>(view);
                for (int i = 0; i < view->count; ++i)
                {
                    ghosts[g_id + i].mappedEntity = renderEntities[i];
                    gameToRenderEntityMap[entities[g_id + i]] = ghosts[g_id + i].mappedEntity;
                    renderToGameEntityMap[renderEntities[i]] = entities[g_id + i];
                    renderGhosts[i].mappedEntity = entities[g_id + i];
                    renderTranslations[i] = translations[g_id + i];
                    renderRotations[i] = rotations[g_id + i];
                    renderScale[i] = scale[g_id + i];
                    if(controllers)
                        renderControllers[i] = controllers[g_id + i];
                }
                g_id += view->count;
                skr_render_effect_attach(renderer, view, u8"ForwardEffect");
            };
            sugoiS_allocate_type(storage, &batchedType, count, SUGOI_LAMBDA(initialize));
        };
        for(int j = 1; j < view->count; ++j)
        {
            auto builder = GetRenderEntityType(prefabs[j].prefab, controllers != nullptr);
            sugoi_entity_type_t type = make_zeroed<sugoi_entity_type_t>();
            type.type = builder.build();
            if(sugoi::equal(batchedType, type))
            {
                ++count;
            }
            else
            {
                create();
                batchedType = type;
                batchedBuilder = builder;
                start = j;
                count = 1;
            }
        }
        create();
    };
    sugoiS_batch(gameWorld->storage, newGameEntities.data(), newGameEntities.size(), SUGOI_LAMBDA(createRenderEntity));
}

void MPRenderWorld::Update()
{
    SkrZoneScopedN("RenderWorld::Update");
    deltaTime = skr_hires_timer_get_seconds(&renderTimer, true);
    //camera follow entity with CController
    {
        sugoi_filter_t filter = make_zeroed<sugoi_filter_t>();
        sugoi::TypeSetBuilder builder;
        builder.with<skr_translation_comp_t, CController>();
        filter.all = builder.build();
        sugoi_meta_filter_t metaFilter = make_zeroed<sugoi_meta_filter_t>();
        skr_float3_t controllerPos = { 0.f, 0.f, 0.f };
        auto getControllerPos = [&](sugoi_chunk_view_t* view)
        {
            auto translations = sugoi::get_owned_ro<skr_translation_comp_t>(view);
            for (uint32_t i = 0; i < view->count; i++)
            {
                controllerPos = translations[i].value;
            }
        };
        sugoiS_query(storage, &filter, &metaFilter, SUGOI_LAMBDA(getControllerPos));
        auto updateCamera = [&](sugoi_chunk_view_t* view)
        {
            auto translations = sugoi::get_owned_rw<skr_translation_comp_t>(view);
            for (uint32_t i = 0; i < view->count; i++)
            {
                //translations[i].value = {controllerPos.x, translations[i].value.y, controllerPos.z};
                auto translation = skr::math::load(translations[i].value);
                auto target = rtm::vector_set(controllerPos.x, translations[i].value.y, controllerPos.z, 1.f);
                auto offset = rtm::vector_sub(target, translation);
                float distance = rtm::vector_length3(offset);
                if(distance > 0.0001f)
                {
                    auto direction = rtm::vector_div(offset, rtm::vector_set(distance));
                    auto move = std::min<float>(std::max(15 * deltaTime, distance * 0.7 * deltaTime), distance);
                    auto newTranslation = rtm::vector_add(translation, rtm::vector_mul(direction, move));
                    skr::math::store(newTranslation, translations[i].value);
                }
            }
        };
        sugoiQ_get_views(cameraQuery, SUGOI_LAMBDA(updateCamera));
    }
    //TODO: switch to new api
    auto updateTransform = +[](void* u, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex)
    {
        auto This = (MPRenderWorld*)u;
        auto rotations = (skr_rotation_comp_t*)sugoiV_get_owned_ro_local(view, localTypes[0]);
        auto translations = (skr_translation_comp_t*)sugoiV_get_owned_ro_local(view, localTypes[1]);
        auto scales = (skr_scale_comp_t*)sugoiV_get_owned_ro_local(view, localTypes[2]);
        auto ghosts = (sugoi_entity_t*)sugoiV_get_owned_ro_local(view, localTypes[3]);
        auto r_id = 0;
        auto update = [&](sugoi_chunk_view_t* view)
        { 
            auto gameRotations = (skr_rotation_comp_t*)sugoiV_get_owned_ro(view, sugoi_id_of<skr_rotation_comp_t>::get());
            auto gameTranslations = (skr_translation_comp_t*)sugoiV_get_owned_ro(view, sugoi_id_of<skr_translation_comp_t>::get());
            auto gameScales = (skr_scale_comp_t*)sugoiV_get_owned_ro(view, sugoi_id_of<skr_scale_comp_t>::get());
            for(EIndex i = 0; i < view->count; ++i)
            {
                auto gameRotation = gameRotations[i].euler.yaw;
                auto gameTranslation = skr::math::load(gameTranslations[i].value);
                auto gameScale = skr::math::load(gameScales[i].value);
                // auto rotation = rotations[r_id + i].euler.yaw;
                auto translation = skr::math::load(translations[r_id + i].value);
                auto scale = skr::math::load(scales[r_id + i].value);
                //TODO: angle interpolation
                auto newRotation = gameRotation;//rtm::scalar_lerp(rotation, gameRotation, 10.f * (float)This->deltaTime);
                auto newTranslation = rtm::vector_lerp(translation, gameTranslation, 10.f * This->deltaTime);
                auto newScale = rtm::vector_lerp(scale, gameScale, 10.f * This->deltaTime);
                rotations[r_id + i].euler.yaw = newRotation;
                skr::math::store(newTranslation, translations[r_id + i].value);
                skr::math::store(newScale, scales[r_id + i].value);
            }
            r_id += view->count;
        };
        sugoiS_batch(This->gameWorld->storage, ghosts, view->count, SUGOI_LAMBDA(update));
    };
    sugoiJ_schedule_ecs(transformQuery, 512, updateTransform, this, nullptr, nullptr, nullptr, nullptr);
    sugoiJ_wait_all();
}
