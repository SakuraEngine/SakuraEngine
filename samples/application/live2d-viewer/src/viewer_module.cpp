#include "../../../cgpu/common/utils.h"
#include "utils/log.h"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "imgui/imgui.h"

#include "ghc/filesystem.hpp"
#include "platform/vfs.h"
#include "utils/io.hpp"

#include "utils/make_zeroed.hpp"
#include "skr_renderer/skr_renderer.h"
#include "skr_live2d/model_resource.h"

#include "tracy/Tracy.hpp"

class SLive2DViewerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

    SWindowHandle window;
    uint32_t backbuffer_index;

    skr_vfs_t* resource_vfs = nullptr;
    skr::io::RAMService* ram_service = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SLive2DViewerModule, Live2DViewer);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "Live2DViewer",
    "prettyname" : "Live2DViewer",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrLive2D", "version":"0.1.0"},
        {"name":"SkrImGui", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
Live2DViewer)

void SLive2DViewerModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer loaded!");

    auto resourceRoot = (ghc::filesystem::current_path() / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = "Live2DViewerRAMIOService";
    ioServiceDesc.sleep_mode = SKR_IO_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_IO_SERVICE_SORT_METHOD_PARTIAL;
    ram_service = skr::io::RAMService::create(&ioServiceDesc);
}

void SLive2DViewerModule::on_unload()
{
    SKR_LOG_INFO("live2d viewer unloaded!");

    skr::io::RAMService::destroy(ram_service);
    skr_free_vfs(resource_vfs);
}

extern void create_imgui_resources(skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs);
int SLive2DViewerModule::main_module_exec(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer executed!");
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
        return -1;
    auto cgpuDevice = skr_renderer_get_cgpu_device();
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.centered = true;
    window_desc.resizable = true;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    window = skr_create_window(
    fmt::format("Live2D Viewer [{}]", gCGPUBackendNames[cgpuDevice->adapter->instance->backend]).c_str(),
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
    create_imgui_resources(renderGraph, resource_vfs);

    auto request = make_zeroed<skr_live2d_ram_io_request_t>();
    skr_io_ram_service_t* ioService = ram_service;
    request.vfs_override = resource_vfs;
    skr_live2d_model_create_from_json(ioService, "Live2DViewer/Haru/Haru.model3.json", &request);
    {
        ZoneScopedN("Idle");
        while(!request.is_ready());
    }

    bool quit = false;
    while (!quit)
    {
        FrameMark
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }
        // LoopBody
        ZoneScopedN("LoopBody");
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
            ZoneScopedN("RenderIMGUI");
            render_graph_imgui_add_render_pass(renderGraph, back_buffer, CGPU_LOAD_ACTION_CLEAR);
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
    cgpu_wait_queue_idle(skr_renderer_get_gfx_queue());
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    render_graph::RenderGraph::destroy(renderGraph);
    render_graph_imgui_finalize();
    skr_live2d_model_free(request.model_resource);
    return 0;
}