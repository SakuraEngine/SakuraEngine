#pragma once
#include "SkrScene/actor.h"
#include "SkrScene/actor_manager.generated.h"

namespace skr
{

struct [[sattr(guid = "00af1514-7d79-4eb3-a477-40657f11840a"
)]] SKR_SCENE_API ActorManager
{
public:
    static ActorManager& GetInstance()
    {
        static ActorManager instance;
        return instance;
    }
    void BindScene(skr::Scene* scene) { this->scene = scene; }
    void UnBind()
    {
        scene = nullptr;
        world = nullptr;
    }

    template <typename T>
    skr::RCWeak<Actor> CreateActor()
    {
        // General Initialize
        auto actor = CreateActorInstance<T>();
        actor->Initialize();
        actor->BindWorld(world);
        scene->actors.add(actor->guid, actor);
        return actor;
    }
    template <typename T>
    skr::RC<Actor> CreateActorInstance()
    {
        return skr::RC<T>::New();
    }

    skr::RC<Actor> CreateActor(skr::GUID actor_rttr_guid);
    skr::RCWeak<Actor> GetActor(skr::GUID guid);

    bool DestroyActor(skr::GUID guid);
    void CreateActorEntity(skr::RCWeak<Actor> actor);
    void DestroyActorEntity(skr::RCWeak<Actor> actor);
    void UpdateHierarchy(skr::RCWeak<Actor> parent, skr::RCWeak<Actor> child, EAttachRule rule = EAttachRule::Default);

    void ClearAllActors();
    skr::RCWeak<RootActor> GetRoot();

private:
    friend RootActor;
    ActorManager() = default;
    ~ActorManager() = default;
    ActorManager(const ActorManager&) = delete;
    ActorManager& operator=(const ActorManager&) = delete;
    ActorManager(ActorManager&&) = delete;
    ActorManager& operator=(ActorManager&&) = delete;

    skr::ecs::ECSWorld* world = nullptr; // Pointer to the ECS world for actor management
    skr::Scene* scene = nullptr;
};

} // namespace skr