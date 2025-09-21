#include <SkrBase/misc/make_zeroed.hpp>
#include <SkrBase/containers/path/path.hpp>
#include <SkrCore/memory/sp.hpp>
#include <SkrCore/module/module_manager.hpp>
#include <SkrCore/log.h>
#include <SkrCore/platform/vfs.h>
#include <SkrCore/time.h>
#include <SkrCore/async/thread_job.hpp>
#include <SkrRuntime/io/vram_io.hpp>

#include "SkrOS/thread.h"
#include "SkrProfile/profile.h"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/misc/cmd_parser.hpp"

#include <SkrOS/filesystem.hpp>
#include "SkrCore/memory/impl/skr_new_delete.hpp"
#include "SkrSceneCore/scene_components.h"
#include "SkrSystem/advanced_input.h"
#include <SkrRuntime/ecs/world.hpp>
#include <SkrRuntime/resource/resource_system.h>
#include <SkrRuntime/resource/local_resource_registry.hpp>
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/imgui_app.hpp"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrTask/fib_task.hpp"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrMeshTool/mesh_asset.hpp"

#include "SkrScene/actor.h"
#include "SkrScene/actor_manager.h"
#include "SkrSceneCore/transform_system.h"

#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrAnim/resources/skin_resource.hpp"
#include "SkrAnim/components/skin_component.hpp"
#include "SkrAnim/components/skeleton_component.hpp"
#include "SkrAnim/ozz/base/containers/vector.h"
#include "SkrAnim/ozz/sampling_job.h"
#include "SkrAnim/ozz/base/maths/soa_transform.h"
#include "SkrAnim/ozz/local_to_model_job.h"
#include "SkrAnim/resources/skin_resource.hpp"

#include "SkrAnimTool/skeleton_asset.h"
#include "SkrAnimTool/animation_asset.h"
#include "SkrAnimTool/skin_asset.h"

#include "scene_renderer.hpp"
#include "scene_render_system.h"
#include "helper.hpp"

using namespace skr::literals;
const auto MeshAssetID = u8"01988203-c467-72ef-916b-c8a5db2ec18d"_guid;
const auto SkelAssetID = u8"0198a7d5-6819-76e2-88c3-fad2f6c3d5d5"_guid;
const auto AnimAssetID = u8"0198a890-b5f8-750e-8e4d-cb200eb53b0e"_guid;
const auto SkinAssetID = u8"0198ab86-af53-72bd-be58-ef888ea9c023"_guid;
const auto FloorAssetID = u8"0198d4c8-0e0b-7048-a51d-6e6c4d6df083"_guid;
const auto BuiltinMeshID = u8"0198cfc4-9bfd-77fd-a9a0-553938b10314"_guid;

// The Three-Triangle Example: simple mesh scene hierarchy
struct SceneSampleSkelMeshModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    void InitializeResourceSystem();
    void InitializeAssetSystem();
    void DestroyAssetSystem();
    void DestroyResourceSystem();
    void CookAndLoadTestGLTF();

    float current_time = 0.0f;

    skr::task::scheduler_t scheduler;
    skr::Scene scene;
    skr::ecs::ECSWorld* world = nullptr;
    skr::ActorManager& actor_manager = skr::ActorManager::GetInstance();

    skr_vfs_t* resource_vfs = nullptr;
    skr::io::IRAMService* ram_service = nullptr;
    skr_io_vram_service_t* vram_service = nullptr;
    skr::SP<skr::JobQueue> job_queue = nullptr;
    SRenderDeviceId render_device = nullptr;

    skr::UPtr<skr::ImGuiApp> imgui_app = nullptr;
    skr::SceneRenderer* scene_renderer = nullptr;

    skr::String gltf_path = u8"";
    skr::String animation_name = u8"";
    skr::LocalResourceRegistry* registry = nullptr;

    skr::MeshFactory* mesh_factory = nullptr;
    skr::SkelFactory* skelFactory = nullptr;
    skr::AnimFactory* animFactory = nullptr;
    skr::SkinFactory* skinFactory = nullptr;

    skd::SProject project;
    skr::TransformSystem* transform_system = nullptr;
    skr::scene::SceneRenderSystem* scene_render_system = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SceneSampleSkelMeshModule, SceneSample_SkelMesh);

