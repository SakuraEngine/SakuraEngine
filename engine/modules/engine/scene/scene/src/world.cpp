#include <SkrScene/world.hpp>
#include <SkrScene/transform_system.hpp>
#include <SkrScene/actor.hpp>

namespace skr
{
// ctor & dtor
World::World()
{
}
World::~World()
{
}

// init & shutdown
void World::Init(skr::task::scheduler_t& scheduler)
{
    // create ecs world
    _ecs_world = UPtr<ecs::ECSWorld>::New();
    _ecs_world->bind_scheduler(scheduler);
    _ecs_world->initialize();

    // create transform system
    _transform_system = TransformSystem::Create(_ecs_world.get());
}
void World::Shutdown()
{
    // destroy all actors
    for (auto& [_, actor] : _actors)
    {
        _DestroyActor(actor);

        // add to pending destroy list
        _pending_destroy_actors.push_back(actor);
    }
    _actors.clear();
    _tick_actors.clear();

    // sweep actors
    SweepActors();

    // destroy transform system
    _transform_system.reset();

    // reset ecs
    _ecs_world->finalize();
    _ecs_world.reset();
}

// actor operations
void World::DestroyActor(Actor* actor)
{
    SKR_ASSERT(_actors.contains(actor->GetGUID()) && "cannot destroy an actor that not in world");

    _DestroyActor(actor);

    // remove from all lists
    _actors.remove(actor->GetGUID());
    if (!actor->IsNeverTick()) 
        _tick_actors.remove(actor);

    // add to pending destroy list
    _pending_destroy_actors.push_back(actor);
}

// actor lookup & iteration
Actor* World::FindActor(GUID guid) const
{
    return _actors.find(guid).value_or(nullptr);
}
void World::EachActor(const ActorEachFuncRef& func) const
{
    for (auto& [_, actor] : _actors)
    {
        func(actor);
    }
}

// tick
void World::Tick(double delta_time)
{
    for (auto* actor : _tick_actors)
    {
        actor->OnTick(delta_time);
    }
}

// sweep actors
void World::SweepActors()
{
    for (auto* actor : _pending_destroy_actors)
    {
        // call begin destroy
        actor->BeginDestroy();

        // delete it
        SkrDelete(actor);
    }
    _pending_destroy_actors.clear();
}

// transform system
void World::TransformUpdate()
{
    _transform_system->update();
}
void World::TransformWait()
{
    _transform_system->wait();
}

// helper for spawn
void World::_SpawnDeferred(Actor* actor, GUID guid)
{
    // initialize
    actor->_Initialize(guid, this);
    actor->OnInitialize();

    // create entity
    struct Spawner
    {
        Actor* actor;

        void build(ecs::ArchetypeBuilder& builder)
        {
            actor->CreateDefaultComponents(builder);
        }
        void run(ecs::TaskContext& ctx)
        {
            actor->CreateDefaultEntity(ctx);
        }
    };
    Spawner spawner{ .actor = actor };
    GetECSWorld()->create_entities(spawner, 1);
}
void World::_FinishSpawn(Actor* actor)
{
    // register
    _actors.add(actor->GetGUID(), actor);

    // call begin play
    actor->BeginPlay();

    // add to tick list
    if (!actor->IsNeverTick()) _tick_actors.add(actor);
}
void World::_DestroyActor(Actor* actor)
{
    // call end play
    actor->EndPlay();

    // clean up hierarchy
    actor->Detach();
    for (auto& child : actor->GetChildren())
    {
        DestroyActor(child);
    }
    SKR_ASSERT(actor->GetParent() == nullptr);
    SKR_ASSERT(actor->GetChildren().is_empty());

    // clean up ecs
    actor->_DestroyEntity();
}
void World::_SyncECSScheduler()
{
    ecs::TaskScheduler::Get()->sync_all();
}
} // namespace skr