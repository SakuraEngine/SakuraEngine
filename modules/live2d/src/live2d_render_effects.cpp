#include "platform/memory.h"
#include "platform/vfs.h"
#include "platform/time.h"
#include "platform/thread.h"
#include "math/vectormath.hpp"
#include "utils/make_zeroed.hpp"

#include "ecs/type_builder.hpp"
#include "ecs/callback.hpp"

#include "skr_live2d/skr_live2d.h"
#include "skr_live2d/render_effect.h"
#include "skr_live2d/render_model.h"

#include "live2d_helpers.hpp"
#include "Framework/Math/CubismMatrix44.hpp"
#include "Framework/Math/CubismViewMatrix.hpp"

#include "render_graph/api.h"

#include "skr_renderer/primitive_draw.h"
#include "skr_renderer/skr_renderer.h"
#include "skr_renderer/mesh_resource.h"

#include "live2d_model_pass.hpp"

#include "tracy/Tracy.hpp"

SKR_IMPORT_API struct dual_storage_t* skr_runtime_get_dual_storage();

static struct RegisterComponentskr_live2d_render_model_comp_tHelper
{
    RegisterComponentskr_live2d_render_model_comp_tHelper()
    {
        using namespace skr::guid::literals;

        dual_type_description_t desc;
        desc.name = "skr_live2d_render_model_comp_t";
        
        desc.size = sizeof(skr_live2d_render_model_comp_t);
        desc.entityFieldsCount = 1;
        static intptr_t entityFields[] = {0};
        desc.entityFields = (intptr_t)entityFields;
        desc.guid = "63524b75-b86d-4b34-ba59-b600eb4b415b"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_live2d_render_model_comp_t);
        type = dualT_register_type(&desc);
    }
    dual_type_index_t type = DUAL_NULL_TYPE;
} _RegisterComponentskr_live2d_render_model_comp_tHelper;

template<>
struct dual_id_of<skr_live2d_render_model_comp_t>
{
    SKR_LIVE2D_API static dual_type_index_t get()
    {
        return _RegisterComponentskr_live2d_render_model_comp_tHelper.type;
    }
};

typedef struct live2d_effect_identity_t {
    dual_entity_t game_entity;
} live2d_effect_identity_t;
skr_render_effect_name_t live2d_effect_name = "Live2DEffect";
struct RenderEffectLive2D : public IRenderEffectProcessor {
    ~RenderEffectLive2D() = default;

    skr_vfs_t* resource_vfs = nullptr;
    const char* push_constants_name = "push_constants";
    // this is a view object, later we will expose it to the world
    live2d_render_view_t view_;

    dual_query_t* effect_query = nullptr;
    dual::type_builder_t type_builder;
    dual_type_index_t identity_type = {};

    void on_register(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        // make identity component type
        {
            auto guid = make_zeroed<skr_guid_t>();
            dual_make_guid(&guid);
            auto desc = make_zeroed<dual_type_description_t>();
            desc.name = "live2d_identity";
            desc.size = sizeof(live2d_effect_identity_t);
            desc.guid = guid;
            desc.alignment = alignof(live2d_effect_identity_t);
            identity_type = dualT_register_type(&desc);
        }
        type_builder
            .with(identity_type)
            .with<skr_live2d_render_model_comp_t>();
        effect_query = dualQ_from_literal(storage, "[in]live2d_identity");
        skr_init_timer(&motion_timer);
        // prepare render resources
        prepare_pipeline_settings();
        prepare_pipeline(renderer);
        prepare_mask_pipeline(renderer);
        prepare_masked_pipeline(renderer);
        // prepare_geometry_resources(renderer);
        skr_live2d_render_view_reset(&view_);
    }

