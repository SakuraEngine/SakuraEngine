#include "gamert.h"
#include "EASTL/shared_ptr.h"
#include "gainput/GainputInputDevicePad.h"
#include "platform/configure.h"
#include "ghc/filesystem.hpp"
#include "platform/memory.h"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.h"
#include "ecs/dual.h"
#include "../../cgpu/common/utils.h"
#include "ftl/task.h"
#include "ftl/task_scheduler.h"
#include "ghc/filesystem.hpp"
#include "platform/time.h"
#include "platform/window.h"
#include "resource/local_resource_registry.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "imgui/imgui.h"
#include "resource/resource_system.h"
#include "skr_input/Interactions.h"
#include "skr_input/InteractionsType.h"
#include "utils/make_zeroed.hpp"
#include "skr_scene/scene.h"
#include "skr_renderer/skr_renderer.h"
#include "gainput/gainput.h"
#include "gainput/GainputInputDeviceKeyboard.h"
#include "gainput/GainputInputDeviceMouse.h"
#include "gamert.h"
#include "ecs/callback.hpp"
#include "ecs/type_builder.hpp"
#include "skr_input/inputSystem.h"
#include "skr_renderer/render_mesh.h"
#include "math/vector.hpp"

#include "tracy/Tracy.hpp"

SWindowHandle window;
uint32_t backbuffer_index;
extern void create_imgui_resources(skr::render_graph::RenderGraph* renderGraph);
extern void initialize_render_effects(skr::render_graph::RenderGraph* renderGraph);
extern void finalize_render_effects(skr::render_graph::RenderGraph* renderGraph);
SKR_IMPORT_API struct dual_storage_t* skr_runtime_get_dual_storage();
#define lerp(a, b, t) (a) + (t) * ((b) - (a))

class SGameModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;
};

IMPLEMENT_DYNAMIC_MODULE(SGameModule, Game);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "Game",
    "prettyname" : "Game",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"GameRT", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
Game)

void SGameModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("game runtime loaded!");
}

void create_test_scene()
{
    // allocate 100 movable cubes
    auto renderableT_builder = make_zeroed<dual::type_builder_t>();
    renderableT_builder
        .with<skr_translation_t, skr_rotation_t, skr_scale_t, skr_movement_t>()
        .with<skr_render_effect_t>();
    // allocate renderable
    auto renderableT = make_zeroed<dual_entity_type_t>();
    renderableT.type = renderableT_builder.build();
    auto primSetup = [&](dual_chunk_view_t* view) {
        auto translations = (skr_translation_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_t>::get());
        auto rotations = (skr_rotation_t*)dualV_get_owned_ro(view, dual_id_of<skr_rotation_t>::get());
        auto scales = (skr_scale_t*)dualV_get_owned_ro(view, dual_id_of<skr_scale_t>::get());
        auto movements = (skr_movement_t*)dualV_get_owned_ro(view, dual_id_of<skr_movement_t>::get());
        for (uint32_t i = 0; i < view->count; i++)
        {
            if (movements)
            {
                translations[i].value = {
                    (float)(i % 10) * 1.5f, ((float)i / 10) * 1.5f + 50.f, 0.f
                };
                rotations[i].euler = { 0.f, 0.f, 0.f };
                scales[i].value = { 8.f, 8.f, 8.f };
            }
            else
            {
                translations[i].value = { 0.f, 0.f, 0.f };
                rotations[i].euler = { 3.14159f * 1.5f, 0.f, 0.f };
                scales[i].value = { 1.f, 1.f, 1.f };
            }
        }
        auto renderer = skr_renderer_get_renderer();
        skr_render_effect_attach(renderer, view, "ForwardEffect");
    };
    dualS_allocate_type(skr_runtime_get_dual_storage(), &renderableT, 100, DUAL_LAMBDA(primSetup));

    // allocate 1 static(unmovable) gltf mesh
    auto static_renderableT_builderT = make_zeroed<dual::type_builder_t>();
    static_renderableT_builderT
        .with<skr_translation_t, skr_rotation_t, skr_scale_t>()
        .with<skr_render_effect_t>();
    // allocate renderable
    auto static_renderableT = make_zeroed<dual_entity_type_t>();
    static_renderableT.type = static_renderableT_builderT.build();
    dualS_allocate_type(skr_runtime_get_dual_storage(), &static_renderableT, 1, DUAL_LAMBDA(primSetup));
}

