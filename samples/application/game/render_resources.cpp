#include "../../cgpu/common/utils.h"
#include "platform/memory.h"
#include "platform/window.h"
#include "utils/make_zeroed.hpp"
#include "utils/log.h"
#include "ecs/dual.h"
#include "ecs/type_builder.hpp"
#include "ecs/callback.hpp"
#include "ecs/array.hpp"
#include "render_graph/frontend/render_graph.hpp"
#include "skr_scene/scene.h"
#include "skr_renderer/effect_processor.h"
#include "skr_renderer/skr_renderer.h"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "runtime_module.h"
#include "gamert.h"

void create_test_scene()
{
    auto renderableT_builder = make_zeroed<dual::type_builder_t>();
    renderableT_builder.with<skr_transform_t>();
    renderableT_builder.with<skr_render_effect_t>();
    // allocate renderable
    auto renderableT = make_zeroed<dual_entity_type_t>();
    renderableT.type = renderableT_builder.build();
    auto primSetup = [&](dual_chunk_view_t* view) {
        auto transforms = (skr_transform_t*)dualV_get_owned_ro(view, dual_id_of<skr_transform_t>::get());
        for (uint32_t i = 0; i < view->count; i++)
        {
            transforms[i].location = {
                (float)(i % 10) * 1.5f, ((float)i / 10) * 1.5f, 0.f
            };
            transforms[i].scale = { 1.f, 1.f, 1.f };
            transforms[i].rotation = { 0.f, 0.f, 0.f, 1.f };
        }
        auto renderer = skr_renderer_get_renderer();
        skr_render_effect_attach(renderer, view, "ForwardEffect");
    };
    dualS_allocate_type(skr_runtime_get_dual_storage(), &renderableT, 100, DUAL_LAMBDA(primSetup));
}

void create_render_resources(skr::render_graph::RenderGraph* renderGraph)
{
    auto moduleManager = skr_get_module_manager();
    auto renderModule = static_cast<SkrRendererModule*>(moduleManager->get_module("SkrRenderer"));
    const auto device = renderGraph->get_backend_device();
    const auto backend = device->adapter->instance->backend;
    const auto gfx_queue = renderGraph->get_gfx_queue();
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
        if (!skr_runtime_is_dpi_aware())
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
    uint32_t im_vs_length = (uint32_t)skr_vfs_fsize(vsfile);
    uint32_t* im_vs_bytes = (uint32_t*)sakura_malloc(im_vs_length);
    skr_vfs_fread(vsfile, im_vs_bytes, 0, im_vs_length);
    skr_vfs_fclose(vsfile);
    auto fsfile = skr_vfs_fopen(gamert->resource_vfs, fsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t im_fs_length = (uint32_t)skr_vfs_fsize(fsfile);
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
    imgui_graph_desc.backbuffer_format = renderModule->get_swapchain_format();
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = CGPU_SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = "main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = CGPU_SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = "main";
    imgui_graph_desc.queue = gfx_queue;
    imgui_graph_desc.static_sampler = renderModule->get_linear_sampler();
    render_graph_imgui_initialize(&imgui_graph_desc);
    cgpu_free_shader_library(imgui_vs);
    cgpu_free_shader_library(imgui_fs);

    // create materials
    create_test_scene();
}