    void on_unregister(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        auto sweepFunction = [&](dual_chunk_view_t* r_cv) {
        auto meshes = (skr_live2d_render_model_comp_t*)dualV_get_owned_ro(r_cv, dual_id_of<skr_live2d_render_model_comp_t>::get());
            for (uint32_t i = 0; i < r_cv->count; i++)
            {
                while (!meshes[i].vram_request.is_ready()) {}
                if (meshes[i].vram_request.render_model)
                {
                    skr_live2d_render_model_free(meshes[i].vram_request.render_model);
                }
                while (!meshes[i].ram_request.is_ready()) {}
                if (meshes[i].ram_request.model_resource)
                {
                    skr_live2d_model_free(meshes[i].ram_request.model_resource);
                }
            }
        };
        dualQ_get_views(effect_query, DUAL_LAMBDA(sweepFunction));
        free_pipeline(renderer);
        free_mask_pipeline(renderer);
        free_masked_pipeline(renderer);
    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        *set = type_builder.build();
    }

    dual_type_index_t get_identity_type() override
    {
        return identity_type;
    }

    void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
    {

    }

    eastl::vector_map<CGPUTextureViewId, CGPUDescriptorSetId> descriptor_sets;
    eastl::vector_map<skr_live2d_render_model_id, skr::span<const uint32_t>> sorted_drawable_list;
    const float kMotionFramesPerSecond = 160.0f;
    STimer motion_timer;
    uint32_t last_ms = 0;
    uint32_t produce_drawcall(IPrimitiveRenderPass* pass, dual_storage_t* storage) override
    {
        if (strcmp(pass->identity(), live2d_pass_name) == 0)
        {
            uint32_t c = 0;
            auto counterF = [&](dual_chunk_view_t* r_cv) {
                auto models = (skr_live2d_render_model_comp_t*)dualV_get_owned_ro(r_cv, dual_id_of<skr_live2d_render_model_comp_t>::get());
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    if (models[i].vram_request.is_ready())
                    {
                        auto&& render_model = models[i].vram_request.render_model;
                        auto&& model_resource = models[i].ram_request.model_resource;
                        updateModelMotion(render_model);
                        updateTexture(render_model);
                        const auto list = skr_live2d_model_get_sorted_drawable_list(model_resource);
                        sorted_drawable_list[render_model] = { list , render_model->index_buffer_views.size() };
                        // grow drawcall size
                        c += render_model->primitive_commands.size();
                    }
                }
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(counterF));
            push_constants.clear();
            push_constants.resize(c);
            return c;
        }
        return 0;
    }

    void peek_drawcall(IPrimitiveRenderPass* pass, skr_primitive_draw_list_view_t* drawcalls, dual_storage_t* storage) override
    {
        if (strcmp(pass->identity(), live2d_pass_name) == 0)
        {
            auto storage = skr_runtime_get_dual_storage();
            uint32_t r_idx = 0;
            uint32_t dc_idx = 0;
            auto r_effect_callback = [&](dual_chunk_view_t* r_cv) {
                auto identities = (live2d_effect_identity_t*)dualV_get_owned_rw(r_cv, identity_type);
                auto unbatched_g_ents = (dual_entity_t*)identities;
                auto r_ents = dualV_get_entities(r_cv);
                auto meshes = (skr_live2d_render_model_comp_t*)dualV_get_owned_ro(r_cv, dual_id_of<skr_live2d_render_model_comp_t>::get());
                if (unbatched_g_ents)
                {
                    auto g_batch_callback = [&](dual_chunk_view_t* g_cv) {
                        auto g_ents = (dual_entity_t*)dualV_get_entities(g_cv);
                        for (uint32_t g_idx = 0; g_idx < g_cv->count; g_idx++, r_idx++)
                        {
                            auto g_ent = g_ents[g_idx];(void)g_ent;
                            auto r_ent = r_ents[r_idx];(void)r_ent;
                            
                            if (meshes)
                            {
                                const auto& async_request = meshes[r_idx].vram_request;
                                if (async_request.is_ready())
                                {
                                    const auto& render_model = async_request.render_model;
                                    const auto& cmds = render_model->primitive_commands;
                                    for (auto drawable : sorted_drawable_list[render_model])
                                    {
                                        const auto& cmd = cmds[drawable];
                                        // resources may be ready after produce_drawcall, so we need to check it here
                                        if (push_constants.size() <= dc_idx) return;
    
                                        // push_constants[dc_idx].projection_matrix = skr::math::float4x4();
                                        // push_constants[dc_idx].clip_matrix = skr::math::multiply(view, proj);
                                        skr_live2d_model_get_drawable_colors(render_model->model_resource_id, drawable,
                                            &push_constants[dc_idx].multiply_color,
                                            &push_constants[dc_idx].screen_color);
                                        push_constants[dc_idx].base_color = { 1.f, 1.f, 1.f, 1.f };
                                        auto visibility = skr_live2d_model_get_drawable_is_visible(render_model->model_resource_id, drawable);
                                        auto& drawcall = drawcalls->drawcalls[dc_idx];
                                        if (!visibility)
                                        {
                                            drawcall.desperated = true;
                                            drawcall.pipeline = pipeline;
                                        }
                                        else
                                        {
                                            drawcall.pipeline = pipeline;
                                            drawcall.push_const_name = push_constants_name;
                                            drawcall.push_const = (const uint8_t*)(push_constants.data() + dc_idx);
                                            drawcall.index_buffer = *cmd.ibv;
                                            drawcall.vertex_buffers = cmd.vbvs.data();
                                            drawcall.vertex_buffer_count = cmd.vbvs.size();
                                            {
                                                auto texture_view = skr_live2d_render_model_get_texture_view(render_model, drawable);
                                                drawcall.descriptor_set_count = 1;
                                                drawcall.descriptor_sets = &descriptor_sets[texture_view];
                                            }
                                        }
                                        dc_idx++;
                                    }
                                }
                            }
                        }
                    };
                    dualS_batch(storage, unbatched_g_ents, r_cv->count, DUAL_LAMBDA(g_batch_callback));
                }
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(r_effect_callback));
        }
    }
