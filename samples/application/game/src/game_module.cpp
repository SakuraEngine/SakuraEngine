#include "SkrCore/memory/sp.hpp"
#include "common/utils.h"
#include "GameRuntime/gamert.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrOS/filesystem.hpp"
#include "SkrRT/config.h"
#include "SkrCore/memory/memory.h"
#include "SkrCore/time.h"

#include "SkrRT/ecs/type_builder.hpp"

#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"

#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#include "SkrRT/resource/resource_system.h"

#include "SkrScene/scene.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrRenderer/render_effect.h"

#include <SkrContainers/string.hpp>
#include "SkrTask/fib_task.hpp"

#include "SkrRT/resource/local_resource_registry.hpp"
#include "SkrRenderer/shader_map.h"
#include "SkrRenderer/render_viewport.h"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrAnim/resources/skin_resource.hpp"
#include "SkrAnim/components/skin_component.hpp"
#include "SkrAnim/components/skeleton_component.hpp"
#include "GameRuntime/game_animation.h"

#include "SkrCore/async/thread_job.hpp"

#include "SkrProfile/profile.h"
#include "SkrLua/skr_lua.h"
// #include "SkrInspector/inspect_value.h" // FIXME. inspector

#include "SkrScene/resources/scene_resource.h"

uint32_t    backbuffer_index;
extern void game_initialize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* resource_vfs);
extern void game_register_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
extern void game_finalize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
#define lerp(a, b, t) (a) + (t) * ((b) - (a))

const bool bUseJob = true;

class SGameModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int  main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    void installResourceFactories();
    void uninstallResourceFactories();

    skr::resource::STextureFactory*        textureFactory        = nullptr;
    skr::resource::STextureSamplerFactory* textureSamplerFactory = nullptr;
    skr::renderer::SMeshFactory*           meshFactory           = nullptr;
    skr::renderer::SShaderResourceFactory* shaderFactory         = nullptr;
    skr::renderer::SMaterialTypeFactory*   matTypeFactory        = nullptr;
    skr::renderer::SMaterialFactory*       matFactory            = nullptr;

    skr::resource::SAnimFactory*  animFactory     = nullptr;
    skr::resource::SSkelFactory*  skeletonFactory = nullptr;
    skr::resource::SSkinFactory*  skinFactory     = nullptr;
    skr::resource::SSceneFactory* sceneFactory    = nullptr;

    skr_vfs_t*             resource_vfs     = nullptr;
    skr_vfs_t*             tex_resource_vfs = nullptr;
    skr_vfs_t*             shader_bytes_vfs = nullptr;
    skr_io_ram_service_t*  ram_service      = nullptr;
    skr_io_vram_service_t* vram_service     = nullptr;
    skr_shader_map_t*      shadermap        = nullptr;

    skr::resource::SLocalResourceRegistry* registry;

    struct sugoi_storage_t* game_world         = nullptr;
    SRenderDeviceId         game_render_device = nullptr;
    SRendererId             game_renderer      = nullptr;
    CGPUSwapChainId         swapchain          = nullptr;
    CGPUFenceId             present_fence      = nullptr;
    // SWindowHandle           main_window        = nullptr;

    skr::SP<skr::JobQueue> job_queue = nullptr;

    skr::task::scheduler_t scheduler;
};
static SGameModule* g_game_module = nullptr;

IMPLEMENT_DYNAMIC_MODULE(SGameModule, Game);

