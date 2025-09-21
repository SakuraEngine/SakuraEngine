#include <SkrBase/misc/make_zeroed.hpp>
#include <SkrCore/memory/sp.hpp>
#include <SkrCore/module/module_manager.hpp>
#include <SkrCore/log.h>
#include <SkrCore/platform/vfs.h>
#include <SkrCore/time.h>
#include <SkrCore/async/thread_job.hpp>
#include "SkrCore/memory/impl/skr_new_delete.hpp"
#include <SkrRuntime/io/vram_io.hpp>
#include "SkrOS/thread.h"
#include "SkrProfile/profile.h"
#include "SkrRuntime/ecs/scheduler.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/misc/cmd_parser.hpp"
#include <SkrBase/type_info.hpp>

#include <SkrOS/filesystem.hpp>
#include "SkrSystem/advanced_input.h"
#include <SkrRuntime/ecs/world.hpp>
#include <SkrRuntime/resource/resource_system.h>
#include <SkrRuntime/resource/local_resource_registry.hpp>
#include <cmath>
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/imgui_app.hpp"

#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/render_mesh.h"

#include "SkrTask/fib_task.hpp"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"

#include "SkrTextureCompiler/texture_compiler.hpp"
#include "SkrTextureCompiler/texture_sampler_asset.hpp"

#include "SkrMeshTool/mesh_asset.hpp"
// #include "SkrMeshCore/builtin_mesh_asset.hpp"
// #include "SkrMeshCore/builtin_mesh.hpp"

#include "SkrScene/actor.h"
#include "SkrScene/actor_manager.h"
#include "SkrSceneCore/transform_system.h"

#include "scene_renderer.hpp"
#include "scene_render_system.h"

#include "helper.hpp"

using namespace skr::literals;
const auto MeshAssetID = u8"01985f1f-8286-773f-8bcc-7d7451b0265d"_guid;
const auto TextureID = u8"0198cb9d-35ab-7342-bd41-21f61e1d0d8e"_guid;
const auto FloorID = u8"0198cfc4-9bfd-77fd-a9a0-553938b10314"_guid;

struct BuiltInMeshAsset
{
    skr::GUID built_in_mesh_tid;
    skr::GUID asset_id;
};

skr::Vector<BuiltInMeshAsset> g_built_in_mesh_assets = {
    { skr::type_id_of<skd::asset::SimpleTriangleMesh>(), u8"0198efb0-4df3-7418-a554-3ca70850cf0f"_guid },
    { skr::type_id_of<skd::asset::SimpleCubeMesh>(), u8"0198efb0-7887-7595-99e5-34ce87cf5754"_guid }
};

// The Three-Triangle Example: simple mesh scene hierarchy
struct SceneSampleMeshModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    void InitializeResourceSystem();
    void InitializeAssetSystem();
    void DestroyAssetSystem();
    void DestroyResourceSystem();

    void CookAndLoadTestGLTF();

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
    skr::LocalResourceRegistry* registry = nullptr;
    skr::MeshFactory* mesh_factory = nullptr;
    skr::TextureSamplerFactory* TextureSamplerFactory = nullptr;
    skr::TextureFactory* TextureFactory = nullptr;
    skr::MaterialFactory* matFactory = nullptr;
    skr::MaterialTypeFactory* matTypeFactory = nullptr;

    skd::SProject project;
    skr::TransformSystem* transform_system = nullptr;
    skr::scene::SceneRenderSystem* scene_render_system = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SceneSampleMeshModule, SceneSample_Mesh);

void SceneSampleMeshModule::InitializeAssetSystem()
{
    auto& system = *skd::asset::GetCookSystem();
    system.Initialize();
}

void SceneSampleMeshModule::DestroyAssetSystem()
{
    auto& system = *skd::asset::GetCookSystem();
    system.Shutdown();
}