void attach_mesh_on_static_ents(skr_io_ram_service_t* ram_service, skr_io_vram_service_t* vram_service, const char* path, skr_render_mesh_request_t* request)
{
    auto filter = make_zeroed<dual_filter_t>();
    auto meta = make_zeroed<dual_meta_filter_t>();
    auto renderable_type = make_zeroed<dual::type_builder_t>();
    renderable_type.with<skr_render_effect_t, skr_translation_t>();
    auto static_type = make_zeroed<dual::type_builder_t>();
    static_type.with<skr_movement_t>();
    filter.all = renderable_type.build();
    filter.none = static_type.build();
    auto attchFunc = [=](dual_chunk_view_t* view) {
        auto ents = (dual_entity_t*)dualV_get_entities(view);
        auto requestSetup = [=](dual_chunk_view_t* view) {
            auto mesh_comps = (skr_render_mesh_comp_t*)dualV_get_owned_rw(view, dual_id_of<skr_render_mesh_comp_t>::get());
            mesh_comps->async_request = *request;
            skr_render_mesh_create_from_gltf(ram_service, vram_service, path, &mesh_comps->async_request);
        };
        skr_render_effect_access(skr_renderer_get_renderer(), ents, view->count, "ForwardEffect", DUAL_LAMBDA(requestSetup));
    };
    dualS_query(skr_runtime_get_dual_storage(), &filter, &meta, DUAL_LAMBDA(attchFunc));
}

