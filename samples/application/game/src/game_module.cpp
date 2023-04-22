#include "common/utils.h"
#include <EASTL/shared_ptr.h>
#include "GameRuntime/gamert.h"
#include "utils/format.hpp"
#include "utils/make_zeroed.hpp"
#include "platform/filesystem.hpp"
#include "platform/system.h"
#include "platform/configure.h"
#include "platform/memory.h"
#include "platform/time.h"
#include "platform/guid.hpp"
#include "platform/window.h"

#include "ecs/type_builder.hpp"
#include "json/writer.h"

#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"

#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#include "resource/resource_system.h"

#include "SkrScene/scene.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrRenderer/render_effect.h"

#include <containers/text.hpp>
#include "task/task.hpp"

#include "resource/local_resource_registry.hpp"
#include "SkrRenderer/shader_map.h"
#include "SkrRenderer/render_viewport.h"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrAnim/resources/animation_resource.h"
#include "SkrAnim/resources/skeleton_resource.h"
#include "SkrAnim/resources/skin_resource.h"
#include "SkrAnim/components/skin_component.h"
#include "SkrAnim/components/skeleton_component.h"
#include "GameRuntime/game_animation.h"

#include "tracy/Tracy.hpp"
#include "utils/types.h"
#include "SkrInspector/inspect_value.h"

#include "SkrScene/resources/scene_resource.h"

#include "lua/skr_lua.h"

uint32_t backbuffer_index;
extern void create_imgui_resources(skr_vfs_t* resource_vfs, SRenderDeviceId render_device, skr::render_graph::RenderGraph* renderGraph);
extern void game_initialize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* resource_vfs);
extern void game_register_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
extern void game_finalize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph);
#define lerp(a, b, t) (a) + (t) * ((b) - (a))

const bool bUseJob = true;

class SGameModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

    void installResourceFactories();
    void uninstallResourceFactories();

    skr::resource::STextureFactory* textureFactory = nullptr;
    skr::resource::STextureSamplerFactory* textureSamplerFactory = nullptr;
    skr::renderer::SMeshFactory* meshFactory = nullptr;
    skr::renderer::SShaderResourceFactory* shaderFactory = nullptr;
    skr::renderer::SMaterialTypeFactory* matTypeFactory = nullptr;
    skr::renderer::SMaterialFactory* matFactory = nullptr;

    skr::resource::SAnimFactory* animFactory = nullptr;
    skr::resource::SSkelFactory* skeletonFactory = nullptr;
    skr::resource::SSkinFactory* skinFactory = nullptr;
    skr::resource::SSceneFactory* sceneFactory = nullptr;

    skr_vfs_t* resource_vfs = nullptr;
    skr_vfs_t* tex_resource_vfs = nullptr;
    skr_vfs_t* shader_bytes_vfs = nullptr;
    skr_io_ram_service_t* ram_service = nullptr;
    skr_shader_map_t* shadermap = nullptr;

    skr::resource::SLocalResourceRegistry* registry;

    struct dual_storage_t* game_world = nullptr;
    SRenderDeviceId game_render_device = nullptr;
    SRendererId game_renderer = nullptr;
    CGPUSwapChainId swapchain = nullptr;
    CGPUFenceId present_fence = nullptr;
    SWindowHandle main_window = nullptr;

    skr::task::scheduler_t scheduler;
};
static SGameModule* g_game_module = nullptr;

IMPLEMENT_DYNAMIC_MODULE(SGameModule, Game);

