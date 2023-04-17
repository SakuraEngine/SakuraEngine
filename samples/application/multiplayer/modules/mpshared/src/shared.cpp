#include "MPShared/shared.h"
#include "math/vector.h"
#include "math/quat.h"
#include "SkrScene/scene.h"
#include "MPShared/components.h"
#include "ecs/type_builder.hpp"
#include "platform/atomic.h"
#include "utils/make_zeroed.hpp"
#include "ecs/callback.hpp"
#include "rtm/quatf.h"
#include "rtm/rtmx.h"

void MPGameWorld::Initialize()
{
    storage = dualS_create();
    controlQuery = dualQ_from_literal(storage, "[in]CController, [inout]CMovement, [inout]CSkill, [atomic]?dirty");
    healthCheckQuery = dualQ_from_literal(storage, "[inout]CHealth, [inout]skr_translation_comp_t, [inout]skr_rotation_comp_t, [inout]CWeapon, [atomic]?dirty");
    fireQuery = dualQ_from_literal(storage, "[in]CController, [in]skr_translation_comp_t, [in]skr_rotation_comp_t, [inout]CWeapon, [atomic]?dirty");
    movementQuery = dualQ_from_literal(storage, "[in]CMovement, [inout]skr_translation_comp_t', [inout]skr_rotation_comp_t, [atomic]?dirty");
    ballQuery = dualQ_from_literal(storage, "[inout]<rand>skr_translation_comp_t', [in]<rand>CSphereCollider2D, [inout]CMovement, [atomic]?dirty, [inout]CBall, [has]skr_rotation_comp_t, [inout]<rand>?CHealth");
    ballChildQuery = dualQ_from_literal(storage, "[in]skr_translation_comp_t', [inout]CHealth, [in]CSphereCollider2D");
    killQuery = dualQ_from_literal(storage, "[inout]CBall");
    relevanceQuery = dualQ_from_literal(storage, "[inout]CRelevance, [in]<rand>skr_translation_comp_t, [in]<rand>?CController");
    relevanceChildQuery = dualQ_from_literal(storage, "[in]skr_translation_comp_t, [in]CController");
    skr_transform_setup(storage, &transformSystem);
}

void MPGameWorld::Shutdown()
{
    dualQ_release(controlQuery);
    dualQ_release(movementQuery);
    dualS_release(storage);
}

void mark_dirty_atomic(uint32_t& mask, int32_t bit)
{
    //CAS
    uint32_t oldMask = mask;
    uint32_t newMask = oldMask | (1 << bit);
    while(!skr_atomicu32_cas_relaxed(&mask, oldMask, newMask))
    {
        oldMask = mask;
        newMask = oldMask | (1 << bit);
    }
}

void mark_dirty_atomic(uint32_t& mask, std::initializer_list<uint32_t> list)
{
    //CAS
    uint32_t oldMask = mask;
    uint32_t newMask = oldMask;
    for(auto bit : list)
    {
        newMask |= (1 << bit);
    }
    while(!skr_atomicu32_cas_relaxed(&mask, oldMask, newMask))
    {
        oldMask = mask;
        newMask = oldMask;
        for(auto bit : list)
        {
            newMask |= (1 << bit);
        }
    }
}

bool collide(const skr_float3_t& a, const CSphereCollider2D& aBox, const skr_float3_t& b, const CSphereCollider2D& bBox, skr_float2_t& outNormal)
{
    auto aPos = skr::math::load(skr_float2_t{a.x, a.z});
    auto bPos = skr::math::load(skr_float2_t{b.x, b.z});
    
    auto offset = rtm::vector_sub(aPos, bPos);
    float distance = rtm::vector_length(offset);
    auto radius = aBox.radius + bBox.radius;
    if(distance < radius)
    {
        auto normal = rtm::vector_div(offset, rtm::vector_set(distance));
        skr::math::store(normal, outNormal);
        return true;
    }

    return false;
}