void SGameModule::installResourceFactories()
{
    std::error_code ec             = {};
    auto            resourceRoot   = (skr::filesystem::current_path(ec) / "../resources");
    auto            u8ResourceRoot = resourceRoot.u8string();
    skr_vfs_desc_t  vfs_desc       = {};
    vfs_desc.mount_type            = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir    = u8ResourceRoot.c_str();
    resource_vfs                   = skr_create_vfs(&vfs_desc);

    auto ioServiceDesc       = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name       = u8"GameRuntime-RAMIOService";
    ioServiceDesc.sleep_time = 1000 / 60;
    ram_service              = skr_io_ram_service_t::create(&ioServiceDesc);
    ram_service->run();

    auto vramServiceDesc               = make_zeroed<skr_vram_io_service_desc_t>();
    vramServiceDesc.name               = u8"GameRuntime-VRAMIOService";
    vramServiceDesc.awake_at_request   = true;
    vramServiceDesc.ram_service        = ram_service;
    vramServiceDesc.callback_job_queue = job_queue.get();
    vramServiceDesc.use_dstorage       = true;
    vramServiceDesc.gpu_device         = game_render_device->get_cgpu_device();
    vram_service                       = skr_io_vram_service_t::create(&vramServiceDesc);
    vram_service->run();

    registry = SkrNew<skr::resource::SLocalResourceRegistry>(resource_vfs);
    skr::resource::GetResourceSystem()->Initialize(registry, ram_service);
    //

    using namespace skr::literals;
    auto resource_system = skr::resource::GetResourceSystem();

    auto gameResourceRoot = resourceRoot / "game";
    auto u8TextureRoot    = gameResourceRoot.u8string();
    // texture sampler factory
    {
        skr::resource::STextureSamplerFactory::Root factoryRoot = {};
        factoryRoot.device                                      = game_render_device->get_cgpu_device();
        textureSamplerFactory                                   = skr::resource::STextureSamplerFactory::Create(factoryRoot);
        resource_system->RegisterFactory(textureSamplerFactory);
    }
    // texture factory
    {
        skr_vfs_desc_t tex_vfs_desc     = {};
        tex_vfs_desc.mount_type         = SKR_MOUNT_TYPE_CONTENT;
        tex_vfs_desc.override_mount_dir = u8TextureRoot.c_str();
        tex_resource_vfs                = skr_create_vfs(&tex_vfs_desc);

        skr::resource::STextureFactory::Root factoryRoot = {};
        auto                                 RootStr     = gameResourceRoot.u8string();
        factoryRoot.dstorage_root                        = RootStr.c_str();
        factoryRoot.vfs                                  = tex_resource_vfs;
        factoryRoot.ram_service                          = ram_service;
        factoryRoot.vram_service                         = vram_service;
        factoryRoot.render_device                        = game_render_device;
        textureFactory                                   = skr::resource::STextureFactory::Create(factoryRoot);
        resource_system->RegisterFactory(textureFactory);
    }
    // mesh factory
    {
        skr::renderer::SMeshFactory::Root factoryRoot = {};
        auto                              RootStr     = gameResourceRoot.u8string();
        factoryRoot.dstorage_root                     = RootStr.c_str();
        factoryRoot.vfs                               = tex_resource_vfs;
        factoryRoot.ram_service                       = ram_service;
        factoryRoot.vram_service                      = vram_service;
        factoryRoot.render_device                     = game_render_device;
        meshFactory                                   = skr::renderer::SMeshFactory::Create(factoryRoot);
        resource_system->RegisterFactory(meshFactory);
    }
    // shader factory
    {
        const auto  backend    = game_render_device->get_backend();
        std::string shaderType = "invalid";
        if (backend == CGPU_BACKEND_D3D12) shaderType = "dxil";
        if (backend == CGPU_BACKEND_VULKAN) shaderType = "spirv";
        auto shaderResourceRoot   = gameResourceRoot / shaderType;
        auto u8ShaderResourceRoot = shaderResourceRoot.u8string();

        // create shader vfs
        skr_vfs_desc_t shader_vfs_desc     = {};
        shader_vfs_desc.mount_type         = SKR_MOUNT_TYPE_CONTENT;
        shader_vfs_desc.override_mount_dir = u8ShaderResourceRoot.c_str();
        shader_bytes_vfs                   = skr_create_vfs(&shader_vfs_desc);

        // create shader map
        skr_shader_map_root_t shadermapRoot = {};
        shadermapRoot.bytecode_vfs          = shader_bytes_vfs;
        shadermapRoot.ram_service           = ram_service;
        shadermapRoot.device                = game_render_device->get_cgpu_device();
        shadermapRoot.job_queue             = job_queue.get();
        shadermap                           = skr_shader_map_create(&shadermapRoot);

        // create shader resource factory
        skr::renderer::SShaderResourceFactory::Root factoryRoot = {};
        factoryRoot.render_device                               = game_render_device;
        factoryRoot.shadermap                                   = shadermap;
        shaderFactory                                           = skr::renderer::SShaderResourceFactory::Create(factoryRoot);
        resource_system->RegisterFactory(shaderFactory);
    }

    // material type factory
    {
        skr::renderer::SMaterialTypeFactory::Root factoryRoot = {};
        factoryRoot.render_device                             = game_render_device;
        matTypeFactory                                        = skr::renderer::SMaterialTypeFactory::Create(factoryRoot);
        resource_system->RegisterFactory(matTypeFactory);
    }

    // material factory
    {
        skr::renderer::SMaterialFactory::Root factoryRoot = {};
        factoryRoot.device                                = game_render_device->get_cgpu_device();
        factoryRoot.shader_map                            = shadermap;
        factoryRoot.job_queue                             = job_queue.get();
        factoryRoot.ram_service                           = ram_service;
        factoryRoot.bytecode_vfs                          = shader_bytes_vfs;
        matFactory                                        = skr::renderer::SMaterialFactory::Create(factoryRoot);
        resource_system->RegisterFactory(matFactory);
    }

    // anim factory
    {
        animFactory = SkrNew<skr::resource::SAnimFactory>();
        resource_system->RegisterFactory(animFactory);
    }

    // skeleton factory
    {
        skeletonFactory = SkrNew<skr::resource::SSkelFactory>();
        resource_system->RegisterFactory(skeletonFactory);
    }

    // skin factory
    {
        skinFactory = SkrNew<skr::resource::SSkinFactory>();
        resource_system->RegisterFactory(skinFactory);
    }

    struct GameSceneFactory : public skr::resource::SSceneFactory {
        virtual ESkrInstallStatus Install(skr_resource_record_t* record) override
        {
            auto renderableT_builder = make_zeroed<sugoi::TypeSetBuilder>();
            renderableT_builder.with<skr_render_effect_t>();
            auto renderableT               = make_zeroed<sugoi_entity_type_t>();
            renderableT.type               = renderableT_builder.build();
            auto detlaT                    = make_zeroed<sugoi_delta_type_t>();
            detlaT.added                   = renderableT;
            skr_scene_resource_t* scene    = (skr_scene_resource_t*)record->resource;
            auto                  callback = [&](sugoi_chunk_view_t* view) {
                auto callback2 = [&](sugoi_chunk_view_t* view, sugoi_chunk_view_t* oldView) {
                    skr_render_effect_attach(renderer, view, u8"ForwardEffect");
                };
                sugoiS_cast_view_delta(scene->storage, view, &detlaT, SUGOI_LAMBDA(callback2));
            };
            sugoiS_all(scene->storage, false, false, SUGOI_LAMBDA(callback));
            return SKR_INSTALL_STATUS_SUCCEED;
        }
        SRendererId renderer;
    };
    // scene factory
    {
        sceneFactory = SkrNew<skr::resource::SSceneFactory>();
        resource_system->RegisterFactory(sceneFactory);
    }
}

