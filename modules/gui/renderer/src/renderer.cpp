#include "SkrGuiRenderer/renderer.hpp"

namespace skr {
namespace gdi {
// HACK
inline static void read_bytes(const char* file_name, char8_t** bytes, uint32_t* length)
{
    FILE* f = fopen(file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (char8_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void read_shader_bytes(
const char* virtual_path, uint32_t** bytes, uint32_t* length,
ECGPUBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../resources/shaders/";
    strcpy(shader_file, shader_path);
    strcat(shader_file, virtual_path);
    switch (backend)
    {
        case CGPU_BACKEND_VULKAN:
            strcat(shader_file, ".spv");
            break;
        case CGPU_BACKEND_D3D12:
        case CGPU_BACKEND_XBOX_D3D12:
            strcat(shader_file, ".dxil");
            break;
        default:
            break;
    }
    read_bytes(shader_file, (char8_t**)bytes, length);
}

CGPURenderPipelineId create_render_pipeline(CGPUDeviceId device, ECGPUFormat target_format, CGPUVertexLayout* pLayout)
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("GUI/vertex", &vs_bytes, &vs_length, device->adapter->instance->backend);
    read_shader_bytes("GUI/pixel", &fs_bytes, &fs_length, device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "FragmentShaderLibrary";
    ps_desc.stage = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = pLayout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &target_format;
    auto pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    return pipeline;
}
// HACK
int SGDIRenderer_RenderGraph::initialize(const SGDIRendererDescriptor* desc) SKR_NOEXCEPT
{
    const auto pDesc = reinterpret_cast<SGDIRendererDescriptor_RenderGraph*>(desc->usr_data);
    const uint32_t pos_offset = static_cast<uint32_t>(offsetof(SGDIVertex, position));
    const uint32_t texcoord_offset = static_cast<uint32_t>(offsetof(SGDIVertex, texcoord));
    const uint32_t aa_offset = static_cast<uint32_t>(offsetof(SGDIVertex, aa));
    const uint32_t uv_offset = static_cast<uint32_t>(offsetof(SGDIVertex, clipUV));
    const uint32_t uv2_offset = static_cast<uint32_t>(offsetof(SGDIVertex, clipUV2));
    const uint32_t color_offset = static_cast<uint32_t>(offsetof(SGDIVertex, color));
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 0, pos_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, texcoord_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "AA", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, aa_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "UV", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { "UV_Two", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv2_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[5] = { "COLOR", 1, CGPU_FORMAT_R8G8B8A8_UNORM, 0, color_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[6] = { "TRANSFORM", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 1, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    vertex_layout.attribute_count = 7;
    pipeline = create_render_pipeline(pDesc->device, CGPU_FORMAT_B8G8R8A8_UNORM, &vertex_layout);
    return 0;
}

int SGDIRenderer_RenderGraph::finalize() SKR_NOEXCEPT
{
    auto rs = pipeline->root_signature;
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(rs);
    return 0;
}

void SGDIRenderer_RenderGraph::render(SGDICanvasGroup* canvas_group, SGDIRenderParams* params) SKR_NOEXCEPT
{
    const auto pParams = reinterpret_cast<SGDIRenderParams_RenderGraph*>(params->usr_data);
    auto rg = pParams->render_graph;
    auto canvas_group_data = SkrNew<SGDICanvasGroupData_RenderGraph>(canvas_group);
    const auto canvas_span = canvas_group->all_canvas();
    if (canvas_span.empty()) return;
    uint64_t vertex_count = 0u;
    uint64_t index_count = 0u;
    uint64_t transform_count = 0u;
    uint64_t command_count = 0u;
    // 1. loop prepare counters & render data
    for (auto canvas : canvas_span)
    {
    for (auto element : canvas->all_elements())
    {
        auto element_private = static_cast<SGDIElementPrivate*>(element);
        vertex_count += element_private->vertices.size();
        index_count += element_private->indices.size();
        transform_count += 1;
        command_count += element_private->commands.size();
    }
    }
    canvas_group_data->render_commands.clear();
    canvas_group_data->render_vertices.clear();
    canvas_group_data->render_indices.clear();
    canvas_group_data->render_transforms.clear();
    canvas_group_data->render_vertices.reserve(vertex_count);
    canvas_group_data->render_indices.reserve(index_count);
    canvas_group_data->render_transforms.reserve(transform_count);
    canvas_group_data->render_commands.reserve(command_count);
    uint64_t vb_cursor = 0u;
    uint64_t ib_cursor = 0u;
    uint64_t tb_cursor = 0u;
    for (auto canvas : canvas_span)
    {
    for (auto element : canvas->all_elements())
    {
        auto element_private = static_cast<SGDIElementPrivate*>(element);
        vb_cursor = canvas_group_data->render_vertices.size();
        ib_cursor = canvas_group_data->render_indices.size();
        tb_cursor = canvas_group_data->render_transforms.size();
        canvas_group_data->render_vertices.insert(canvas_group_data->render_vertices.end(), element_private->vertices.begin(), element_private->vertices.end());
        canvas_group_data->render_indices.insert(canvas_group_data->render_indices.end(), element_private->indices.begin(), element_private->indices.end());
        // TODO: deal with transform
        auto& transform = canvas_group_data->render_transforms.emplace_back();
        transform.transform.M[0][0] = 900.0f;
        transform.transform.M[0][1] = 900.0f;
        for (auto command : element_private->commands)
        {
            SGDIElementDrawCommand_RenderGraph command2 = {};
            command2.first_index = command.first_index;
            command2.index_count = command.index_count;
            command2.ib_offset = ib_cursor * sizeof(index_t);
            command2.vb_offset = vb_cursor * sizeof(SGDIVertex);
            command2.tb_offset = tb_cursor * sizeof(SGDITransform);
            canvas_group_data->render_commands.emplace_back(command2);
        }
    }
    }

    // 2. prepare render resource
    const uint64_t vertices_size = vertex_count * sizeof(SGDIVertex);
    const uint64_t indices_size = index_count * sizeof(index_t);
    const uint64_t transform_size = transform_count * sizeof(SGDITransform);
    const bool useCVV = false;
    auto vertex_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("nvg_vertex_buffer")
                .size(vertices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto transform_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("nvg_transform_buffer")
                .size(transform_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto index_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("nvg_index_buffer")
                .size(indices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_index_buffer();
        });
    canvas_group_data->vertex_buffers.clear();
    canvas_group_data->transform_buffers.clear();
    canvas_group_data->index_buffers.clear();
    canvas_group_data->vertex_buffers.emplace_back(vertex_buffer);
    canvas_group_data->transform_buffers.emplace_back(transform_buffer);
    canvas_group_data->index_buffers.emplace_back(index_buffer);

    // 3. copy/upload geometry data to GPU
    if (!useCVV)
    {
        auto upload_buffer_handle = rg->create_buffer(
            [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            ZoneScopedN("ConstructUploadPass");
            builder.set_name("nvg_upload_buffer")
                    .size(indices_size + vertices_size + transform_size)
                    .with_tags(kRenderGraphDefaultResourceTag)
                    .as_upload_buffer();
            });
        rg->add_copy_pass(
            [=](render_graph::RenderGraph& g, render_graph::CopyPassBuilder& builder) {
            ZoneScopedN("ConstructCopyPass");
            builder.set_name("nvg_copy_pass")
                .buffer_to_buffer(upload_buffer_handle.range(0, vertices_size), vertex_buffer.range(0, vertices_size))
                .buffer_to_buffer(upload_buffer_handle.range(vertices_size, vertices_size + indices_size), index_buffer.range(0, indices_size))
                .buffer_to_buffer(upload_buffer_handle.range(vertices_size + indices_size, vertices_size + indices_size + transform_size), transform_buffer.range(0, transform_size));
            },
            [upload_buffer_handle, canvas_group_data](render_graph::RenderGraph& g, render_graph::CopyPassContext& context){
                auto upload_buffer = context.resolve(upload_buffer_handle);
                const uint64_t vertices_count = canvas_group_data->render_vertices.size();
                const uint64_t indices_count = canvas_group_data->render_indices.size();
                SGDIVertex* vtx_dst = (SGDIVertex*)upload_buffer->cpu_mapped_address;
                index_t* idx_dst = (index_t*)(vtx_dst + vertices_count);
                SGDITransform* transform_dst = (SGDITransform*)(idx_dst + indices_count);
                const skr::span<SGDIVertex> render_vertices = canvas_group_data->render_vertices;
                const skr::span<index_t> render_indices = canvas_group_data->render_indices;
                const skr::span<SGDITransform> render_transforms = canvas_group_data->render_transforms;
                memcpy(vtx_dst, render_vertices.data(), vertices_count * sizeof(SGDIVertex));
                memcpy(idx_dst, render_indices.data(), indices_count * sizeof(index_t));
                memcpy(transform_dst, render_transforms.data(), render_transforms.size() * sizeof(SGDITransform));
            });
    }

    // 4. loop & record render commands
    skr::render_graph::TextureRTVHandle target = rg->get_texture("backbuffer");
    // skr::vector<SGDICanvas*> canvas_copy(canvas_span.begin(), canvas_span.end());
    rg->add_render_pass([&](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
        ZoneScopedN("ConstructRenderPass");
        builder.set_name("nvg_gdi_render_pass")
            .set_pipeline(pipeline)
            .use_buffer(vertex_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(transform_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(index_buffer, CGPU_RESOURCE_STATE_INDEX_BUFFER)
            .write(0, target, CGPU_LOAD_ACTION_CLEAR);
    },
    [target, canvas_group_data, useCVV, index_buffer, vertex_buffer, transform_buffer]
    (render_graph::RenderGraph& g, render_graph::RenderPassContext& context) {
        ZoneScopedN("GDI(NVG)RenderPass");
        const auto target_desc = g.resolve_descriptor(target);
        auto resolved_ib = context.resolve(index_buffer);
        auto resolved_vb = context.resolve(vertex_buffer);
        auto resolved_tb = context.resolve(transform_buffer);
        CGPUBufferId vertex_streams[2] = { resolved_vb, resolved_tb };
        const uint32_t vertex_stream_strides[2] = { sizeof(SGDIVertex), sizeof(SGDITransform) };
        cgpu_render_encoder_set_viewport(context.encoder,
            0.0f, 0.0f,
            (float)target_desc->width,
            (float)target_desc->height,
            0.f, 1.f);
        cgpu_render_encoder_set_scissor(context.encoder,
            0, 0, 
            target_desc->width, target_desc->height);
        const skr::span<SGDIElementDrawCommand_RenderGraph> render_commands = canvas_group_data->render_commands;
        for (const auto& command : render_commands)
        {
            const uint32_t vertex_stream_offsets[2] = { command.vb_offset, command.tb_offset };
            cgpu_render_encoder_bind_index_buffer(context.encoder,
                resolved_ib, sizeof(index_t), command.ib_offset);
            cgpu_render_encoder_bind_vertex_buffers(context.encoder,
                2, vertex_streams, vertex_stream_strides, vertex_stream_offsets);
            cgpu_render_encoder_draw_indexed_instanced(context.encoder,
                command.index_count,command.first_index,
                1, 0, 0);
        }
        SkrDelete(canvas_group_data);
    });
}

} }