void SceneSampleMeshModule::InitializeResourceSystem()
{
    using namespace skr::literals;
    auto resource_system = skr::GetResourceSystem();
    registry = SkrNew<skr::LocalResourceRegistry>(project.GetResourceVFS());
    resource_system->Initialize(registry, project.GetRamService());

    const auto resource_root = project.GetResourceVFS()->mount_dir;
    {
        skr::String qn = u8"SceneSampleMesh-JobQueue";
        auto job_queueDesc = make_zeroed<skr::JobQueueDesc>();
        job_queueDesc.thread_count = 2;
        job_queueDesc.priority = SKR_THREAD_NORMAL;
        job_queueDesc.name = qn.u8_str();
        job_queue = skr::SP<skr::JobQueue>::New(job_queueDesc);

        auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
        ioServiceDesc.name = u8"SceneSampleMesh-RAMIOService";
        ioServiceDesc.sleep_time = 1000 / 60;
        ram_service = skr_io_ram_service_t::create(&ioServiceDesc);
        ram_service->run();

        auto vramServiceDesc = make_zeroed<skr_vram_io_service_desc_t>();
        vramServiceDesc.name = u8"SceneSampleMesh-VRAMIOService";
        vramServiceDesc.awake_at_request = true;
        vramServiceDesc.ram_service = ram_service;
        vramServiceDesc.callback_job_queue = job_queue.get();
        vramServiceDesc.use_dstorage = true;
        vramServiceDesc.gpu_device = render_device->get_cgpu_device();
        vram_service = skr_io_vram_service_t::create(&vramServiceDesc);
        vram_service->run();
    }
    // texture sampler factory
    {
        skr::TextureSamplerFactory::Root factoryRoot = {};
        factoryRoot.device = render_device->get_cgpu_device();
        TextureSamplerFactory = skr::TextureSamplerFactory::Create(factoryRoot);
        resource_system->RegisterFactory(TextureSamplerFactory);
    }
    // texture factory
    {
        skr::TextureFactory::Root factoryRoot = {};
        factoryRoot.dstorage_root = resource_root;
        factoryRoot.vfs = project.GetResourceVFS();
        factoryRoot.ram_service = ram_service;
        factoryRoot.vram_service = vram_service;
        factoryRoot.render_device = render_device;
        TextureFactory = skr::TextureFactory::Create(factoryRoot);
        resource_system->RegisterFactory(TextureFactory);
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
    // material type factory
    {
        skr::MaterialTypeFactory::Root factoryRoot = {};
        factoryRoot.render_device = render_device;
        matTypeFactory = skr::MaterialTypeFactory::Create(factoryRoot);
        resource_system->RegisterFactory(matTypeFactory);
    }

    // material factory
    {
        skr::MaterialFactory::Root factoryRoot = {};
        factoryRoot.render_device = render_device;
        factoryRoot.job_queue = job_queue.get();
        factoryRoot.ram_service = ram_service;
        matFactory = skr::MaterialFactory::Create(factoryRoot);
        resource_system->RegisterFactory(matFactory);
    }
}

void SceneSampleMeshModule::DestroyResourceSystem()
{
    auto resource_system = skr::GetResourceSystem();
    resource_system->Shutdown();

    skr::MeshFactory::Destroy(mesh_factory);
    skr::TextureFactory::Destroy(TextureFactory);
    skr::TextureSamplerFactory::Destroy(TextureSamplerFactory);

    skr_io_ram_service_t::destroy(ram_service);
    skr_io_vram_service_t::destroy(vram_service);
    job_queue.reset();
}

void SceneSampleMeshModule::on_load(int argc, char8_t** argv)
{
    skr_log_set_level(SKR_LOG_LEVEL_INFO);
    SKR_LOG_INFO(u8"Scene Sample Mesh Module Loaded");

    skr::cmd::parser parser(argc, (char**)argv);
    parser.add(u8"gltf", u8"use gltf", u8"-g", false);

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

    auto projectRoot = skr::fs::current_directory() / u8"../resources/scene/sample_mesh";
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

    skr::String projectName = u8"SceneSampleMesh";
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

void SceneSampleMeshModule::on_unload()
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

void SceneSampleMeshModule::CookAndLoadTestGLTF()
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
        u8"floor.builtin.meta",
        FloorID,
        skr::type_id_of<skr::MeshResource>(),
        skr::type_id_of<skd::asset::MeshCooker>());
    cook_system.ImportAssetMeta(&project, floor_asset, floor_importer, grid_metadata);

    for (auto i = 0; i < g_built_in_mesh_assets.size(); i++)
    {
        auto& built_in = g_built_in_mesh_assets[i];
        auto metadata = skd::asset::MeshAsset::Create<skd::asset::MeshAsset>();
        metadata->vertexType = u8"C35BD99A-B0A8-4602-AFCC-6BBEACC90321"_guid; // GLTFVertexLayoutWithJointId

        auto builtin_importer = skd::asset::ProceduralMeshImporter::Create<skd::asset::ProceduralMeshImporter>();
        builtin_importer->built_in_mesh_tid = built_in.built_in_mesh_tid;
        auto builtin_asset = skr::RC<skd::asset::AssetMetaFile>::New(
            skr::format(u8"builtin_{}.meta", i).c_str(),
            built_in.asset_id,
            skr::type_id_of<skr::MeshResource>(),
            skr::type_id_of<skd::asset::MeshCooker>());
        metadata->vertexType = u8"C35BD99A-B0A8-4602-AFCC-6BBEACC90321"_guid; // GLTFVertexLayoutWithJointId
        cook_system.ImportAssetMeta(&project, builtin_asset, builtin_importer, metadata);
    }

    auto metadata = skd::asset::MeshAsset::Create<skd::asset::MeshAsset>();
    metadata->vertexType = u8"C35BD99A-B0A8-4602-AFCC-6BBEACC90321"_guid; //    GLTFVertexLayoutWithJointId

    auto importer = skd::asset::GltfMeshImporter::Create<skd::asset::GltfMeshImporter>();

    // metadata->materials.push_back(TextureID);                             // Use the texture we just imported
    auto asset = skr::RC<skd::asset::AssetMetaFile>::New(
        u8"test.gltf.meta",
        MeshAssetID,
        skr::type_id_of<skr::MeshResource>(),
        skr::type_id_of<skd::asset::MeshCooker>());
    importer->assetPath = gltf_path.c_str();
    // importer->invariant_vertices = true;
    importer->import_all_materials = true;

    cook_system.ImportAssetMeta(&project, asset, importer, metadata);

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
}

