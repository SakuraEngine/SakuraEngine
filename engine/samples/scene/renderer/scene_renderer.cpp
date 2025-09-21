#include "SkrGraphics/api.h"
#include "SkrGraphics/flags.h"
#include "SkrProfile/profile.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrCore/time.h"
#include "SkrCore/platform/vfs.h"
#include "SkrRuntime/ecs/query.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "scene_renderer.hpp"
#include "helper.hpp"

struct SceneRendererImpl : public skr::SceneRenderer
{
    const char8_t* push_constants_name = u8"push_constants";
    const ECGPUFormat color_format = CGPU_FORMAT_R8G8B8A8_UNORM;
    const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT;

    skr_vfs_t* resource_vfs;
    utils::Camera* mp_camera;
    CGPURenderPipelineId pipeline = nullptr;
    CGPUVertexLayout vertex_layout = {};
    CGPURasterizerStateDescriptor rs_state = {};
    CGPUDepthStateDescriptor ds_desc = {};

    virtual void initialize(skr::RenderDevice* render_device, skr::ecs::ECSWorld* world, struct skr_vfs_t* resource_vfs) override
    {
        this->resource_vfs = resource_vfs;
        prepare_pipeline_settings();
        prepare_pipeline(render_device);
    }
    virtual void finalize(skr::RenderDevice* renderer) override { free_pipeline(renderer); }
    virtual void set_camera(utils::Camera* camera) override { mp_camera = camera; }
    virtual utils::Camera* get_camera() const override { return mp_camera; }
    virtual CGPURenderPipelineId get_render_pso() const override { return pipeline; }

    void draw_primitives(skr::render_graph::RenderGraph* rg, skr::Span<skr_primitive_draw_t> dallcalls) override;

    void prepare_pipeline_settings();
    void prepare_pipeline(skr::RenderDevice* render_device);
    void free_pipeline(skr::RenderDevice* renderer);
};

// SceneRendererImpl* scene_effect = SkrNew<SceneRendererImpl>();
skr::SceneRenderer* skr::SceneRenderer::Create()
{
    return SkrNew<SceneRendererImpl>();
}

void skr::SceneRenderer::Destroy(skr::SceneRenderer* renderer)
{
    SkrDelete(renderer);
}

skr::SceneRenderer::~SceneRenderer() {}