void SGameModule::installResourceFactories()
{
    std::error_code ec = {};
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources");
    auto u8ResourceRoot = resourceRoot.u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = u8ResourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = u8"GameRuntimeRAMIOService";
    ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
    ram_service = skr_io_ram_service_t::create(&ioServiceDesc);

    registry = SkrNew<skr::resource::SLocalResourceRegistry>(resource_vfs);
    skr::resource::GetResourceSystem()->Initialize(registry, ram_service);
    //

    using namespace skr::guid::literals;
    auto resource_system = skr::resource::GetResourceSystem();

    auto gameResourceRoot = resourceRoot / "game";
    auto u8TextureRoot = gameResourceRoot.u8string();
    // texture sampler factory
    {
        skr::resource::STextureSamplerFactory::Root factoryRoot = {};
        factoryRoot.device = game_render_device->get_cgpu_device();
        textureSamplerFactory = skr::resource::STextureSamplerFactory::Create(factoryRoot);
        resource_system->RegisterFactory(textureSamplerFactory);
    }
    // texture factory
    {
        skr_vfs_desc_t tex_vfs_desc = {};
        tex_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
        tex_vfs_desc.override_mount_dir = u8TextureRoot.c_str();
        tex_resource_vfs = skr_create_vfs(&tex_vfs_desc);

        skr::resource::STextureFactory::Root factoryRoot = {};
        auto RootStr = gameResourceRoot.u8string();
        factoryRoot.dstorage_root = RootStr.c_str();
        factoryRoot.vfs = tex_resource_vfs;
        factoryRoot.ram_service = ram_service;
        factoryRoot.vram_service = game_render_device->get_vram_service();
        factoryRoot.render_device = game_render_device;
        textureFactory = skr::resource::STextureFactory::Create(factoryRoot);
        resource_system->RegisterFactory(textureFactory);
    }
    // mesh factory
    {
        skr::renderer::SMeshFactory::Root factoryRoot = {};
        auto RootStr = gameResourceRoot.u8string();
        factoryRoot.dstorage_root = RootStr.c_str();
        factoryRoot.vfs = tex_resource_vfs;
        factoryRoot.ram_service = ram_service;
        factoryRoot.vram_service = game_render_device->get_vram_service();
        factoryRoot.render_device = game_render_device;
        meshFactory = skr::renderer::SMeshFactory::Create(factoryRoot);
        resource_system->RegisterFactory(meshFactory);
    }
    // shader factory
    {
        const auto backend = game_render_device->get_backend();
        std::string shaderType = "invalid";
        if (backend == CGPU_BACKEND_D3D12) shaderType = "dxil";
        if (backend == CGPU_BACKEND_VULKAN) shaderType = "spirv";
        auto shaderResourceRoot = gameResourceRoot / shaderType;
        auto u8ShaderResourceRoot = shaderResourceRoot.u8string();

        // create shader vfs
        skr_vfs_desc_t shader_vfs_desc = {};
        shader_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
        shader_vfs_desc.override_mount_dir = u8ShaderResourceRoot.c_str();
        shader_bytes_vfs = skr_create_vfs(&shader_vfs_desc);

        // create shader map
        skr_shader_map_root_t shadermapRoot = {};
        shadermapRoot.bytecode_vfs = shader_bytes_vfs;
        shadermapRoot.ram_service = ram_service;
        shadermapRoot.device = game_render_device->get_cgpu_device();
        shadermapRoot.aux_service = game_render_device->get_aux_service(0);
        shadermap = skr_shader_map_create(&shadermapRoot);

        // create shader resource factory
        skr::renderer::SShaderResourceFactory::Root factoryRoot = {};
        factoryRoot.render_device = game_render_device;
        factoryRoot.shadermap = shadermap;
        shaderFactory = skr::renderer::SShaderResourceFactory::Create(factoryRoot);
        resource_system->RegisterFactory(shaderFactory);
    }

    // material type factory
    {
        skr::renderer::SMaterialTypeFactory::Root factoryRoot = {};
        factoryRoot.render_device = game_render_device;
        matTypeFactory = skr::renderer::SMaterialTypeFactory::Create(factoryRoot);
        resource_system->RegisterFactory(matTypeFactory);
    }

    // material factory
    {
        skr::renderer::SMaterialFactory::Root factoryRoot = {};
        factoryRoot.device = game_render_device->get_cgpu_device();
        factoryRoot.aux_service = game_render_device->get_aux_service(0);
        factoryRoot.ram_service = ram_service;
        factoryRoot.bytecode_vfs = shader_bytes_vfs;
        matFactory = skr::renderer::SMaterialFactory::Create(factoryRoot);
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
            auto renderableT_builder = make_zeroed<dual::type_builder_t>();
            renderableT_builder.with<skr_render_effect_t>();
            auto renderableT = make_zeroed<dual_entity_type_t>();
            renderableT.type = renderableT_builder.build();
            auto detlaT = make_zeroed<dual_delta_type_t>();
            detlaT.added = renderableT;
            skr_scene_resource_t* scene = (skr_scene_resource_t*)record->resource;
            auto callback = [&](dual_chunk_view_t* view) {
                auto callback2 = [&](dual_chunk_view_t* view, dual_chunk_view_t* oldView) {
                    skr_render_effect_attach(renderer, view, "ForwardEffect");
                };
                dualS_cast_view_delta(scene->storage, view, &detlaT, DUAL_LAMBDA(callback2));
            };
            dualS_all(scene->storage, false, false, DUAL_LAMBDA(callback));
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
    dualS_release(game_world);
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
    skr_free_vfs(resource_vfs);
    skr_free_vfs(tex_resource_vfs);
    skr_free_vfs(shader_bytes_vfs);

    SKR_LOG_INFO("game runtime unloaded!");
}

void SGameModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("game runtime loaded!");

    game_world = dualS_create();
    game_render_device = skr_get_default_render_device();
    game_renderer = skr_create_renderer(game_render_device, game_world);
    if (bUseJob)
    {
        scheduler.initialize(skr::task::scheudler_config_t{});
        scheduler.bind();
        dualJ_bind_storage(game_world);
    }
    installResourceFactories();
    g_game_module = this;
}

