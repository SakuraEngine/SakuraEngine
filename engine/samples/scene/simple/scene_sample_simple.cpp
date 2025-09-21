// The SceneSample_Simple: The Principle for Scene Transform Update task
#include "SkrBase/math/gen/misc/transform.hpp"
#include <SkrCore/module/module_manager.hpp>
#include <SkrCore/log.h>

#include <SkrScene/actor.h>
#include <SkrScene/actor_manager.h>
#include <SkrSceneCore/scene_components.h>
#include <SkrSceneCore/transform_system.h>
#include <SkrRuntime/ecs/world.hpp>

struct SceneSampleSimpleModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    skr::TransformSystem* transform_system = nullptr;
    skr::task::scheduler_t scheduler;
    skr::Scene scene;
    skr::ecs::ECSWorld* world = nullptr;
    skr::ActorManager& actor_manager = skr::ActorManager::GetInstance();
};

IMPLEMENT_DYNAMIC_MODULE(SceneSampleSimpleModule, SceneSample_Simple);
void SceneSampleSimpleModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"Scene Sample Simple Module Loaded");
    scheduler.initialize(skr::task::scheudler_config_t());
    scheduler.bind();
    scene.root_actor_guid = skr::GUID::Create();
    actor_manager.BindScene(&scene);
    auto root = actor_manager.GetRoot();
    root.lock()->InitWorld();
    world = root.lock()->GetWorld();
    world->bind_scheduler(scheduler);
    world->initialize();
    transform_system = skr_transform_system_create(world);
}

void SceneSampleSimpleModule::on_unload()
{
    skr_transform_system_destroy(transform_system);
    actor_manager.ClearAllActors();
    scheduler.unbind();
    SKR_LOG_INFO(u8"Scene Sample Simple Module Unloaded");
}

int SceneSampleSimpleModule::main_module_exec(int argc, char8_t** argv)
{
    SkrZoneScopedN("SceneSampleSimpleModule::main_module_exec");
    SKR_LOG_INFO(u8"Running Scene Sample Simple Module");

    auto root = skr::Actor::GetRoot();
    auto actor1 = actor_manager.CreateActor<skr::Actor>();
    auto actor2 = actor_manager.CreateActor<skr::Actor>();
    root.lock()->CreateEntity();
    actor1.lock()->CreateEntity();
    actor2.lock()->CreateEntity();

    actor1.lock()->AttachTo(root);
    actor2.lock()->AttachTo(actor1);

    root.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, 0.0f, 0.0f }); // trigger update for the first time

    actor1.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, 1.0f, 0.0f });
    actor1.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 1.0f, 1.0f, 1.0f });
    actor1.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });

    actor2.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 1.0f, 0.0f, 0.0f });
    actor2.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 1.0f, 1.0f, 1.0f });
    actor2.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });

    transform_system->update();
    skr::ecs::TaskScheduler::Get()->flush_all();
    skr::ecs::TaskScheduler::Get()->sync_all();

    auto trans_accessor = world->random_read<const skr::scene::TransformComponent>();
    auto transform = trans_accessor[(skr::ecs::Entity)actor2.lock()->GetEntity()].get();
    SKR_LOG_INFO(u8"Transform Position: ({%f}, {%f}, {%f})", transform.position.x, transform.position.y, transform.position.z);
    SKR_LOG_INFO(u8"Transform Rotation: ({%f}, {%f}, {%f}, {%f})", transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
    SKR_LOG_INFO(u8"Transform Scale: ({%f}, {%f}, {%f})", transform.scale.x, transform.scale.y, transform.scale.z);

    return 0;
}