protected:
    void updateTexture(skr_live2d_render_model_id render_model)
    {
        // create descriptor sets if not existed
        const auto ib_c = render_model->index_buffer_views.size();
        for (uint32_t j = 0; j < ib_c; j++)
        {
            auto texture_view = skr_live2d_render_model_get_texture_view(render_model, j);
            auto iter = descriptor_sets.find(texture_view);
            if (iter == descriptor_sets.end())
            {
                CGPUDescriptorSetDescriptor desc_set_desc = {};
                desc_set_desc.root_signature = pipeline->root_signature;
                desc_set_desc.set_index = 0;
                auto desc_set = cgpu_create_descriptor_set(pipeline->device, &desc_set_desc);
                descriptor_sets[texture_view] = desc_set;
                CGPUDescriptorData datas[1];
                datas[0].name = "color_texture";
                datas[0].count = 1;
                datas[0].textures = &texture_view;
                datas[0].binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
                cgpu_update_descriptor_set(desc_set, datas, 1);
            }
        }
    }

    void updateModelMotion(skr_live2d_render_model_id render_model)
    {
        const auto model_resource = render_model->model_resource_id;
        last_ms = skr_timer_get_msec(&motion_timer, true);
        static float delta_sum = 0.f;
        delta_sum += ((float)last_ms / 1000.f);
        if (delta_sum > (1.f / kMotionFramesPerSecond))
        {
            skr_live2d_model_update(model_resource, delta_sum);
            delta_sum = 0.f;
            // update buffer
            if (render_model->use_dynamic_buffer)
            {
                const auto vb_c = render_model->vertex_buffer_views.size();
                for (uint32_t j = 0; j < vb_c; j++)
                {
                    auto& view = render_model->vertex_buffer_views[j];
                    const void* pSrc = nullptr;
                    uint32_t vcount = 0;
                    // pos-uv-pos-uv...
                    if (j % 2 == 0)
                    {
                        pSrc = skr_live2d_model_get_drawable_vertex_positions(
                            model_resource, j / 2, &vcount);
                    }
                    else
                    {
                        pSrc = skr_live2d_model_get_drawable_vertex_uvs(
                            model_resource, (j - 1) / 2, &vcount);
                    }
                    memcpy((uint8_t*)view.buffer->cpu_mapped_address + view.offset, pSrc, vcount * view.stride);
                }
            }
        }
    }

    void prepare_pipeline_settings();
    void prepare_pipeline(ISkrRenderer* renderer);
    void prepare_mask_pipeline(ISkrRenderer* renderer);
    void prepare_masked_pipeline(ISkrRenderer* renderer);
    void free_pipeline(ISkrRenderer* renderer);
    void free_mask_pipeline(ISkrRenderer* renderer);
    void free_masked_pipeline(ISkrRenderer* renderer);
    uint32_t* read_shader_bytes(ISkrRenderer* renderer, const char* name, uint32_t* out_length);
    CGPUShaderLibraryId create_shader_library(ISkrRenderer* renderer, const char* name, ECGPUShaderStage stage);

    struct PushConstants {
        skr::math::float4x4 projection_matrix;
        skr::math::float4x4 clip_matrix;
        skr_float4_t base_color;
        skr_float4_t multiply_color;
        skr_float4_t screen_color;
        skr_float4_t channel_flag;
    };
    eastl::vector<PushConstants> push_constants;

    CGPUVertexLayout vertex_layout = {};
    CGPURasterizerStateDescriptor rs_state = {};
    CGPUDepthStateDescriptor depth_state = {};

    CGPURenderPipelineId pipeline = nullptr;
    CGPURenderPipelineId masked_pipeline = nullptr;
    CGPURenderPipelineId mask_pipeline = nullptr;
};

