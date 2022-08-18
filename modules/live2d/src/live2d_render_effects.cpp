#include "platform/memory.h"
#include "platform/vfs.h"
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

#include "skr_renderer/primitive_draw.h"
#include "skr_renderer/skr_renderer.h"
#include "skr_renderer/mesh_resource.h"

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
    const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT;
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
        type_builder.with(identity_type)
            .with<skr_live2d_render_model_comp_t>();
        effect_query = dualQ_from_literal(storage, "[in]live2d_identity");
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
                        c += 1;
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
        
    }
protected:
    void prepare_pipeline(ISkrRenderer* renderer);
    void free_pipeline(ISkrRenderer* renderer);

    struct PushConstants {
        skr::math::float4x4 world;
        skr::math::float4x4 view_proj;
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

    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = skr_renderer_get_root_signature_pool();
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
    CGPURasterizerStateDescriptor raster_desc = {};
    raster_desc.cull_mode = CGPU_CULL_MODE_BACK;
    raster_desc.depth_bias = 0;
    raster_desc.fill_mode = CGPU_FILL_MODE_SOLID;
    raster_desc.front_face = CGPU_FRONT_FACE_CCW;
    rp_desc.rasterizer_state = &raster_desc;
    CGPUDepthStateDescriptor ds_desc = {};
    ds_desc.depth_func = CGPU_CMP_LEQUAL;
    ds_desc.depth_write = true;
    ds_desc.depth_test = true;
    rp_desc.depth_state = &ds_desc;
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