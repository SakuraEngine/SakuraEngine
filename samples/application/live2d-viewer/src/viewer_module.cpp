#include <EASTL/algorithm.h>

#include "../../../cgpu/common/utils.h"

#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#include "platform/vfs.h"
#include "platform/thread.h"
#include "platform/time.h"

#include "utils/log.hpp"
#include "utils/io.hpp"
#include "utils/make_zeroed.hpp"

#include "cgpu/io.hpp"

#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_effect.h"

#include "SkrLive2D/l2d_model_resource.h"
#include "SkrLive2D/l2d_render_model.h"
#include "SkrLive2D/l2d_render_effect.h"

#include "tracy/Tracy.hpp"

#ifdef _WIN32
#include "SkrImageCoder/extensions/win_dstorage_decompressor.h"
#endif

class SLive2DViewerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

public:
    static SLive2DViewerModule* Get();

    SWindowHandle window;
    uint32_t backbuffer_index;

    struct dual_storage_t* l2d_world = nullptr;
    SRendererId l2d_renderer = nullptr;
    skr_vfs_t* resource_vfs = nullptr;
    skr::io::RAMService* ram_service = nullptr;

};
#include "platform/filesystem.hpp"

IMPLEMENT_DYNAMIC_MODULE(SLive2DViewerModule, Live2DViewer);

SLive2DViewerModule* SLive2DViewerModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SLive2DViewerModule*>(mm->get_module("Live2DViewer"));
    return rm;
}

void SLive2DViewerModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer loaded!");

    std::error_code ec = {};
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    l2d_world = dualS_create();

    auto render_device = skr_get_default_render_device();
    l2d_renderer = skr_create_renderer(render_device, l2d_world);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = "Live2DViewerRAMIOService";
    ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
    ram_service = skr::io::RAMService::create(&ioServiceDesc);

#ifdef _WIN32
    auto decompress_service = skr_renderer_get_win_dstorage_decompress_service();
    cgpu_win_decompress_service_register_callback(decompress_service, 
        SKR_WIN_DSTORAGE_COMPRESSION_TYPE_IMAGE, 
        &skr_image_coder_win_dstorage_decompressor, nullptr);
#endif
}

void SLive2DViewerModule::on_unload()
{
    SKR_LOG_INFO("live2d viewer unloaded!");

    skr::io::RAMService::destroy(ram_service);
    skr_free_vfs(resource_vfs);

    dualS_release(l2d_world);
}

extern void create_imgui_resources(SRenderDeviceId render_device, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs);

#include "ecs/dual.h"
#include "ecs/callback.hpp"
#include "ecs/type_builder.hpp"

void create_test_scene(SRendererId renderer, skr_vfs_t* resource_vfs, skr_io_ram_service_t* ram_service, skr_io_vram_service_t* vram_service)
{
    auto renderableT_builder = make_zeroed<dual::type_builder_t>();
    renderableT_builder
        .with<skr_render_effect_t>();
    // allocate renderable
    auto renderableT = make_zeroed<dual_entity_type_t>();
    renderableT.type = renderableT_builder.build();
    auto live2dEntSetup = [&](dual_chunk_view_t* view) {
        skr_render_effect_attach(renderer, view, "Live2DEffect");
        
        auto ents = (dual_entity_t*)dualV_get_entities(view);
        auto modelSetup = [=](dual_chunk_view_t* view) {
            auto render_device = renderer->get_render_device();
            auto file_dstorage_queue = render_device->get_file_dstorage_queue();
            auto memory_dstorage_queue = render_device->get_memory_dstorage_queue();
            auto mesh_comps = (skr_live2d_render_model_comp_t*)dualV_get_owned_rw(view, dual_id_of<skr_live2d_render_model_comp_t>::get());
            for (uint32_t i = 0; i < view->count; i++)
            {
                auto& vram_request = mesh_comps[i].vram_request;
                auto& ram_request = mesh_comps[i].ram_request;
                vram_request.file_dstorage_queue_override = file_dstorage_queue;
                vram_request.memory_dstorage_queue_override = memory_dstorage_queue;
                vram_request.vfs_override = resource_vfs;
                vram_request.queue_override = render_device->get_gfx_queue();
                ram_request.vfs_override = resource_vfs;
                ram_request.callback_data = &vram_request;
                ram_request.finish_callback = +[](skr_live2d_ram_io_request_t* request, void* data)
                {
                    auto pRenderModelRequest = (skr_live2d_render_model_request_t*)data;
                    auto ram_service = SLive2DViewerModule::Get()->ram_service;
                    auto renderer = SLive2DViewerModule::Get()->l2d_renderer;
                    auto render_device = renderer->get_render_device();
                    auto vram_service = render_device->get_vram_service();
                    auto cgpu_device = render_device->get_cgpu_device();
                    skr_live2d_render_model_create_from_raw(ram_service, vram_service, cgpu_device, request->model_resource, pRenderModelRequest);
                };
                if (i == 1)
                    skr_live2d_model_create_from_json(ram_service, "Live2DViewer/Mao/mao_pro_t02.model3.json", &ram_request);
                else
                    skr_live2d_model_create_from_json(ram_service, "Live2DViewer/Hiyori/Hiyori.model3.json", &ram_request);
            }
        };
        skr_render_effect_access(renderer, ents, view->count, "Live2DEffect", DUAL_LAMBDA(modelSetup));
    };
    dualS_allocate_type(renderer->get_dual_storage(), &renderableT, 1, DUAL_LAMBDA(live2dEntSetup));
}