void create_test_scene(SRendererId renderer)
{
    // allocate 100 movable cubes
    auto renderableT_builder = make_zeroed<dual::type_builder_t>();
    renderableT_builder
    .with<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t>()
    .with<skr_index_comp_t, skr_movement_comp_t>()
    .with<skr_render_effect_t>()
    .with(DUAL_COMPONENT_GUID);
    // allocate renderable
    auto renderableT = make_zeroed<dual_entity_type_t>();
    renderableT.type = renderableT_builder.build();
    uint32_t init_idx = 0;
    auto primSetup = [&](dual_chunk_view_t* view) {
        auto translations = dual::get_owned_rw<skr_translation_comp_t>(view);
        auto rotations = dual::get_owned_rw<skr_rotation_comp_t>(view);
        auto scales = dual::get_owned_rw<skr_scale_comp_t>(view);
        auto indices = dual::get_owned_rw<skr_index_comp_t>(view);
        auto movements = dual::get_owned_rw<skr_movement_comp_t>(view);
        auto states = dual::get_owned_rw<game::anim_state_t>(view);
        auto guids = (skr_guid_t*)dualV_get_owned_ro(view, DUAL_COMPONENT_GUID);
        for (uint32_t i = 0; i < view->count; i++)
        {
            if(guids)
                dual_make_guid(&guids[i]);
            if (movements)
            {
                translations[i].value = { 0.f, 0.f, 0.f };
                rotations[i].euler = { 0.f, 0.f, 0.f };
                scales[i].value = { 8.f, 8.f, 8.f };
                if (indices) indices[i].value = init_idx++;
            }
            else
            {
                translations[i].value = { 0.f, 30.f, -10.f };
                rotations[i].euler = { 90.f, 0.f, 0.f };
                scales[i].value = { .25f, .25f, .25f };
            }
            if (states)
            {
                using namespace skr::guid::literals;
                states[i].animation_resource = "83c0db0b-08cd-4951-b1c3-65c2008d0113"_guid;
                states[i].animation_resource.resolve(true, renderer->get_dual_storage());
            }
        }
        if (auto feature_arrs = dualV_get_owned_rw(view, dual_id_of<skr_render_effect_t>::get()))
        {
            if (movements)
                skr_render_effect_attach(renderer, view, "ForwardEffect");
            else
                skr_render_effect_attach(renderer, view, "ForwardEffectSkin");
        }
    };
    dualS_allocate_type(renderer->get_dual_storage(), &renderableT, 512, DUAL_LAMBDA(primSetup));

    SKR_LOG_DEBUG("Create Scene 0!");

    // allocate 1 player entity
    auto playerT_builder = make_zeroed<dual::type_builder_t>();
    playerT_builder
        .with<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t>()
        .with<skr_movement_comp_t>()
        .with<skr_camera_comp_t>();
    auto playerT = make_zeroed<dual_entity_type_t>();
    playerT.type = playerT_builder.build();
    dualS_allocate_type(renderer->get_dual_storage(), &playerT, 1, DUAL_LAMBDA(primSetup));

    SKR_LOG_DEBUG("Create Scene 1!");

    // allocate 1 static(unmovable) gltf mesh
    auto static_renderableT_builderT = make_zeroed<dual::type_builder_t>();
    static_renderableT_builderT
        .with<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t>()
        .with<skr_render_effect_t, game::anim_state_t>();
    auto static_renderableT = make_zeroed<dual_entity_type_t>();
    static_renderableT.type = static_renderableT_builderT.build();
    dualS_allocate_type(renderer->get_dual_storage(), &static_renderableT, 1, DUAL_LAMBDA(primSetup));

    SKR_LOG_DEBUG("Create Scene 2!");
}