void SGameModule::uninstallResourceFactories()
{
    sugoiS_release(game_world);
    auto resource_system = skr::resource::GetResourceSystem();
    resource_system->Shutdown();

    skr::resource::STextureSamplerFactory::Destroy(textureSamplerFactory);
    skr::resource::STextureFactory::Destroy(textureFactory);
    skr::renderer::SMeshFactory::Destroy(meshFactory);
    skr::renderer::SShaderResourceFactory::Destroy(shaderFactory);
    skr::renderer::SMaterialTypeFactory::Destroy(matTypeFactory);
    skr::renderer::SMaterialFactory::Destroy(matFactory);

    SkrDelete(animFactory);
    SkrDelete(skeletonFactory);
    SkrDelete(skinFactory);
    SkrDelete(sceneFactory);

    skr_shader_map_free(shadermap);
    skr_free_renderer(game_renderer);

    skr::resource::GetResourceSystem()->Shutdown();
    SkrDelete(registry);

    skr_io_ram_service_t::destroy(ram_service);
    skr_io_vram_service_t::destroy(vram_service);
    skr_free_vfs(resource_vfs);
    skr_free_vfs(tex_resource_vfs);
    skr_free_vfs(shader_bytes_vfs);

    SKR_LOG_INFO(u8"game runtime unloaded!");
}

void SGameModule::on_load(int argc, char8_t** argv)
{
    SKR_LOG_INFO(u8"game runtime loaded!");

    if (!job_queue)
    {
        skr::String qn             = u8"GameJobQueue";
        auto        job_queueDesc  = make_zeroed<skr::JobQueueDesc>();
        job_queueDesc.thread_count = 2;
        job_queueDesc.priority     = SKR_THREAD_NORMAL;
        job_queueDesc.name         = qn.u8_str();
        job_queue                  = skr::SP<skr::JobQueue>::New(job_queueDesc);
    }
    SKR_ASSERT(job_queue);

    game_world         = sugoiS_create();
    game_render_device = skr_get_default_render_device();
    game_renderer      = skr_create_renderer(game_render_device, game_world);
    if (bUseJob)
    {
        scheduler.initialize(skr::task::scheudler_config_t{});
        scheduler.bind();
        sugoiJ_bind_storage(game_world);
    }
    installResourceFactories();
    g_game_module = this;
}

void create_test_scene(SRendererId renderer)
{
    // allocate 100 movable cubes
    auto renderableT_builder = make_zeroed<sugoi::TypeSetBuilder>();
    renderableT_builder
    .with<skr::TranslationComponent, skr::RotationComponent, skr::ScaleComponent>()
    .with<skr_index_comp_t, skr_movement_comp_t>()
    .with<skr_render_effect_t>()
    .with(SUGOI_COMPONENT_GUID);
    // allocate renderable
    auto renderableT   = make_zeroed<sugoi_entity_type_t>();
    renderableT.type   = renderableT_builder.build();
    uint32_t init_idx  = 0;
    auto     primSetup = [&](sugoi_chunk_view_t* view) {
        auto translations = sugoi::get_owned_rw<skr::TranslationComponent>(view);
        auto rotations    = sugoi::get_owned_rw<skr::RotationComponent>(view);
        auto scales       = sugoi::get_owned_rw<skr::ScaleComponent>(view);
        auto indices      = sugoi::get_owned_rw<skr_index_comp_t>(view);
        auto movements    = sugoi::get_owned_rw<skr_movement_comp_t>(view);
        auto states       = sugoi::get_owned_rw<game::anim_state_t>(view);
        auto guids        = (skr_guid_t*)sugoiV_get_owned_ro(view, SUGOI_COMPONENT_GUID);
        for (uint32_t i = 0; i < view->count; i++)
        {
            if (guids)
                sugoi_make_guid(&guids[i]);
            if (movements)
            {
                translations[i].value = { 0.f, 0.f, 0.f };
                rotations[i].euler    = { 0.f, 0.f, 0.f };
                scales[i].value       = { 8.f, 8.f, 8.f };
                if (indices) indices[i].value = init_idx++;
            }
            else
            {
                translations[i].value = { 0.f, 30.f, -10.f };
                rotations[i].euler    = { 90.f, 0.f, 0.f };
                scales[i].value       = { .25f, .25f, .25f };
            }
            if (states)
            {
                using namespace skr::literals;
                states[i].animation_resource = u8"83c0db0b-08cd-4951-b1c3-65c2008d0113"_guid;
                states[i].animation_resource.resolve(true, renderer->get_sugoi_storage());
            }
        }
        if (auto feature_arrs = sugoiV_get_owned_rw(view, sugoi_id_of<skr_render_effect_t>::get()))
        {
            if (movements)
                skr_render_effect_attach(renderer, view, u8"ForwardEffect");
            else
                skr_render_effect_attach(renderer, view, u8"ForwardEffectSkin");
        }
    };
    sugoiS_allocate_type(renderer->get_sugoi_storage(), &renderableT, 512, SUGOI_LAMBDA(primSetup));

    SKR_LOG_DEBUG(u8"Create Scene 0!");

    // allocate 1 player entity
    auto playerT_builder = make_zeroed<sugoi::TypeSetBuilder>();
    playerT_builder
    .with<skr::TranslationComponent, skr::RotationComponent, skr::ScaleComponent>()
    .with<skr_movement_comp_t>()
    .with<skr::CameraComponent>();
    auto playerT = make_zeroed<sugoi_entity_type_t>();
    playerT.type = playerT_builder.build();
    sugoiS_allocate_type(renderer->get_sugoi_storage(), &playerT, 1, SUGOI_LAMBDA(primSetup));

    SKR_LOG_DEBUG(u8"Create Scene 1!");

    // allocate 1 static(unmovable) gltf mesh
    auto static_renderableT_builderT = make_zeroed<sugoi::TypeSetBuilder>();
    static_renderableT_builderT
    .with<skr::TranslationComponent, skr::RotationComponent, skr::ScaleComponent>()
    .with<skr_render_effect_t, game::anim_state_t>();
    auto static_renderableT = make_zeroed<sugoi_entity_type_t>();
    static_renderableT.type = static_renderableT_builderT.build();
    sugoiS_allocate_type(renderer->get_sugoi_storage(), &static_renderableT, 1, SUGOI_LAMBDA(primSetup));

    SKR_LOG_DEBUG(u8"Create Scene 2!");
}