int SLive2DViewerModule::main_module_exec(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer executed!");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) 
        return -1;
    auto render_device = skr_get_default_render_device();
    auto cgpu_device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();
    auto adapter_detail = cgpu_query_adapter_detail(cgpu_device->adapter);
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE;
    // TODO: Resizable swapchain
    window_desc.height = 1500;
    window_desc.width = 1500;
    window = skr_create_window(
        fmt::format("Live2D Viewer Inner [{}]", gCGPUBackendNames[cgpu_device->adapter->instance->backend]).c_str(),
        &window_desc);

    auto ram_service = SLive2DViewerModule::Get()->ram_service;
    auto vram_service = render_device->get_vram_service();
    // Initialize renderer
    auto swapchain = skr_render_device_register_window(render_device, window);
    auto present_fence = cgpu_create_fence(cgpu_device);
    namespace render_graph = skr::render_graph;
    auto renderGraph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(cgpu_device)
            .with_gfx_queue(gfx_queue)
            .enable_memory_aliasing();
    });
    create_imgui_resources(render_device, renderGraph, resource_vfs);
    skr_live2d_initialize_render_effects(l2d_renderer, renderGraph, resource_vfs);
    create_test_scene(l2d_renderer, resource_vfs, ram_service, vram_service);
    uint64_t frame_index = 0;
    SHiresTimer tick_timer;
    int64_t elapsed_us = 0;
    int64_t elapsed_frame = 0;
    uint32_t fps = 60;
    skr_init_hires_timer(&tick_timer);

    bool quit = false;
    while (!quit)
    {
        FrameMark;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID((SDL_Window*)window) == event.window.windowID)
            {
                if (event.type == SDL_WINDOWEVENT)
                {
                    Uint8 window_event = event.window.event;
                    if (window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        cgpu_wait_queue_idle(gfx_queue);
                        cgpu_wait_fences(&present_fence, 1);
                        swapchain = skr_render_device_recreate_window_swapchain(render_device, window);
                    }
                }
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                Uint8 window_event = event.window.event;
                if (window_event == SDL_WINDOWEVENT_CLOSE || window_event == SDL_WINDOWEVENT_MOVED || window_event == SDL_WINDOWEVENT_RESIZED)
                if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(event.window.windowID)))
                {
                    if (window_event == SDL_WINDOWEVENT_CLOSE)
                        viewport->PlatformRequestClose = true;
                    if (window_event == SDL_WINDOWEVENT_MOVED)
                        viewport->PlatformRequestMove = true;
                    if (window_event == SDL_WINDOWEVENT_RESIZED)
                        viewport->PlatformRequestResize = true;
                }
            }
            if (event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }
        // LoopBody
        ZoneScopedN("LoopBody");
        int64_t us = skr_hires_timer_get_usec(&tick_timer, true);
        elapsed_us += us;
        elapsed_frame += 1;
        if (elapsed_us > (1000 * 1000))
        {
            fps = (uint32_t)elapsed_frame;
            elapsed_frame = 0;
            elapsed_us = 0;
        }
        {
            ZoneScopedN("ImGUI");

            auto& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(
            (float)swapchain->back_buffers[0]->width,
            (float)swapchain->back_buffers[0]->height);
            skr_imgui_new_frame(window, 1.f / 60.f);
        }
        {
            ImGui::Begin("Live2DViewer");
#ifdef _DEBUG
            ImGui::Text("Debug Build");
#else
            ImGui::Text("Shipping Build");
#endif
            ImGui::Text("Graphics: %s", adapter_detail->vendor_preset.gpu_name);
            int32_t wind_width = 0, wind_height = 0;
            skr_window_get_extent(window, &wind_width, &wind_height);
            ImGui::Text("Resolution: %dx%d", wind_width, wind_height);
            ImGui::Text("MotionEvalFPS(Fixed): %d", 240);
            ImGui::Text("PhysicsEvalFPS(Fixed): %d", 240);
            ImGui::Text("RenderFPS: %d", (uint32_t)fps);
            ImGui::End();
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
            skr_renderer_render_frame(l2d_renderer, renderGraph);
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
            if (frame_index == 1000)
                render_graph::RenderGraphViz::write_graphviz(*renderGraph, "render_graph_L2D.gv");
            frame_index = renderGraph->execute();
            {
                ZoneScopedN("CollectGarbage");
                if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                    renderGraph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
            }
        }
        {
            ZoneScopedN("QueuePresentSwapchain");
            // present
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
            render_graph_imgui_present_sub_viewports();
        }
    }
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    render_graph::RenderGraph::destroy(renderGraph);
    skr_live2d_finalize_render_effects(l2d_renderer, renderGraph, resource_vfs);
    render_graph_imgui_finalize();
    skr_free_renderer(l2d_renderer);
    return 0;
}