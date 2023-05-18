
#include "../../../../samples/common/common/utils.h"
#include "platform/memory.h"
#include "platform/window.h"
#include "utils/make_zeroed.hpp"
#include "utils/log.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "platform/vfs.h"
#include "SkrRenderer/render_effect.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#include "containers/string.hpp"

RUNTIME_EXTERN_C RUNTIME_API bool skr_runtime_is_dpi_aware();

void create_imgui_resources(SRenderDeviceId render_device, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* vfs)
{
    const auto device = renderGraph->get_backend_device();
    const auto backend = device->adapter->instance->backend;
    const auto gfx_queue = renderGraph->get_gfx_queue();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();
    {
        auto& style = ImGui::GetStyle();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        const char8_t* font_path = u8"./../resources/font/SourceSansPro-Regular.ttf";
        uint32_t *font_bytes, font_length;
        read_bytes(font_path, &font_bytes, &font_length);
        float dpi_scaling = 1.f;
        if (!skr_runtime_is_dpi_aware())
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(1.f / dpi_scaling);
            ImGui::GetIO().FontGlobalScale = 1.f / dpi_scaling;
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
    skr::string vsname = u8"shaders/imgui_vertex";
    skr::string fsname = u8"shaders/imgui_fragment";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? u8".dxil" : u8".spv");
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? u8".dxil" : u8".spv");
    auto vsfile = skr_vfs_fopen(vfs, vsname.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t im_vs_length = (uint32_t)skr_vfs_fsize(vsfile);
    uint32_t* im_vs_bytes = (uint32_t*)sakura_malloc(im_vs_length);
    skr_vfs_fread(vsfile, im_vs_bytes, 0, im_vs_length);
    skr_vfs_fclose(vsfile);
    auto fsfile = skr_vfs_fopen(vfs, fsname.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t im_fs_length = (uint32_t)skr_vfs_fsize(fsfile);
    uint32_t* im_fs_bytes = (uint32_t*)sakura_malloc(im_fs_length);
    skr_vfs_fread(fsfile, im_fs_bytes, 0, im_fs_length);
    skr_vfs_fclose(fsfile);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = u8"imgui_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = im_vs_bytes;
    vs_desc.code_size = im_vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = u8"imgui_fragment_shader";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = im_fs_bytes;
    fs_desc.code_size = im_fs_length;
    CGPUShaderLibraryId imgui_vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId imgui_fs = cgpu_create_shader_library(device, &fs_desc);
    sakura_free(im_vs_bytes);
    sakura_free(im_fs_bytes);
    RenderGraphImGuiDescriptor imgui_graph_desc = {};
    imgui_graph_desc.render_graph = renderGraph;
    imgui_graph_desc.backbuffer_format = render_device->get_swapchain_format();
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = CGPU_SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = u8"main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = CGPU_SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = u8"main";
    imgui_graph_desc.queue = gfx_queue;
    imgui_graph_desc.static_sampler = render_device->get_linear_sampler();
    render_graph_imgui_initialize(&imgui_graph_desc);
    cgpu_free_shader_library(imgui_vs);
    cgpu_free_shader_library(imgui_fs);
}