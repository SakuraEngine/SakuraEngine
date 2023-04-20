#include "render_world.h"

#include "MPShared/components.h"
#include "SkrRenderer/render_effect.h"
#include "ecs/type_builder.hpp"
#include "utils/make_zeroed.hpp"
#include "ecs/set.hpp"
#include "math/vector.h"
#include "math/quat.h"


void MPRenderWorld::Initialize(MPClientWorld* gameWorld)
{
    this->storage = dualS_create();
    this->gameWorld = gameWorld;
    skr_init_hires_timer(&renderTimer);
    renderGhostsQuery = dualQ_from_literal(storage, "[in]CGhost");
    gameGhostsQuery = dualQ_from_literal(gameWorld->storage, "[in]CGhost, [has]CPrefab");
    transformQuery = dualQ_from_literal(storage, "[in]skr_rotation_comp_t, [in]skr_translation_comp_t, [in]skr_scale_comp_t, [in]CGhost");
    cameraQuery = dualQ_from_literal(storage,
        "[inout]skr_translation_comp_t, [inout]skr_camera_comp_t");
    dualJ_bind_storage(storage);
}

void MPRenderWorld::Shutdown()
{
    dualJ_unbind_storage(storage);
    dualQ_release(renderGhostsQuery);
    dualQ_release(gameGhostsQuery);
    dualQ_release(cameraQuery);
    dualS_release(storage);
}

void MPRenderWorld::LoadScene()
{
    {
        auto setup = [&](dual_chunk_view_t* view) {
            auto translations = dual::get_owned_rw<skr_translation_comp_t>(view);
            auto rotations = dual::get_owned_rw<skr_rotation_comp_t>(view);
            auto scales = dual::get_owned_rw<skr_scale_comp_t>(view);
            
            for (uint32_t i = 0; i < view->count; i++)
            {
                translations[i].value = { 0.f, -100.f, 0.f };
                rotations[i].euler = { 0.f, 0.f, 0.f };
                scales[i].value = { 8.f, 8.f, 8.f };
            }
        };          
        // allocate 1 player entity
        auto playerT_builder = make_zeroed<dual::type_builder_t>();
        playerT_builder
            .with<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t>()
            .with<skr_camera_comp_t>();
        auto playerT = make_zeroed<dual_entity_type_t>();
        playerT.type = playerT_builder.build();
        dualS_allocate_type(storage, &playerT, 1, DUAL_LAMBDA(setup));
    }
}


