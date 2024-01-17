#include "MPShared/shared.h"
#include "SkrBase/math/vector.h"
#include "SkrBase/math/quat.h"
#include "SkrScene/scene.h"
#include "MPShared/components.h"
#include "SkrRT/ecs/type_builder.hpp"

#include "SkrBase/math/rtm/quatf.h"
#include "SkrBase/math/rtm/rtmx.h"

#include "SkrProfile/profile.h"

void MPGameWorld::Initialize()
{
    storage = sugoiS_create();
    sugoiQ_make_alias(storage, u8"skr_translation_comp_t", u8"skr_translation_comp_t:move");
    controlQuery.Initialize(storage);
    healthCheckQuery.Initialize(storage);
    fireQuery.Initialize(storage);
    movementQuery.Initialize(storage);
    ballQuery.Initialize(storage);
    ballChildQuery = sugoiQ_from_literal(storage, u8"[in]skr_translation_comp_t@move, [inout]CHealth, [in]CSphereCollider2D");
    sugoiQ_add_child(ballQuery.query, ballChildQuery);
    killBallQuery.Initialize(storage);
    killZombieQuery.Initialize(storage);
    relevanceQuery.Initialize(storage);
    relevanceChildQuery = sugoiQ_from_literal(storage, u8"[in]skr_translation_comp_t, [in]CController");
    sugoiQ_add_child(relevanceQuery.query, relevanceChildQuery);
    zombieAIQuery.Initialize(storage);
    zombieAIChildQuery = sugoiQ_from_literal(storage, u8"[has]CPlayer, [in]skr_translation_comp_t");
    sugoiQ_add_child(zombieAIQuery.query, zombieAIChildQuery);
    gameStateQuery = sugoiQ_from_literal(storage, u8"[inout]CMPGameModeState");
    skr_transform_setup(storage, &transformSystem);
    config = 
    {
        0.5f,
        100,
        50,
        15,
        3,
        5,
        5,
        20,
        20,
        30,
        10
    };
    sugoi::entity_spawner_T<CCollisionScene> sceneSpawner;
    sceneSpawner(storage, 1, [](auto){});
}

