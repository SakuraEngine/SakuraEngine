#include "SkrGuiRenderer/renderer.hpp"
#include "rtm/qvvf.h"

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
    CGPURasterizerStateDescriptor rs_state = {};
    rs_state.cull_mode = CGPU_CULL_MODE_NONE;
    rs_state.fill_mode = CGPU_FILL_MODE_SOLID;
    rs_state.front_face = CGPU_FRONT_FACE_CW;
    rs_state.slope_scaled_depth_bias = 0.f;
    rs_state.enable_depth_clamp = false;
    rs_state.enable_scissor = true;
    rs_state.enable_multi_sample = false;
    rs_state.depth_bias = 0;
    rp_desc.rasterizer_state = &rs_state;
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
    vertex_layout.attributes[7] = { "PROJECTION", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 2, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    vertex_layout.attribute_count = 8;
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

/*
skr_float4x4_t orthographic(float Left, float Right, float Bottom, float Top, float NearZ, float FarZ)
{
    float RL, TB, FN;
    RL = 1.0f / (Right  - Left);
    TB = 1.0f / (Top    - Bottom);
    FN =-1.0f / (FarZ - NearZ);

    return 
    {{
        { 2.0f * RL, 0.f, 0.f, 0.f },
        { 0.f, 2.0f * TB, 0.f, 0.f },
        { 0.f, 0.f, 2.0f * FN, -1.f },
        { -(Right + Left) * RL, -(Top + Bottom) * TB, (FarZ + NearZ) * FN, 1.f }
    }};
}
*/

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
    uint64_t projection_count = 0u;
    uint64_t command_count = 0u;
    // 1. loop prepare counters & render data
    for (auto canvas : canvas_span)
    {
    for (auto element : canvas->all_elements())
    {
        const auto element_vertices = fetch_element_vertices(element);
        const auto element_indices = fetch_element_indices(element);
        const auto element_commands = fetch_element_draw_commands(element);
        
        vertex_count += element_vertices.size();
        index_count += element_indices.size();
        transform_count += 1;
        projection_count += 1;
        command_count += element_commands.size();
    }
    }
    canvas_group_data->render_vertices.reserve(vertex_count);
    canvas_group_data->render_indices.reserve(index_count);
    canvas_group_data->render_transforms.reserve(transform_count);
    canvas_group_data->render_projections.reserve(projection_count);
    canvas_group_data->render_commands.reserve(command_count);
    uint64_t vb_cursor = 0u, ib_cursor = 0u, tb_cursor = 0u, pb_cursor = 0u;
    for (auto canvas : canvas_span)
    {
    for (auto element : canvas->all_elements())
    {
        const auto element_vertices = fetch_element_vertices(element);
        const auto element_indices = fetch_element_indices(element);
        const auto element_commands = fetch_element_draw_commands(element);

        // insert data
        vb_cursor = canvas_group_data->render_vertices.size();
        ib_cursor = canvas_group_data->render_indices.size();
        tb_cursor = canvas_group_data->render_transforms.size();
        pb_cursor = canvas_group_data->render_projections.size();
        canvas_group_data->render_vertices.insert(canvas_group_data->render_vertices.end(), element_vertices.begin(), element_vertices.end());
        canvas_group_data->render_indices.insert(canvas_group_data->render_indices.end(), element_indices.begin(), element_indices.end());

        // transform
        auto& transform = canvas_group_data->render_transforms.emplace_back();
        const auto scaleX = 1.f;
        const auto scaleY = 1.f;
        const auto scaleZ = 1.f;
        const auto transformX = 0.f;
        const auto transformY = 0.f;
        const auto transformZ = 1.f;
        const auto transformW = 1.f;
        const auto pitchInDegrees = 0.f;
        const auto yawInDegrees = 0.f;
        const auto rollInDegrees = 0.f;
        const auto quat = rtm::quat_from_euler_rh(
            rtm::scalar_deg_to_rad(-pitchInDegrees),
            rtm::scalar_deg_to_rad(yawInDegrees),
            rtm::scalar_deg_to_rad(rollInDegrees));
        const rtm::vector4f translation = rtm::vector_set(transformX, transformY, transformZ, transformW);
        const rtm::vector4f scale = rtm::vector_set(scaleX, scaleY, scaleZ, 0.f);
        const auto transform_qvv = rtm::qvv_set(quat, translation, scale);
        transform = rtm::matrix_cast(rtm::matrix_from_qvv(transform_qvv));
        
        // projection
        auto& projection = canvas_group_data->render_projections.emplace_back();
        const auto view = rtm::look_at_matrix_lh(
            {450.f, 450.f, 0.f} /*eye*/, 
            {450.f, 450.f, 1000.f} /*at*/,
            { 0.f, 1.f, 0.f } /*up*/
        );
        const auto proj = rtm::orthographic_lh(900.f, 900.f, 0.f, 1000.f);
        projection = rtm::matrix_mul(view, proj);

        for (auto command : element_commands)
        {
            SGDIElementDrawCommand_RenderGraph command2 = {};
            command2.first_index = command.first_index;
            command2.index_count = command.index_count;
            command2.ib_offset = ib_cursor * sizeof(index_t);
            command2.vb_offset = vb_cursor * sizeof(SGDIVertex);
            command2.tb_offset = tb_cursor * sizeof(rtm::matrix4x4f);
            command2.pb_offset = pb_cursor * sizeof(rtm::matrix4x4f);
            canvas_group_data->render_commands.emplace_back(command2);
        }
    }
    }

    // 2. prepare render resource
    const uint64_t vertices_size = vertex_count * sizeof(SGDIVertex);
    const uint64_t indices_size = index_count * sizeof(index_t);
    const uint64_t transform_size = transform_count * sizeof(rtm::matrix4x4f);
    const uint64_t projection_size = projection_count * sizeof(rtm::matrix4x4f);
    const bool useCVV = false;
    auto vertex_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("gdi_vertex_buffer")
                .size(vertices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto transform_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("gdi_transform_buffer")
                .size(transform_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto projection_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("gdi_projection_buffer")
                .size(projection_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto index_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("gdi_index_buffer")
                .size(indices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_index_buffer();
        });
    canvas_group_data->vertex_buffers.emplace_back(vertex_buffer);
    canvas_group_data->transform_buffers.emplace_back(transform_buffer);
    canvas_group_data->projection_buffers.emplace_back(projection_buffer);
    canvas_group_data->index_buffers.emplace_back(index_buffer);

    // 3. copy/upload geometry data to GPU
    if (!useCVV)
    {
        auto upload_buffer_handle = rg->create_buffer(
            [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            ZoneScopedN("ConstructUploadPass");
            builder.set_name("gdi_upload_buffer")
                    .size(indices_size + vertices_size + transform_size + projection_size)
                    .with_tags(kRenderGraphDefaultResourceTag)
                    .as_upload_buffer();
            });
        rg->add_copy_pass(
            [=](render_graph::RenderGraph& g, render_graph::CopyPassBuilder& builder) {
            ZoneScopedN("ConstructCopyPass");
            builder.set_name("gdi_copy_pass")
                .buffer_to_buffer(upload_buffer_handle.range(0, vertices_size), vertex_buffer.range(0, vertices_size))
                .buffer_to_buffer(upload_buffer_handle.range(vertices_size, vertices_size + indices_size), index_buffer.range(0, indices_size))
                .buffer_to_buffer(upload_buffer_handle.range(vertices_size + indices_size, vertices_size + indices_size + transform_size), transform_buffer.range(0, transform_size))
                .buffer_to_buffer(upload_buffer_handle.range(vertices_size + indices_size + transform_size, vertices_size + indices_size + transform_size + projection_size), projection_buffer.range(0, projection_size));
            },
            [upload_buffer_handle, canvas_group_data](render_graph::RenderGraph& g, render_graph::CopyPassContext& context){
                auto upload_buffer = context.resolve(upload_buffer_handle);
                const uint64_t vertices_count = canvas_group_data->render_vertices.size();
                const uint64_t indices_count = canvas_group_data->render_indices.size();
                const uint64_t transforms_count = canvas_group_data->render_transforms.size();
                const uint64_t projections_count = canvas_group_data->render_projections.size();

                SGDIVertex* vtx_dst = (SGDIVertex*)upload_buffer->cpu_mapped_address;
                index_t* idx_dst = (index_t*)(vtx_dst + vertices_count);
                rtm::matrix4x4f* transform_dst = (rtm::matrix4x4f*)(idx_dst + indices_count);
                rtm::matrix4x4f* projection_dst = (rtm::matrix4x4f*)(transform_dst + transforms_count);

                const skr::span<SGDIVertex> render_vertices = canvas_group_data->render_vertices;
                const skr::span<index_t> render_indices = canvas_group_data->render_indices;
                const skr::span<rtm::matrix4x4f> render_transforms = canvas_group_data->render_transforms;
                const skr::span<rtm::matrix4x4f> render_projections = canvas_group_data->render_projections;

                memcpy(vtx_dst, render_vertices.data(), vertices_count * sizeof(SGDIVertex));
                memcpy(idx_dst, render_indices.data(), indices_count * sizeof(index_t));
                memcpy(transform_dst, render_transforms.data(), transforms_count * sizeof(rtm::matrix4x4f));
                memcpy(projection_dst, render_projections.data(), projections_count * sizeof(rtm::matrix4x4f));
            });
    }

    // 4. loop & record render commands
    skr::render_graph::TextureRTVHandle target = rg->get_texture("backbuffer");
    // skr::vector<SGDICanvas*> canvas_copy(canvas_span.begin(), canvas_span.end());
    rg->add_render_pass([&](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
        ZoneScopedN("ConstructRenderPass");
        builder.set_name("gdi_render_pass")
            .set_pipeline(pipeline)
            .use_buffer(vertex_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(transform_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(projection_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(index_buffer, CGPU_RESOURCE_STATE_INDEX_BUFFER)
            .write(0, target, CGPU_LOAD_ACTION_CLEAR);
    },
    [target, canvas_group_data, useCVV, index_buffer, vertex_buffer, transform_buffer, projection_buffer]
    (render_graph::RenderGraph& g, render_graph::RenderPassContext& ctx) {
        ZoneScopedN("GDI-RenderPass");
        const auto target_desc = g.resolve_descriptor(target);
        auto resolved_ib = ctx.resolve(index_buffer);
        auto resolved_vb = ctx.resolve(vertex_buffer);
        auto resolved_tb = ctx.resolve(transform_buffer);
        auto resolved_pb = ctx.resolve(projection_buffer);
        CGPUBufferId vertex_streams[3] = { resolved_vb, resolved_tb, resolved_pb };
        const uint32_t vertex_stream_strides[3] = { sizeof(SGDIVertex), sizeof(rtm::matrix4x4f), sizeof(rtm::matrix4x4f) };

        cgpu_render_encoder_set_viewport(ctx.encoder,
            0.0f, 0.0f,
            (float)target_desc->width,
            (float)target_desc->height,
            0.f, 1.f);
        cgpu_render_encoder_set_scissor(ctx.encoder,
            0, 0, 
            target_desc->width, target_desc->height);

        const skr::span<SGDIElementDrawCommand_RenderGraph> render_commands = canvas_group_data->render_commands;
        for (const auto& command : render_commands)
        {
            const uint32_t vertex_stream_offsets[3] = { command.vb_offset, command.tb_offset, command.pb_offset };
            cgpu_render_encoder_bind_index_buffer(ctx.encoder, resolved_ib, sizeof(index_t), command.ib_offset);
            cgpu_render_encoder_bind_vertex_buffers(ctx.encoder,
                3, vertex_streams, vertex_stream_strides, vertex_stream_offsets);
            cgpu_render_encoder_draw_indexed_instanced(ctx.encoder,
                command.index_count,command.first_index,
                1, 0, 0);
        }
        SkrDelete(canvas_group_data);
    });
}

} }