void imgui_button_spawn_girl()
{
    static bool onceGuard  = true;
    if (onceGuard)
    {
        auto girl_mesh_request = make_zeroed<skr_render_mesh_request_t>();
        ImGui::Begin(u8"AsyncMesh");
        auto dstroage_queue = skr_renderer_get_file_dstorage_queue();
        auto resource_vfs = skr_game_runtime_get_vfs();
        auto ram_service = skr_game_runtime_get_ram_service();
        auto vram_service = skr_renderer_get_vram_service();
        girl_mesh_request.mesh_name = "girl";
        if (dstroage_queue && ImGui::Button(u8"LoadMesh(DirectStorage[Disk])"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.dstorage_queue_override = dstroage_queue;
            girl_mesh_request.dstorage_source = CGPU_DSTORAGE_SOURCE_FILE;
            attach_mesh_on_static_ents(ram_service, vram_service, "scene.gltf", &girl_mesh_request);
            onceGuard = false;
        }
        else if (dstroage_queue && ImGui::Button(u8"LoadMesh(DirectStorage[Memory])"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.dstorage_queue_override = dstroage_queue;
            girl_mesh_request.dstorage_source = CGPU_DSTORAGE_SOURCE_MEMORY;
            attach_mesh_on_static_ents(ram_service, vram_service, "scene.gltf", &girl_mesh_request);
            onceGuard = false;
        }
        else if (ImGui::Button(u8"LoadMesh(CopyQueue)"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.queue_override = skr_renderer_get_cpy_queue();
            attach_mesh_on_static_ents(ram_service, vram_service, "scene.gltf", &girl_mesh_request);
            onceGuard = false;
        }
        else if (ImGui::Button(u8"LoadMesh(GraphicsQueue)"))
        {
            girl_mesh_request.mesh_resource_request.vfs_override = resource_vfs;
            girl_mesh_request.queue_override = skr_renderer_get_gfx_queue();
            attach_mesh_on_static_ents(ram_service, vram_service, "scene.gltf", &girl_mesh_request);
            onceGuard = false;
        }
        ImGui::End();  
    }
}

int SGameModule::main_module_exec(int argc, char** argv)
{
    SKR_LOG_INFO("game executed as main module!");
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
        return -1;
    auto cgpuDevice = skr_renderer_get_cgpu_device();
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.centered = true;
    window_desc.resizable = true;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    window = skr_create_window(
    fmt::format("Game [{}]", gCGPUBackendNames[cgpuDevice->adapter->instance->backend]).c_str(),
    &window_desc);
    // Initialize renderer
    auto swapchain = skr_renderer_register_window(window);
    auto present_fence = cgpu_create_fence(skr_renderer_get_cgpu_device());
    namespace render_graph = skr::render_graph;
    auto renderGraph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(skr_renderer_get_cgpu_device())
            .with_gfx_queue(skr_renderer_get_gfx_queue())
            .enable_memory_aliasing();
    });
    initialize_render_effects(renderGraph);
    create_test_scene();
    create_imgui_resources(renderGraph);
    // Initialize Input
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    using namespace skr::input;
    using namespace gainput;
    InputSystem inputSystem;
    inputSystem.Init(window);
    inputSystem.SetDisplaySize(BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT);
    // InputAction
    {
        auto action = eastl::make_shared<InputAction<float>>();
        auto controls1 = eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeySpace);
        controls1->AddInteraction(eastl::make_shared<InteractionTap<float>>());
        action->AddControl(controls1);
        action->ListenEvent([](float value, ControlsBase<float>* _c, Interaction* i, Interaction::EvendId eventId)
        {
            SKR_LOG_DEBUG("Tap_Float %f", value);
        });
        inputSystem.AddInputAction(action);
    }
    {
        auto action = eastl::make_shared<InputAction<float>>();
        auto controls1 = eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyP);
        auto interactionPress = eastl::make_shared<InteractionPress<float>>(PressBehavior::PressAndRelease, 0.5f);
        controls1->AddInteraction(interactionPress);
        action->AddControl(controls1);
        action->ListenEvent([interactionPress](float value, ControlsBase<float>* _c, Interaction* i, Interaction::EvendId eventId)
        {
            if(interactionPress.get() == i)
            {
                if(((InteractionPress<float>*)i)->GetPressEventType(eventId) == PressEventType::Press)
                    SKR_LOG_DEBUG("Press_Float Press %f", value);
                else
                    SKR_LOG_DEBUG("Press_Float Release %f", value);
            }
        });
        inputSystem.AddInputAction(action);
    }
    {
        auto action = eastl::make_shared<InputAction<skr::math::Vector2f>>();
        auto controls1 = eastl::make_shared<Vector2Control>();
        controls1->Bind(
            Vector2Control::ButtonDirection{
                eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyW),
                eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyS),
                eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyA),
                eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_KEYBOARD, KeyD),
            }
        );
        controls1->Bind(
            Vector2Control::StickDirection{
                eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_PAD, PadButtonLeftStickX),
                eastl::make_shared<ControlsFloat>(InputDevice::DeviceType::DT_PAD, PadButtonLeftStickY),
            }
        );
        auto interactionPress = eastl::make_shared<InteractionPress<skr::math::Vector2f>>(PressBehavior::PressAndRelease, 0.5f);
        controls1->AddInteraction(interactionPress);
        action->AddControl(controls1);
        action->ListenEvent([interactionPress](skr::math::Vector2f value, ControlsBase<skr::math::Vector2f>* _c, Interaction* i, Interaction::EvendId eventId)
        {
            if(interactionPress.get() == i)
            {
                if(((InteractionPress<skr::math::Vector2f>*)i)->GetPressEventType(eventId) == PressEventType::Press)
                    SKR_LOG_DEBUG("Press_Float Press    x:%f,y:%f", value.X, value.Y);
                else
                    SKR_LOG_DEBUG("Press_Float Release  x:%f,y:%f", value.X, value.Y);
            }
        });
        inputSystem.AddInputAction(action);
    }
    // Time
    STimer timer;
    skr_init_timer(&timer);
    // loop
    bool quit = false;
    skg::GameContext ctx;
    while (!quit)
    {
        FrameMark
        ZoneScopedN("LoopBody");

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ZoneScopedN("PollEvent");

            if (event.type == SDL_SYSWMEVENT)
            {
                SDL_SysWMmsg* msg = event.syswm.msg;
#if defined(GAINPUT_PLATFORM_WIN)
                inputSystem.GetHardwareManager().HandleMessage((MSG&)msg->msg);
#elif defined(GAINPUT_PLATFORM_LINUX)
                inputSystem.GetHardwareManager().HandleMessage((XEvent&)msg->msg);
#endif
            }
            if (event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }
        // Input
        {
            ZoneScopedN("Input");
        
            const double deltaTime = skr_timer_get_seconds(&timer, true);
            inputSystem.Update(deltaTime);
        }
        {
            ZoneScopedN("ImGUI");

            auto& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(
            (float)swapchain->back_buffers[0]->width,
            (float)swapchain->back_buffers[0]->height);
            skr_imgui_new_frame(window, 1.f / 60.f);
            imgui_button_spawn_girl();
            quit |= skg::GameLoop(ctx);
        }
        // move
        {
            ZoneScopedN("MoveSystem");

            auto filter = make_zeroed<dual_filter_t>();
            auto meta = make_zeroed<dual_meta_filter_t>();
            auto movable_type = make_zeroed<dual::type_builder_t>();
            filter.all = movable_type.with<skr_movement_t, skr_translation_t>().build();
            float lerps[] = { 12.5, 20 };
            auto timer = clock();
            auto total_sec = (double)timer / CLOCKS_PER_SEC;
            auto moveFunc = [&](dual_chunk_view_t* view) {
                auto translations = (skr_translation_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_t>::get());
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto lscale = (float)abs(sin(total_sec * 0.5));
                    lscale = (float)lerp(lerps[0], lerps[1], lscale);
                    translations[i].value = {
                        ((float)(i % 10) - 4.5f) * lscale,
                        ((float)(i / 10) - 4.5f) * lscale + 50.f, 
                        0.f
                    };
                }
            };
            dualS_query(skr_runtime_get_dual_storage(), &filter, &meta, DUAL_LAMBDA(moveFunc));
        }
        {
            ZoneScopedN("AcquireFrame");

            // acquire frame
            cgpu_wait_fences(&present_fence, 1);
            CGPUAcquireNextDescriptor acquire_desc = {};
            acquire_desc.fence = present_fence;
            backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        }
        // render graph setup & compile & exec
        CGPUTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];
        auto back_buffer = renderGraph->create_texture(
        [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
            builder.set_name("backbuffer")
            .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
            .allow_render_target();
        });
        {
            ZoneScopedN("RenderScene");
            skr_renderer_render_frame(renderGraph);
        }
        {
            ZoneScopedN("RenderIMGUI");
            render_graph_imgui_add_render_pass(renderGraph, back_buffer, CGPU_LOAD_ACTION_LOAD);
        }
        renderGraph->add_present_pass(
        [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
            builder.set_name("present_pass")
            .swapchain(swapchain, backbuffer_index)
            .texture(back_buffer, true);
        });
        {
            ZoneScopedN("CompileRenderGraph");
            renderGraph->compile();
        }
        {
            ZoneScopedN("ExecuteRenderGraph");
            auto frame_index = renderGraph->execute();
            {
                ZoneScopedN("CollectGarbage");
                if (frame_index >= RG_MAX_FRAME_IN_FLIGHT)
                    renderGraph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT);
            }
        }
        {
            ZoneScopedN("QueuePresentSwapchain");
            // present
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(skr_renderer_get_gfx_queue(), &present_desc);
        }
    }
    // clean up
    cgpu_wait_queue_idle(skr_renderer_get_gfx_queue());
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    finalize_render_effects(renderGraph);
    render_graph::RenderGraph::destroy(renderGraph);
    render_graph_imgui_finalize();
    skr_free_window(window);
    SDL_Quit();
    return 0;
}

void SGameModule::on_unload()
{
    SKR_LOG_INFO("game unloaded!");
}