dual::type_builder_t MPRenderWorld::GetRenderEntityType(skr_resource_handle_t prefab, bool controller)
{
    dual::type_builder_t builder;
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
    ZoneScopedN("MPRenderWorld::UpdateStructuralChanges");
    gameToRenderEntityMap.clear();
    toDeleteRenderEntities.clear();
    newGameEntities.clear();
    auto buildMap = [&](dual_chunk_view_t* view)
    {
        auto entities = dualV_get_entities(view);
        auto ghosts = (CGhost*)dualV_get_owned_ro(view, dual_id_of<CGhost>::get());
        for (int i = 0; i < view->count; ++i)
        {
            if(ghosts[i].mappedEntity != DUAL_NULL_ENTITY)
            {
                gameToRenderEntityMap[entities[i]] = ghosts[i].mappedEntity;
            }
            else 
            {
                newGameEntities.push_back(entities[i]);
            }
        }
    };
    dualQ_get_views(gameGhostsQuery, DUAL_LAMBDA(buildMap));
    for(auto& pair : renderToGameEntityMap)
    {
        if(gameToRenderEntityMap.find(pair.second) == gameToRenderEntityMap.end())
        {
            toDeleteRenderEntities.push_back(pair.first);
        }
    }
    auto deleteRenderEntity = [&](dual_chunk_view_t* view)
    {
        auto entities = (dual_entity_t*)dualV_get_entities(view);
        for (int i = 0; i < view->count; ++i)
        {
            renderToGameEntityMap.erase(entities[i]);
        }
        auto modelFree = [=](dual_chunk_view_t* view) {
        };
        skr_render_effect_access(renderer, view, "ForwardEffect", DUAL_LAMBDA(modelFree));
        skr_render_effect_detach(renderer, view, "ForwardEffect");
        dualS_destroy(storage, view);
    };
    dualS_batch(storage, toDeleteRenderEntities.data(), toDeleteRenderEntities.size(), DUAL_LAMBDA(deleteRenderEntity));
    auto createRenderEntity = [&](dual_chunk_view_t* view)
    {
        auto entities = dualV_get_entities(view);
        auto ghosts = (CGhost*)dualV_get_owned_rw(view, dual_id_of<CGhost>::get());
        auto prefabs = (CPrefab*)dualV_get_owned_ro(view, dual_id_of<CPrefab>::get());
        auto controllers = (CController*)dualV_get_owned_ro(view, dual_id_of<CController>::get());
        auto translations = dual::get_owned_ro<skr_translation_comp_t>(view);
        auto rotations = dual::get_owned_ro<skr_rotation_comp_t>(view);
        auto scale = dual::get_owned_ro<skr_scale_comp_t>(view);
        
        dual_entity_type_t batchedType = make_zeroed<dual_entity_type_t>();
        auto batchedBuilder = GetRenderEntityType(prefabs[0].prefab, controllers != nullptr);
        batchedType.type = batchedBuilder.build();
        EIndex start = 0;
        EIndex count = 1;
        auto create = [&]()
        {
            EIndex g_id = start;
            auto initialize = [&](dual_chunk_view_t* view) {
                auto renderGhosts = (CGhost*)dualV_get_owned_rw(view, dual_id_of<CGhost>::get());
                auto renderEntities = dualV_get_entities(view);
                auto renderControllers = (CController*)dualV_get_owned_rw(view, dual_id_of<CController>::get());
                auto renderTranslations = dual::get_owned_rw<skr_translation_comp_t>(view);
                auto renderRotations = dual::get_owned_rw<skr_rotation_comp_t>(view);
                auto renderScale = dual::get_owned_rw<skr_scale_comp_t>(view);
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
                skr_render_effect_attach(renderer, view, "ForwardEffect");
            };
            dualS_allocate_type(storage, &batchedType, count, DUAL_LAMBDA(initialize));
        };
        for(int j = 1; j < view->count; ++j)
        {
            auto builder = GetRenderEntityType(prefabs[j].prefab, controllers != nullptr);
            dual_entity_type_t type = make_zeroed<dual_entity_type_t>();
            type.type = builder.build();
            if(dual::equal(batchedType, type))
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
    dualS_batch(gameWorld->storage, newGameEntities.data(), newGameEntities.size(), DUAL_LAMBDA(createRenderEntity));
}

void MPRenderWorld::Update()
{
    ZoneScopedN("RenderWorld::Update");
    deltaTime = skr_hires_timer_get_seconds(&renderTimer, true);
    //camera follow entity with CController
    {
        dual_filter_t filter = make_zeroed<dual_filter_t>();
        dual::type_builder_t builder;
        builder.with<skr_translation_comp_t, CController>();
        filter.all = builder.build();
        dual_meta_filter_t metaFilter = make_zeroed<dual_meta_filter_t>();
        skr_float3_t controllerPos = { 0.f, 0.f, 0.f };
        auto getControllerPos = [&](dual_chunk_view_t* view)
        {
            auto translations = dual::get_owned_ro<skr_translation_comp_t>(view);
            for (uint32_t i = 0; i < view->count; i++)
            {
                controllerPos = translations[i].value;
            }
        };
        dualS_query(storage, &filter, &metaFilter, DUAL_LAMBDA(getControllerPos));
        auto updateCamera = [&](dual_chunk_view_t* view)
        {
            auto translations = dual::get_owned_rw<skr_translation_comp_t>(view);
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
        dualQ_get_views(cameraQuery, DUAL_LAMBDA(updateCamera));
    }
    //TODO: switch to new api
    auto updateTransform = +[](void* u, dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
    {
        auto This = (MPRenderWorld*)u;
        auto rotations = (skr_rotation_comp_t*)dualV_get_owned_ro_local(view, localTypes[0]);
        auto translations = (skr_translation_comp_t*)dualV_get_owned_ro_local(view, localTypes[1]);
        auto scales = (skr_scale_comp_t*)dualV_get_owned_ro_local(view, localTypes[2]);
        auto ghosts = (dual_entity_t*)dualV_get_owned_ro_local(view, localTypes[3]);
        auto r_id = 0;
        auto update = [&](dual_chunk_view_t* view)
        { 
            auto gameRotations = (skr_rotation_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_rotation_comp_t>::get());
            auto gameTranslations = (skr_translation_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_comp_t>::get());
            auto gameScales = (skr_scale_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_scale_comp_t>::get());
            for(int i = 0; i < view->count; ++i)
            {
                auto gameRotation = gameRotations[i].euler.yaw;
                auto gameTranslation = skr::math::load(gameTranslations[i].value);
                auto gameScale = skr::math::load(gameScales[i].value);
                auto rotation = rotations[r_id + i].euler.yaw;
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
        dualS_batch(This->gameWorld->storage, ghosts, view->count, DUAL_LAMBDA(update));
    };
    dualJ_schedule_ecs(transformQuery, 512, updateTransform, this, nullptr, nullptr, nullptr, nullptr);
    dualJ_wait_all();
}