#include "../common/utils.h"
#include "ftl/task.h"
#include "ftl/task_scheduler.h"
#include "ghc/filesystem.hpp"
#include "platform/window.h"
#include "resource/local_resource_registry.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "resource/resource_system.h"
#include <thread>
#include <chrono>
#ifdef SKR_OS_WINDOWS
    #include <shellscalingapi.h>
#endif

extern SWindowHandle window;
extern uint32_t backbuffer_index;
extern bool DPIAware;
extern CGPUSurfaceId surface;
extern CGPUSwapChainId swapchain;
extern CGPUFenceId present_fence;
extern ECGPUBackend backend;
extern CGPUInstanceId instance;
extern CGPUAdapterId adapter;
extern CGPUDeviceId device;
extern CGPUQueueId gfx_queue;
extern CGPUSamplerId sampler;
extern void create_api_objects();
extern void free_api_objects();
extern void create_render_resources(skr::render_graph::RenderGraph* renderGraph);

#include "gamert.h"
#include "render-scene.h"

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    auto root = ghc::filesystem::current_path();
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph("GameRT", true);
    moduleManager->init_module_graph();
#ifdef SKR_OS_WINDOWS
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    DPIAware = true;
#endif
    assert(gamert_get_ecs_world());
    gfx_shader_set_t set0 = {(CGPUShaderLibraryId)1, (CGPUShaderLibraryId)2, (CGPUShaderLibraryId)3, (CGPUShaderLibraryId)4, (CGPUShaderLibraryId)5};
    gfx_shader_set_t set2 = {(CGPUShaderLibraryId)2, (CGPUShaderLibraryId)3, (CGPUShaderLibraryId)4, (CGPUShaderLibraryId)5, (CGPUShaderLibraryId)6};
    auto setId0 = ecsr_register_gfx_shader_set(&set0);
    auto setId1 = ecsr_register_gfx_shader_set(&set2);
    auto setId00 = ecsr_register_gfx_shader_set(&set0);
    assert(setId0 == setId00);
    assert(ecsr_query_gfx_shader_set(setId0) == ecsr_query_gfx_shader_set(setId00));
    assert(ecsr_query_gfx_shader_set(setId0));
    assert(ecsr_query_gfx_shader_set(setId0)->vs == (CGPUShaderLibraryId)1);
    assert(ecsr_query_gfx_shader_set(setId1)->vs == (CGPUShaderLibraryId)2);
    gfx_material_t mat0 = {0};
    mat0.m_gfx = setId0;
    mat0.formats[0] = CGPU_FORMAT_R8G8B8A8_UNORM;
    auto matId0 = ecsr_register_gfx_material(&mat0);
    gfx_material_t mat1 = mat0;
    mat1.formats[0] = CGPU_FORMAT_B8G8R8A8_UNORM;
    auto matId1 = ecsr_register_gfx_material(&mat1);
    gfx_material_t mat2 = mat1;
    mat2.m_gfx = setId1; 
    auto matId2 = ecsr_register_gfx_material(&mat2);
    auto matId00 = ecsr_register_gfx_material(&mat0);
    assert(matId0 != 0);
    assert(matId0 == matId00);
    assert(ecsr_query_gfx_material(matId0)->m_gfx == setId0);
    assert(matId0 != matId1);
    assert(matId0 != matId2);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    SWindowDescroptor window_desc = {};
    window_desc.centered = true;
    window_desc.resizable = true;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    window = skr_create_window(gCGPUBackendNames[backend], &window_desc);
    create_api_objects();
    namespace render_graph = skr::render_graph;
    auto renderGraph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(device)
        .with_gfx_queue(gfx_queue)
        .enable_memory_aliasing();
    });
    create_render_resources(renderGraph);
    // loop
    bool quit = false;
    skg::GameContext ctx;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            auto sdl_window = (SDL_Window*)window;
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    quit = true;
                }
            }
        }
        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
        (float)swapchain->back_buffers[0]->width,
        (float)swapchain->back_buffers[0]->height);
        skr_imgui_new_frame(window, 1.f / 60.f);
        quit |= skg::GameLoop(ctx);
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
        render_graph_imgui_add_render_pass(renderGraph, back_buffer, CGPU_LOAD_ACTION_CLEAR);
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
        cgpu_queue_present(gfx_queue, &present_desc);
    }
    // clean up
    cgpu_wait_queue_idle(gfx_queue);
    render_graph::RenderGraph::destroy(renderGraph);
    render_graph_imgui_finalize();
    free_api_objects();
    skr_free_window(window);
    SDL_Quit();
    return 0;
}