void async_attach_skin_mesh(SRendererId renderer)
{
    auto filter = make_zeroed<dual_filter_t>();
    auto meta = make_zeroed<dual_meta_filter_t>();
    auto renderable_type = make_zeroed<dual::type_builder_t>();
    renderable_type.with<skr_render_effect_t, skr_translation_comp_t>();
    auto static_type = make_zeroed<dual::type_builder_t>();
    static_type.with<skr_movement_comp_t>();
    filter.all = renderable_type.build();
    filter.none = static_type.build();
    auto attchFunc = [=](dual_chunk_view_t* view) {
        auto requestSetup = [=](dual_chunk_view_t* view) {
            using namespace skr::guid::literals;
            auto mesh_comps = dual::get_owned_rw<skr_render_mesh_comp_t>(view);
            auto skin_comps = dual::get_owned_rw<skr_render_skin_comp_t>(view);
            auto skel_comps = dual::get_owned_rw<skr_render_skel_comp_t>(view);
            // auto anim_comps = dual::get_owned_rw<skr_render_anim_comp_t>(view);

            for (uint32_t i = 0; i < view->count; i++)
            {
                auto& mesh_comp = mesh_comps[i];
                auto& skin_comp = skin_comps[i];
                auto& skel_comp = skel_comps[i];
                // auto& anim_comp = anim_comps[i];
                mesh_comp.mesh_resource = "18db1369-ba32-4e91-aa52-b2ed1556f576"_guid;
                mesh_comp.mesh_resource.resolve(true, renderer->get_dual_storage());
                skin_comp.skin_resource = "40ce668a-d6bb-4134-b244-b0a7ac552245"_guid;
                skin_comp.skin_resource.resolve(true, renderer->get_dual_storage());
                skel_comp.skeleton = "d1acf969-91d6-4233-8d2b-33fca7c98a1c"_guid;
                skel_comp.skeleton.resolve(true, renderer->get_dual_storage());
            }
        };
        skr_render_effect_access(renderer, view, "ForwardEffectSkin", DUAL_LAMBDA(requestSetup));
    };
    dualS_query(renderer->get_dual_storage(), &filter, &meta, DUAL_LAMBDA(attchFunc));
}

