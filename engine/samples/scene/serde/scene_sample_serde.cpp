// The SceneSample_Serde: The Principle for Scene Transform Update task
#include "SkrBase/math/gen/misc/transform.hpp"
#include <SkrCore/serialize/binary_archive.hpp>
#include <SkrCore/serialize/json_archive.hpp>
#include <SkrCore/module/module_manager.hpp>
#include <SkrCore/log.h>
#include <SkrOS/filesystem.hpp>
#include <SkrContainers/span.hpp>
#include <SkrContainers/path.hpp>

#include <SkrScene/actor.h>
#include <SkrScene/actor_manager.h>
#include <SkrSceneCore/scene_components.h>
#include <SkrSceneCore/transform_system.h>
#include <SkrRuntime/ecs/world.hpp>
#include <SkrRuntime/misc/cmd_parser.hpp>

inline static void _recursive_create_dir(const skr::Path& dir)
{
    if (dir.is_empty() || skr::fs::Directory::exists(dir))
        return;
    _recursive_create_dir(dir.parent_directory());
    skr::fs::Directory::create(dir, false);
}

inline static void _create_dir_if_not_exist(const skr::Path& dir)
{
    if (!skr::fs::Directory::exists(dir))
    {
        SKR_LOG_FMT_INFO(u8"Create Directory: {}", dir);
        skr::fs::Directory::create(dir, true);
    }
}

struct SceneSampleSerdeModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    bool bIsSerialize = false;
    bool bIsDeserialize = false;

    skr::TransformSystem* transform_system = nullptr;
    skr::task::scheduler_t scheduler;
    skr::ActorManager& actor_manager = skr::ActorManager::GetInstance();
    skr::Scene scene;
};

IMPLEMENT_DYNAMIC_MODULE(SceneSampleSerdeModule, SceneSample_Serde);
void SceneSampleSerdeModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"Scene Sample Simple Module Loaded");
    skr::cmd::parser parser(argc, (char**)argv);
    parser.add(u8"ser", u8"serialize", u8"-s", false);
    parser.add(u8"deser", u8"deserialize", u8"-d", false);
    if (!parser.parse())
    {
        SKR_LOG_ERROR(u8"Failed to parse command line arguments.");
        return;
    }
    auto ser_opt = parser.get_optional<bool>(u8"ser");
    auto des_opt = parser.get_optional<bool>(u8"deser");
    if (ser_opt) { bIsSerialize = *ser_opt; }
    if (des_opt) { bIsDeserialize = *des_opt; }

    SKR_LOG_INFO(u8"Serialize: %d", bIsSerialize);
    SKR_LOG_INFO(u8"Deserialize: %d", bIsDeserialize);

    scheduler.initialize(skr::task::scheudler_config_t());
    scheduler.bind();
}

void SceneSampleSerdeModule::on_unload()
{
    scheduler.unbind();
    actor_manager.UnBind();

    SKR_LOG_INFO(u8"Scene SceneSampleSerde Unloaded");
}

