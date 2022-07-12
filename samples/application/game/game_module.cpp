#include "gamert.h"
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
#include "platform/window.h"
#include "resource/local_resource_registry.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "imgui/imgui.h"
#include "resource/resource_system.h"
#include "utils/make_zeroed.hpp"
#include "skr_scene/scene.h"
#include "skr_renderer/skr_renderer.h"
#include "gainput/gainput.h"
#include "gainput/GainputInputDeviceKeyboard.h"
#include "gainput/GainputInputDeviceMouse.h"
#include "gamert.h"
#include "ecs/callback.hpp"
#include "ecs/type_builder.hpp"

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
    auto renderableT_builder = make_zeroed<dual::type_builder_t>();
    renderableT_builder.with<skr_translation_t>();
    renderableT_builder.with<skr_rotation_t>();
    renderableT_builder.with<skr_scale_t>();
    renderableT_builder.with<skr_render_effect_t>();
    // allocate renderable
    auto renderableT = make_zeroed<dual_entity_type_t>();
    renderableT.type = renderableT_builder.build();
    auto primSetup = [&](dual_chunk_view_t* view) {
        auto translations = (skr_translation_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_t>::get());
        auto rotations = (skr_rotation_t*)dualV_get_owned_ro(view, dual_id_of<skr_rotation_t>::get());
        auto scales = (skr_scale_t*)dualV_get_owned_ro(view, dual_id_of<skr_scale_t>::get());
        for (uint32_t i = 0; i < view->count; i++)
        {
            translations[i].value = {
                (float)(i % 10) * 1.5f, ((float)i / 10) * 1.5f, 0.f
            };
            rotations[i].euler = { 0.f, 0.f, 0.f };
            scales[i].value = { 1.f, 1.f, 1.f };
        }
        auto renderer = skr_renderer_get_renderer();
        skr_render_effect_attach(renderer, view, "ForwardEffect");
    };
    dualS_allocate_type(skr_runtime_get_dual_storage(), &renderableT, 100, DUAL_LAMBDA(primSetup));
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
    // Setup Gainput
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    enum InputAction
    {
        Quit,
        Cao,
    };
    auto hwnd = skr_window_get_native_handle(window);
    // Initialize Input
    gainput::InputManager manager;
    manager.Init(hwnd);
    manager.SetWindowsInstance(hwnd);
    manager.SetDisplaySize(BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT);
    gainput::DeviceId keyboardId = manager.CreateDevice<gainput::InputDeviceKeyboard>();
    gainput::DeviceId mouseId = manager.CreateDevice<gainput::InputDeviceMouse>();
    gainput::InputMap inputMap(manager);
    inputMap.MapBool(InputAction::Quit, keyboardId, gainput::KeyEscape);
    inputMap.MapBool(InputAction::Cao, mouseId, gainput::MouseButtonLeft);
    // loop
    bool quit = false;
    skg::GameContext ctx;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_SYSWMEVENT)
            {
                SDL_SysWMmsg* msg = event.syswm.msg;
#if defined(GAINPUT_PLATFORM_WIN)
                manager.HandleMessage((MSG&)msg->msg);
#elif defined(GAINPUT_PLATFORM_LINUX)
                manager.HandleMessage((XEvent&)msg->msg);
#endif
            }
            if (event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }

        manager.Update();
        if (inputMap.GetBoolWasDown(InputAction::Cao))
        {
            SKR_LOG_DEBUG("Cao");
        }
        if (inputMap.GetBoolWasDown(InputAction::Quit))
        {
            quit = true;
        }

        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
        (float)swapchain->back_buffers[0]->width,
        (float)swapchain->back_buffers[0]->height);
        skr_imgui_new_frame(window, 1.f / 60.f);
        quit |= skg::GameLoop(ctx);
        // move
        {
            auto filter = make_zeroed<dual_filter_t>();
            auto meta = make_zeroed<dual_meta_filter_t>();
            auto translation_type = dual_id_of<skr_translation_t>::get();
            filter.all.data = &translation_type;
            filter.all.length = 1;
            float lerps[] = { 1.25, 2.0 };
            auto timer = clock();
            auto total_sec = (double)timer / CLOCKS_PER_SEC;
            auto moveFunc = [&](dual_chunk_view_t* view) {
                auto translations = (skr_translation_t*)dualV_get_owned_ro(view, translation_type);
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto lscale = (float)abs(sin(total_sec * 0.5));
                    lscale = (float)lerp(lerps[0], lerps[1], lscale);
                    translations[i].value = {
                        ((float)(i % 10) - 4.5f) * lscale,
                        ((float)(i / 10) - 4.5f) * lscale, 0.f
                    };
                }
            };
            dualS_query(skr_runtime_get_dual_storage(), &filter, &meta, DUAL_LAMBDA(moveFunc));
        }
        {
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
        skr_renderer_render_frame(renderGraph);
        render_graph_imgui_add_render_pass(renderGraph, back_buffer, CGPU_LOAD_ACTION_LOAD);
        renderGraph->add_present_pass(
        [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
            builder.set_name("present_pass")
            .swapchain(swapchain, backbuffer_index)
            .texture(back_buffer, true);
        });
        renderGraph->compile();
        renderGraph->execute();
        // present
        CGPUQueuePresentDescriptor present_desc = {};
        present_desc.index = backbuffer_index;
        present_desc.swapchain = swapchain;
        cgpu_queue_present(skr_renderer_get_gfx_queue(), &present_desc);
        FrameMark
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