#include "../common/utils.h"
#include "platform/window.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "gamert.h"
#include "platform/memory.h"
#include "utils/log.h"
#include "skr_renderer.h"
#include "runtime_module.h"

SWindowHandle window;
CGPUBufferId index_buffer;
CGPUBufferId vertex_buffer;

void create_test_materials(skr::render_graph::RenderGraph* renderGraph);
void free_test_materials();

void free_api_objects()
{
    // Free cgpu objects
    free_test_materials();
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
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
    create_test_materials(renderGraph);
}

#include "utils/make_zeroed.hpp"
#include "render-scene.h"
#include "ecs/callback.hpp"
#include "ecs/array.hpp"
#include "EASTL/sort.h"
#include <math/vectormath.hpp>

namespace smath = skr::math;

::gfx_material_id_t material_id;
::gfx_shader_set_id_t shaderset_id;
struct CubeGeometry {
    const skr::math::Vector3f g_Positions[24] = {
        { -0.5f, 0.5f, -0.5f }, // front face
        { 0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, 0.5f, -0.5f },

        { 0.5f, -0.5f, -0.5f }, // right side face
        { 0.5f, 0.5f, 0.5f },
        { 0.5f, -0.5f, 0.5f },
        { 0.5f, 0.5f, -0.5f },

        { -0.5f, 0.5f, 0.5f }, // left side face
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, 0.5f },
        { -0.5f, 0.5f, -0.5f },

        { 0.5f, 0.5f, 0.5f }, // back face
        { -0.5f, -0.5f, 0.5f },
        { 0.5f, -0.5f, 0.5f },
        { -0.5f, 0.5f, 0.5f },

        { -0.5f, 0.5f, -0.5f }, // top face
        { 0.5f, 0.5f, 0.5f },
        { 0.5f, 0.5f, -0.5f },
        { -0.5f, 0.5f, 0.5f },

        { 0.5f, -0.5f, 0.5f }, // bottom face
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, 0.5f },
    };
    const skr::math::Vector2f g_TexCoords[24] = {
        { 0.0f, 0.0f }, // front face
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },

        { 0.0f, 1.0f }, // right side face
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },

        { 0.0f, 0.0f }, // left side face
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },

        { 0.0f, 0.0f }, // back face
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },

        { 0.0f, 1.0f }, // top face
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },

        { 1.0f, 1.0f }, // bottom face
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
    };
    const uint32_t g_Normals[24] = {
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, -1.0f, 0.0f)), // front face
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, -1.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, -1.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, -1.0f, 0.0f)),

        skr::math::vector_to_snorm8(skr::math::Vector4f(1.0f, 0.0f, 0.0f, 0.0f)), // right side face
        skr::math::vector_to_snorm8(skr::math::Vector4f(1.0f, 0.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(1.0f, 0.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(1.0f, 0.0f, 0.0f, 0.0f)),

        skr::math::vector_to_snorm8(skr::math::Vector4f(-1.0f, 0.0f, 0.0f, 0.0f)), // left side face
        skr::math::vector_to_snorm8(skr::math::Vector4f(-1.0f, 0.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(-1.0f, 0.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(-1.0f, 0.0f, 0.0f, 0.0f)),

        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, 1.0f, 0.0f)), // back face
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, 1.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, 1.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 0.0f, 1.0f, 0.0f)),

        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 1.0f, 0.0f, 0.0f)), // top face
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 1.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 1.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, 1.0f, 0.0f, 0.0f)),

        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, -1.0f, 0.0f, 0.0f)), // bottom face
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, -1.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, -1.0f, 0.0f, 0.0f)),
        skr::math::vector_to_snorm8(skr::math::Vector4f(0.0f, -1.0f, 0.0f, 0.0f)),
    };
    static constexpr uint32_t g_Indices[] = {
        0, 1, 2, 0, 3, 1,       // front face
        4, 5, 6, 4, 7, 5,       // left face
        8, 9, 10, 8, 11, 9,     // right face
        12, 13, 14, 12, 15, 13, // back face
        16, 17, 18, 16, 19, 17, // top face
        20, 21, 22, 20, 23, 21, // bottom face
    };
};

