#pragma once
#include "SkrScene/scene.h"
#include "SkrScene/actor_manager.h"

namespace skr
{

Scene::~Scene() SKR_NOEXCEPT
{
    SKR_LOG_INFO(u8"Scene Delete");
}

void Scene::serialize()
{
    for (auto& actor : actors)
    {
        actor.value->serialize();
    }
}

void Scene::deserialize()
{
    for (auto& actor : actors)
    {
        actor.value->deserialize();
    }
}

void Serialize<Scene>::read(ArchiveRead& r, skr::Scene& v)
{
    Archive::ObjectScope obj_scope(r);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // read root actor
    SKR_FAST_CHECK(r.key_value_required(u8"root_actor", v.root_actor_guid), );

    // init world
    auto root = skr::ActorManager::GetInstance().GetRoot();
    root.lock()->InitWorld();

    // read actors
    SKR_FAST_CHECK(r.key_required(u8"actors"), );
    {
        Archive::ArrayScope arr_scope(r);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // read actors count
        uint64_t actors_count = 0;
        SKR_FAST_CHECK(r.array_size(actors_count), );
        v.actors.reserve(actors_count);

        // read each actor
        for (size_t i = 0; i < actors_count; i++)
        {
            Archive::ObjectScope obj_scope_actor(r);
            SKR_FAST_CHECK(obj_scope_actor.is_success(), );

            // read actor guid
            skr::GUID actor_guid;
            SKR_FAST_CHECK(r.key_value_required(u8"guid", actor_guid), );

            // construct it if needed
            skr::RC<Actor> actor_ref;
            if (auto found_actor = v.actors.find(actor_guid))
            {
                actor_ref = found_actor.value();
            }
            else
            {
                // read actor type id
                skr::GUID actor_type_id;
                SKR_FAST_CHECK(r.key_value_required(u8"type_id", actor_type_id), );

                // create actor
                auto actor = skr::ActorManager::GetInstance().CreateActor(
                    actor_type_id
                );
                actor->Initialize(actor_guid);
                v.actors.add(actor_guid, actor);

                actor_ref = actor;
            }

            // read actor data
            SKR_FAST_CHECK(r.key_value_required(u8"data", *actor_ref), );
        }

        // finalize all actors
        for (auto& [k, actor] : v.actors)
        {
            actor->deserialize();
        }
    }
}
void Serialize<Scene>::write(ArchiveWrite& w, const skr::Scene& v)
{
    Archive::ObjectScope obj_scope(w);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    // write root actor
    SKR_FAST_CHECK(w.key_value(u8"root_actor", v.root_actor_guid), );

    // write actors
    SKR_FAST_CHECK(w.key(u8"actors"), );
    {
        Archive::ArrayScope arr_scope(w);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write actors count
        SKR_FAST_CHECK(w.array_size<uint64_t>((uint64_t)v.actors.size()), );

        // write each actor
        for (const auto& [k, actor] : v.actors)
        {
            Archive::ObjectScope obj_scope_actor(w);
            SKR_FAST_CHECK(obj_scope_actor.is_success(), );

            SKR_FAST_CHECK(w.key_value(u8"guid", k), );
            SKR_FAST_CHECK(w.key_value(u8"type_id", actor->GetRTTRTypeGUID()), );
            SKR_FAST_CHECK(w.key_value(u8"data", *actor), );
        }
    }
}
} // namespace skr