RenderPassLive2D* live2d_pass = new RenderPassLive2D();
RenderEffectLive2D* live2d_effect = new RenderEffectLive2D();

uint32_t* RenderEffectLive2D::read_shader_bytes(ISkrRenderer* renderer, const char* name, uint32_t* out_length)
{
    const auto device = renderer->get_cgpu_device();
    const auto backend = device->adapter->instance->backend;
    eastl::string shader_name = name;
    shader_name.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto shader_file = skr_vfs_fopen(resource_vfs, shader_name.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const uint32_t shader_length = (uint32_t)skr_vfs_fsize(shader_file);
    auto shader_bytes = (uint32_t*)sakura_malloc(shader_length);
    skr_vfs_fread(shader_file, shader_bytes, 0, shader_length);
    skr_vfs_fclose(shader_file);
    if (out_length) *out_length = shader_length;
    return shader_bytes;
}

CGPUShaderLibraryId RenderEffectLive2D::create_shader_library(ISkrRenderer* renderer, const char* name, ECGPUShaderStage stage)
{
    const auto device = renderer->get_cgpu_device();
    uint32_t shader_length = 0;
    uint32_t* shader_bytes = read_shader_bytes(renderer, name, &shader_length);
    CGPUShaderLibraryDescriptor shader_desc = {};
    shader_desc.name = name;
    shader_desc.stage = stage;
    shader_desc.code = shader_bytes;
    shader_desc.code_size = shader_length;
    CGPUShaderLibraryId shader = cgpu_create_shader_library(device, &shader_desc);
    sakura_free(shader_bytes);
    return shader;
}

void RenderEffectLive2D::prepare_pipeline_settings()
{
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 2;

    rs_state.cull_mode = CGPU_CULL_MODE_NONE;
    rs_state.fill_mode = CGPU_FILL_MODE_SOLID;
    rs_state.front_face = CGPU_FRONT_FACE_CCW;
    rs_state.slope_scaled_depth_bias = 0.f;
    rs_state.enable_depth_clamp = true;
    rs_state.enable_scissor = true;
    rs_state.enable_multi_sample = false;
    rs_state.depth_bias = 0;

    depth_state.depth_write = false;
    depth_state.depth_test = false;
}

void RenderEffectLive2D::prepare_pipeline(ISkrRenderer* renderer)
{
    const auto device = renderer->get_cgpu_device();

    CGPUShaderLibraryId vs = create_shader_library(renderer, "shaders/live2d_vs", CGPU_SHADER_STAGE_VERT);
    CGPUShaderLibraryId ps = create_shader_library(renderer, "shaders/live2d_ps", CGPU_SHADER_STAGE_FRAG);

    CGPUPipelineShaderDescriptor ppl_shaders[2];
    CGPUPipelineShaderDescriptor& ppl_vs = ppl_shaders[0];
    ppl_vs.library = vs;
    ppl_vs.stage = CGPU_SHADER_STAGE_VERT;
    ppl_vs.entry = "main";
    CGPUPipelineShaderDescriptor& ppl_ps = ppl_shaders[1];
    ppl_ps.library = ps;
    ppl_ps.stage = CGPU_SHADER_STAGE_FRAG;
    ppl_ps.entry = "main";

    const char* static_sampler_name = "color_sampler";
    auto static_sampler = skr_renderer_get_linear_sampler();
    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = skr_renderer_get_root_signature_pool();
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_vs;
    rp_desc.fragment_shader = &ppl_ps;
    rp_desc.render_target_count = 1;
    const auto fmt = CGPU_FORMAT_B8G8R8A8_UNORM;
    rp_desc.color_formats = &fmt;
    rp_desc.depth_stencil_format = depth_format;

    CGPUBlendStateDescriptor blend_state = {};
    blend_state.blend_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.masks[0] = CGPU_COLOR_MASK_ALL;
    blend_state.independent_blend = false;

    // Normal
    blend_state.src_factors[0] = CGPU_BLEND_CONST_SRC_ALPHA;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    // Multiply
    blend_state.src_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    rp_desc.blend_state = &blend_state;
    rp_desc.rasterizer_state = &rs_state;
    rp_desc.depth_state = &depth_state;
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);

    cgpu_free_shader_library(vs);
    cgpu_free_shader_library(ps);
}