void SceneSampleSkelMeshModule::InitializeAssetSystem()
{
    auto& system = *skd::asset::GetCookSystem();
    system.Initialize();
}

void SceneSampleSkelMeshModule::DestroyAssetSystem()
{
    auto& system = *skd::asset::GetCookSystem();
    system.Shutdown();
}

void SceneSampleSkelMeshModule::InitializeResourceSystem()
{
    using namespace skr::literals;
    auto resource_system = skr::GetResourceSystem();
    registry = SkrNew<skr::LocalResourceRegistry>(project.GetResourceVFS());
    resource_system->Initialize(registry, project.GetRamService());
    const auto resource_root = project.GetResourceVFS()->mount_dir;
    {
        skr::String qn = u8"SceneSampleSkelMesh-JobQueue";
        auto job_queueDesc = make_zeroed<skr::JobQueueDesc>();
        job_queueDesc.thread_count = 2;
        job_queueDesc.priority = SKR_THREAD_NORMAL;
        job_queueDesc.name = qn.u8_str();
        job_queue = skr::SP<skr::JobQueue>::New(job_queueDesc);

        auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();

        ioServiceDesc.name = u8"S012-3456-789A-BCDE-FGHI-JKLM-N"; // 31 byte, pass
        //ioServiceDesc.name = u8"S012-3456-789A-BCDE-FGHI-JKLM-NO"; // 32 byte, fail

        ioServiceDesc.sleep_time = 1000 / 60;
        ram_service = skr_io_ram_service_t::create(&ioServiceDesc);
        ram_service->run();

        auto vramServiceDesc = make_zeroed<skr_vram_io_service_desc_t>();
        vramServiceDesc.name = u8"SkelMesh-VRAMIOService";
        vramServiceDesc.awake_at_request = true;
        vramServiceDesc.ram_service = ram_service;
        vramServiceDesc.callback_job_queue = job_queue.get();
        vramServiceDesc.use_dstorage = true;
        vramServiceDesc.gpu_device = render_device->get_cgpu_device();
        vram_service = skr_io_vram_service_t::create(&vramServiceDesc);
        vram_service->run();
    }
    // mesh factory
    {
        skr::MeshFactory::Root factoryRoot = {};
        factoryRoot.dstorage_root = resource_root;
        factoryRoot.vfs = project.GetResourceVFS();
        factoryRoot.ram_service = ram_service;
        factoryRoot.vram_service = vram_service;
        factoryRoot.render_device = render_device;
        mesh_factory = skr::MeshFactory::Create(factoryRoot);
        resource_system->RegisterFactory(mesh_factory);
    }
    // skel factory
    {
        skelFactory = SkrNew<skr::SkelFactory>();
        resource_system->RegisterFactory(skelFactory);
    }
    // anim factory
    {
        animFactory = SkrNew<skr::AnimFactory>();
        resource_system->RegisterFactory(animFactory);
    }
    // skin factory
    {
        skinFactory = SkrNew<skr::SkinFactory>();
        resource_system->RegisterFactory(skinFactory);
    }
}

void SceneSampleSkelMeshModule::DestroyResourceSystem()
{
    auto resource_system = skr::GetResourceSystem();
    resource_system->Shutdown();

    skr::MeshFactory::Destroy(mesh_factory);
    SkrDelete(skelFactory);
    SkrDelete(animFactory);
    SkrDelete(skinFactory);

    skr_io_ram_service_t::destroy(ram_service);
    skr_io_vram_service_t::destroy(vram_service);
    job_queue.reset();
}