void async_attach_render_mesh(SRendererId renderer)
{
    auto filter = make_zeroed<dual_filter_t>();
    auto meta = make_zeroed<dual_meta_filter_t>();
    auto renderable_type = make_zeroed<dual::type_builder_t>();
    renderable_type.with<skr_render_effect_t, skr_translation_comp_t>();
    auto static_type = make_zeroed<dual::type_builder_t>();
    static_type.with<skr_movement_comp_t>();
    filter.all = renderable_type.build();
    filter.none = static_type.build();
    auto attchFunc = [=](dual_chunk_view_t* view) {
        auto requestSetup = [=](dual_chunk_view_t* view) {
            using namespace skr::guid::literals;
            auto mesh_comps = dual::get_owned_rw<skr_render_mesh_comp_t>(view);

            for (uint32_t i = 0; i < view->count; i++)
            {
                auto& mesh_comp = mesh_comps[i];
                mesh_comp.mesh_resource = "79bb81eb-4e9f-4301-bf0c-a15b10a1cc3b"_guid;
                mesh_comp.mesh_resource.resolve(true, renderer->get_dual_storage());
            }
        };
        skr_render_effect_access(renderer, view, "ForwardEffectSkin", DUAL_LAMBDA(requestSetup));
    };
    dualS_query(renderer->get_dual_storage(), &filter, &meta, DUAL_LAMBDA(attchFunc));
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

RUNTIME_EXTERN_C int luaopen_clonefunc(lua_State* L);
int SGameModule::main_module_exec(int argc, char** argv)
{
    ZoneScopedN("GameExecution");
    // auto moduleManager = skr_get_module_manager();
    SKR_LOG_INFO("game executed as main module!");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return -1;

    auto render_device = skr_get_default_render_device();
    auto cgpu_device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE; // | SKR_WINDOW_BOARDLESS;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    main_window = skr_create_window(
        skr::text::format(u8"Game [{}]", gCGPUBackendNames[cgpu_device->adapter->instance->backend]).u8_str(),
        &window_desc);
    // Initialize renderer
    swapchain = skr_render_device_register_window(render_device, main_window);
    present_fence = cgpu_create_fence(cgpu_device);
    
    namespace render_graph = skr::render_graph;
    auto renderGraph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(cgpu_device)
        .with_gfx_queue(gfx_queue)
        .enable_memory_aliasing();
    });
    game_initialize_render_effects(game_renderer, renderGraph, resource_vfs);
    create_test_scene(game_renderer);
    create_imgui_resources(resource_vfs, render_device, renderGraph);
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
    lua_pushcfunction(L, GetStorage);
    lua_setfield(L, -2, "GetStorage");
    lua_pop(L, 1);
    if (luaL_dostring(L, "local module = require \"game\"; module:init()") != LUA_OK)
    {
        SKR_LOG_ERROR("luaL_dostring error: {}", lua_tostring(L, -1));
    }
    namespace res = skr::resource;
    res::TResourceHandle<skr_scene_resource_t> scene_handle = skr::guid::make_guid_unsafe("FB84A5BD-2FD2-46A2-ABF4-2D2610CFDAD9");
    scene_handle.resolve(true, 0, SKR_REQUESTER_SYSTEM);

    // Viewport
    {
        auto viewport_manager = game_renderer->get_viewport_manager();
        viewport_manager->register_viewport("main_viewport");
    }
    
    // Time
    SHiresTimer tick_timer;
    int64_t elapsed_us = 0;
    int64_t elapsed_frame = 0;
    int64_t fps = 60;
    skr_init_hires_timer(&tick_timer);

    // loop
    bool quit = false;
    skr::task::event_t pSkinCounter(nullptr);
    dual_query_t* initAnimSkinQuery;
    dual_query_t* skinQuery;
    dual_query_t* moveQuery;
    dual_query_t* cameraQuery;
    dual_query_t* animQuery;
    moveQuery = dualQ_from_literal(game_world,
        "[has]skr_movement_comp_t, [inout]skr_translation_comp_t, [in]skr_scale_comp_t, [in]skr_index_comp_t, !skr_camera_comp_t");
    cameraQuery = dualQ_from_literal(game_world,
        "[has]skr_movement_comp_t, [inout]skr_translation_comp_t, [inout]skr_camera_comp_t");
    animQuery = dualQ_from_literal(game_world,
        "[in]skr_render_effect_t, [in]game::anim_state_t, [out]<unseq>skr_render_anim_comp_t, [in]<unseq>skr_render_skel_comp_t");
    initAnimSkinQuery = dualQ_from_literal(game_world, 
        "[inout]skr_render_anim_comp_t, [inout]skr_render_skin_comp_t, [in]skr_render_mesh_comp_t, [in]skr_render_skel_comp_t");
    skinQuery = dualQ_from_literal(game_world, 
        "[in]skr_render_anim_comp_t, [inout]skr_render_skin_comp_t, [in]skr_render_mesh_comp_t, [in]skr_render_skel_comp_t");

    auto handler = skr_system_get_default_handler();
    handler->add_window_close_handler(
        +[](SWindowHandle window, void* pQuit) {
            bool& quit = *(bool*)pQuit;
            quit = true;
        }, &quit);
    handler->add_window_resize_handler(
        +[](SWindowHandle window, int32_t w, int32_t h, void* usr_data) {
            auto _this = (SGameModule*)usr_data;
            if (window != _this->main_window) return;

            cgpu_wait_queue_idle(_this->game_render_device->get_gfx_queue());
            cgpu_wait_fences(&_this->present_fence, 1);
            _this->swapchain = skr_render_device_recreate_window_swapchain(_this->game_render_device, window);
        }, this);
    skr_imgui_initialize(handler);

    while (!quit)
    {
        FrameMark;
        ZoneScopedN("LoopBody");
        static auto main_thread_id = skr_current_thread_id();
        auto current_thread_id = skr_current_thread_id();
        SKR_ASSERT(main_thread_id == current_thread_id && "This is not the main thread");

        float delta = 1.f / 60.f;
        {
            ZoneScopedN("PollEvent");
            handler->pump_messages(delta);
            handler->process_messages(delta);
        }

        {
            ZoneScopedN("dualJ GC");
            dualJ_gc();
        }
        int64_t us = skr_hires_timer_get_usec(&tick_timer, true);
        double deltaTime = (double)us / 1000 / 1000;
        elapsed_us += us;
        elapsed_frame += 1;
        if (elapsed_us > (1000 * 1000))
        {
            fps = elapsed_frame;
            elapsed_frame = 0;
            elapsed_us = 0;
        }
        
        // Update resources
        auto resource_system = skr::resource::GetResourceSystem();
        resource_system->Update();

        // Update camera
        auto cameraUpdate = [=](dual_chunk_view_t* view) {
            auto cameras = dual::get_owned_rw<skr_camera_comp_t>(view);
            for (uint32_t i = 0; i < view->count; i++)
            {
                cameras[i].renderer = game_renderer;
                cameras[i].viewport_id = 0u; // TODO: viewport id
                cameras[i].viewport_width = swapchain->back_buffers[0]->width;
                cameras[i].viewport_height = swapchain->back_buffers[0]->height;
            }
        };
        dualQ_get_views(cameraQuery, DUAL_LAMBDA(cameraUpdate));

        // Input
        if (auto scene = scene_handle.get_resolved())
        {
            ZoneScopedN("MergeScene");
            dualS_merge(game_renderer->get_dual_storage(), scene->storage);
            scene_handle.reset();
        }
        {
            ZoneScopedN("ImGUI");

            skr_imgui_new_frame(main_window, (float)deltaTime);
            {
                ImGui::Begin("Information");
                ImGui::Text("RenderFPS: %d", (uint32_t)fps);
                ImGui::End();
            }
            {
                ImGui::Begin("Lua");
                if (ImGui::Button("Hotfix"))
                {
                    if (luaL_dostring(L, "local module = require \"hotfix\"; module.reload({\"game\"})") != LUA_OK)
                    {
                        SKR_LOG_ERROR("luaL_dostring error: %s", lua_tostring(L, -1));
                        lua_pop(L, 1);
                    }
                }
                ImGui::End();
            }
            {
                ImGui::Begin("Scene");
                if (ImGui::Button("Save"))
                {
                    skr_json_writer_t writer(5);
                    skr_save_scene(game_renderer->get_dual_storage(), &writer);
                    auto file = skr_vfs_fopen(resource_vfs, u8"scene.json", SKR_FM_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
                    if (file)
                    {
                        auto str = writer.Str();
                        skr_vfs_fwrite(file, str.data(), 0, str.length());
                        skr_vfs_fclose(file);
                    }
                }
                ImGui::End();
            }
            imgui_button_spawn_girl(game_renderer);
            skr::inspect::update_value_inspector();
            // quit |= skg::GameLoop(ctx);
        }
        {
            ZoneScopedN("Lua");
            if (luaL_dostring(L, "local module = require \"game\"; module:update()") != LUA_OK)
            {
                SKR_LOG_ERROR("luaL_dostring error: %s", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }

        // move
        // [has]skr_movement_comp_t, [inout]skr_translation_comp_t, [in]skr_scale_comp_t, [in]skr_index_comp_t, !skr_camera_comp_t
        if (bUseJob)
        {
            ZoneScopedN("MoveSystem");
            auto timer = clock();
            auto total_sec = (double)timer / CLOCKS_PER_SEC;

            auto moveJob = SkrNewLambda(
                [=](dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex) {
                ZoneScopedN("MoveJob");

                float lerps[] = { 12.5, 20 };
                auto translations = (skr_translation_comp_t*)dualV_get_owned_rw_local(view, localTypes[0]);
                auto scales = (skr_scale_comp_t*)dualV_get_owned_ro_local(view, localTypes[1]);
                (void)scales;
                auto indices = (skr_index_comp_t*)dualV_get_owned_ro_local(view, localTypes[2]);
                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto actual_idx = indices[i].value;

                    auto lscale = (float)abs(sin(total_sec * 0.5));
                    lscale = (float)lerp(lerps[0], lerps[1], lscale);
                    const auto col = (actual_idx % 10);
                    const auto row = (actual_idx / 10);
                    translations[i].value = {
                        ((float)col - 4.5f) * lscale,
                        ((float)row - 4.5f) * lscale + 50.f,
                        0.f
                    };
                }
            });
            dualJ_schedule_ecs(moveQuery, 1024, DUAL_LAMBDA_POINTER(moveJob), nullptr, nullptr);
        }

        // sync all jobs here ?
        {
            // ZoneScopedN("DualJSync");
            // dualJ_wait_all();
        }

        // [inout]skr_render_anim_comp_t, [in]game::anim_state_t, [in]skr_render_skel_comp_t
        {
            ZoneScopedN("AnimSystem");
            auto animJob = SkrNewLambda([=](dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex) {
                ZoneScopedN("AnimJob");
                auto states = (game::anim_state_t*)dualV_get_owned_ro_local(view, localTypes[1]);
                uint32_t g_id = 0;
                auto syncEffect = [&](dual_chunk_view_t* view) {
                    auto anims = dual::get_owned_rw<skr_render_anim_comp_t>(view);
                    auto skels = dual::get_component_ro<skr_render_skel_comp_t>(view);
                    for (uint32_t i = 0; i < view->count; ++i, ++g_id)
                    {
                        auto& anim = anims[i];
                        auto& skel = skels[i];
                        auto& state = states[g_id];
                        auto skeleton_resource = skel.skeleton.get_resolved();
                        if (!skeleton_resource)
                            continue;
                        if (anim.buffers.empty())
                            continue;
                        if (state.sampling_context.max_tracks() == 0)
                        {
                            ZoneScopedN("InitializeAnimState");
                            game::InitializeAnimState(&state, skeleton_resource);
                        }
                        {
                            ZoneScopedN("UpdateAnimState");
                            game::UpdateAnimState(&state, skeleton_resource, (float)deltaTime, &anim);
                        }
                    }
                };
                skr_render_effect_access(game_renderer, view, "ForwardEffectSkin", DUAL_LAMBDA(syncEffect));
            });
            dualJ_schedule_ecs(animQuery, 128, DUAL_LAMBDA_POINTER(animJob), nullptr, nullptr);
        }
        {
            ZoneScopedN("SkinSystem");

            auto initAnimSkinComps = [&](dual_chunk_view_t* r_cv) {
                const auto meshes = dual::get_component_ro<skr_render_mesh_comp_t>(r_cv);
                const auto skels = dual::get_component_ro<skr_render_skel_comp_t>(r_cv);
                const auto anims = dual::get_owned_rw<skr_render_anim_comp_t>(r_cv);
                const auto skins = dual::get_owned_rw<skr_render_skin_comp_t>(r_cv);
                
                ZoneScopedN("InitializeAnimSkinComponents");
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    const auto mesh_resource = meshes[i].mesh_resource.get_resolved();
                    const auto skel_resource = skels[i].skeleton.get_resolved();
                    const auto skin_resource = skins[i].skin_resource.get_resolved();
                    if (!mesh_resource || !skel_resource || !skin_resource) continue;

                    if(skins[i].joint_remaps.empty())
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
                dualQ_get_views(initAnimSkinQuery, DUAL_LAMBDA(initAnimSkinComps));
            }

            // wait last skin dispatch
            if(pSkinCounter)
                pSkinCounter.wait(true);
                
            // skin dispatch for the frame
            auto cpuSkinJob = SkrNewLambda(
                [&](dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex) {
                const auto meshes = dual::get_component_ro<skr_render_mesh_comp_t>(view);
                const auto anims = dual::get_component_ro<skr_render_anim_comp_t>(view);
                auto skins = dual::get_owned_rw<skr_render_skin_comp_t>(view);
                
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto mesh_resource = meshes[i].mesh_resource.get_resolved();
                    if(!mesh_resource)
                        continue;
                    if (!skins[i].joint_remaps.empty() && !anims[i].buffers.empty())
                    {
                        ZoneScopedN("CPU Skin");

                        skr_cpu_skin(skins + i, anims + i, mesh_resource);
                    }
                }
            });
            dualJ_schedule_ecs(skinQuery, 4, DUAL_LAMBDA_POINTER(cpuSkinJob), nullptr, &pSkinCounter);
        }
        // [has]skr_movement_comp_t, [inout]skr_translation_comp_t, [in]skr_camera_comp_t
        if (bUseJob)
        {
            ZoneScopedN("PlayerSystem");

            auto playerJob = SkrNewLambda([=](dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex) {
                ZoneScopedN("PlayerJob");

                auto translations = (skr_translation_comp_t*)dualV_get_owned_rw_local(view, localTypes[0]);
                auto forward = skr_float3_t{ 0.f, 1.f, 0.f };
                auto right = skr_float3_t{ 1.f, 0.f, 0.f };
                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto kSpeed = 15.f;
                    auto qdown = skr_key_down(EKeyCode::KEY_CODE_Q);
                    auto edown = skr_key_down(EKeyCode::KEY_CODE_E);
                    auto wdown = skr_key_down(EKeyCode::KEY_CODE_W);
                    auto sdown = skr_key_down(EKeyCode::KEY_CODE_S);
                    auto adown = skr_key_down(EKeyCode::KEY_CODE_A);
                    auto ddown = skr_key_down(EKeyCode::KEY_CODE_D);

                    if (edown) translations[i].value.z += (float)deltaTime * kSpeed;
                    if (qdown) translations[i].value.z -= (float)deltaTime * kSpeed;

                    using namespace skr::scalar_math;
                    if (wdown) translations[i].value = forward * (float)deltaTime * kSpeed + translations[i].value;
                    if (sdown) translations[i].value = -1.f * forward * (float)deltaTime * kSpeed + translations[i].value;
                    if (adown) translations[i].value = -1.f * right * (float)deltaTime * kSpeed + translations[i].value;
                    if (ddown) translations[i].value = 1.f * right * (float)deltaTime * kSpeed + translations[i].value;
                }
            });
            dualJ_schedule_ecs(cameraQuery, 128, DUAL_LAMBDA_POINTER(playerJob), nullptr, nullptr);
        }

        // resolve camera to viewports
        auto viewport_manager = game_renderer->get_viewport_manager();
        skr_resolve_cameras_to_viewport(viewport_manager, game_world);

        // register passes
        {
            ZoneScopedN("RegisterPasses");
            game_register_render_effects(game_renderer, renderGraph);
        }

        // early render jobs. When the main timeline is doing present jobs, we can do some work in parallel
        auto back_buffer = renderGraph->create_texture(
        [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
            builder.set_name(u8"backbuffer")
                .import(swapchain->back_buffers[backbuffer_index], CGPU_RESOURCE_STATE_UNDEFINED)
                .allow_render_target();
        });
        {
            ZoneScopedN("RenderScene");
            skr_renderer_render_frame(game_renderer, renderGraph);
        }

        // present, blocks the main timeline, early render jobs can take their time
        static bool bHasFrameToPresent = false;
        if (bHasFrameToPresent)
        {
            ZoneScopedN("QueuePresentSwapchain");
            // present
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
            render_graph_imgui_present_sub_viewports();
        }
        else
        {
            bHasFrameToPresent = true;
        }

        // render graph setup & compile & exec
        {
            ZoneScopedN("RenderIMGUI");
            render_graph_imgui_add_render_pass(renderGraph, back_buffer, CGPU_LOAD_ACTION_LOAD);
        }

        // blit backbuffer & present
        auto present_pass = renderGraph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name(u8"present_pass")
                    .swapchain(swapchain, backbuffer_index)
                    .texture(back_buffer, true);
            });
        
        // compile render graph
        {
            ZoneScopedN("CompileRenderGraph");
            renderGraph->compile();
        }

        // acquire & reimport underlying buffer just before graph execution
        // prevents the main timeline from blocking by the acquire image call
        {
            ZoneScopedN("AcquireFrame");

            // acquire frame
            cgpu_wait_fences(&present_fence, 1);
            CGPUAcquireNextDescriptor acquire_desc = {};
            acquire_desc.fence = present_fence;
            backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        }
        CGPUTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];
        bool reimported = renderGraph->resolve(back_buffer)->reimport(native_backbuffer);
        reimported &= static_cast<skr::render_graph::PresentPassNode*>(renderGraph->resolve(present_pass))->reimport(swapchain, backbuffer_index);

        SKR_ASSERT(reimported && "Failed to reimport backbuffer");

        // execute render graph
        {
            ZoneScopedN("ExecuteRenderGraph");
            auto frame_index = renderGraph->execute();
            {
                ZoneScopedN("CollectGarbage");
                if ((frame_index > (RG_MAX_FRAME_IN_FLIGHT * 10)) && (frame_index % (RG_MAX_FRAME_IN_FLIGHT * 10) == 0))
                    renderGraph->collect_garbage(frame_index - 10 * RG_MAX_FRAME_IN_FLIGHT);
            }
        }
    }
    // clean up
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    render_graph::RenderGraph::destroy(renderGraph);
    game_finalize_render_effects(game_renderer, renderGraph);
    render_graph_imgui_finalize();
    skr_free_window(main_window);
    SDL_Quit();
    return 0;
}

void SGameModule::on_unload()
{
    g_game_module = nullptr;
    SKR_LOG_INFO("game unloaded!");
    if (bUseJob)
    {
        dualJ_unbind_storage(game_world);
        scheduler.unbind();
    }
    uninstallResourceFactories();
}