void create_geom_resources(skr::render_graph::RenderGraph* renderGraph)
{
    const auto device = renderGraph->get_backend_device();
    const auto gfx_queue = renderGraph->get_gfx_queue();
    // upload
    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "UploadBuffer";
    upload_buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
    auto upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGPUBufferDescriptor vb_desc = {};
    vb_desc.name = "VertexBuffer";
    vb_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(CubeGeometry);
    vertex_buffer = cgpu_create_buffer(device, &vb_desc);
    CGPUBufferDescriptor ib_desc = {};
    ib_desc.name = "IndexBuffer";
    ib_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    ib_desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
    ib_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    ib_desc.size = sizeof(CubeGeometry::g_Indices);
    index_buffer = cgpu_create_buffer(device, &ib_desc);
    auto pool_desc = CGPUCommandPoolDescriptor();
    auto cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    auto cmd_desc = CGPUCommandBufferDescriptor();
    auto cpy_cmd = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = CubeGeometry();
        memcpy(upload_buffer->cpu_mapped_address, &geom, upload_buffer_desc.size);
    }
    cgpu_cmd_begin(cpy_cmd);
    CGPUBufferToBufferTransfer vb_cpy = {};
    vb_cpy.dst = vertex_buffer;
    vb_cpy.dst_offset = 0;
    vb_cpy.src = upload_buffer;
    vb_cpy.src_offset = 0;
    vb_cpy.size = sizeof(CubeGeometry);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &vb_cpy);
    {
        memcpy((char8_t*)upload_buffer->cpu_mapped_address + sizeof(CubeGeometry),
        CubeGeometry::g_Indices, sizeof(CubeGeometry::g_Indices));
    }
    CGPUBufferToBufferTransfer ib_cpy = {};
    ib_cpy.dst = index_buffer;
    ib_cpy.dst_offset = 0;
    ib_cpy.src = upload_buffer;
    ib_cpy.src_offset = sizeof(CubeGeometry);
    ib_cpy.size = sizeof(CubeGeometry::g_Indices);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &ib_cpy);
    // barriers
    CGPUBufferBarrier barriers[2] = {};
    CGPUBufferBarrier& vb_barrier = barriers[0];
    vb_barrier.buffer = vertex_buffer;
    vb_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGPUBufferBarrier& ib_barrier = barriers[1];
    ib_barrier.buffer = index_buffer;
    ib_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state = CGPU_RESOURCE_STATE_INDEX_BUFFER;
    CGPUResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers = barriers;
    barrier_desc.buffer_barriers_count = 2;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
    cgpu_cmd_end(cpy_cmd);
    CGPUQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cmd_pool);
}