void SceneSampleSkelMeshModule::on_load(int argc, char8_t** argv)
{
    skr_log_set_level(SKR_LOG_LEVEL_INFO);
    SKR_LOG_INFO(u8"Scene Sample Mesh Module Loaded");

    skr::cmd::parser parser(argc, (char**)argv);
    parser.add(u8"gltf", u8"gltf file path", u8"-g", false);
    parser.add(u8"anim", u8"animation name", u8"-a", false);
    if (!parser.parse())
    {
        SKR_LOG_ERROR(u8"Failed to parse command line arguments.");
        return;
    }
    auto gltf_path_opt = parser.get_optional<skr::String>(u8"gltf");
    if (gltf_path_opt)
    {
        gltf_path = *gltf_path_opt;
        SKR_LOG_INFO(u8"gltf file path set to: %s", gltf_path.c_str());
    }
    else
    {
        SKR_LOG_INFO(u8"No gltf file specified");
    }

    auto anim_name_opt = parser.get_optional<skr::String>(u8"anim");
    if (anim_name_opt)
    {
        animation_name = *anim_name_opt;
        SKR_LOG_INFO(u8"animation name set to: %s", animation_name.c_str());
    }
    else
    {
        SKR_LOG_INFO(u8"No animation name specified, will use the first animation in the gltf file");
    }

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
    scene_render_system = skr::scene::SceneRenderSystem::Create(world);

    render_device = SkrRendererModule::Get()->get_render_device();

    auto resourceRoot = (skr::fs::current_directory() / u8"../resources");
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    auto projectRoot = skr::fs::current_directory() / u8"../resources/scene/sample_skelmesh";
    // if not exists, create the project root directory
    if (!skr::fs::Directory::exists(projectRoot))
    {
        skr::fs::Directory::create(projectRoot, true);
    }
    skd::SProjectConfig projectConfig = {
        .assetDirectory = (projectRoot / u8"assets").string().c_str(),
        .resourceDirectory = (projectRoot / u8"resources").string().c_str(),
        .artifactsDirectory = (projectRoot / u8"artifacts").string().c_str()
    };

    skr::String projectName = u8"SceneSampleSkelMesh";
    skr::String rootPath = projectRoot.string().c_str();
    project.OpenProject(projectName.c_str(), rootPath.c_str(), projectConfig);

    {
        InitializeResourceSystem();
        InitializeAssetSystem();
    }
    scene_renderer = skr::SceneRenderer::Create();
    scene_renderer->initialize(render_device, world, resource_vfs);
    scene_render_system->bind_renderer(scene_renderer);
}

void SceneSampleSkelMeshModule::on_unload()
{
    project.CloseProject();
    scene_renderer->finalize(SkrRendererModule::Get()->get_render_device());
    skr::SceneRenderer::Destroy(scene_renderer);
    // stop all ram_service
    {
        DestroyAssetSystem();
        DestroyResourceSystem();
    }
    skr_transform_system_destroy(transform_system);
    skr::scene::SceneRenderSystem::Destroy(scene_render_system);

    actor_manager.ClearAllActors();
    scheduler.unbind();
    SKR_LOG_INFO(u8"Scene Sample Mesh Module Unloaded");
}