void RenderEffectLive2D::free_pipeline(ISkrRenderer* renderer)
{
    auto sig_to_free = pipeline->root_signature;
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(sig_to_free);
}

void RenderEffectLive2D::prepare_masked_pipeline(ISkrRenderer* renderer)
{
    const auto device = renderer->get_cgpu_device();

    CGPUShaderLibraryId vs = create_shader_library(renderer, "shaders/live2d_vs", CGPU_SHADER_STAGE_VERT);
    CGPUShaderLibraryId ps = create_shader_library(renderer, "shaders/live2d_ps", CGPU_SHADER_STAGE_FRAG);

    CGPUPipelineShaderDescriptor ppl_shaders[2];
    CGPUPipelineShaderDescriptor& ppl_vs = ppl_shaders[0];
    ppl_vs.library = vs;
    ppl_vs.stage = CGPU_SHADER_STAGE_VERT;
    ppl_vs.entry = "main";
    CGPUPipelineShaderDescriptor& ppl_ps = ppl_shaders[1];
    ppl_ps.library = ps;
    ppl_ps.stage = CGPU_SHADER_STAGE_FRAG;
    ppl_ps.entry = "main";

    const char* static_sampler_name = "color_sampler";
    auto static_sampler = skr_renderer_get_linear_sampler();
    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = skr_renderer_get_root_signature_pool();
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_vs;
    rp_desc.fragment_shader = &ppl_ps;
    rp_desc.render_target_count = 1;
    const auto fmt = CGPU_FORMAT_B8G8R8A8_UNORM;
    rp_desc.color_formats = &fmt;
    rp_desc.depth_stencil_format = depth_format;

    CGPUBlendStateDescriptor blend_state = {};
    blend_state.blend_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.masks[0] = CGPU_COLOR_MASK_ALL;
    blend_state.independent_blend = false;

    // Normal
    blend_state.src_factors[0] = CGPU_BLEND_CONST_SRC_ALPHA;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    // Multiply
    blend_state.src_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    rp_desc.blend_state = &blend_state;
    rp_desc.rasterizer_state = &rs_state;
    rp_desc.depth_state = &depth_state;
    masked_pipeline = cgpu_create_render_pipeline(device, &rp_desc);

    cgpu_free_shader_library(vs);
    cgpu_free_shader_library(ps);
}

