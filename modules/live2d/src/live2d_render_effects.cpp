#include "platform/memory.h"
#include "platform/vfs.h"
#include "platform/time.h"
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

#include "tracy/Tracy.hpp"

SKR_IMPORT_API struct dual_storage_t* skr_runtime_get_dual_storage();
const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT;

skr_render_pass_name_t live2d_pass_name = "Live2DPass";
struct RenderPassLive2D : public IPrimitiveRenderPass {
    void on_register(ISkrRenderer* renderer) override
    {

    }

    void on_unregister(ISkrRenderer* renderer) override
    {

    }

    void execute(skr::render_graph::RenderGraph* renderGraph, skr_primitive_draw_list_view_t drawcalls) override
    {
        auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            builder.set_name("depth")
                .extent(900, 900)
                .format(depth_format)
                .owns_memory()
                .allow_depth_stencil();
        });
        if (drawcalls.count)
        {
            renderGraph->add_render_pass(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                const auto out_color = renderGraph->get_texture("backbuffer");
                const auto depth_buffer = renderGraph->get_texture("depth");
                builder.set_name("live2d_forward_pass")
                    // we know that the drawcalls always have a same pipeline
                    .set_pipeline(drawcalls.drawcalls->pipeline)
                    .write(0, out_color, CGPU_LOAD_ACTION_CLEAR)
                    .set_depth_stencil(depth_buffer);
            },
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)900, (float)900,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, 900, 900);
                for (uint32_t i = 0; i < drawcalls.count - 2; i++)
                {
                    ZoneScopedN("DrawCall");

                    auto&& dc = drawcalls.drawcalls[i];
                    if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;
                    {
                        ZoneScopedN("BindTextures");
                        for (uint32_t j = 0; j < dc.descriptor_set_count; j++)
                        {
                            cgpu_render_encoder_bind_descriptor_set(stack.encoder, dc.descriptor_sets[j]);
                        }
                    }
                    {
                        ZoneScopedN("BindGeometry");
                        cgpu_render_encoder_bind_index_buffer(stack.encoder, dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
                        CGPUBufferId vertex_buffers[2] = {
                            dc.vertex_buffers[0].buffer, dc.vertex_buffers[1].buffer
                        };
                        const uint32_t strides[2] = {
                            dc.vertex_buffers[0].stride, dc.vertex_buffers[1].stride
                        };
                        const uint32_t offsets[2] = {
                            dc.vertex_buffers[0].offset, dc.vertex_buffers[1].offset
                        };
                        cgpu_render_encoder_bind_vertex_buffers(stack.encoder, 2, vertex_buffers, strides, offsets);
                    }
                    {
                        ZoneScopedN("PushConstants");
                        cgpu_render_encoder_push_constants(stack.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                    }
                    {
                        ZoneScopedN("DrawIndexed");
                        cgpu_render_encoder_draw_indexed_instanced(stack.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
                    }
                }
            });
        }
    }

    skr_render_pass_name_t identity() const override
    {
        return live2d_pass_name;
    }
};
RenderPassLive2D* live2d_pass = new RenderPassLive2D();

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
        prepare_pipeline(renderer);
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
                        last_ms = skr_timer_get_msec(&motion_timer, true);
                        skr_live2d_model_update(model_resource, (float)last_ms / 1000.f);
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
                        // create descriptor sets if not existed
                        {
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
    void prepare_pipeline(ISkrRenderer* renderer);
    void free_pipeline(ISkrRenderer* renderer);

    struct PushConstants {
        skr::math::float4x4 projection_matrix;
        skr::math::float4x4 clip_matrix;
        skr_float4_t base_color;
        skr_float4_t multiply_color;
        skr_float4_t screen_color;
        skr_float4_t channel_flag;
    };
    eastl::vector<PushConstants> push_constants;
    CGPURenderPipelineId pipeline = nullptr;
};
RenderEffectLive2D* live2d_effect = new RenderEffectLive2D();

void RenderEffectLive2D::prepare_pipeline(ISkrRenderer* renderer)
{
    const auto device = renderer->get_cgpu_device();
    const auto backend = device->adapter->instance->backend;

    uint32_t *vs_bytes, *ps_bytes;
    eastl::string vsname = u8"shaders/live2d_vertex_shader";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto vsfile = skr_vfs_fopen(resource_vfs, vsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const uint32_t vs_length = (uint32_t)skr_vfs_fsize(vsfile);
    vs_bytes = (uint32_t*)sakura_malloc(vs_length);
    skr_vfs_fread(vsfile, vs_bytes, 0, vs_length);
    skr_vfs_fclose(vsfile);

    eastl::string psname = u8"shaders/live2d_pixel_shader";
    psname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto psfile = skr_vfs_fopen(resource_vfs, psname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const uint32_t ps_length = (uint32_t)skr_vfs_fsize(psfile);
    ps_bytes = (uint32_t*)sakura_malloc(ps_length);
    skr_vfs_fread(psfile, ps_bytes, 0, ps_length);
    skr_vfs_fclose(psfile);

    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "live2d_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "live2d_pixel_shader";
    ps_desc.stage = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code = ps_bytes;
    ps_desc.code_size = ps_length;
    CGPUShaderLibraryId vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId ps = cgpu_create_shader_library(device, &ps_desc);
    sakura_free(vs_bytes);
    sakura_free(ps_bytes);

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

    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 2;

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

    CGPURasterizerStateDescriptor rs_state = {};
    rs_state.cull_mode = CGPU_CULL_MODE_NONE;
    rs_state.fill_mode = CGPU_FILL_MODE_SOLID;
    rs_state.front_face = CGPU_FRONT_FACE_CCW;
    rs_state.slope_scaled_depth_bias = 0.f;
    rs_state.enable_depth_clamp = true;
    rs_state.enable_scissor = true;
    rs_state.enable_multi_sample = false;
    rs_state.depth_bias = 0;
    rp_desc.rasterizer_state = &rs_state;

    CGPUDepthStateDescriptor depth_state = {};
    depth_state.depth_write = false;
    depth_state.depth_test = false;
    rp_desc.depth_state = &depth_state;

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