void SceneRendererImpl::draw_primitives(skr::render_graph::RenderGraph* rg, skr::Span<skr_primitive_draw_t> drawcalls)
{
    for (auto& drawcall : drawcalls)
    {
        drawcall.pipeline = pipeline;
        drawcall.push_const_name = push_constants_name;
    }

    auto backbuffer = rg->get_texture(u8"backbuffer");
    const auto back_desc = rg->resolve_descriptor(backbuffer);
    auto depthbuffer = rg->get_texture(u8"render_depth");

    rg->add_render_pass(
        [this, backbuffer, depthbuffer](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
            builder.set_name(u8"scene_render_pass")
                .set_pipeline(pipeline) // captured this->pipeline
                .set_depth_stencil(depthbuffer.clear_depth(100.0f))
                .write(0, backbuffer, CGPU_LOAD_ACTION_CLEAR);
        },
        [back_desc, drawcalls](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& pass_context) {
            {
                // do render pass
                cgpu_render_encoder_set_viewport(pass_context.encoder, 0.0, 0.0, (float)back_desc->width, (float)back_desc->height, 0.0f, 1.0f);
                cgpu_render_encoder_set_scissor(pass_context.encoder, 0, 0, back_desc->width, back_desc->height);
                skr::InlineMap<CGPUXBindTableId, CGPUXMergedBindTableId, 32> merged_tables;

                for (const auto& dc : drawcalls)
                {
                    if (dc.deprecated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0))
                    {
                        continue;
                    }
                    if (dc.bind_table)
                    {
                        if (auto bd = merged_tables.find(dc.bind_table))
                        {
                            pass_context.bind(bd.value());
                        }
                        else
                        {
                            CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                            auto merged = pass_context.merge_tables(tables, 2);
                            merged_tables.add(dc.bind_table, merged);
                            pass_context.bind(merged);
                        }
                    }

                    CGPUBufferId vertex_buffers[16] = { 0 };
                    uint32_t strides[16] = { 0 };
                    uint32_t offsets[16] = { 0 };
                    for (uint32_t i = 0; i < dc.vertex_buffer_count; i++)
                    {
                        vertex_buffers[i] = dc.vertex_buffers[i].buffer;
                        strides[i] = dc.vertex_buffers[i].stride;
                        offsets[i] = dc.vertex_buffers[i].offset;
                    }

                    cgpu_render_encoder_bind_index_buffer(pass_context.encoder, dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
                    cgpu_render_encoder_bind_vertex_buffers(pass_context.encoder, dc.vertex_buffer_count, vertex_buffers, strides, offsets);
                    cgpu_render_encoder_push_constants(pass_context.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                    cgpu_render_encoder_draw_indexed_instanced(pass_context.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0); // 3 vertices, 1 instance
                }
            }
        });
}

void SceneRendererImpl::prepare_pipeline_settings()
{
    // max vertex attributes is 15
    vertex_layout.attributes[0] = { u8"position", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr::float3), CGPU_INPUT_RATE_VERTEX }; // per vertex position
    vertex_layout.attributes[1] = { u8"uv", 2, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr::float2), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { u8"normal", 3, CGPU_FORMAT_R32G32B32_SFLOAT, 2, 0, sizeof(skr::float3), CGPU_INPUT_RATE_VERTEX }; // per vertex normal

    vertex_layout.attribute_count = 3;

    // rasterize state
    rs_state.cull_mode = CGPU_CULL_MODE_BACK;
    rs_state.depth_bias = 0;
    rs_state.fill_mode = CGPU_FILL_MODE_SOLID;
    rs_state.front_face = CGPU_FRONT_FACE_CCW;

    ds_desc.depth_func = CGPU_CMP_LEQUAL;
    // ds_desc.depth_func = CGPU_CMP_GEQUAL;
    ds_desc.depth_write = true;
    ds_desc.depth_test = true;
}

void SceneRendererImpl::prepare_pipeline(skr::RenderDevice* render_device)
{
    const auto cgpu_device = render_device->get_cgpu_device();
    CGPUShaderLibraryId vs = utils::create_shader_library(render_device, resource_vfs, u8"shaders/scene/debug.vs", CGPU_SHADER_STAGE_VERT);
    CGPUShaderLibraryId fs = utils::create_shader_library(render_device, resource_vfs, u8"shaders/scene/debug.fs", CGPU_SHADER_STAGE_FRAG);

    CGPUShaderEntryDescriptor ppl_shaders[2];
    CGPUShaderEntryDescriptor& ppl_vs = ppl_shaders[0];
    ppl_vs.library = vs;
    ppl_vs.entry = u8"vs";
    CGPUShaderEntryDescriptor& ppl_fs = ppl_shaders[1];
    ppl_fs.library = fs;
    ppl_fs.entry = u8"fs";

    const char8_t* static_sampler_name = u8"color_sampler";
    auto static_sampler = render_device->get_linear_sampler();
    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    rs_desc.pool = render_device->get_root_signature_pool();
    auto root_sig = cgpu_create_root_signature(cgpu_device, &rs_desc);

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

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_vs;
    rp_desc.fragment_shader = &ppl_fs;
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &color_format;
    rp_desc.depth_stencil_format = depth_format;

    rp_desc.blend_state = &blend_state;
    rp_desc.rasterizer_state = &rs_state;
    rp_desc.depth_state = &ds_desc;
    pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);

    cgpu_free_shader_library(fs);
    cgpu_free_shader_library(vs);
}

void SceneRendererImpl::free_pipeline(skr::RenderDevice* renderer)
{
    auto sig_to_free = pipeline->root_signature;
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(sig_to_free);
}