void async_attach_skin_mesh(SRendererId renderer)
{
    using namespace skr;

    auto filter          = make_zeroed<sugoi_filter_t>();
    auto meta            = make_zeroed<sugoi_meta_filter_t>();
    auto renderable_type = make_zeroed<sugoi::TypeSetBuilder>();
    renderable_type.with<skr_render_effect_t, skr::TranslationComponent>();
    auto static_type = make_zeroed<sugoi::TypeSetBuilder>();
    static_type.with<skr_movement_comp_t>();
    filter.all     = renderable_type.build();
    filter.none    = static_type.build();
    auto skin_type = make_zeroed<sugoi::TypeSetBuilder>();
    auto filter2   = make_zeroed<sugoi_filter_t>();
    filter2.all    = skin_type.with<renderer::MeshComponent, anim::SkinComponent, anim::SkeletonComponent>().build();
    auto attchFunc = [=](sugoi_chunk_view_t* view) {
        auto requestSetup = [=](sugoi_chunk_view_t* view) {
            using namespace skr::literals;

            auto mesh_comps = sugoi::get_owned_rw<renderer::MeshComponent>(view);
            auto skin_comps = sugoi::get_owned_rw<anim::SkinComponent>(view);
            auto skel_comps = sugoi::get_owned_rw<anim::SkeletonComponent>(view);
            // auto anim_comps = sugoi::get_owned_rw<anim::AnimComponent>(view);

            for (uint32_t i = 0; i < view->count; i++)
            {
                auto& mesh_comp = mesh_comps[i];
                auto& skin_comp = skin_comps[i];
                auto& skel_comp = skel_comps[i];
                // auto& anim_comp = anim_comps[i];
                mesh_comp.mesh_resource = u8"18db1369-ba32-4e91-aa52-b2ed1556f576"_guid;
                mesh_comp.mesh_resource.resolve(true, renderer->get_sugoi_storage());
                skin_comp.skin_resource = u8"40ce668a-d6bb-4134-b244-b0a7ac552245"_guid;
                skin_comp.skin_resource.resolve(true, renderer->get_sugoi_storage());
                skel_comp.skeleton = u8"d1acf969-91d6-4233-8d2b-33fca7c98a1c"_guid;
                skel_comp.skeleton.resolve(true, renderer->get_sugoi_storage());
            }
        };
        skr_render_effect_access(renderer, view, u8"ForwardEffectSkin", SUGOI_LAMBDA(requestSetup));
    };
    // 手动 sync skin mesh
    sugoiS_filter(renderer->get_sugoi_storage(), &filter2, &meta, nullptr, nullptr);
    sugoiS_filter(renderer->get_sugoi_storage(), &filter, &meta, SUGOI_LAMBDA(attchFunc));
}

void async_attach_render_mesh(SRendererId renderer)
{
    auto filter          = make_zeroed<sugoi_filter_t>();
    auto meta            = make_zeroed<sugoi_meta_filter_t>();
    auto renderable_type = make_zeroed<sugoi::TypeSetBuilder>();
    renderable_type.with<skr_render_effect_t, skr::TranslationComponent>();
    auto static_type = make_zeroed<sugoi::TypeSetBuilder>();
    static_type.with<skr_movement_comp_t>();
    filter.all     = renderable_type.build();
    filter.none    = static_type.build();
    auto skin_type = make_zeroed<sugoi::TypeSetBuilder>();
    auto filter2   = make_zeroed<sugoi_filter_t>();
    filter2.all    = skin_type.with<skr::renderer::MeshComponent>().build();
    auto attchFunc = [=](sugoi_chunk_view_t* view) {
        auto requestSetup = [=](sugoi_chunk_view_t* view) {
            using namespace skr::literals;
            auto mesh_comps = sugoi::get_owned_rw<skr::renderer::MeshComponent>(view);

            for (uint32_t i = 0; i < view->count; i++)
            {
                auto& mesh_comp         = mesh_comps[i];
                mesh_comp.mesh_resource = u8"79bb81eb-4e9f-4301-bf0c-a15b10a1cc3b"_guid;
                mesh_comp.mesh_resource.resolve(true, renderer->get_sugoi_storage());
            }
        };
        skr_render_effect_access(renderer, view, u8"ForwardEffectSkin", SUGOI_LAMBDA(requestSetup));
    };
    // 手动 sync mesh
    sugoiS_filter(renderer->get_sugoi_storage(), &filter2, &meta, nullptr, nullptr);
    sugoiS_filter(renderer->get_sugoi_storage(), &filter, &meta, SUGOI_LAMBDA(attchFunc));
}