void MPGameWorld::Tick(const MPInputFrame &inInput)
{
    ZoneScopedN("MP Tick");
    static constexpr float deltaTime = (float)serverTickInterval;
    input = inInput;

    if(authoritative)
    {
        skr::vector<dual_entity_t> ballsToKill;
        ballsToKill.reserve(16);
        auto collectBallsToKill = [&](dual_chunk_view_t* view)
        {
            auto balls = (CBall*)dualV_get_owned_ro(view, dual_id_of<CBall>::get());
            auto entities = (dual_entity_t*)dualV_get_entities(view);
            for(int i=0; i<view->count; ++i)
            {
                dual_chunk_view_t v;
                dualS_access(storage, entities[i], &v);
                SKR_ASSERT(v.chunk == view->chunk && v.start == i + view->start);
                balls[i].lifeTime -= deltaTime;
                if(balls[i].lifeTime <= 0)
                {
                    ballsToKill.push_back(entities[i]);
                }
            }
        };
        dualQ_get_views(killQuery, DUAL_LAMBDA(collectBallsToKill));
        auto killBalls = [&](dual_chunk_view_t* view)
        {
            dualS_destroy(storage, view);
        };
        dualS_batch(storage, ballsToKill.data(), ballsToKill.size(), DUAL_LAMBDA(killBalls));
    }

    dualJ_schedule_ecs(controlQuery, 512, 
    +[](void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
    {
        auto This = (MPGameWorld*)u;
        auto controllers = (CController*)dualV_get_owned_rw_local(view, localTypes[0]);
        auto movements = (CMovement*)dualV_get_owned_rw_local(view, localTypes[1]);
        auto skills = (CSkill*)dualV_get_owned_rw_local(view, localTypes[2]);
        auto dirtyMasks = (uint32_t*)dualV_get_owned_ro_local(view, localTypes[3]);
        for(int i=0; i<view->count; ++i)
        {
            auto& input = This->input.inputs[controllers[i].playerId];
            bool skillDirty = false;
            if(skills[i].cooldownTimer > 0)
            {
                skills[i].cooldownTimer -= deltaTime;
                if(skills[i].cooldownTimer <= 0)
                {
                    skillDirty = true;
                }
            }
            if(input.skill && skills[i].cooldownTimer <= 0)
            {
                skills[i].cooldownTimer = skills[i].cooldown;
                skills[i].durationTimer = skills[i].duration;
                movements[i].speed = skills[i].speedMultiplier * movements[i].baseSpeed;
                skillDirty = true;
            }
            if(skills[i].durationTimer > 0)
            {
                skills[i].durationTimer -= deltaTime;
                if(skills[i].durationTimer <= 0)
                {
                    skillDirty = true;
                    movements[i].speed = movements[i].baseSpeed;
                }
            }
            skr::math::store(rtm::vector_mul(skr::math::load(input.move), movements[i].speed), movements[i].velocity);
            if(dirtyMasks)
            {
                mark_dirty_atomic(dirtyMasks[i], localTypes[1]);
                if(skillDirty)
                {
                    mark_dirty_atomic(dirtyMasks[i], localTypes[2]);
                }
            }
        }
        
    }, this, nullptr, nullptr, nullptr, nullptr);

    //TODO: predict spawn
    if(authoritative)
    {
        dual::type_builder_t builder;
        builder.with<CBall, CMovement, skr_translation_comp_t, skr_scale_comp_t, skr_rotation_comp_t, CSphereCollider2D, 
        CPrefab, CAuth, CAuthTypeData, CRelevance>().with(DUAL_COMPONENT_DIRTY);
        dual_entity_type_t ballType = make_zeroed<dual_entity_type_t>();
        ballType.type = builder.build();
        auto fire = [&](dual_chunk_view_t* view)
        {
            auto weapons = (CWeapon*)dual::get_owned_rw<CWeapon>(view);
            auto otranslations = (skr_translation_comp_t*)dual::get_owned_ro<skr_translation_comp_t>(view);
            auto orotations = (skr_rotation_comp_t*)dual::get_owned_ro<skr_rotation_comp_t>(view);
            auto controllers = (CController*)dual::get_owned_ro<CController>(view);
            for(int i=0; i<view->count; ++i)
            {
                auto& input = this->input.inputs[controllers[i].playerId];
                if(input.fire)
                {
                    if(weapons[i].fireTimer < 0)
                    {
                        weapons[i].fireTimer = weapons[i].fireRate;
                        auto ballSetup = [&](dual_chunk_view_t* view)
                        {
                            auto translations = (skr_translation_comp_t*)dualV_get_owned_rw(view, dual_id_of<skr_translation_comp_t>::get());
                            auto scales = (skr_scale_comp_t*)dualV_get_owned_rw(view, dual_id_of<skr_scale_comp_t>::get());
                            auto rotations = (skr_rotation_comp_t*)dualV_get_owned_rw(view, dual_id_of<skr_rotation_comp_t>::get());
                            auto movements = (CMovement*)dualV_get_owned_rw(view, dual_id_of<CMovement>::get());
                            auto spheres = (CSphereCollider2D*)dualV_get_owned_rw(view, dual_id_of<CSphereCollider2D>::get());
                            auto balls = (CBall*)dualV_get_owned_rw(view, dual_id_of<CBall>::get());
                            
                            for(int j=0; j<view->count; ++j)
                            {
                                auto rot = skr::math::load(orotations[i].euler);
                                auto dir = skr::math::load(skr_float3_t{1, 0, 0});
                                dir = rtm::quat_mul_vector3(dir, rot);
                                skr::math::store(rtm::vector_add(rtm::vector_mul(dir, 5), skr::math::load(otranslations[i].value)), translations[j].value);
                                scales[j].value = skr_float3_t{2, 1, 2};
                                rotations[j].euler = orotations[i].euler;
                                //random direction
                                //TODO: use seeded random in shared code
                                movements[j].speed = 40;
                                movements[j].velocity = skr_float2_t{rtm::vector_get_x(dir) * movements[j].speed, rtm::vector_get_z(dir) * movements[j].speed};
                                spheres[j].radius = 1;
                                balls[j].lifeTime = 10;
                                balls[j].playerId = controllers[i].serverPlayerId;
                            }
                        };

                        dualS_allocate_type(storage, &ballType, 1, DUAL_LAMBDA(ballSetup));
                    }
                }
                weapons[i].fireTimer -= deltaTime;
            }
        };
        dualQ_get_views(fireQuery, DUAL_LAMBDA(fire));
    }
    if(authoritative)
    {

    }
    dualJ_schedule_ecs(healthCheckQuery, 512,
    +[](void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
    {
        auto healths = (CHealth*)dualV_get_owned_rw_local(view, localTypes[0]);
        auto translations = (skr_translation_comp_t*)dualV_get_owned_rw_local(view, localTypes[1]);
        auto rotations = (skr_rotation_comp_t*)dualV_get_owned_rw_local(view, localTypes[2]);
        auto weapons = (CWeapon*)dualV_get_owned_rw_local(view, localTypes[3]);
        auto dirtyMasks = (uint32_t*)dualV_get_owned_ro_local(view, localTypes[4]);
        for(int i=0; i<view->count; ++i)
        {
            if(healths[i].health <= 0)
            {
                healths[i].health = healths[i].maxHealth;
                translations[i].value = skr_float3_t{0, 0, 0};
                rotations[i].euler = skr_rotator_t{0, 0, 0};
                weapons[i].fireTimer = 5;
                if(dirtyMasks)
                {
                    mark_dirty_atomic(dirtyMasks[i], localTypes[0]);
                    mark_dirty_atomic(dirtyMasks[i], localTypes[1]);
                    mark_dirty_atomic(dirtyMasks[i], localTypes[2]);
                }
            }
        }
    }, this, nullptr, nullptr, nullptr, nullptr);
    dualJ_schedule_ecs(movementQuery, 512, 
    +[](void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
    {
        auto movements = (CMovement*)dualV_get_owned_rw_local(view, localTypes[0]);
        auto translations = (skr_float3_t*)dualV_get_owned_rw_local(view, localTypes[1]);
        auto rotations = (skr_rotator_t*)dualV_get_owned_rw_local(view, localTypes[2]);
        auto dirtyMasks = (uint32_t*)dualV_get_owned_ro_local(view, localTypes[3]);
        for(int i=0; i<view->count; ++i)
        {
            translations[i].x += movements[i].velocity.x * deltaTime;
            translations[i].z += movements[i].velocity.y * deltaTime;
            
            if(std::abs(movements[i].velocity.x) > 0.001f || std::abs(movements[i].velocity.y) > 0.001f)
            {
                //get yaw from velocity
                float targetYaw = atan2(movements[i].velocity.y, movements[i].velocity.x) * 180.f / 3.14159265358979323846f;
                //slerp angle
                float delta = targetYaw - rotations[i].yaw;
                if(delta > 180.f)
                    delta -= 360.f;
                else if(delta < -180.f)
                    delta += 360.f;
                rotations[i].yaw += delta * 0.4f;
                if(rotations[i].yaw > 180.f)
                    rotations[i].yaw -= 360.f;
                else if(rotations[i].yaw < -180.f)
                    rotations[i].yaw += 360.f;
            }

            if(dirtyMasks)
            {
                mark_dirty_atomic(dirtyMasks[i], localTypes[1]);
                mark_dirty_atomic(dirtyMasks[i], localTypes[2]);
            }
        }
        
    }, this, nullptr, nullptr, nullptr, nullptr);

    if(authoritative)
    {
        dualJ_schedule_ecs(ballQuery, 512, 
        +[](void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
        {
            auto This = (MPGameWorld*)u;
            auto translations = (skr_float3_t*)dualV_get_owned_rw_local(view, localTypes[0]);
            auto colliders = (CSphereCollider2D*)dualV_get_owned_rw_local(view, localTypes[1]);
            auto dirtyMasks = (uint32_t*)dualV_get_owned_ro_local(view, localTypes[3]);
            auto movements = (CMovement*)dualV_get_owned_rw_local(view, localTypes[2]);
            auto balls = (CBall*)dualV_get_owned_rw_local(view, localTypes[4]);
            for(int i=0; i<view->count; ++i)
            {
                translations[i].x += movements[i].velocity.x * deltaTime;
                translations[i].z += movements[i].velocity.y * deltaTime;
                if(dirtyMasks)
                    mark_dirty_atomic(dirtyMasks[i], localTypes[0]);
                auto checkAndSet = [&](dual_chunk_view_t* view)
                {
                    auto otherTranslations = (skr_float3_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_comp_t>::get());
                    auto otherColliders = (CSphereCollider2D*)dualV_get_owned_ro(view, dual_id_of<CSphereCollider2D>::get());
                    auto otherHealths = (CHealth*)dualV_get_owned_ro(view, dual_id_of<CHealth>::get());
                    auto otherControllers = (CController*)dualV_get_owned_ro(view, dual_id_of<CController>::get());
                    auto otherDirtyMasks = (uint32_t*)dualV_get_owned_ro(view, DUAL_COMPONENT_DIRTY);
                    auto healthId = dualV_get_local_type(view, dual_id_of<CHealth>::get());
                    for(int j=0; j<view->count; ++j)
                    {
                        auto& translation = translations[i];
                        auto& collider = colliders[i];
                        auto& otherTranslation = otherTranslations[j];
                        auto& otherCollider = otherColliders[j];
                        skr_float2_t normal;

                        if(collide(translation, collider, otherTranslation, otherCollider, normal))
                        {
                            if(!otherControllers || otherControllers[j].serverPlayerId != balls[i].playerId)
                            {
                                auto& health = otherHealths[j];
                                balls[i].lifeTime = 0;
                                health.health -= 1;
                                if(otherDirtyMasks)
                                {
                                    mark_dirty_atomic(otherDirtyMasks[j], healthId);
                                }
                            }
                        }
                    }
                };
                dualQ_get_views(This->ballChildQuery, DUAL_LAMBDA(checkAndSet));
            }
        }, this, nullptr, nullptr, nullptr, nullptr);
    }

    dualJ_schedule_ecs(relevanceQuery, 512,
    +[] (void* u, dual_storage_t* storage, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
    {
        auto This = (MPGameWorld*)u;
        auto relevances = (CRelevance*)dualV_get_owned_rw_local(view, localTypes[0]);
        auto translations = (skr_float3_t*)dualV_get_owned_ro_local(view, localTypes[1]);
        for(int i=0; i<view->count; ++i)
        {
            auto checkAndSet = [&](dual_chunk_view_t* view)
            {
                auto otherTranslations = (skr_float3_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_comp_t>::get());
                auto otherControllers = (CController*)dualV_get_owned_ro(view, dual_id_of<CController>::get());
                for(int j=0; j<view->count; ++j)
                {
                    float distance = rtm::vector_distance3(skr::math::load(translations[i]), skr::math::load(otherTranslations[j]));
                    if(distance < 200.f)
                        relevances[i].mask[otherControllers[j].connectionId] = true;
                    else
                        relevances[i].mask[otherControllers[j].connectionId] = false;
                }
            };
            dualQ_get_views(This->relevanceChildQuery, DUAL_LAMBDA(checkAndSet));
        }
    }, this, nullptr, nullptr, nullptr, nullptr);
    skr_transform_update(&transformSystem);
    dualJ_wait_all();
}