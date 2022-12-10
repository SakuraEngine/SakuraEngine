#include "SkrAssetTool/module.h"
#include <EASTL/algorithm.h>

#include "../../../cgpu/common/utils.h"

#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#include "platform/vfs.h"
#include "platform/thread.h"
#include "platform/time.h"

#include "utils/format.hpp"
#include "utils/log.h"
#include "cgpu/io.h"

#include "utils/make_zeroed.hpp"

#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"

#include "module/module.hpp"
#include "module/module_manager.hpp"

#include "tracy/Tracy.hpp"

#include "SkrAssetTool/usd_factory.h"

class SAssetImportModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

public:
    static SAssetImportModule* Get();

    SWindowHandle window;
    uint32_t backbuffer_index;

    struct dual_storage_t* l2d_world = nullptr;
    skr_vfs_t* resource_vfs = nullptr;

};
#include "platform/filesystem.hpp"

IMPLEMENT_DYNAMIC_MODULE(SAssetImportModule, SkrAssetImport);

SAssetImportModule* SAssetImportModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SAssetImportModule*>(mm->get_module("SkrAssetTool"));
    return rm;
}

void SAssetImportModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("live2d viewer loaded!");

    std::error_code ec = {};
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources").u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = resourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);
}

void SAssetImportModule::on_unload()
{
    SKR_LOG_INFO("live2d viewer unloaded!");

    skr_free_vfs(resource_vfs);
}

extern void create_imgui_resources(SRenderDeviceId render_device, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs);

int SAssetImportModule::main_module_exec(int argc, char** argv)
{
    // if(argc != 2)
    // {
    //     SKR_LOG_ERROR("Usage: asset_tool.exe <asset_path>");
    //     return -1;
    // }
    SKR_LOG_INFO("asset tool executed with args: {}", argv[1]);
    auto factory = skd::asset::GetUsdImporterFactory();
    //auto path = argv[1];
    //C:\Users\BenzzZX\Desktop\Development\Sakura.Runtime\samples\application\game\assets\scenes\Kitchen_set\Kitchen_set_instanced.usd
    auto path = "C:\\Users\\BenzzZX\\Desktop\\Development\\Sakura.Runtime\\samples\\application\\game\\assets\\scenes\\Kitchen_set\\Kitchen_set_instanced.usd";
    if(!factory->CanImport(path))
    {
        SKR_LOG_ERROR("Can't import asset: %s", path);
        return -1;
    }
    factory->Import(path);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) 
        return -1;
    auto render_device = skr_get_default_render_device();
    auto cgpu_device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();
    auto window_desc = make_zeroed<SWindowDescroptor>();
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE;
    // TODO: Resizable swapchain
    window_desc.height = 1500;
    window_desc.width = 1500;
    window = skr_create_window(
        skr::format("Asset Tool [{}]", gCGPUBackendNames[cgpu_device->adapter->instance->backend]).c_str(),
        &window_desc);

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
    uint64_t frame_index = 0;
    SHiresTimer tick_timer;
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
        {
            ZoneScopedN("ImGUINewFrame");

            auto& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(
            (float)swapchain->back_buffers[0]->width,
            (float)swapchain->back_buffers[0]->height);
            skr_imgui_new_frame(window, 1.f / 60.f);
        }
        {
            if(factory->Update() != 0)
                quit = true;
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
    render_graph_imgui_finalize();
    return 0;
}


int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph("SkrAssetImport", true);
    auto result = moduleManager->init_module_graph(argc, argv);
    if (result != 0) {
        SKR_LOG_ERROR("module graph init failed!");
    }
    moduleManager->destroy_module_graph();
    return 0;
}