#include "../common/utils.h"
#include "platform/window.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "gamert.h"
#include "platform/memory.h"
#include "utils/log.h"

SWindowHandle window;
uint32_t backbuffer_index;
bool DPIAware = false;
CGPUSurfaceId surface;
CGPUSwapChainId swapchain;
CGPUFenceId present_fence;
ECGPUBackend backend = CGPU_BACKEND_VULKAN;
CGPUInstanceId instance;
CGPUAdapterId adapter;
CGPUDeviceId device;
CGPUQueueId gfx_queue;
CGPUSamplerId sampler;

void create_api_objects()
{
    // Create instance
    CGPUInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name = true;
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queueType = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queueCount = 1;
    CGPUDeviceDescriptor device_desc = {};
    device_desc.queueGroups = &queue_group_desc;
    device_desc.queueGroupCount = 1;
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);
    // Sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    sampler = cgpu_create_sampler(device, &sampler_desc);

    // Create swapchain
    surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(window));
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.presentQueues = &gfx_queue;
    chain_desc.presentQueuesCount = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 2;
    chain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    chain_desc.enableVsync = false;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
}

void free_api_objects()
{
    // Free cgpu objects
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_sampler(sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

void create_render_resources(skr::render_graph::RenderGraph* renderGraph)
{
    auto moduleManager = skr_get_module_manager();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    {
        auto& style = ImGui::GetStyle();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        const char* font_path = "./../resources/font/SourceSansPro-Regular.ttf";
        uint32_t *font_bytes, font_length;
        read_bytes(font_path, (char**)&font_bytes, &font_length);
        float dpi_scaling = 1.f;
        if (!DPIAware)
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(1.f / dpi_scaling);
            ImGui::GetIO().FontGlobalScale = 0.5f;
        }
        else
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(dpi_scaling);
        }
        ImFontConfig cfg = {};
        cfg.SizePixels = 16.f * dpi_scaling;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.PixelSnapH = true;
        ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_bytes,
        font_length, cfg.SizePixels, &cfg);
        ImGui::GetIO().Fonts->Build();
        free(font_bytes);
    }
    eastl::string vsname = u8"shaders/imgui_vertex";
    eastl::string fsname = u8"shaders/imgui_fragment";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto gamert = (SGameRTModule*)moduleManager->get_module("GameRT");
    auto vsfile = skr_vfs_fopen(gamert->resource_vfs, vsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t im_vs_length = skr_vfs_fsize(vsfile);
    uint32_t* im_vs_bytes = (uint32_t*)sakura_malloc(im_vs_length);
    skr_vfs_fread(vsfile, im_vs_bytes, 0, im_vs_length);
    skr_vfs_fclose(vsfile);
    auto fsfile = skr_vfs_fopen(gamert->resource_vfs, fsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t im_fs_length = skr_vfs_fsize(fsfile);
    uint32_t* im_fs_bytes = (uint32_t*)sakura_malloc(im_fs_length);
    skr_vfs_fread(fsfile, im_fs_bytes, 0, im_fs_length);
    skr_vfs_fclose(fsfile);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "imgui_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = im_vs_bytes;
    vs_desc.code_size = im_vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "imgui_fragment_shader";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = im_fs_bytes;
    fs_desc.code_size = im_fs_length;
    CGPUShaderLibraryId imgui_vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId imgui_fs = cgpu_create_shader_library(device, &fs_desc);
    sakura_free(im_vs_bytes);
    sakura_free(im_fs_bytes);
    RenderGraphImGuiDescriptor imgui_graph_desc = {};
    imgui_graph_desc.render_graph = renderGraph;
    imgui_graph_desc.backbuffer_format = (ECGPUFormat)swapchain->back_buffers[backbuffer_index]->format;
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = CGPU_SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = "main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = CGPU_SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = "main";
    imgui_graph_desc.queue = gfx_queue;
    imgui_graph_desc.static_sampler = sampler;
    render_graph_imgui_initialize(&imgui_graph_desc);
    cgpu_free_shader_library(imgui_vs);
    cgpu_free_shader_library(imgui_fs);
}