int SceneSampleMeshModule::main_module_exec(int argc, char8_t** argv)
{
    using namespace skr;

    SkrZoneScopedN("SceneSampleMeshModule::main_module_exec");
    SKR_LOG_INFO(u8"Running Scene Sample Mesh Module");

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

    skr::Vector<skr::RCWeak<skr::MeshActor>> builtin_actors;
    builtin_actors.resize_default(g_built_in_mesh_assets.size());

    auto root = skr::Actor::GetRoot();
    auto actor1 = actor_manager.CreateActor<skr::MeshActor>().cast_static<skr::MeshActor>();

    actor1.lock()->SetDisplayName(u8"Actor 1");

    root.lock()->CreateEntity();

    actor1.lock()->CreateEntity();

    actor1.lock()->AttachTo(root);

    root.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, 0.0f, 0.0f });
    actor1.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, 10.0f, 0.0f });
    actor1.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ .1f, .1f, .1f });
    actor1.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });

    //auto actor2 = actor_manager.CreateActor<skr::MeshActor>().cast_static<skr::MeshActor>();
    //actor2.lock()->SetDisplayName(u8"Actor 2");
    //actor2.lock()->AttachTo(root);
    //actor2.lock()->CreateEntity();
    //actor2.lock()->GetComponent<skr::scene::PositionComponent>()->set({ 0.0f, 0.0f, 0.0f });
    //actor2.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 1.f, 1.f, 1.0f });
    //actor2.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });

    //for (size_t i = 0; i < g_built_in_mesh_assets.size(); i++)
    //{
    //    auto actor = actor_manager.CreateActor<skr::MeshActor>().cast_static<skr::MeshActor>();
    //    builtin_actors[i] = actor;
    //    actor.lock()->SetDisplayName(skr::format(u8"Builtin Actor {}", i).c_str());
    //    actor.lock()->CreateEntity();
    //    actor.lock()->AttachTo(actor2);

    //    actor.lock()->GetComponent<skr::scene::PositionComponent>()->set({ (float)(i * 5.0f), 2.0f, (float)(i * 5.0f) });
    //    actor.lock()->GetComponent<skr::scene::ScaleComponent>()->set({ 5.f, 5.f, 5.0f });
    //    actor.lock()->GetComponent<skr::scene::RotationComponent>()->set({ 0.0f, 0.0f, 0.0f });
    //}
    transform_system->update();
    transform_system->get_context()->update_finish.wait(true);

    actor1.lock()->GetComponent<skr::MeshComponent>()->mesh_resource = MeshAssetID;
    //actor2.lock()->GetComponent<skr::MeshComponent>()->mesh_resource = FloorID;
    //for (size_t i = 0; i < g_built_in_mesh_assets.size(); i++)
    //{
    //    builtin_actors[i].lock()->GetComponent<skr::MeshComponent>()->mesh_resource = g_built_in_mesh_assets[i].asset_id;
    //}

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

    while (!imgui_app->want_exit().comsume())
    {
        SkrZoneScopedN("LoopBody");
        {
            SkrZoneScopedN("PumpMessage");
            imgui_app->pump_message();
        }
        {
            // ResourceSystem Update
            resource_system->Update();
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
            transform_system->get_context()->update_finish.wait(true);
            scene_render_system->update();
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

            scene_renderer->draw_primitives(
                render_graph,
                scene_render_system->get_drawcalls());
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