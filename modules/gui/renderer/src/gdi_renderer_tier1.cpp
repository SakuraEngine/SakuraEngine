#include "SkrGuiRenderer/gdi_renderer.hpp"
#include "utils/cartesian_product.hpp"

#include "rtm/qvvf.h"
#include <cmath>

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

inline static void read_shader_bytes(const char* virtual_path, uint32_t** bytes, uint32_t* length, ECGPUBackend backend)
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

CGPURenderPipelineId GDIRenderer_RenderGraph::createRenderPipeline(
    GDIRendererPipelineAttributes attributes, ECGPUSampleCount sample_count)
{
    const bool use_texture = attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED;
    uint32_t *vs_bytes = nullptr, vs_length = 0;
    uint32_t *fs_bytes = nullptr, fs_length = 0;
    read_shader_bytes("GUI/vertex", &vs_bytes, &vs_length, device->adapter->instance->backend);
    if (use_texture)
    {
        read_shader_bytes("GUI/pixel2", &fs_bytes, &fs_length, device->adapter->instance->backend);
    }
    else
    {
        read_shader_bytes("GUI/pixel", &fs_bytes, &fs_length, device->adapter->instance->backend);
    }
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

    const char* static_sampler_name = "color_sampler";
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.pool = rs_pool;
    if ((!(attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_CUSTOM_SAMPLER)) && use_texture)
    {
        rs_desc.static_sampler_count = 1;
        rs_desc.static_sampler_names = &static_sampler_name;
        rs_desc.static_samplers = &static_color_sampler;
    }
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &target_format;
    rp_desc.depth_stencil_format = CGPU_FORMAT_D32_SFLOAT;
    
    CGPURasterizerStateDescriptor rs_state = {};
    rs_state.cull_mode = CGPU_CULL_MODE_NONE;
    rs_state.fill_mode = CGPU_FILL_MODE_SOLID;
    rs_state.front_face = CGPU_FRONT_FACE_CCW;
    rs_state.slope_scaled_depth_bias = 0.f;
    rs_state.enable_depth_clamp = false;
    rs_state.enable_scissor = true;
    rs_state.enable_multi_sample = false;
    rs_state.depth_bias = 0;
    rp_desc.rasterizer_state = &rs_state;

    CGPUDepthStateDescriptor depth_state = {};
    depth_state.depth_test = attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_TEST_Z;
    depth_state.depth_func = depth_state.depth_test ? CGPU_CMP_LEQUAL : CGPU_CMP_NEVER; 
    depth_state.depth_write = attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_WRITE_Z;
    rp_desc.depth_state = &depth_state;

    CGPUBlendStateDescriptor blend_state = {};
    for (uint32_t i = 0; i < 1; i++)
    {
        blend_state.blend_modes[i] = CGPU_BLEND_MODE_ADD; 
        blend_state.blend_alpha_modes[i] = CGPU_BLEND_MODE_ADD; 
        blend_state.masks[i] = CGPU_COLOR_MASK_ALL; 

        blend_state.src_factors[i] = CGPU_BLEND_CONST_SRC_ALPHA; 
        blend_state.dst_factors[i] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA; 
        blend_state.src_alpha_factors[i] = CGPU_BLEND_CONST_ONE;
        blend_state.dst_alpha_factors[i] = CGPU_BLEND_CONST_ZERO;
    }
    rp_desc.blend_state = &blend_state;

    rp_desc.sample_count = sample_count;

    auto pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    return pipeline;
}

CGPURenderPipelineId GDIRenderer_RenderGraph::findOrCreateRenderPipeline(GDIRendererPipelineAttributes attributes, ECGPUSampleCount sample_count)
{
    PipelineKey key = {attributes, sample_count};
    auto it = pipelines.find(key);
    if (it != pipelines.end()) return it->second;
    auto pipeline = createRenderPipeline(attributes, sample_count);
    pipelines[key] = pipeline;
    return pipeline;
}

bool validateAttributes(GDIRendererPipelineAttributes attributes)
{
    const bool use_custom_sampler = attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_CUSTOM_SAMPLER;
    const bool use_texture = attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED;
    if (use_custom_sampler && !use_texture) return false;
    return true;
}

void GDIRenderer_RenderGraph::createRenderPipelines()
{
    eastl::vector<eastl::vector<bool>> option_selections(GDI_RENDERER_PIPELINE_ATTRIBUTE_COUNT, {true, false});
    skr::cartesian_product<bool> cartesian(option_selections);
    while (cartesian.has_next())
    {
        const auto sequence = cartesian.next();
        GDIRendererPipelineAttributes attributes = 0;
        for (uint32_t i = 0; i < sequence.size(); i++)
        {
            const bool toggle = sequence[i];
            const auto flag = static_cast<EGDIRendererPipelineAttribute>(toggle ? 0x000001 << i : 0u);
            attributes |= flag;
        }
        if (!validateAttributes(attributes)) continue;
        PipelineKey key = { attributes, CGPU_SAMPLE_COUNT_1 };
        pipelines[key] = createRenderPipeline(attributes);
    }
}
// HACK