void RenderEffectLive2D::free_masked_pipeline(ISkrRenderer* renderer)
{
    auto sig_to_free = masked_pipeline->root_signature;
    cgpu_free_render_pipeline(masked_pipeline);
    cgpu_free_root_signature(sig_to_free);
}

void RenderEffectLive2D::prepare_mask_pipeline(ISkrRenderer* renderer)
{
    const auto device = renderer->get_cgpu_device();
    
    CGPUShaderLibraryId vs = create_shader_library(renderer, "shaders/live2d_vs", CGPU_SHADER_STAGE_VERT);
    CGPUShaderLibraryId ps = create_shader_library(renderer, "shaders/live2d_ps", CGPU_SHADER_STAGE_FRAG);

    CGPUPipelineShaderDescriptor ppl_shaders[2];
    CGPUPipelineShaderDescriptor& ppl_vs = ppl_shaders[0];
    ppl_vs.library = vs;
    ppl_vs.stage = CGPU_SHADER_STAGE_VERT;
    ppl_vs.entry = "main";
    CGPUPipelineShaderDescriptor& ppl_ps = ppl_shaders[1];
    ppl_ps.library = ps;
    ppl_ps.stage = CGPU_SHADER_STAGE_FRAG;
    ppl_ps.entry = "main";

    const char* static_sampler_name = "color_sampler";
    auto static_sampler = skr_renderer_get_linear_sampler();
    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = skr_renderer_get_root_signature_pool();
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_vs;
    rp_desc.fragment_shader = &ppl_ps;
    rp_desc.render_target_count = 1;
    const auto fmt = CGPU_FORMAT_B8G8R8A8_UNORM;
    rp_desc.color_formats = &fmt;
    rp_desc.depth_stencil_format = depth_format;

    CGPUBlendStateDescriptor blend_state = {};
    blend_state.blend_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.masks[0] = CGPU_COLOR_MASK_ALL;
    blend_state.independent_blend = false;

    // Normal
    blend_state.src_factors[0] = CGPU_BLEND_CONST_SRC_ALPHA;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    // Multiply
    blend_state.src_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    rp_desc.blend_state = &blend_state;
    rp_desc.rasterizer_state = &rs_state;
    rp_desc.depth_state = &depth_state;
    mask_pipeline = cgpu_create_render_pipeline(device, &rp_desc);

    cgpu_free_shader_library(vs);
    cgpu_free_shader_library(ps);
}

void RenderEffectLive2D::free_mask_pipeline(ISkrRenderer* renderer)
{
    auto sig_to_free = mask_pipeline->root_signature;
    cgpu_free_render_pipeline(mask_pipeline);
    cgpu_free_root_signature(sig_to_free);
}

void skr_live2d_initialize_render_effects(live2d_render_graph_t* render_graph, struct skr_vfs_t* resource_vfs)
{
    auto renderer = skr_renderer_get_renderer();
    live2d_effect->resource_vfs = resource_vfs;
    skr_renderer_register_render_pass(renderer, live2d_pass_name, live2d_pass);
    skr_renderer_register_render_effect(renderer, live2d_effect_name, live2d_effect);
}

void skr_live2d_finalize_render_effects(live2d_render_graph_t* render_graph, struct skr_vfs_t* resource_vfs)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_remove_render_pass(renderer, live2d_pass_name);
    skr_renderer_remove_render_effect(renderer, live2d_effect_name);
    delete live2d_effect;
    delete live2d_pass;
}