#include "MPShared/shared.h"
#include "math/vector.h"
#include "math/quat.h"
#include "SkrScene/scene.h"
#include "MPShared/components.h"
#include "ecs/type_builder.hpp"
#include "platform/atomic.h"
#include "utils/make_zeroed.hpp"

#include "rtm/quatf.h"
#include "rtm/rtmx.h"
#include "utils/log.h"

void MPGameWorld::Initialize()
{
    storage = dualS_create();
    controlQuery.Initialize(storage);
    healthCheckQuery.Initialize(storage);
    fireQuery.Initialize(storage);
    movementQuery.Initialize(storage);
    ballQuery.Initialize(storage);
    ballChildQuery = dualQ_from_literal(storage, "[in]skr_translation_comp_t', [inout]CHealth, [in]CSphereCollider2D");
    killQuery.Initialize(storage);
    relevanceQuery.Initialize(storage);
    relevanceChildQuery = dualQ_from_literal(storage, "[in]skr_translation_comp_t, [in]CController");
    skr_transform_setup(storage, &transformSystem);
}

void MPGameWorld::Shutdown()
{
    controlQuery.Release();
    movementQuery.Release();
    fireQuery.Release();
    healthCheckQuery.Release();
    ballQuery.Release();
    killQuery.Release();
    relevanceQuery.Release();
    dualQ_release(ballChildQuery);
    dualQ_release(relevanceChildQuery);
    dualS_release(storage);
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
            auto balls = dual::get_owned_ro<CBall>(view);
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
        dualQ_get_views(killQuery.query, DUAL_LAMBDA(collectBallsToKill));
        auto killBalls = [&](dual_chunk_view_t* view)
        {
            dualS_destroy(storage, view);
        };
        dualS_batch(storage, ballsToKill.data(), ballsToKill.size(), DUAL_LAMBDA(killBalls));
    }

    dual::schedule_task(controlQuery, 512, [this](QControl::TaskContext ctx)
    {
        auto [controllers, movements, skills, players, dirtyMasks] = ctx.Unpack();
        for(int i=0; i<ctx.count(); ++i)
        {
            auto& input = this->input.inputs[controllers[i].playerId];
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
                players[i].speed = skills[i].speedMultiplier * players[i].baseSpeed;
                skillDirty = true;
            }
            if(skills[i].durationTimer > 0)
            {
                skills[i].durationTimer -= deltaTime;
                if(skills[i].durationTimer <= 0)
                {
                    skillDirty = true;
                    players[i].speed = players[i].baseSpeed;
                }
            }
            skr::math::store(rtm::vector_mul(skr::math::load(input.move), players[i].speed), movements[i].velocity);
            if(dirtyMasks)
            {
                ctx.set_dirty(dirtyMasks[i], 1);
                if(skillDirty)
                {
                    ctx.set_dirty(dirtyMasks[i], 2);
                }
            }
        }
    }, nullptr);

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
                            auto translations = dual::get_owned_rw<skr_translation_comp_t>(view);
                            auto scales = dual::get_owned_rw<skr_scale_comp_t>(view);
                            auto rotations = dual::get_owned_rw<skr_rotation_comp_t>(view);
                            auto movements = dual::get_owned_rw<CMovement>(view);
                            auto spheres = dual::get_owned_rw<CSphereCollider2D>(view);
                            auto balls = dual::get_owned_rw<CBall>(view);
                            
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
                                float speed = 40;
                                movements[j].velocity = skr_float2_t{rtm::vector_get_x(dir) * speed, rtm::vector_get_z(dir) * speed};
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
        dualQ_get_views(fireQuery.query, DUAL_LAMBDA(fire));
    }
    if(authoritative)
    {
        dual::schedule_task(healthCheckQuery, 512,
        [this](QHeathCheck::TaskContext ctx)
        {
            auto [healths, translations, rotations, weapons, dirtyMasks] = ctx.Unpack();
            for(int i=0; i<ctx.count(); ++i)
            {
                if(healths[i].health <= 0)
                {
                    healths[i].health = healths[i].maxHealth;
                    translations[i].value = skr_float3_t{0, 0, 0};
                    rotations[i].euler = skr_rotator_t{0, 0, 0};
                    weapons[i].fireTimer = 5;
                    if(dirtyMasks)
                    {
                        ctx.set_dirty(dirtyMasks[i], 0);
                        ctx.set_dirty(dirtyMasks[i], 1);
                        ctx.set_dirty(dirtyMasks[i], 2);
                    }
                }
            }
        }, nullptr);
    }
    dual::schedule_task(movementQuery, 512, [](QMovement::TaskContext ctx)
    {
        auto [movements, translations, rotations, dirtyMasks] = ctx.Unpack();
        for(int i=0; i<ctx.count(); ++i)
        {
            translations[i].value.x += movements[i].velocity.x * deltaTime;
            translations[i].value.z += movements[i].velocity.y * deltaTime;
            
            if(std::abs(movements[i].velocity.x) > 0.001f || std::abs(movements[i].velocity.y) > 0.001f)
            {
                //get yaw from velocity
                float targetYaw = atan2(movements[i].velocity.y, movements[i].velocity.x) * 180.f / 3.14159265358979323846f;
                //slerp angle
                float delta = targetYaw - rotations[i].euler.yaw;
                if(delta > 180.f)
                    delta -= 360.f;
                else if(delta < -180.f)
                    delta += 360.f;
                rotations[i].euler.yaw += delta * 0.4f;
                if(rotations[i].euler.yaw > 180.f)
                    rotations[i].euler.yaw -= 360.f;
                else if(rotations[i].euler.yaw < -180.f)
                    rotations[i].euler.yaw += 360.f;
            }

            if(dirtyMasks)
            {
                ctx.set_dirty(dirtyMasks[i], 1);
                ctx.set_dirty(dirtyMasks[i], 2);
            }
        }
    }, nullptr);

    if(authoritative)
    {
        dual::schedule_task(ballQuery, 512, 
        [this](QBallMovement::TaskContext ctx)
        {
            auto [translations, colliders, movements, dirtyMasks, balls] = ctx.Unpack();
            for(int i=0; i<ctx.count(); ++i)
            {
                translations[i].value.x += movements[i].velocity.x * deltaTime;
                translations[i].value.z += movements[i].velocity.y * deltaTime;
                if(dirtyMasks)
                    ctx.set_dirty(dirtyMasks[i], 0);
                auto& translation = translations[i];
                auto& collider = colliders[i];
                auto& ball = balls[i];
                auto checkAndSet = [&](dual_chunk_view_t* view)
                {
                    auto otherTranslations = dual::get_owned_ro<skr_translation_comp_t>(view);
                    auto otherColliders = dual::get_owned_ro<CSphereCollider2D>(view);
                    auto otherHealths = dual::get_owned_ro<CHealth>(view);
                    auto otherControllers = dual::get_owned_ro<CController>(view);
                    auto otherDirtyMasks = dual::get_owned_rw<dual::dirty_comp_t>(view);
                    auto healthId = dualV_get_local_type(view, dual_id_of<CHealth>::get());
                    for(int j=0; j<view->count; ++j)
                    {
                        auto& otherTranslation = otherTranslations[j];
                        auto& otherCollider = otherColliders[j];
                        skr_float2_t normal;

                        if(collide(translation.value, collider, otherTranslation.value, otherCollider, normal))
                        {
                            if(!otherControllers || otherControllers[j].serverPlayerId != ball.playerId)
                            {
                                auto& health = otherHealths[j];
                                ball.lifeTime = 0;
                                health.health -= 1;
                                if(otherDirtyMasks)
                                {
                                    dual_set_bit(&otherDirtyMasks[j].value, healthId);
                                }
                            }
                        }
                    }
                };
                dualQ_get_views(ballChildQuery, DUAL_LAMBDA(checkAndSet));
            }
        }, nullptr);
    }

    dual::schedule_task(relevanceQuery, 512,
    [this](QUpdateRelevance::TaskContext ctx)
    {
        auto [relevances, translations] = ctx.Unpack();
        for(int i=0; i<ctx.count(); ++i)
        {
            auto& relevance = relevances[i];
            auto& translation = translations[i];
            auto checkAndSet = [&](dual_chunk_view_t* view)
            {
                auto otherTranslations = dual::get_owned_ro<skr_translation_comp_t>(view);
                auto otherControllers = dual::get_owned_ro<CController>(view);
                for(int j=0; j<view->count; ++j)
                {
                    float distance = rtm::vector_distance3(skr::math::load(translation.value), skr::math::load(otherTranslations[j].value));
                    if(distance < 200.f)
                        relevance.mask[otherControllers[j].connectionId] = true;
                    else
                        relevance.mask[otherControllers[j].connectionId] = false;
                }
            };
            dualQ_get_views(relevanceChildQuery, DUAL_LAMBDA(checkAndSet));
        }
    }, nullptr);
    skr_transform_update(&transformSystem);
    dualJ_wait_all();
}