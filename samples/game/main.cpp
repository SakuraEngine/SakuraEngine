#include "../common/utils.h"
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
#include "skr_renderer.h"
#include <thread>
#include <chrono>
#include "input/gainput/gainput.h"
#include "input/gainput/GainputInputDeviceKeyboard.h"
#include "input/gainput/GainputInputDeviceMouse.h"

extern SWindowHandle window;
uint32_t backbuffer_index;
extern void free_api_objects();
extern void create_render_resources(skr::render_graph::RenderGraph* renderGraph);

#include "gamert.h"
#include "render-scene.h"
#include "ecs/callback.hpp"
#include <time.h>

#define lerp(a, b, t) (a) + (t) * ((b) - (a))

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    auto root = ghc::filesystem::current_path();
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph("GameRT", true);
    moduleManager->init_module_graph();

    assert(gamert_get_ecs_world());

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    SWindowDescroptor window_desc = {};
    window_desc.centered = true;
    window_desc.resizable = true;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    window = skr_create_window("Game", &window_desc);
    auto swapchain = skr_renderer_register_window(window);
    auto present_fence = cgpu_create_fence(skr_renderer_get_cgpu_device());
    namespace render_graph = skr::render_graph;
    auto renderGraph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(skr_renderer_get_cgpu_device())
        .with_gfx_queue(skr_renderer_get_gfx_queue())
        .enable_memory_aliasing();
    });
    create_render_resources(renderGraph);

    // Setup Gainput
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    enum InputAction
    {
    	Quit,
        Cao,
    };
    auto hwnd = skr_window_get_native_handle(window);
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
            if(event.type == SDL_SYSWMEVENT)
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
            filter.all.data = &transform_type;
            filter.all.length = 1;
            float lerps[] = { 1.25, 2.0 };
            auto timer = clock();
            auto total_sec = (double)timer / CLOCKS_PER_SEC;
            auto moveFunc = [&](dual_chunk_view_t* view) {
                auto transforms = (transform_t*)dualV_get_owned_ro(view, transform_type);
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto lscale = (float)abs(sin(total_sec * 0.5));
                    lscale = (float)lerp(lerps[0], lerps[1], lscale);
                    transforms[i].location = {
                        ((float)(i % 10) - 4.5f) * lscale,
                        ((float)(i / 10) - 4.5f) * lscale, 0.f
                    };
                }
            };
            dualS_query(gamert_get_ecs_world(), &filter, &meta, DUAL_LAMBDA(moveFunc));
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
        ecsr_draw_scene((struct skr_render_graph_t*)renderGraph);
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
    cgpu_free_fence(present_fence);
    render_graph::RenderGraph::destroy(renderGraph);
    render_graph_imgui_finalize();
    free_api_objects();
    moduleManager->destroy_module_graph();
    skr_free_window(window);
    SDL_Quit();
    return 0;
}