void create_test_materials(skr::render_graph::RenderGraph* renderGraph)
{
    auto moduleManager = skr_get_module_manager();
    const auto device = renderGraph->get_backend_device();
    const auto backend = device->adapter->instance->backend;

    // read shaders
    eastl::string vsname = u8"shaders/Game/gbuffer_vs";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto gamert = (SGameRTModule*)moduleManager->get_module("GameRT");
    auto vsfile = skr_vfs_fopen(gamert->resource_vfs, vsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _vs_length = skr_vfs_fsize(vsfile);
    uint32_t* _vs_bytes = (uint32_t*)sakura_malloc(_vs_length);
    skr_vfs_fread(vsfile, _vs_bytes, 0, _vs_length);
    skr_vfs_fclose(vsfile);

    eastl::string fsname = u8"shaders/Game/gbuffer_fs";
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto fsfile = skr_vfs_fopen(gamert->resource_vfs, fsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _fs_length = skr_vfs_fsize(fsfile);
    uint32_t* _fs_bytes = (uint32_t*)sakura_malloc(_fs_length);
    skr_vfs_fread(fsfile, _fs_bytes, 0, _fs_length);
    skr_vfs_fclose(fsfile);

    // create default deferred material
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "gbuffer_vertex_buffer";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = _vs_bytes;
    vs_desc.code_size = _vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "gbuffer_fragment_buffer";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = _fs_bytes;
    fs_desc.code_size = _fs_length;
    CGPUShaderLibraryId _vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId _fs = cgpu_create_shader_library(device, &fs_desc);
    sakura_free(_vs_bytes);
    sakura_free(_fs_bytes);
    auto shaderSet = make_zeroed<gfx_shader_set_t>();
    shaderSet.vs = _vs;
    shaderSet.ps = _fs;
    shaderset_id = ecsr_register_gfx_shader_set(&shaderSet);
    auto mat = make_zeroed<gfx_material_t>();
    mat.m_gfx = shaderset_id;
    mat.device = device;
    mat.push_constant_count = 1;
    const char8_t* push_const_name = u8"push_constants";
    mat.push_constant_names = &push_const_name;
    material_id = ecsr_register_gfx_material(&mat);
    auto type = ecsr_query_material_parameter_type(material_id, "push_constants");
    assert(type != UINT32_MAX);
    auto bindingTypeDesc = dualT_get_desc(type);
    SKR_LOG_FMT_INFO(
        "ECS Type generated for binding push_constant:"
        "\n    name:{} size:{}\n    guid: {}", 
        bindingTypeDesc->name, bindingTypeDesc->size, 
        bindingTypeDesc->guid);
    auto prim_desc = make_zeroed<skr_scene_primitive_desc_t>();
    prim_desc.material = material_id;
    uint32_t ctype_count = 0;
    uint32_t metaent_count = 0;
    dual_type_index_t ctypes[16];
    dual_entity_t metas[16];
    const bool renderable = ecsr_renderable_primitive_type(&prim_desc, ctypes, &ctype_count, metas, &metaent_count);
    assert(renderable);
    for(uint32_t i = 0; i < ctype_count; i++)
    {
        auto cdesc = dualT_get_desc(ctypes[i]);
        SKR_LOG_FMT_INFO(
            "Component {} of rendereable: name: {}",
        i, cdesc->name);
    }
    // create vbs & ib
    create_geom_resources(renderGraph);
    // allocate renderable
    eastl::stable_sort(ctypes, ctypes + ctype_count);
    dual_entity_type_t renderableT = {};
    renderableT.type.data = ctypes;
    renderableT.type.length = ctype_count;
    renderableT.meta.data = metas;
    renderableT.meta.length = metaent_count;
    auto primSetup = [&](dual_chunk_view_t* view) {
        using vertex_buffers_t = dual::array_component_T<ecsr_vertex_buffer_t, 8>;
        auto index_buffers = (CGPUBufferId*)dualV_get_owned_ro(view, index_buffer_type);
        auto vertex_buffers = (vertex_buffers_t*)dualV_get_owned_ro(view, dual_id_of<ecsr_vertex_buffer_t>::get());
        auto material_instances = (gfx_material_inst_t*)dualV_get_owned_ro(view, gfx_material_inst_type);
        auto transforms = (transform_t*)dualV_get_owned_ro(view, transform_type);
        for (uint32_t i = 0; i < view->count; i++)
        {
            transforms[i].location = { 
                (float)(i % 10) * 1.5f, ((float)i / 10) * 1.5f, 0.f };
            transforms[i].scale = { 1.f, 1.f, 1.f };
            transforms[i].rotation = { 0.f, 0.f, 0.f, 1.f };
            index_buffers[i] = index_buffer;
            vertex_buffers[i].emplace_back(vertex_buffer, sizeof(skr::math::Vector3f), offsetof(CubeGeometry, g_Positions));
            vertex_buffers[i].emplace_back(vertex_buffer, sizeof(skr::math::Vector2f), offsetof(CubeGeometry, g_TexCoords));
            vertex_buffers[i].emplace_back(vertex_buffer, sizeof(uint32_t), offsetof(CubeGeometry, g_Normals));
            material_instances[i].material = material_id;
        }
    };
    dualS_allocate_type(gamert->ecs_world, &renderableT, 100, DUAL_LAMBDA(primSetup));
}

void free_test_materials()
{
    ecsr_unregister_gfx_shader_set(shaderset_id);
    ecsr_unregister_gfx_material(material_id);
}