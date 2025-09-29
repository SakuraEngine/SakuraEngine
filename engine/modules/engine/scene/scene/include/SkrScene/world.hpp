#pragma once
#include <SkrRTTR/type.hpp>
#include "SkrScene/world.generated.h"

//! World 的概念：
//! - World 是 Actor 的容器，负责 Actor 的生命周期管理
//! - World 销毁 Actor 是延迟的，通常要等待到帧末才进行销毁
//!
//! Actor 的创建：
//! - SpawnActor：创建一个 Actor，并立即调用 FinishSpawn
//! - SpawnActorDeferred + FinishSpawn：分两步创建 Actor，允许在 FinishSpawn 之前对 Actor 做一些配置

// fwd
namespace skr
{
namespace ecs
{
struct ECSWorld;
}
struct TransformSystem;
struct Actor;
} // namespace skr

namespace skr
{
struct [[sattr(
    guid = "193d14de-454e-4150-a7dd-cdc8deead7db"
)]] SKR_SCENE_API World
{
    SKR_DELETE_COPY_MOVE(World);

    // ctor & dtor
    World();
    ~World();

    // init & shutdown
    void Init(skr::task::scheduler_t& scheduler);
    void Shutdown();

    // actor operations
    template <typename T>
    T* SpawnActor(GUID guid = GUID::Create());
    template <typename T>
    T* SpawnActorDeferred(GUID guid = GUID::Create());
    void FinishSpawn(Actor* actor);
    void DestroyActor(Actor* actor);

    // actor lookup & iteration
    Actor* FindActor(GUID guid) const;
    using ActorEachFuncRef = FunctionRef<void(Actor*)>;
    void EachActor(const ActorEachFuncRef& func) const;

    // getter
    inline ecs::ECSWorld* GetECSWorld() const { return _ecs_world.get(); }

    // update step 1. tick
    void Tick(double delta_time);

    // update step 2. sweep actors
    void SweepActors();

    // update step 3. transform system
    void TransformUpdate();
    void TransformWait();

    // one frame work
    void DoOneFrameWork(double delta_time);

private:
    // helper for spawn
    void _SpawnDeferred(Actor* actor, GUID guid);
    void _FinishSpawn(Actor* actor);
    void _DestroyActor(Actor* actor);
    void _SyncECSScheduler();

private:
    UPtr<ecs::ECSWorld> _ecs_world = nullptr;
    UPtr<TransformSystem> _transform_system = nullptr;
    Map<GUID, Actor*> _actors = {};
    Vector<Actor*> _tick_actors = {};
    Vector<Actor*> _pending_destroy_actors = {};
};
} // namespace skr

// impl world
namespace skr
{
// actor operations
template <typename T>
inline T* World::SpawnActor(GUID guid)
{
    auto* actor = SkrNew<T>();
    _SpawnDeferred(actor, guid);
    _FinishSpawn(actor);
    return actor;
}
template <typename T>
inline T* World::SpawnActorDeferred(GUID guid)
{
    auto* actor = SkrNew<T>();
    _SpawnDeferred(actor, guid);
    return actor;
}
inline void World::FinishSpawn(Actor* actor)
{
    _FinishSpawn(actor);
}

// do one frame work
inline void World::DoOneFrameWork(double delta_time)
{
    SkrZoneScopedN("DoWorldFrameWork");
    _SyncECSScheduler();
    Tick(delta_time);
    TransformUpdate(); // 提前发射，以便主线程可以同时进行 Actor 销毁
    SweepActors();
}

} // namespace skr