int GDIRenderer_RenderGraph::initialize(const GDIRendererDescriptor* desc) SKR_NOEXCEPT
{
    const auto pDesc = reinterpret_cast<GDIRendererDescriptor_RenderGraph*>(desc->usr_data);
    const uint32_t pos_offset = static_cast<uint32_t>(offsetof(GDIVertex, position));
    const uint32_t texcoord_offset = static_cast<uint32_t>(offsetof(GDIVertex, texcoord));
    const uint32_t aa_offset = static_cast<uint32_t>(offsetof(GDIVertex, aa));
    const uint32_t uv_offset = static_cast<uint32_t>(offsetof(GDIVertex, clipUV));
    const uint32_t uv2_offset = static_cast<uint32_t>(offsetof(GDIVertex, clipUV2));
    const uint32_t color_offset = static_cast<uint32_t>(offsetof(GDIVertex, color));
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 0, pos_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, texcoord_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "AA", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, aa_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "UV", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { "UV_Two", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv2_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[5] = { "COLOR", 1, CGPU_FORMAT_R8G8B8A8_UNORM, 0, color_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[6] = { "TRANSFORM", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 1, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    vertex_layout.attributes[7] = { "PROJECTION", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 2, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    vertex_layout.attribute_count = 8;
    
    target_format = pDesc->target_format;
    aux_service = pDesc->aux_service;
    vfs = pDesc->vfs;
    ram_service = pDesc->ram_service;
    vram_service = pDesc->vram_service;
    device = pDesc->device;
    transfer_queue = pDesc->transfer_queue;

    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_NEAREST;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_NEAREST;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_NEAREST;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    static_color_sampler = cgpu_create_sampler(device, &sampler_desc);

    CGPURootSignaturePoolDescriptor rs_pool_desc = {};
    rs_pool_desc.name = "GUI_RS_POOL";
    rs_pool = cgpu_create_root_signature_pool(device, &rs_pool_desc);

    createRenderPipelines();

    return 0;
}

int GDIRenderer_RenderGraph::finalize() SKR_NOEXCEPT
{
    aux_service = nullptr;

    auto free_rs_and_pipeline = +[](CGPURenderPipelineId pipeline)
    {
        if (!pipeline) return;
        auto rs = pipeline->root_signature;
        cgpu_free_render_pipeline(pipeline);
        cgpu_free_root_signature(rs);
    };
    for (auto [attributes, pipeline] : pipelines)
    {
        free_rs_and_pipeline(pipeline);
    }

    if (rs_pool) cgpu_free_root_signature_pool(rs_pool);
    if (static_color_sampler) cgpu_free_sampler(static_color_sampler);
    return 0;
}

void GDIRenderer_RenderGraph::render(GDIViewport* viewport, const ViewportRenderParams* params) SKR_NOEXCEPT
{
    const auto pParams = reinterpret_cast<ViewportRenderParams_RenderGraph*>(params->usr_data);
    auto rg = pParams->render_graph;
    auto viewport_data = SkrNew<GDIViewportData_RenderGraph>(viewport);
    const auto all_canvas = viewport->all_canvas();
    if (all_canvas.empty()) return;
    uint64_t vertex_count = 0u;
    uint64_t index_count = 0u;
    uint64_t transform_count = 0u;
    uint64_t projection_count = 0u;
    uint64_t command_count = 0u;
    // 1. loop prepare counters & render data
    for (auto canvas : all_canvas)
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
    
    if (!vertex_count) return;

    viewport_data->render_vertices.reserve(vertex_count);
    viewport_data->render_indices.reserve(index_count);
    viewport_data->render_transforms.reserve(transform_count);
    viewport_data->render_projections.reserve(projection_count);
    viewport_data->render_commands.reserve(command_count);
    uint64_t vb_cursor = 0u, ib_cursor = 0u, tb_cursor = 0u, pb_cursor = 0u;
    for (auto canvas : all_canvas)
    {
    for (auto element : canvas->all_elements())
    {
        const auto element_vertices = fetch_element_vertices(element);
        const auto element_indices = fetch_element_indices(element);
        const auto element_commands = fetch_element_draw_commands(element);

        // insert data
        vb_cursor = viewport_data->render_vertices.size();
        ib_cursor = viewport_data->render_indices.size();
        tb_cursor = viewport_data->render_transforms.size();
        pb_cursor = viewport_data->render_projections.size();
        viewport_data->render_vertices.insert(viewport_data->render_vertices.end(), element_vertices.begin(), element_vertices.end());
        viewport_data->render_indices.insert(viewport_data->render_indices.end(), element_indices.begin(), element_indices.end());

        // calculate z offset
        float hardware_zmin, hardware_zmax;
        float transformZ = 0.f;
        const bool use_hardware_z = support_hardware_z(&hardware_zmin, &hardware_zmax) && canvas->is_hardware_z_enabled();
        if (use_hardware_z)
        {
            const auto hardware_zrange = hardware_zmax - hardware_zmin;
            int32_t z_min = 0, z_max = 1000;
            canvas->get_zrange(&z_min, &z_max);

            // remap z range from [min, max] to [0, max - min]
            const auto element_z =  (float)::fmax(element->get_z(), z_min) - z_min;
            z_max = std::max(z_max - z_min, 0);
            z_min = 0;

            const auto z_unit = hardware_zrange / static_cast<float>(z_max - z_min);
            const auto element_hardware_z = element_z * z_unit;
            transformZ = hardware_zmax - element_hardware_z + hardware_zmin;
        }
        else
        {
            // TODO: element sort
            transformZ = 0.f;
        }

        // compose transform
        auto& transform = viewport_data->render_transforms.emplace_back();
        const auto scaleX = 1.f;
        const auto scaleY = 1.f;
        const auto scaleZ = 1.f;
        const auto transformX = 0.f;
        const auto transformY = 0.f;
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
        auto& projection = viewport_data->render_projections.emplace_back();
        skr_float2_t canvas_size;
        canvas->get_size(&canvas_size.x, &canvas_size.y);
        skr_float2_t canvas_pivot;
        canvas->get_pivot(&canvas_pivot.x, &canvas_pivot.y);
        const skr_float2_t abs_canvas_pivot = { canvas_pivot.x * canvas_size.x, canvas_pivot.y * canvas_size.y };
        const skr_float2_t zero_point =  { canvas_size.x * 0.5f, canvas_size.y * 0.5f };
        const skr_float2_t eye_position = { zero_point.x - abs_canvas_pivot.x, zero_point.y - abs_canvas_pivot.y };
        const auto view = rtm::look_at_matrix(
            {eye_position.x, eye_position.y, 0.f} /*eye*/, 
            {eye_position.x, eye_position.y, 1000.f} /*at*/,
            { 0.f, -1.f, 0.f } /*up*/
        );
        const auto proj = rtm::orthographic(canvas_size.x, canvas_size.y, 0.f, 1000.f);
        projection = rtm::matrix_mul(view, proj);

        for (const auto& command : element_commands)
        {
            GDIElementDrawCommand_RenderGraph command2 = {};
            command2.texture = command.texture;
            command2.material = command.material;
            command2.first_index = command.first_index;
            command2.index_count = command.index_count;
            command2.ib_offset = static_cast<uint32_t>(ib_cursor * sizeof(index_t));
            command2.vb_offset = static_cast<uint32_t>(vb_cursor * sizeof(GDIVertex));
            command2.tb_offset = static_cast<uint32_t>(tb_cursor * sizeof(rtm::matrix4x4f));
            command2.pb_offset = static_cast<uint32_t>(pb_cursor * sizeof(rtm::matrix4x4f));

            command2.attributes |= command2.texture ? GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED : 0;
            command2.attributes |= use_hardware_z ? GDI_RENDERER_PIPELINE_ATTRIBUTE_TEST_Z : 0;
            command2.attributes |= use_hardware_z ? GDI_RENDERER_PIPELINE_ATTRIBUTE_WRITE_Z : 0;

            viewport_data->render_commands.emplace_back(command2);
        }
    }
    }

    // 2. prepare render resource
    const uint64_t vertices_size = vertex_count * sizeof(GDIVertex);
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
    viewport_data->vertex_buffers.emplace_back(vertex_buffer);
    viewport_data->transform_buffers.emplace_back(transform_buffer);
    viewport_data->projection_buffers.emplace_back(projection_buffer);
    viewport_data->index_buffers.emplace_back(index_buffer);

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
            [upload_buffer_handle, viewport_data](render_graph::RenderGraph& g, render_graph::CopyPassContext& context){
                auto upload_buffer = context.resolve(upload_buffer_handle);
                const uint64_t vertices_count = viewport_data->render_vertices.size();
                const uint64_t indices_count = viewport_data->render_indices.size();
                const uint64_t transforms_count = viewport_data->render_transforms.size();
                const uint64_t projections_count = viewport_data->render_projections.size();

                GDIVertex* vtx_dst = (GDIVertex*)upload_buffer->cpu_mapped_address;
                index_t* idx_dst = (index_t*)(vtx_dst + vertices_count);
                rtm::matrix4x4f* transform_dst = (rtm::matrix4x4f*)(idx_dst + indices_count);
                rtm::matrix4x4f* projection_dst = (rtm::matrix4x4f*)(transform_dst + transforms_count);

                const skr::span<GDIVertex> render_vertices = viewport_data->render_vertices;
                const skr::span<index_t> render_indices = viewport_data->render_indices;
                const skr::span<rtm::matrix4x4f> render_transforms = viewport_data->render_transforms;
                const skr::span<rtm::matrix4x4f> render_projections = viewport_data->render_projections;

                memcpy(vtx_dst, render_vertices.data(), vertices_count * sizeof(GDIVertex));
                memcpy(idx_dst, render_indices.data(), indices_count * sizeof(index_t));
                memcpy(transform_dst, render_transforms.data(), transforms_count * sizeof(rtm::matrix4x4f));
                memcpy(projection_dst, render_projections.data(), projections_count * sizeof(rtm::matrix4x4f));
            });
    }

    // 4. loop & record render commands
    skr::render_graph::TextureRTVHandle target = rg->get_texture("backbuffer");
    skr::render_graph::TextureDSVHandle depth = rg->get_texture("depth");
    // skr::vector<GDIViewport*> canvas_copy(canvas_span.begin(), canvas_span.end());
    rg->add_render_pass([&](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
        ZoneScopedN("ConstructRenderPass");
        const auto back_desc = g.resolve_descriptor(target);
        builder.set_name("gdi_render_pass")
            .use_buffer(vertex_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(transform_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(projection_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(index_buffer, CGPU_RESOURCE_STATE_INDEX_BUFFER)
            .set_depth_stencil(depth.clear_depth(1.f))
            .write(0, target, CGPU_LOAD_ACTION_CLEAR);
        if (back_desc->sample_count > 1)
        {
            skr::render_graph::TextureHandle real_target = rg->get_texture("presentbuffer");
            builder.resolve_msaa(0, real_target);
        }
    },
    [this, target, viewport_data, useCVV, index_buffer, vertex_buffer, transform_buffer, projection_buffer]
    (render_graph::RenderGraph& g, render_graph::RenderPassContext& ctx) {
        ZoneScopedN("GDI-RenderPass");
        const auto target_desc = g.resolve_descriptor(target);
        auto resolved_ib = ctx.resolve(index_buffer);
        auto resolved_vb = ctx.resolve(vertex_buffer);
        auto resolved_tb = ctx.resolve(transform_buffer);
        auto resolved_pb = ctx.resolve(projection_buffer);
        CGPUBufferId vertex_streams[3] = { resolved_vb, resolved_tb, resolved_pb };
        const uint32_t vertex_stream_strides[3] = { sizeof(GDIVertex), sizeof(rtm::matrix4x4f), sizeof(rtm::matrix4x4f) };

        cgpu_render_encoder_set_viewport(ctx.encoder,
            0.0f, 0.0f,
            (float)target_desc->width,
            (float)target_desc->height,
            0.f, 1.f);
        cgpu_render_encoder_set_scissor(ctx.encoder,
            0, 0, 
            target_desc->width, target_desc->height);

        const skr::span<GDIElementDrawCommand_RenderGraph> render_commands = viewport_data->render_commands;
        PipelineKey pipeline_key_cache = { UINT32_MAX, CGPU_SAMPLE_COUNT_1 };

        for (const auto& command : render_commands)
        {
            const bool use_texture = command.texture && (command.texture->get_state() == EGDIResourceState::Okay);
            PipelineKey key = { command.attributes, target_desc->sample_count };
            if (pipeline_key_cache != key)
            {
                CGPURenderPipelineId this_pipeline = findOrCreateRenderPipeline(key.attributes, key.sample_count);
                cgpu_render_encoder_bind_pipeline(ctx.encoder, this_pipeline);
                pipeline_key_cache = key;
            }

            if (use_texture)
            {
                const auto gui_texture = static_cast<GDITexture_RenderGraph*>(command.texture);
                cgpux_render_encoder_bind_bind_table(ctx.encoder, gui_texture->bind_table);
            }

            const uint32_t vertex_stream_offsets[3] = { command.vb_offset, command.tb_offset, command.pb_offset };
            cgpu_render_encoder_bind_index_buffer(ctx.encoder, resolved_ib, sizeof(index_t), command.ib_offset);
            cgpu_render_encoder_bind_vertex_buffers(ctx.encoder,
                3, vertex_streams, vertex_stream_strides, vertex_stream_offsets);
            cgpu_render_encoder_draw_indexed_instanced(ctx.encoder,
                command.index_count,command.first_index,
                1, 0, 0);
        }
        SkrDelete(viewport_data);
    });
}

} }