int SceneSampleSerdeModule::main_module_exec(int argc, char8_t** argv)
{
    SkrZoneScopedN("SceneSampleSerdeModule::main_module_exec");
    SKR_LOG_INFO(u8"Running SceneSampleSerde Module");

    skr::Path path = u8"D://ws/data/mid/skr/test.bin";
    skr::Path json_path = u8"D://ws/data/mid/skr/test.json";

    if (bIsSerialize)
    {
        // initialize world
        SKR_LOG_INFO(u8"Serialize");
        scene.root_actor_guid = skr::GUID::Create();
    }
    actor_manager.BindScene(&scene);

    skr::Path scene_path = u8"D://ws/data/mid/skr/scene.json";

    skr::GUID target_actor_guid;
    if (bIsSerialize)
    {
        auto root = actor_manager.GetRoot();
        root.lock()->InitWorld();
        auto* world = root.lock()->GetWorld();
        transform_system = skr_transform_system_create(world);
        world->bind_scheduler(scheduler);
        world->initialize();

        root.lock()->CreateEntity();
        auto actor1 = actor_manager.CreateActor<skr::Actor>();
        actor1.lock()->CreateEntity();
        actor1.lock()->AttachTo(root);

        auto pos_accessor = world->random_read<const skr::scene::PositionComponent>();

        root.lock()
            ->GetComponent<skr::scene::PositionComponent>()
            ->set({ 3.0f, 5.0f, 7.0f }); // trigger update for the first time

        actor1.lock()
            ->GetComponent<skr::scene::PositionComponent>()
            ->set({ 2.0f, 4.0f, 6.0f });
        actor1.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 1.0f, 1.0f, 1.0f });
        actor1.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });

        auto actor2 = actor_manager.CreateActor<skr::MeshActor>().cast_static<skr::MeshActor>();
        actor2.lock()->CreateEntity();
        actor2.lock()->AttachTo(actor1);
        actor2.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 17.0f, 0.0f, 0.0f });
        actor2.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 1.0f, 1.0f, 1.0f });
        actor2.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });

        transform_system->update();

        skr::ecs::TaskScheduler::Get()->flush_all();
        skr::ecs::TaskScheduler::Get()->sync_all();

        target_actor_guid = actor2.lock()->GetGUID();

        auto target_actor = actor_manager.GetActor(target_actor_guid);
        auto pos = pos_accessor[(skr::ecs::Entity)target_actor.lock()->GetEntity()].get();
        SKR_LOG_INFO(u8"Position: ({%f}, {%f}, {%f})", pos.x, pos.y, pos.z);
        auto trans_accessor = world->random_read<const skr::scene::TransformComponent>();
        auto transform = trans_accessor[(skr::ecs::Entity)target_actor.lock()->GetEntity()].get();
        SKR_LOG_INFO(u8"Transform Position: ({%f}, {%f}, {%f})", transform.position.x, transform.position.y, transform.position.z);
        SKR_LOG_INFO(u8"Transform Rotation: ({%f}, {%f}, {%f}, {%f})", transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
        SKR_LOG_INFO(u8"Transform Scale: ({%f}, {%f}, {%f})", transform.scale.x, transform.scale.y, transform.scale.z);

        // create directory if not exist
        _create_dir_if_not_exist(scene_path.parent_directory());
        _create_dir_if_not_exist(path.parent_directory());

        // write scene
        {
            scene.serialize();

            auto writer = skr::ArWriteJson::Create();
            writer.value(scene);
            writer.assume_succeeded_or_dump_error();
            writer.write_to_file(scene_path, skr::EJsonWriteFlags::Pretty);
        }

        // write ecs
        {
            // skr::ArWriteBin world_writer;
            // world_writer.value(*world);
            // world_writer.assume_succeeded_or_dump_error();
            // skr::fs::File::write_all_bytes(path, world_writer.buffer());

            auto world_writer = skr::ArWriteJson::Create();
            world_writer.value(*world);
            world_writer.assume_succeeded_or_dump_error();
            world_writer.write_to_file(json_path, skr::EJsonWriteFlags::Pretty);
        }

        actor_manager.ClearAllActors();
        SKR_LOG_INFO(u8"Serialize Done");
    }
    if (bIsDeserialize)
    {

        SKR_LOG_INFO(u8"Deserialize");
        // first deserialize the scene, will automatically create the root actor

        // read scene
        {
            auto reader = skr::ArReadJson::ReadFile(scene_path);
            reader.value(scene);
            reader.assume_succeeded_or_dump_error();
        }

        auto root = skr::Actor::GetRoot();

        auto* world = root.lock()->GetWorld();
        transform_system = skr_transform_system_create(world);
        world->bind_scheduler(scheduler);
        world->initialize();

        // read ecs
        {
            // skr::Vector<uint8_t> rbuffer;
            // skr::fs::File::read_all_bytes(path, rbuffer);
            // skr::ArReadBin world_reader;
            // world_reader.use_buffer(rbuffer);
            // world_reader.value(*world);
            // world_reader.assume_succeeded_or_dump_error();

            auto world_reader = skr::ArReadJson::ReadFile(json_path);
            world_reader.value(*world);
            world_reader.assume_succeeded_or_dump_error();
        }

        transform_system->update();
        skr::ecs::TaskScheduler::Get()->flush_all();
        skr::ecs::TaskScheduler::Get()->sync_all();
        auto children_guids = root.lock()->GetChildrenGUIDs();
        auto first_child = actor_manager.GetActor(children_guids[0]);
        children_guids = first_child.lock()->GetChildrenGUIDs();
        first_child = actor_manager.GetActor(children_guids[0]);

        target_actor_guid = first_child.lock()->GetGUID();
        auto target_actor = actor_manager.GetActor(target_actor_guid);

        auto pos_accessor = world->random_read<const skr::scene::PositionComponent>();
        auto pos = pos_accessor[(skr::ecs::Entity)target_actor.lock()->GetEntity()].get();
        SKR_LOG_INFO(u8"Position: ({%f}, {%f}, {%f})", pos.x, pos.y, pos.z);
        auto trans_accessor = world->random_read<const skr::scene::TransformComponent>();
        auto transform = trans_accessor[(skr::ecs::Entity)target_actor.lock()->GetEntity()].get();
        SKR_LOG_INFO(u8"Transform Position: ({%f}, {%f}, {%f})", transform.position.x, transform.position.y, transform.position.z);
        SKR_LOG_INFO(u8"Transform Rotation: ({%f}, {%f}, {%f}, {%f})", transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
        SKR_LOG_INFO(u8"Transform Scale: ({%f}, {%f}, {%f})", transform.scale.x, transform.scale.y, transform.scale.z);

        actor_manager.ClearAllActors();
        SKR_LOG_INFO(u8"Deserialize Done");
    }
    skr_transform_system_destroy(transform_system);
    return 0;
}