void SceneSampleSkelMeshModule::CookAndLoadTestGLTF()
{
    auto& cook_system = *skd::asset::GetCookSystem();

    auto grid_metadata = skd::asset::SimpleGridMeshAsset::Create<skd::asset::SimpleGridMeshAsset>();
    grid_metadata->x_segments = 10;
    grid_metadata->y_segments = 10;
    grid_metadata->x_size = 10.0f;
    grid_metadata->y_size = 10.0f;
    grid_metadata->vertexType = u8"C35BD99A-B0A8-4602-AFCC-6BBEACC90321"_guid; // GLTFVertexLayoutWithJointId

    auto floor_importer = skd::asset::ProceduralMeshImporter::Create<skd::asset::ProceduralMeshImporter>();
    // builtin_importer->built_in_mesh_tid = skr::type_id_of<skd::asset::SimpleTriangleMesh>();
    // builtin_importer->built_in_mesh_tid = skr::type_id_of<skd::asset::SimpleCubeMesh>();
    floor_importer->built_in_mesh_tid = skr::type_id_of<skd::asset::SimpleGridMesh>();
    auto floor_asset = skr::RC<skd::asset::AssetMetaFile>::New(
        u8"floor.meta",
        FloorAssetID,
        skr::type_id_of<skr::MeshResource>(),
        skr::type_id_of<skd::asset::MeshCooker>());
    cook_system.ImportAssetMeta(&project, floor_asset, floor_importer, grid_metadata);
    auto metadata = skd::asset::MeshAsset::Create<skd::asset::MeshAsset>();
    // metadata->vertexType = u8"1b357a40-83ff-471c-8903-23e99d95b273"_guid; // GLTFVertexLayoutWithoutTangentId
    metadata->vertexType = u8"C35BD99A-B0A8-4602-AFCC-6BBEACC90321"_guid; // GLTFVertexLayoutWithJointId
    auto builtin_importer = skd::asset::ProceduralMeshImporter::Create<skd::asset::ProceduralMeshImporter>();
    // builtin_importer->built_in_mesh_tid = skr::type_id_of<skd::asset::SimpleTriangleMesh>();
    builtin_importer->built_in_mesh_tid = skr::type_id_of<skd::asset::SimpleCubeMesh>();
    // builtin_importer->built_in_mesh_tid = skr::type_id_of<skd::asset::SimpleGridMesh>();
    auto builtin_asset = skr::RC<skd::asset::AssetMetaFile>::New(
        u8"simple_cube.meta",
        BuiltinMeshID,
        skr::type_id_of<skr::MeshResource>(),
        skr::type_id_of<skd::asset::MeshCooker>());
    cook_system.ImportAssetMeta(&project, builtin_asset, builtin_importer, metadata);

    auto importer = skd::asset::GltfMeshImporter::Create<skd::asset::GltfMeshImporter>();

    auto asset = skr::RC<skd::asset::AssetMetaFile>::New(
        u8"girl.gltf.meta",
        MeshAssetID,
        skr::type_id_of<skr::MeshResource>(),
        skr::type_id_of<skd::asset::MeshCooker>());
    importer->assetPath = gltf_path.c_str();
    cook_system.ImportAssetMeta(&project, asset, importer, metadata);

    auto skelImporter = skd::asset::GltfSkelImporter::Create<skd::asset::GltfSkelImporter>();
    auto meshdata = skd::asset::SkeletonAsset::Create<skd::asset::SkeletonAsset>();
    auto skel_asset = skr::RC<skd::asset::AssetMetaFile>::New(
        u8"test_skeleton.gltf.meta",
        SkelAssetID,
        skr::type_id_of<skr::SkeletonResource>(),
        skr::type_id_of<skd::asset::SkelCooker>());
    skelImporter->assetPath = gltf_path.c_str();
    cook_system.ImportAssetMeta(&project, skel_asset, skelImporter, meshdata);

    auto animImporter = skd::asset::GltfAnimImporter::Create<skd::asset::GltfAnimImporter>();
    auto animdata = skd::asset::AnimAsset::Create<skd::asset::AnimAsset>();

    animdata->skeletonAsset = skel_asset->GetGUID();
    auto anim_asset = skr::RC<skd::asset::AssetMetaFile>::New(
        u8"test_animation.gltf.meta",
        AnimAssetID,
        skr::type_id_of<skr::AnimResource>(),
        skr::type_id_of<skd::asset::AnimCooker>());
    animImporter->assetPath = gltf_path.c_str();
    animImporter->animationName = animation_name.c_str();

    cook_system.ImportAssetMeta(&project, anim_asset, animImporter, animdata);

    auto skin_importer = skd::asset::GltfMeshImporter::Create<skd::asset::GltfMeshImporter>();
    skin_importer->assetPath = gltf_path.c_str();
    auto skin_asset = skr::RC<skd::asset::AssetMetaFile>::New(
        u8"test_skin.gltf.meta",
        SkinAssetID,
        skr::type_id_of<skr::SkinResource>(),
        skr::type_id_of<skd::asset::SkinCooker>());
    cook_system.ImportAssetMeta(&project, skin_asset, skin_importer, metadata);

    {
        cook_system.ParallelForEachAsset(1,
            [&](skr::Span<skr::RC<skd::asset::AssetMetaFile>> assets) {
                SkrZoneScopedN("Cook");
                for (auto asset : assets)
                {
                    cook_system.EnsureCooked(asset->GetGUID());
                }
            });
    }

    auto resource_system = skr::GetResourceSystem();
    skr::task::schedule([&] {
        cook_system.WaitForAll();
        // resource_system->Quit();
    },
        nullptr);
    resource_system->Update();
    //----- wait
}