void imgui_button_spawn_girl(SRendererId renderer)
{
    static bool onceGuard = true;
    if (onceGuard)
    {
        ImGui::Begin("AsyncMesh");
        if (ImGui::Button("LoadSkinMesh(AsResource)"))
        {
            async_attach_skin_mesh(renderer);
            onceGuard = false;
        }
        else if (ImGui::Button("LoadMesh(AsResource)"))
        {
            async_attach_render_mesh(renderer);
            onceGuard = false;
        }
        ImGui::End();
    }
}

SKR_EXTERN_C int luaopen_clonefunc(lua_State* L);
int              SGameModule::main_module_exec(int argc, char8_t** argv)
{
    SkrZoneScopedN("GameExecution");
    // auto moduleManager = skr_get_module_manager();
    SKR_LOG_INFO(u8"game executed as main module!");

    auto render_device = skr_get_default_render_device();
    auto cgpu_device   = render_device->get_cgpu_device();
    auto gfx_queue     = render_device->get_gfx_queue();
    auto window_desc   = make_zeroed<SWindowDescriptor>();
    window_desc.flags  = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE; // | SKR_WINDOW_BOARDLESS;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width  = BACK_BUFFER_WIDTH;
    main_window        = skr_create_window(
    skr::format(u8"Game [{}]", gCGPUBackendNames[cgpu_device->adapter->instance->backend]).u8_str(),
    &window_desc);
    // Initialize renderer
    swapchain     = skr_render_device_register_window(render_device, main_window);
    present_fence = cgpu_create_fence(cgpu_device);

    namespace render_graph = skr::render_graph;
    auto renderGraph       = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(cgpu_device)
        .with_gfx_queue(gfx_queue)
        .enable_memory_aliasing();
    });
    game_initialize_render_effects(game_renderer, renderGraph, resource_vfs);
    create_test_scene(game_renderer);
    // Lua
    auto L = skr_lua_newstate(resource_vfs);
    skr_lua_bind_imgui(L);
    skr_lua_open_game_runtime(L);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "game");
    auto GetStorage = +[](lua_State* L) -> int {
        lua_pushlightuserdata(L, g_game_module->game_world);
        return 1;
    };
    lua_pushcfunction(L, GetStorage, "GetStorage");
    lua_setfield(L, -2, "GetStorage");
    lua_pop(L, 1);
    if (skr_lua_loadfile(L, "main") != 0)
    {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            SKR_LOG_ERROR(u8"lua_pcall error: {}", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    lua_getglobal(L, "GameMain");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        SKR_LOG_ERROR(u8"lua_pcall error: {}", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    namespace res = skr::resource;
    using namespace skr::literals;
    res::TResourceHandle<skr_scene_resource_t> scene_handle = u8"FB84A5BD-2FD2-46A2-ABF4-2D2610CFDAD9"_guid;
    scene_handle.resolve(true, 0, SKR_REQUESTER_SYSTEM);

    // Viewport
    {
        auto viewport_manager = game_renderer->get_viewport_manager();
        viewport_manager->register_viewport(u8"main_viewport");
    }

    // Time
    SHiresTimer tick_timer;
    int64_t     elapsed_us    = 0;
    int64_t     elapsed_frame = 0;
    int64_t     fps           = 60;
    skr_init_hires_timer(&tick_timer);

    // loop
    bool               quit = false;
    skr::task::event_t pSkinCounter(nullptr);
    sugoi_query_t*     initAnimSkinQuery;
    sugoi_query_t*     skinQuery;
    sugoi_query_t*     moveQuery;
    sugoi_query_t*     cameraQuery;
    sugoi_query_t*     animQuery;
    moveQuery         = sugoiQ_from_literal(game_world,
                                            u8"[has]skr_movement_comp_t, [inout]skr::TranslationComponent, [in]skr::ScaleComponent, [in]skr_index_comp_t,!skr::CameraComponent");
    cameraQuery       = sugoiQ_from_literal(game_world,
                                            u8"[has]skr_movement_comp_t, [inout]skr::TranslationComponent, [inout]skr::CameraComponent");
    animQuery         = sugoiQ_from_literal(game_world,
                                            u8"[in]skr_render_effect_t, [in]game::anim_state_t, [out]<unseq>skr::anim::AnimComponent, [in]<unseq>skr::anim::SkeletonComponent");
    initAnimSkinQuery = sugoiQ_from_literal(game_world,
                                            u8"[inout]skr::anim::AnimComponent, [inout]skr::anim::SkinComponent, [in]skr::renderer::MeshComponent, [in]skr::anim::SkeletonComponent");
    skinQuery         = sugoiQ_from_literal(game_world,
                                            u8"[in]skr::anim::AnimComponent, [inout]skr::anim::SkinComponent, [in]skr::renderer::MeshComponent, [in]skr::anim::SkeletonComponent");

    // auto handler = skr_system_get_default_handler();
    // handler->add_window_close_handler(
    // +[](SWindowHandle window, void* pQuit) {
    //     bool& quit = *(bool*)pQuit;
    //     quit       = true;
    // },
    // &quit);
    // handler->add_window_resize_handler(
    // +[](SWindowHandle window, int32_t w, int32_t h, void* usr_data) {
    //     auto _this = (SGameModule*)usr_data;
    //     if (window != _this->main_window) return;

    //     cgpu_wait_queue_idle(_this->game_render_device->get_gfx_queue());
    //     cgpu_wait_fences(&_this->present_fence, 1);
    //     _this->swapchain = skr_render_device_recreate_window_swapchain(_this->game_render_device, window);
    // },
    // this);
    skr_imgui_initialize(handler);

    while (!quit)
    {
        FrameMark;
        SkrZoneScopedN("LoopBody");
        static auto main_thread_id    = skr_current_thread_id();
        auto        current_thread_id = skr_current_thread_id();
        SKR_ASSERT(main_thread_id == current_thread_id && "This is not the main thread");

        float delta = 1.f / 60.f;
        {
            SkrZoneScopedN("PollEvent");
            handler->pump_messages(delta);
            handler->process_messages(delta);
        }

        {
            SkrZoneScopedN("sugoiJ GC");
            sugoiJ_gc();
        }
        int64_t us        = skr_hires_timer_get_usec(&tick_timer, true);
        double  deltaTime = (double)us / 1000 / 1000;
        elapsed_us += us;
        elapsed_frame += 1;
        if (elapsed_us > (1000 * 1000))
        {
            fps           = elapsed_frame;
            elapsed_frame = 0;
            elapsed_us    = 0;
        }

        // Update resources
        auto resource_system = skr::resource::GetResourceSystem();
        resource_system->Update();

        // Update camera
        auto cameraUpdate = [this](sugoi_chunk_view_t* view) {
            auto cameras = sugoi::get_owned_rw<skr::CameraComponent>(view);
            for (uint32_t i = 0; i < view->count; i++)
            {
                const auto pInfo           = swapchain->back_buffers[0]->info;
                cameras[i].renderer        = game_renderer;
                cameras[i].viewport_id     = 0u; // TODO: viewport id
                cameras[i].viewport_width  = (uint32_t)pInfo->width;
                cameras[i].viewport_height = (uint32_t)pInfo->height;
            }
        };
        sugoiQ_get_views(cameraQuery, SUGOI_LAMBDA(cameraUpdate));

        // Input
        if (auto scene = scene_handle.get_resolved())
        {
            SkrZoneScopedN("MergeScene");
            sugoiS_merge(game_renderer->get_sugoi_storage(), scene->storage);
            scene_handle.reset();
        }
        {
            SkrZoneScopedN("ImGUI");

            skr_imgui_new_frame(main_window, (float)deltaTime);
            {
                ImGui::Begin("Information");
                ImGui::Text("RenderFPS: %d", (uint32_t)fps);
                ImGui::End();
            }
            {
                // ImGui::Begin("Lua");
                // if (ImGui::Button("Hotfix"))
                //{
                //     if (luaL_dostring(L, "local module = require \"hotfix\"; module.reload({\"game\"})") != LUA_OK)
                //     {
                //         SKR_LOG_ERROR(u8"luaL_dostring error: %s", lua_tostring(L, -1));
                //         lua_pop(L, 1);
                //     }
                // }
                // ImGui::End();
            }
            {
                ImGui::Begin("Scene");
                if (ImGui::Button("Save"))
                {
                    skr::archive::JsonWriter writer(5);
                    skr_save_scene(game_renderer->get_sugoi_storage(), &writer);
                    auto file = skr_vfs_fopen(resource_vfs, u8"scene.json", SKR_FM_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
                    if (file)
                    {
                        auto str = writer.Str();
                        skr_vfs_fwrite(file, str.u8_str(), 0, str.size());
                        skr_vfs_fclose(file);
                    }
                }
                ImGui::End();
            }
            imgui_button_spawn_girl(game_renderer);
            // skr::inspect::update_value_inspector(); // FIXME. inspector
            // quit |= skg::GameLoop(ctx);
        }
        {
            SkrZoneScopedN("Lua");
            lua_getglobal(L, "GameUpdate");
            if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            {
                SKR_LOG_ERROR(u8"lua_pcall error: %s", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }

        // move
        // [has]skr_movement_comp_t, [inout]skr::TranslationComponent, [in]skr::ScaleComponent, [in]skr_index_comp_t, !skr::CameraComponent
        if (bUseJob)
        {
            SkrZoneScopedN("MoveSystem");
            auto timer     = clock();
            auto total_sec = (double)timer / CLOCKS_PER_SEC;

            auto moveJob = SkrNewLambda(
            [=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
                SkrZoneScopedN("MoveJob");

                float lerps[]      = { 12.5, 20 };
                auto  translations = (skr::TranslationComponent*)sugoiV_get_owned_rw_local(view, localTypes[0]);
                auto  scales       = (skr::ScaleComponent*)sugoiV_get_owned_ro_local(view, localTypes[1]);
                (void)scales;
                auto indices = (skr_index_comp_t*)sugoiV_get_owned_ro_local(view, localTypes[2]);
                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto actual_idx = indices[i].value;

                    auto lscale           = (float)abs(sin(total_sec * 0.5));
                    lscale                = (float)lerp(lerps[0], lerps[1], lscale);
                    const auto col        = (actual_idx % 10);
                    const auto row        = (actual_idx / 10);
                    translations[i].value = {
                        ((float)col - 4.5f) * lscale,
                        ((float)row - 4.5f) * lscale + 50.f,
                        0.f
                    };
                }
            });
            sugoiJ_schedule_ecs(moveQuery, 1024, SUGOI_LAMBDA_POINTER(moveJob), nullptr, nullptr);
        }

        // sync all jobs here ?
        {
            // SkrZoneScopedN("sugoiJSync");
            // sugoiJ_wait_all_jobs();
        }

        // [inout]skr::anim::AnimComponent, [in]game::anim_state_t, [in]skr::anim::SkeletonComponent
        {
            SkrZoneScopedN("AnimSystem");
            auto animJob = SkrNewLambda([=, this](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
                SkrZoneScopedN("AnimJob");
                auto     states     = (game::anim_state_t*)sugoiV_get_owned_ro_local(view, localTypes[1]);
                uint32_t g_id       = 0;
                auto     syncEffect = [&](sugoi_chunk_view_t* view) {
                    auto anims = sugoi::get_owned_rw<skr::anim::AnimComponent>(view);
                    auto skels = sugoi::get_component_ro<skr::anim::SkeletonComponent>(view);
                    for (uint32_t i = 0; i < view->count; ++i, ++g_id)
                    {
                        auto& anim              = anims[i];
                        auto& skel              = skels[i];
                        auto& state             = states[g_id];
                        auto  skeleton_resource = skel.skeleton.get_resolved();
                        if (!skeleton_resource)
                            continue;
                        if (anim.buffers.empty())
                            continue;
                        if (state.sampling_context.max_tracks() == 0)
                        {
                            SkrZoneScopedN("InitializeAnimState");
                            game::InitializeAnimState(&state, skeleton_resource);
                        }
                        {
                            SkrZoneScopedN("UpdateAnimState");
                            game::UpdateAnimState(&state, skeleton_resource, (float)deltaTime, &anim);
                        }
                    }
                };
                skr_render_effect_access(game_renderer, view, u8"ForwardEffectSkin", SUGOI_LAMBDA(syncEffect));
            });
            sugoiJ_schedule_ecs(animQuery, 128, SUGOI_LAMBDA_POINTER(animJob), nullptr, nullptr);
        }
        {
            SkrZoneScopedN("SkinSystem");

            auto initAnimSkinComps = [&](sugoi_chunk_view_t* r_cv) {
                const auto meshes = sugoi::get_component_ro<skr::renderer::MeshComponent>(r_cv);
                const auto skels  = sugoi::get_component_ro<skr::anim::SkeletonComponent>(r_cv);
                const auto anims  = sugoi::get_owned_rw<skr::anim::AnimComponent>(r_cv);
                const auto skins  = sugoi::get_owned_rw<skr::anim::SkinComponent>(r_cv);

                SkrZoneScopedN("InitializeAnimSkinComponents");
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    const auto mesh_resource = meshes[i].mesh_resource.get_resolved();
                    const auto skel_resource = skels[i].skeleton.get_resolved();
                    const auto skin_resource = skins[i].skin_resource.get_resolved();
                    if (!mesh_resource || !skel_resource || !skin_resource) continue;

                    if (skins[i].joint_remaps.empty())
                    {
                        skr_init_skin_component(&skins[i], skel_resource);
                    }
                    if (anims[i].buffers.empty())
                    {
                        skr_init_anim_component(&anims[i], mesh_resource, skel_resource);
                    }
                    skr_init_anim_buffers(cgpu_device, &anims[i], mesh_resource);
                }
            };
            {
                // prepare skin mesh resources for rendering
                sugoiQ_sync(initAnimSkinQuery);
                sugoiQ_get_views(initAnimSkinQuery, SUGOI_LAMBDA(initAnimSkinComps));
            }

            // wait last skin dispatch
            if (pSkinCounter)
                pSkinCounter.wait(true);

            // skin dispatch for the frame
            auto cpuSkinJob = SkrNewLambda(
            [&](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
                const auto meshes = sugoi::get_component_ro<skr::renderer::MeshComponent>(view);
                const auto anims  = sugoi::get_component_ro<skr::anim::AnimComponent>(view);
                auto       skins  = sugoi::get_owned_rw<skr::anim::SkinComponent>(view);

                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto mesh_resource = meshes[i].mesh_resource.get_resolved();
                    if (!mesh_resource)
                        continue;
                    if (!skins[i].joint_remaps.empty() && !anims[i].buffers.empty())
                    {
                        SkrZoneScopedN("CPU Skin");

                        skr_cpu_skin(skins + i, anims + i, mesh_resource);
                    }
                }
            });
            sugoiJ_schedule_ecs(skinQuery, 4, SUGOI_LAMBDA_POINTER(cpuSkinJob), nullptr, &pSkinCounter);
        }
        // [has]skr_movement_comp_t, [inout]skr::TranslationComponent, [in]skr::CameraComponent
        if (bUseJob)
        {
            SkrZoneScopedN("PlayerSystem");

            auto playerJob = SkrNewLambda([=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
                SkrZoneScopedN("PlayerJob");

                auto translations = (skr::TranslationComponent*)sugoiV_get_owned_rw_local(view, localTypes[0]);
                auto forward      = skr_float3_t{ 0.f, 1.f, 0.f };
                auto right        = skr_float3_t{ 1.f, 0.f, 0.f };
                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto kSpeed = 15.f;
                    auto       qdown  = skr_key_down(EKeyCode::KEY_CODE_Q);
                    auto       edown  = skr_key_down(EKeyCode::KEY_CODE_E);
                    auto       wdown  = skr_key_down(EKeyCode::KEY_CODE_W);
                    auto       sdown  = skr_key_down(EKeyCode::KEY_CODE_S);
                    auto       adown  = skr_key_down(EKeyCode::KEY_CODE_A);
                    auto       ddown  = skr_key_down(EKeyCode::KEY_CODE_D);

                    if (edown) translations[i].value.z += (float)deltaTime * kSpeed;
                    if (qdown) translations[i].value.z -= (float)deltaTime * kSpeed;

                    using namespace skr::scalar_math;
                    if (wdown) translations[i].value = forward * (float)deltaTime * kSpeed + translations[i].value;
                    if (sdown) translations[i].value = -1.f * forward * (float)deltaTime * kSpeed + translations[i].value;
                    if (adown) translations[i].value = -1.f * right * (float)deltaTime * kSpeed + translations[i].value;
                    if (ddown) translations[i].value = 1.f * right * (float)deltaTime * kSpeed + translations[i].value;
                }
            });
            sugoiJ_schedule_ecs(cameraQuery, 128, SUGOI_LAMBDA_POINTER(playerJob), nullptr, nullptr);
        }

        // resolve camera to viewports
        auto viewport_manager = game_renderer->get_viewport_manager();
        skr_resolve_cameras_to_viewport(viewport_manager, game_world);

        // register passes
        {
            SkrZoneScopedN("RegisterPasses");
            game_register_render_effects(game_renderer, renderGraph);
        }

        // early render jobs. When the main timeline is doing present jobs, we can do some work in parallel
        auto back_buffer = renderGraph->create_texture(
        [=, this](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
            builder.set_name(u8"backbuffer")
            .import(swapchain->back_buffers[backbuffer_index], CGPU_RESOURCE_STATE_UNDEFINED)
            .allow_render_target();
        });
        {
            SkrZoneScopedN("RenderScene");
            skr_renderer_render_frame(game_renderer, renderGraph);
        }

        // present, blocks the main timeline, early render jobs can take their time
        static bool bHasFrameToPresent = false;
        if (bHasFrameToPresent)
        {
            SkrZoneScopedN("QueuePresentSwapchain");
            // present
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index                      = backbuffer_index;
            present_desc.swapchain                  = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
            // render_graph_imgui_present_sub_viewports();
        }
        else
        {
            bHasFrameToPresent = true;
        }

        // render graph setup & compile & exec
        {
            SkrZoneScopedN("RenderIMGUI");
            // render_graph_imgui_add_render_pass(renderGraph, back_buffer, CGPU_LOAD_ACTION_LOAD);
        }

        // blit backbuffer & present
        auto present_pass = renderGraph->add_present_pass(
        [=, this](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
            builder.set_name(u8"present_pass")
            .swapchain(swapchain, backbuffer_index)
            .texture(back_buffer, true);
        });

        // compile render graph
        {
            SkrZoneScopedN("CompileRenderGraph");
            renderGraph->compile();
        }

        // acquire & reimport underlying buffer just before graph execution
        // prevents the main timeline from blocking by the acquire image call
        {
            SkrZoneScopedN("AcquireFrame");

            // acquire frame
            cgpu_wait_fences(&present_fence, 1);
            CGPUAcquireNextDescriptor acquire_desc = {};
            acquire_desc.fence                     = present_fence;
            backbuffer_index                       = cgpu_acquire_next_image(swapchain, &acquire_desc);
        }
        CGPUTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];
        bool          reimported        = renderGraph->resolve(back_buffer)->reimport(native_backbuffer);
        reimported &= static_cast<skr::render_graph::PresentPassNode*>(renderGraph->resolve(present_pass))->reimport(swapchain, backbuffer_index);

        SKR_ASSERT(reimported && "Failed to reimport backbuffer");

        // execute render graph
        {
            SkrZoneScopedN("ExecuteRenderGraph");
            auto frame_index = renderGraph->execute();
            {
                SkrZoneScopedN("CollectGarbage");
                if ((frame_index > (RG_MAX_FRAME_IN_FLIGHT * 10)) && (frame_index % (RG_MAX_FRAME_IN_FLIGHT * 10) == 0))
                    renderGraph->collect_garbage(frame_index - 10 * RG_MAX_FRAME_IN_FLIGHT);
            }
        }

        // gc
        {
            shadermap->garbage_collect(15);
        }
    }
    // clean up
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    render_graph::RenderGraph::destroy(renderGraph);
    game_finalize_render_effects(game_renderer, renderGraph);
    // render_graph_imgui_finalize();
    skr_free_window(main_window);
    return 0;
}

void SGameModule::on_unload()
{
    g_game_module = nullptr;
    SKR_LOG_INFO(u8"game unloaded!");
    if (bUseJob)
    {
        sugoiJ_unbind_storage(game_world);
        scheduler.unbind();
    }
    uninstallResourceFactories();
}