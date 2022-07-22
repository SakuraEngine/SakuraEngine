#include "skr_renderer/skr_renderer.h"
#include "cgpu/api.h"
#include "module/module_manager.hpp"
#include "utils/log.h"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include <string.h>

IMPLEMENT_DYNAMIC_MODULE(SkrRendererModule, SkrRenderer);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrRenderer",
    "prettyname" : "SakuraRenderer",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrScene", "version":"0.1.0"},
        {"name":"SkrRenderGraph", "version":"0.1.0"},
        {"name":"SkrImGui", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrRenderer)

extern skr::Renderer* create_renderer_impl();

void SkrRendererModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("skr renderer loaded!");

    renderer = create_renderer_impl();
    for (auto i = 0; i < argc; i++)
    {
        if (::strcmp(argv[i], "--vulkan") == 0)
        {
            renderer->backend = CGPU_BACKEND_VULKAN;
        }
        else if (::strcmp(argv[i], "--d3d12") == 0)
        {
            renderer->backend = CGPU_BACKEND_D3D12;
        }
        else
        {
#ifdef _WIN32
            renderer->backend = CGPU_BACKEND_VULKAN;
#else
            renderer->backend = CGPU_BACKEND_VULKAN;
#endif
        }
    }
    renderer->initialize();
}

void SkrRendererModule::on_unload()
{
    SKR_LOG_INFO("skr renderer unloaded!");

    renderer->finalize();
}

SkrRendererModule* SkrRendererModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SkrRendererModule*>(mm->get_module("SkrRenderer"));
    return rm;
}

ECGPUFormat SkrRendererModule::get_swapchain_format() const
{
    if (renderer->swapchains.size() > 0)
        return (ECGPUFormat)renderer->swapchains.at(0).second->back_buffers[0]->format;
    return CGPU_FORMAT_B8G8R8A8_UNORM;
}

CGPUSamplerId SkrRendererModule::get_linear_sampler() const
{
    return renderer->linear_sampler;
}

CGPUDeviceId SkrRendererModule::get_cgpu_device() const
{
    return renderer->device;
}

CGPUQueueId SkrRendererModule::get_gfx_queue() const
{
    return renderer->gfx_queue;
}

CGPUQueueId SkrRendererModule::get_cpy_queue(uint32_t idx) const
{
    if (idx < renderer->cpy_queues.size())
        return renderer->cpy_queues[idx];
    return renderer->cpy_queues[0];
}

struct ISkrRenderer* skr_renderer_get_renderer()
{
    return (ISkrRenderer*)SkrRendererModule::Get()->get_renderer();
}

CGPUSwapChainId skr_renderer_register_window(SWindowHandle window)
{
    return SkrRendererModule::Get()->get_renderer()->register_window(window);
}

ECGPUFormat skr_renderer_get_swapchain_format()
{
    return SkrRendererModule::Get()->get_swapchain_format();
}

CGPUSamplerId skr_renderer_get_linear_sampler()
{
    return SkrRendererModule::Get()->get_linear_sampler();
}

CGPUQueueId skr_renderer_get_gfx_queue()
{
    return SkrRendererModule::Get()->get_gfx_queue();
}

CGPUQueueId skr_renderer_get_cpy_queue()
{
    return SkrRendererModule::Get()->get_cpy_queue();
}

CGPUQueueId skr_renderer_get_nth_cpy_queue(uint32_t n)
{
    return SkrRendererModule::Get()->get_cpy_queue(n);
}

CGPUDeviceId skr_renderer_get_cgpu_device()
{
    return SkrRendererModule::Get()->get_cgpu_device();
}

void skr_renderer_render_frame(skr::render_graph::RenderGraph* render_graph)
{
    SkrRendererModule::Get()->get_renderer()->render(render_graph);
}