void MPGameWorld::Shutdown()
{
    controlQuery.Release();
    movementQuery.Release();
    fireQuery.Release();
    healthCheckQuery.Release();
    ballQuery.Release();
    killBallQuery.Release();
    killZombieQuery.Release();
    relevanceQuery.Release();
    zombieAIQuery.Release();
    sugoiQ_release(ballChildQuery);
    sugoiQ_release(relevanceChildQuery);
    sugoiS_release(storage);
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

void MPGameWorld::SetupCollsionWorld()
{
    
}

static constexpr float deltaTime = (float)serverTickInterval;

void MPGameWorld::ClearDeadBall()
{
    if(authoritative)
    {
        skr::Vector<sugoi_entity_t> ballsToKill;
        ballsToKill.reserve(16);
        auto collectBallsToKill = [&](sugoi_chunk_view_t* view)
        {
            auto balls = sugoi::get_owned_rw<CBall>(view);
            auto entities = (sugoi_entity_t*)sugoiV_get_entities(view);
            for(int i=0; i<view->count; ++i)
            {
                sugoi_chunk_view_t v;
                sugoiS_access(storage, entities[i], &v);
                balls[i].lifeTime -= deltaTime;
                if(balls[i].lifeTime <= 0)
                {
                    ballsToKill.add(entities[i]);
                }
            }
        };
        sugoiQ_get_views(killBallQuery.query, SUGOI_LAMBDA(collectBallsToKill));
        auto killBalls = [&](sugoi_chunk_view_t* view)
        {
            sugoiS_destroy(storage, view);
        };
        sugoiS_batch(storage, ballsToKill.data(), ballsToKill.size(), SUGOI_LAMBDA(killBalls));
    }
}

void MPGameWorld::ClearDeadZombie()
{
    if(authoritative)
    {
        skr::Vector<sugoi_entity_t> zombiesToKill;
        zombiesToKill.reserve(16);
        auto collectBallsToKill = [&](sugoi_chunk_view_t* view)
        {
            auto health = sugoi::get_owned_rw<CHealth>(view);
            auto entities = (sugoi_entity_t*)sugoiV_get_entities(view);
            for(int i=0; i<view->count; ++i)
            {
                sugoi_chunk_view_t v;
                sugoiS_access(storage, entities[i], &v);
                SKR_ASSERT(v.chunk == view->chunk && v.start == i + view->start);
                if(health[i].health <= 0)
                {
                    zombiesToKill.add(entities[i]);
                }
            }
        };
        sugoiQ_get_views(killZombieQuery.query, SUGOI_LAMBDA(collectBallsToKill));
        auto killBalls = [&](sugoi_chunk_view_t* view)
        {
            sugoiS_destroy(storage, view);
        };
        sugoiS_batch(storage, zombiesToKill.data(), zombiesToKill.size(), SUGOI_LAMBDA(killBalls));
    }
}

void MPGameWorld::SpawnZombie()
{
    if(authoritative)
    {
        using spawner_t = sugoi::entity_spawner_T<CZombie, CMovement, CHealth, skr_translation_comp_t, skr_scale_comp_t, skr_rotation_comp_t, CSphereCollider2D, 
        CPrefab, CAuth, CAuthTypeData, CRelevance, sugoi::dirty_comp_t>;
        static spawner_t spawner;
        auto state= sugoi::get_singleton<CMPGameModeState>(gameStateQuery);
        state->zombieSpawnTimer += deltaTime;
        
        while(state->zombieSpawnTimer > config.ZombieWaveInterval)
        {
            state->zombieSpawnTimer -= config.ZombieWaveInterval;
            state->zombiesToSpawn += config.ZombieCountPerWave + state->currentZombieWave * config.AdditionalZombieCountPerWave;
            state->zombieSpawnInterval = config.ZombieSpawnTime / state->zombiesToSpawn;
            state->currentZombieWave++;
        }
        uint32_t zombiesToSpawn = 0;
        state->zombieWaveTimer += deltaTime;
        while(state->zombiesToSpawn > 0 && state->zombieSpawnTimer > state->zombieSpawnInterval)
        {
            state->zombieSpawnTimer -= state->zombieSpawnInterval;
            ++zombiesToSpawn;
            --state->zombiesToSpawn;
        }
        if(zombiesToSpawn > 0)
        {
            spawner(storage, zombiesToSpawn, [&](spawner_t::View view)
            {
                auto [zombies, movements, healths, translations, scales, rotations, 
                    colliders, prefabs, auths, authTypeDatas, relevances, dirties] = view.unpack();
                for(int i=0; i<view.count(); ++i)
                {
                    float angle = (float)rand() / RAND_MAX * 2 * 3.1415926f;
                    float radius = config.ZombieSpawnRadiusMin + (config.ZombieSpawnRadiusMax - config.ZombieSpawnRadiusMin) * (float)rand() / RAND_MAX;
                    translations[i] = {radius * cosf(angle), 0, radius * sinf(angle)};
                    rotations[i] = {0, 0, 0};
                    scales[i] = {4, 4, 4};
                    colliders[i] = {4};
                    prefabs[i].prefab = GetZombiePrefab();
                    relevances[i].mask.flip();
                    zombies[i].speed = config.ZombieSpeed;
                    zombies[i].knockBack = 0;
                    healths[i].maxHealth = healths[i].health = config.ZombieHealth;
                }
            });
        }
    }
}

void MPGameWorld::ZombieAI()
{
    sugoi::schedule_task(zombieAIQuery, 32, [this](QZombieAI::TaskContext ctx)
    {
        auto [translations, movements, dirtyMasks, zombies] = ctx.unpack();
        for(int i=0; i<ctx.count(); ++i)
        {
            if(zombies[i].knockBack > 0)
            {
                zombies[i].knockBack -= deltaTime;
                if(zombies[i].knockBack <= 0)
                {
                    if(dirtyMasks)
                        ctx.set_dirty<CZombie>(dirtyMasks[i]);
                }
                continue;
            }
            auto translation = skr::math::load(skr_float2_t{translations[i].value.x, translations[i].value.z});
            float minDistance = 1000000;
            rtm::vector4f minDistancePlayerPos;
            auto findNearestPlayer = [&](sugoi_chunk_view_t* view)
            {
                auto ptranslations = sugoi::get_owned_ro<skr_translation_comp_t>(view);
                auto phealths = sugoi::get_owned_rw<CHealth>(view);
                auto healthId = sugoiV_get_local_type(view, sugoi_id_of<CHealth>::get());
                auto pdirties = sugoi::get_owned_rw<sugoi::dirty_comp_t>(view);
                // auto entities = (sugoi_entity_t*)sugoiV_get_entities(view);
                for(int j=0; j<view->count; ++j)
                {
                    auto ptranslation = skr::math::load(skr_float2_t{ptranslations[j].value.x, ptranslations[j].value.z});
                    float distance = rtm::vector_distance3(translation, ptranslation);
                    if(distance < minDistance)
                    {
                        minDistance = distance;
                        minDistancePlayerPos = ptranslation;
                    }
                    if(distance < config.ZombieAttackRadius)
                    {
                        phealths[j].health -= config.ZombieDamage * deltaTime;
                        if(pdirties)
                            sugoi_set_bit(&pdirties[j].value, healthId);
                    }
                }
            };
            sugoiQ_get_views(zombieAIChildQuery, SUGOI_LAMBDA(findNearestPlayer));
            
            if(minDistance < 1000 && minDistance > 0.001)
            {
                auto dir = rtm::vector_sub(minDistancePlayerPos, translation);
                dir = rtm::vector_normalize3(dir);
                movements[i].velocity = {rtm::vector_get_x(dir) * zombies[i].speed, rtm::vector_get_y(dir) * zombies[i].speed};
                if(dirtyMasks)
                    ctx.set_dirty<CMovement>(dirtyMasks[i]);
            }
            else
            {
                movements[i].velocity = {0, 0};
                if(dirtyMasks)
                    ctx.set_dirty<CMovement>(dirtyMasks[i]);
            }
        }
    }, nullptr);
}

void MPGameWorld::PlayerControl()
{
    sugoi::schedule_task(controlQuery, 512, [this](QControl::TaskContext ctx)
    {
        for(int i=0; i<ctx.count(); ++i)
        {
            auto [controllers, movements, skills, players, dirtyMasks] = ctx.unpack(i);
            auto& input = this->input.inputs[controllers.playerId];
            bool skillDirty = false;
            if(skills.cooldownTimer > 0)
            {
                skills.cooldownTimer -= deltaTime;
                if(skills.cooldownTimer <= 0)
                {
                    skillDirty = true;
                }
            }
            if(input.skill && skills.cooldownTimer <= 0)
            {
                skills.cooldownTimer = skills.cooldown;
                skills.durationTimer = skills.duration;
                players.speed = skills.speedMultiplier * players.baseSpeed;
                skillDirty = true;
            }
            if(skills.durationTimer > 0)
            {
                skills.durationTimer -= deltaTime;
                if(skills.durationTimer <= 0)
                {
                    skillDirty = true;
                    players.speed = players.baseSpeed;
                }
            }
            skr::math::store(rtm::vector_mul(skr::math::load(input.move), players.speed), movements.velocity);
            if(dirtyMasks)
            {
                ctx.set_dirty<CMovement>(*dirtyMasks);
                if(skillDirty)
                {
                    ctx.set_dirty<CSkill>(*dirtyMasks);
                }
            }
        }
    }, nullptr);
}

void MPGameWorld::PlayerShoot()
{
    //TODO: predict spawn
    if(authoritative)
    {
        using spawner_t = sugoi::entity_spawner_T<CBall, CMovement, skr_translation_comp_t, skr_scale_comp_t, skr_rotation_comp_t, CSphereCollider2D, 
        CPrefab, CAuth, CAuthTypeData, CRelevance, sugoi::dirty_comp_t>;
        static spawner_t spawner;
        auto fire = [&](sugoi_chunk_view_t* view)
        {
            auto weapons = (CWeapon*)sugoi::get_owned_rw<CWeapon>(view);
            auto otranslations = (skr_translation_comp_t*)sugoi::get_owned_ro<skr_translation_comp_t>(view);
            auto orotations = (skr_rotation_comp_t*)sugoi::get_owned_ro<skr_rotation_comp_t>(view);
            auto controllers = (CController*)sugoi::get_owned_ro<CController>(view);
            for(int i=0; i<view->count; ++i)
            {
                auto& input = this->input.inputs[controllers[i].playerId];
                if(input.fire)
                {
                    if(weapons[i].fireTimer < 0)
                    {
                        weapons[i].fireTimer = weapons[i].fireRate;
                        spawner(storage, 1, [&](spawner_t::View view)
                        {
                            auto [balls, movements, translations, scales, 
                                rotations, spheres, prefabs, 
                                auths, authTypes, relevances, dirties] = view.unpack();
                            for(int j=0; j<view.count(); ++j)
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
                                prefabs[i].prefab = GetBulletPrefab();
                            }
                        });
                    }
                }
                weapons[i].fireTimer -= deltaTime;
            }
        };
        sugoiQ_get_views(fireQuery.query, SUGOI_LAMBDA(fire));
    }
}
void MPGameWorld::PlayerHealthCheck()
{
    if(authoritative)
    {
        sugoi::schedule_task(healthCheckQuery, 512,
        [](QHeathCheck::TaskContext ctx)
        {
            auto [healths, translations, rotations, weapons, dirtyMasks] = ctx.unpack();
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
                        ctx.set_dirty<CHealth>(dirtyMasks[i]);
                        ctx.set_dirty<skr_translation_comp_t>(dirtyMasks[i]);
                        ctx.set_dirty<skr_rotation_comp_t>(dirtyMasks[i]);
                    }
                }
            }
        }, nullptr);
    }
}
void MPGameWorld::PlayerMovement()
{
    sugoi::schedule_task(movementQuery, 512, [](QMovement::TaskContext ctx)
    {
        auto [movements, translations, rotations, dirtyMasks] = ctx.unpack();
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
                ctx.set_dirty<skr_translation_comp_t>(dirtyMasks[i]);
                ctx.set_dirty<skr_rotation_comp_t>(dirtyMasks[i]);
            }
        }
    }, nullptr);
}
void MPGameWorld::BulletMovement()
{
        sugoi::schedule_task(ballQuery, 512, 
        [this](QBallMovement::TaskContext ctx)
        {
            auto [translations, colliders, movements, dirtyMasks, balls] = ctx.unpack();
            for(int i=0; i<ctx.count(); ++i)
            {
                translations[i].value.x += movements[i].velocity.x * deltaTime;
                translations[i].value.z += movements[i].velocity.y * deltaTime;
                if(authoritative)
                {
                    if(dirtyMasks)
                        ctx.set_dirty<skr_translation_comp_t>(dirtyMasks[i]);
                    auto& translation = translations[i];
                    auto& collider = colliders[i];
                    auto& ball = balls[i];
                    auto checkAndSet = [&](sugoi_chunk_view_t* view)
                    {
                        auto otherTranslations = sugoi::get_owned_ro<skr_translation_comp_t>(view);
                        auto otherColliders = sugoi::get_owned_ro<CSphereCollider2D>(view);
                        auto otherHealths = sugoi::get_owned_rw<CHealth>(view);
                        auto otherControllers = sugoi::get_owned_ro<CController>(view);
                        auto otherDirtyMasks = sugoi::get_owned_rw<sugoi::dirty_comp_t>(view);
                        auto healthId = sugoiV_get_local_type(view, sugoi_id_of<CHealth>::get());
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
                                        sugoi_set_bit(&otherDirtyMasks[j].value, healthId);
                                    }
                                }
                            }
                        }
                    };
                    sugoiQ_get_views(ballChildQuery, SUGOI_LAMBDA(checkAndSet));
                }
            }
        }, nullptr);
}
void MPGameWorld::RelevenceUpdate()
{
    sugoi::schedule_task(relevanceQuery, 512,
    [this](QUpdateRelevance::TaskContext ctx)
    {
        auto [relevances, translations] = ctx.unpack();
        for(int i=0; i<ctx.count(); ++i)
        {
            auto& relevance = relevances[i];
            auto& translation = translations[i];
            auto checkAndSet = [&](sugoi_chunk_view_t* view)
            {
                auto otherTranslations = sugoi::get_owned_ro<skr_translation_comp_t>(view);
                auto otherControllers = sugoi::get_owned_ro<CController>(view);
                for(int j=0; j<view->count; ++j)
                {
                    float distance = rtm::vector_distance3(skr::math::load(translation.value), skr::math::load(otherTranslations[j].value));
                    if(distance < 250.f)
                        relevance.mask[otherControllers[j].connectionId] = true;
                    else
                        relevance.mask[otherControllers[j].connectionId] = false;
                }
            };
            sugoiQ_get_views(relevanceChildQuery, SUGOI_LAMBDA(checkAndSet));
        }
    }, nullptr);
}

void MPGameWorld::Tick(const MPInputFrame &inInput)
{
    SkrZoneScopedN("MP Tick");
    input = inInput;
    SpawnZombie();
    ClearDeadBall();
    ClearDeadZombie();
    ZombieAI();
    PlayerControl();
    PlayerShoot();
    PlayerHealthCheck();
    PlayerMovement();
    BulletMovement();
    RelevenceUpdate();
    skr_transform_update(&transformSystem);
    sugoiJ_wait_all();
}