int SceneSampleSkelMeshModule::main_module_exec(int argc, char8_t** argv)
{

    SkrZoneScopedN("SceneSampleSkelMeshModule::main_module_exec");
    SKR_LOG_INFO(u8"Running Scene Sample SkelMesh Module");

    SKR_LOG_INFO(u8"gltf file path: {%s}", gltf_path.c_str());
    if (gltf_path.is_empty())
    {
        SKR_LOG_ERROR(u8"gltf file path is empty, please specify a valid gltf file path.");
        return 1;
    }

    utils::Camera camera;
    scene_renderer->set_camera(&camera);
    utils::CameraController cam_ctrl;
    cam_ctrl.set_camera(&camera);

    auto cgpu_device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();

    skr::Vector<skr::RCWeak<skr::SkelMeshActor>> hierarchy_actors;
    constexpr int hierarchy_count = 4; // Number of actors in the hierarchy

    auto root = skr::Actor::GetRoot();
    auto actor1 = actor_manager.CreateActor<skr::SkelMeshActor>().cast_static<skr::SkelMeshActor>();
    auto actor2 = actor_manager.CreateActor<skr::MeshActor>().cast_static<skr::MeshActor>();
    auto actor3 = actor_manager.CreateActor<skr::MeshActor>().cast_static<skr::MeshActor>();
    {
        actor1.lock()->SetDisplayName(u8"Actor 1");
        actor2.lock()->SetDisplayName(u8"Actor 2");
        actor3.lock()->SetDisplayName(u8"Actor 3");
        root.lock()->CreateEntity();
        actor1.lock()->CreateEntity();
        actor2.lock()->CreateEntity();
        actor3.lock()->CreateEntity();
        actor1.lock()->AttachTo(root);
        actor2.lock()->AttachTo(root);
        actor3.lock()->AttachTo(actor2);

        root.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, 0.0f, 0.0f });

        actor1.lock()->GetComponent<skr::scene::PositionComponent>()->set({ -10.0f, 1.0f, 10.0f });
        actor1.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ .1f, .1f, .1f });
        actor1.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.8f, 0.0f });

        actor2.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, 0.0f, 0.0f });
        actor2.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 1.f, 1.f, 1.0f });
        actor2.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });

        actor3.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 2.0f, 2.0f, 2.0f });
        actor3.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 5.f, 5.f, 5.0f });
        actor3.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });
    }

    for (auto i = 0; i < hierarchy_count; ++i)
    {
        auto actor = actor_manager.CreateActor<skr::MeshActor>().cast_static<skr::MeshActor>();
        hierarchy_actors.push_back(actor.cast_static<skr::SkelMeshActor>());

        actor.lock()->SetDisplayName(skr::format(u8"HActor {}", i).c_str());
        actor.lock()->CreateEntity();

        if (i == 0)
        {
            actor.lock()->AttachTo(actor2);
            actor.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ .1f, .1f, .1f });
        }
        else
        {
            actor.lock()->AttachTo(hierarchy_actors[i - 1]);
            actor.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ .5f, .5f, .5f });
            actor.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, (float)i * 0.5f });
        }

        actor.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, (float)(i + 1) * 5.0f, (float)(i + 1) * 5.0f });
    }

    transform_system->update();
    skr::ecs::TaskScheduler::Get()->sync_all();

    actor1.lock()->GetComponent<skr::MeshComponent>()->mesh_resource = MeshAssetID;
    actor2.lock()->GetComponent<skr::MeshComponent>()->mesh_resource = FloorAssetID;
    actor3.lock()->GetComponent<skr::MeshComponent>()->mesh_resource = BuiltinMeshID;

    for (auto& actor : hierarchy_actors)
    {
        actor.lock()->GetComponent<skr::MeshComponent>()->mesh_resource = MeshAssetID;
    }

    CookAndLoadTestGLTF();

    {
        skr::render_graph::RenderGraphBuilder graph_builder;
        graph_builder.with_device(cgpu_device)
            .with_gfx_queue(gfx_queue)
            .enable_memory_aliasing();

        skr::SystemWindowCreateInfo main_window_info = {
            .title = skr::format(u8"Scene Viewer [{}]", gCGPUBackendNames[cgpu_device->adapter->instance->backend]),
            .size = { 1024, 768 },
        };

        imgui_app = skr::UPtr<skr::ImGuiApp>::New(main_window_info, render_device, graph_builder);
        imgui_app->initialize();
        imgui_app->enable_docking();
    }
    auto render_graph = imgui_app->render_graph();
    // Time
    SHiresTimer tick_timer;
    int64_t elapsed_us = 0;
    int64_t elapsed_frame = 0;
    int64_t fps = 60;
    skr_init_hires_timer(&tick_timer);
    uint64_t frame_index = 0;

    skr::input::Input::Initialize();

    auto resource_system = skr::GetResourceSystem();

    actor1.lock()->GetComponent<skr::SkeletonComponent>()->skeleton_resource = SkelAssetID;
    actor1.lock()->GetComponent<skr::SkinComponent>()->skin_resource = SkinAssetID;

    skr::AsyncResource<skr::AnimResource> anim_resource_handle = AnimAssetID;
    ozz::animation::SamplingJob::Context context_;

    bool controlling_actor = false;

    while (!imgui_app->want_exit().comsume())
    {
        SkrZoneScopedN("LoopBody");
        {
            SkrZoneScopedN("PumpMessage");
            imgui_app->pump_message();
        }
        {
            SkrZoneScopedN("ResourceSystemUpdate");
            resource_system->Update();
        }

        {
            auto* mesh_comp = actor1.lock()->GetComponent<skr::MeshComponent>();
            auto* skel_comp = actor1.lock()->GetComponent<skr::SkeletonComponent>();
            auto* skin_comp = actor1.lock()->GetComponent<skr::SkinComponent>();

            mesh_comp->mesh_resource.resolve(true, 0);
            skel_comp->skeleton_resource.resolve(true, 0);
            skin_comp->skin_resource.resolve(true, 0);
            anim_resource_handle.resolve(true, 0);
            if (
                mesh_comp->mesh_resource.is_resolved() &&
                skel_comp->skeleton_resource.is_resolved() &&
                skin_comp->skin_resource.is_resolved() &&
                anim_resource_handle.is_resolved())

            {
                auto* skeleton_resource = skel_comp->skeleton_resource.get_resolved(true);
                auto* skin_resource = skin_comp->skin_resource.get_resolved(true);
                auto* anim = anim_resource_handle.get_resolved(true);
                auto* mesh_resource = mesh_comp->mesh_resource.get_resolved(true);
                auto* runtime_anim_component = actor1.lock()->GetComponent<skr::AnimComponent>();

                if (skeleton_resource && skin_resource && anim && runtime_anim_component && mesh_resource && mesh_resource->render_mesh)
                {
                    skr_init_skin_component(skin_comp, skeleton_resource);
                    skr_init_anim_component(runtime_anim_component, mesh_resource, skeleton_resource);
                    skr_init_anim_buffers(cgpu_device, runtime_anim_component, mesh_resource);
                }
                {
                    int64_t us = skr_hires_timer_get_usec(&tick_timer, true);
                    double deltaTime = (double)us / 1000 / 1000; // in seconds
                    current_time += deltaTime;
                    if (current_time > anim->animation.duration())
                    {
                        current_time = 0.0f;
                    }

                    float ratio = current_time / anim->animation.duration();
                    ozz::animation::SamplingJob::Context context_;
                    context_.Resize(skeleton_resource->skeleton.num_joints());
                    ozz::vector<ozz::math::SoaTransform> locals_;
                    locals_.resize(skeleton_resource->skeleton.num_soa_joints());
                    ozz::animation::SamplingJob sampling_job;
                    sampling_job.animation = &anim->animation;
                    sampling_job.ratio = ratio;
                    sampling_job.context = &context_;
                    sampling_job.output = ozz::make_span(locals_);
                    if (!sampling_job.Run())
                    {
                        SKR_LOG_ERROR(u8"Failed to run SamplingJob");
                    }

                    ozz::animation::LocalToModelJob ltm_job;
                    ltm_job.skeleton = &skeleton_resource->skeleton;
                    ltm_job.input = ozz::make_span(locals_);
                    ltm_job.output = ozz::span{ runtime_anim_component->joint_matrices.data(), runtime_anim_component->joint_matrices.size() };
                    if (!ltm_job.Run())
                    {
                        SKR_LOG_ERROR(u8"Failed to run LocalToModelJob");
                    }
                }
                {
                    // update joint matrices
                    SkrZoneScopedN("UpdateJointMatrices");
                    auto* anim = actor1.lock()->GetComponent<skr::AnimComponent>();
                }
                {
                    if (!(skin_comp->joint_remaps.is_empty() || runtime_anim_component->buffers.is_empty()))
                    {
                        skr_cpu_skin(skin_comp, runtime_anim_component, mesh_resource);
                    }
                }
                {
                    // upload skin mesh data
                    uint64_t skinVerticesSize = 0;
                    {
                        SkrZoneScopedN("CalculateSkinMeshSize");
                        auto* anim = actor1.lock()->GetComponent<skr::AnimComponent>();
                        for (size_t j = 0u; j < anim->buffers.size(); j++)
                        {
                            skinVerticesSize += anim->buffers[j]->get_size();
                        }
                        // SKR_LOG_INFO(u8"Skin mesh size: %llu bytes", skinVerticesSize);
                    }
                    auto upload_buffer_handle = render_graph->create_buffer(
                        [=](skr::render_graph::RenderGraph& g, skr::render_graph::BufferBuilder& builder) {
                            builder.set_name(SKR_UTF8("SkinMeshUploadBuffer"))
                                .size(skinVerticesSize)
                                .with_tags(kRenderGraphDefaultResourceTag)
                                .as_upload_buffer();
                        });

                    render_graph->add_copy_pass(
                        [=](skr::render_graph::RenderGraph& g, skr::render_graph::CopyPassBuilder& builder) {
                            builder.set_name(SKR_UTF8("CopySkinMesh"))
                                .from_buffer(upload_buffer_handle.range(0, skinVerticesSize))
                                .can_be_lone();
                        },
                        [=](skr::render_graph::RenderGraph& g, skr::render_graph::CopyPassContext& context) {
                            SkrZoneScopedN("CopySkinMesh");

                            auto upload_buffer = context.resolve(upload_buffer_handle);
                            auto mapped = (uint8_t*)upload_buffer->info->cpu_mapped_address;

                            // barrier from vb to copy dest
                            {
                                SkrZoneScopedN("Barriers");
                                CGPUResourceBarrierDescriptor barrier_desc = {};
                                skr::Vector<CGPUBufferBarrier> barriers;

                                auto* anim = actor1.lock()->GetComponent<skr::AnimComponent>();
                                for (size_t j = 0u; j < anim->buffers.size(); j++)
                                {
                                    const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                                    if (anim->vbs[j] && !use_dynamic_buffer)
                                    {
                                        CGPUBufferBarrier& barrier = barriers.emplace().ref();
                                        barrier.buffer = anim->vbs[j];
                                        barrier.src_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                                        barrier.dst_state = CGPU_RESOURCE_STATE_COPY_DEST;
                                    }
                                }

                                barrier_desc.buffer_barriers = barriers.data();
                                barrier_desc.buffer_barriers_count = (uint32_t)barriers.size();
                                cgpu_cmd_resource_barrier(context.cmd, &barrier_desc);
                            }

                            // upload
                            {
                                SkrZoneScopedN("MemCopies");
                                uint64_t cursor = 0;

                                auto* anim = actor1.lock()->GetComponent<skr::AnimComponent>();
                                const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                                if (!use_dynamic_buffer)
                                {
                                    for (size_t j = 0u; j < anim->buffers.size(); j++)
                                    {
                                        // memcpy
                                        memcpy(mapped + cursor, anim->buffers[j]->get_data(), anim->buffers[j]->get_size());

                                        // queue cpy
                                        CGPUBufferToBufferTransfer b2b = {};
                                        b2b.src = upload_buffer;
                                        b2b.src_offset = cursor;
                                        b2b.dst = anim->vbs[j];
                                        b2b.dst_offset = 0;
                                        b2b.size = anim->buffers[j]->get_size();
                                        cgpu_cmd_transfer_buffer_to_buffer(context.cmd, &b2b);

                                        cursor += anim->buffers[j]->get_size();
                                    }
                                }
                            }
                        });
                }
            }
        }

        {
            // Update Actor Transform
            transform_system->update();
        }

        // Camera control
        {
            SkrZoneScopedN("CameraControl");
            cam_ctrl.imgui_control_frame();
        }

        {
            SkrZoneScopedN("ViewportRender");
            imgui_app->acquire_frames();
            auto main_window = imgui_app->main_window();
            const auto size = main_window->get_physical_size();
            camera.aspect = (float)size.x / (float)size.y;
        };
        {
            SkrZoneScopedN("AnimRenderJob");
            transform_system->get_context()->update_finish.wait(true); // wait for transform system to finish updating
            scene_render_system->update();

            // skr::ecs::TaskScheduler::Get()->sync_all();
            scene_render_system->get_context()->update_finish.wait(true);

            auto backbuffer = render_graph->get_texture(u8"backbuffer");
            const auto back_desc = render_graph->resolve_descriptor(backbuffer);
            auto depthbuffer = render_graph->create_texture(
                [=, this](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                    builder.set_name(SKR_UTF8("render_depth"))
                        .extent(back_desc->width, back_desc->height)
                        .format(CGPU_FORMAT_D32_SFLOAT)
                        .sample_count(CGPU_SAMPLE_COUNT_1)
                        .allow_depth_stencil();
                    if (back_desc->width > 2048) builder.heap_dedicated();
                });

            scene_renderer->draw_primitives(render_graph, scene_render_system->get_drawcalls());
        }

        {
            SkrZoneScopedN("ImGuiRender");
            imgui_app->set_load_action(CGPU_LOAD_ACTION_LOAD);
            imgui_app->render_imgui();
        }
        {
            SkrZoneScopedN("RenderGraphExecute");
            frame_index = render_graph->execute();
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                render_graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
        }
        // present
        imgui_app->present_all();
    }

    cgpu_wait_queue_idle(gfx_queue);
    imgui_app->shutdown();
    skr::input::Input::Finalize();
    return 0;
}