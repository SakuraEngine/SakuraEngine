#include "../../cgpu/common/utils.h"
#include "ecs/callback.hpp"
#include "ecs/dual.h"
#include "ecs/type_builder.hpp"
#include "platform/thread.h"
#include "platform/window.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "gamert.h"
#include "platform/memory.h"
#include "utils/log.h"
#include "skr_renderer/skr_renderer.h"
#include "runtime_module.h"
#include "imgui/skr_imgui_rg.h"
#include "skr_scene/scene.h"
#include "skr_renderer/effect_processor.h"
#include "skr_renderer/skr_renderer.h"
#include "math/vectormath.hpp"
#include "utils/make_zeroed.hpp"
#include <mutex>

skr_render_pass_name_t forward_pass_name = "ForwardPass";
struct RenderPassForward : public IPrimitiveRenderPass {
    void on_register(ISkrRenderer* renderer) override
    {
    }

    void on_unregister(ISkrRenderer* renderer) override
    {
    }

    void execute(skr::render_graph::RenderGraph* renderGraph, const skr_primitive_draw_list_view_t* dc) override
    {
    }

    skr_render_pass_name_t identity() const override
    {
        return forward_pass_name;
    }
};
RenderPassForward* forward_pass = new RenderPassForward();

typedef struct forward_effect_identity_t {
    dual_entity_t game_entity;
} forward_effect_identity_t;
skr_render_effect_name_t forward_effect_name = "ForwardEffect";
struct RenderEffectForward : public IRenderEffectProcessor {
    ~RenderEffectForward() = default;

    void on_register(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        // make identity component type
        {
            auto guid = make_zeroed<skr_guid_t>();
            dual_make_guid(&guid);
            auto desc = make_zeroed<dual_type_description_t>();
            desc.name = "fwdIdentity";
            desc.size = sizeof(forward_effect_identity_t);
            desc.guid = guid;
            desc.alignment = alignof(forward_effect_identity_t);
            identity_type = dualT_register_type(&desc);
        }
        type_builder.with(identity_type);
        type_builder.with<skr_transform_t>();
        effect_query = dualQ_from_literal(storage, "[in]fwdIdentity");
        prepare_geometry_resources(renderer);
    }

    void on_unregister(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        free_geometry_resources(renderer);
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
        auto game_ents = dualV_get_entities(game_cv);
        auto identities = (forward_effect_identity_t*)dualV_get_owned_ro(render_cv, identity_type);
        for (uint32_t i = 0u; i < game_cv->count; ++i)
        {
            identities[i].game_entity = game_ents[i];
        }
    }

    uint32_t produce_drawcall(IPrimitiveRenderPass* pass, dual_storage_t* storage) override
    {
        // query from identity component
        using render_effects_t = dual::array_component_T<skr_render_effect_t, 4>;
        // SKR_LOG_FMT_INFO("Pass {} asked Feature {} to produce drawcall", pass->identity(), forward_effect_name);
        if (strcmp(pass->identity(), forward_pass_name) == 0)
        {
            uint32_t c = 0;
            auto counterF = [&](dual_chunk_view_t* cv) {
                c += cv->count;
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(counterF));
            push_constants.clear();
            push_constants.resize(c);
            return c;
        }
        return 0;
    }

    void peek_drawcall(IPrimitiveRenderPass* pass, skr_primitive_draw_list_view_t* drawcalls) override
    {
        // SKR_LOG_FMT_INFO("Pass {} asked Feature {} to peek drawcall", pass->identity(), forward_effect_name);
        if (strcmp(pass->identity(), forward_pass_name) == 0)
        {
            auto storage = skr_runtime_get_dual_storage();
            auto r_effect_callback = [&](dual_chunk_view_t* r_cv) {
                auto identities = (forward_effect_identity_t*)dualV_get_owned_rw(r_cv, identity_type);
                auto unbatched_g_ents = (dual_entity_t*)identities;
                auto r_ents = dualV_get_entities(r_cv);
                if (unbatched_g_ents)
                {
                    uint32_t idx = 0;
                    auto g_batch_callback = [&](dual_chunk_view_t* g_cv) {
                        auto g_ents = (dual_entity_t*)dualV_get_entities(g_cv);
                        auto transforms = (skr_transform_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_transform_t>::get());
                        for (uint32_t i = 0; i < g_cv->count; i++)
                        {
                            auto g_ent = g_ents[i];
                            auto r_ent = r_ents[idx];
                            (void)g_ent;
                            (void)r_ent;
                            push_constants[idx].world = skr::math::make_transform(
                            transforms[idx].location,
                            transforms[idx].scale,
                            skr::math::Quaternion::identity());
                            auto view = skr::math::look_at_matrix({ 0.f, 0.f, 12.5f } /*eye*/, { 0.f, 0.f, 0.f } /*at*/);
                            auto proj = skr::math::perspective_fov(3.1415926f / 2.f, (float)900 / (float)900, 1.f, 1000.f);
                            push_constants[idx].view_proj = skr::math::multiply(view, proj);
                            // drawcall
                            auto& drawcall = drawcalls->drawcalls[idx];
                            drawcall.push_const_name = push_constants_name;
                            drawcall.push_const = (const uint8_t*)push_constants.data() + idx;
                            drawcall.index_buffer = {};
                            drawcall.vertex_buffers = nullptr;
                            drawcall.vertex_buffer_count = 0;
                            drawcall.pipeline = nullptr;
                            idx++;
                        }
                    };
                    dualS_batch(storage, unbatched_g_ents, r_cv->count, DUAL_LAMBDA(g_batch_callback));
                }
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(r_effect_callback));
        }
    }

protected:
    // TODO: move these anywhere else
    void prepare_geometry_resources(ISkrRenderer* renderer);
    void free_geometry_resources(ISkrRenderer* renderer);
    CGPUBufferId vertex_buffer;
    CGPUBufferId index_buffer;
    // effect processor data
    const char* push_constants_name = "push_constants";
    dual_query_t* effect_query = nullptr;
    dual_type_index_t identity_type = {};
    dual::type_builder_t type_builder;
    struct PushConstants {
        skr::math::float4x4 world;
        skr::math::float4x4 view_proj;
    };
    eastl::vector<PushConstants> push_constants;
};
RenderEffectForward* forward_effect = new RenderEffectForward();

struct FwdCubeGeometry {
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

void RenderEffectForward::prepare_geometry_resources(ISkrRenderer* renderer)
{
    const auto device = renderer->get_cgpu_device();
    const auto gfx_queue = renderer->get_gfx_queue();
    // upload
    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "UploadBuffer";
    upload_buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(FwdCubeGeometry) + sizeof(FwdCubeGeometry::g_Indices);
    auto upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGPUBufferDescriptor vb_desc = {};
    vb_desc.name = "VertexBuffer";
    vb_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(FwdCubeGeometry);
    vertex_buffer = cgpu_create_buffer(device, &vb_desc);
    CGPUBufferDescriptor ib_desc = {};
    ib_desc.name = "IndexBuffer";
    ib_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    ib_desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
    ib_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    ib_desc.size = sizeof(FwdCubeGeometry::g_Indices);
    index_buffer = cgpu_create_buffer(device, &ib_desc);
    auto pool_desc = CGPUCommandPoolDescriptor();
    auto cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    auto cmd_desc = CGPUCommandBufferDescriptor();
    auto cpy_cmd = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = FwdCubeGeometry();
        memcpy(upload_buffer->cpu_mapped_address, &geom, upload_buffer_desc.size);
    }
    cgpu_cmd_begin(cpy_cmd);
    CGPUBufferToBufferTransfer vb_cpy = {};
    vb_cpy.dst = vertex_buffer;
    vb_cpy.dst_offset = 0;
    vb_cpy.src = upload_buffer;
    vb_cpy.src_offset = 0;
    vb_cpy.size = sizeof(FwdCubeGeometry);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &vb_cpy);
    {
        memcpy((char8_t*)upload_buffer->cpu_mapped_address + sizeof(FwdCubeGeometry),
        FwdCubeGeometry::g_Indices, sizeof(FwdCubeGeometry::g_Indices));
    }
    CGPUBufferToBufferTransfer ib_cpy = {};
    ib_cpy.dst = index_buffer;
    ib_cpy.dst_offset = 0;
    ib_cpy.src = upload_buffer;
    ib_cpy.src_offset = sizeof(FwdCubeGeometry);
    ib_cpy.size = sizeof(FwdCubeGeometry::g_Indices);
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

void RenderEffectForward::free_geometry_resources(ISkrRenderer* renderer)
{
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
}

void initialize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_register_render_pass(renderer, forward_pass_name, forward_pass);
    skr_renderer_register_render_effect(renderer, forward_effect_name, forward_effect);
}

void finalize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_remove_render_pass(renderer, forward_pass_name);
    skr_renderer_remove_render_effect(renderer, forward_effect_name);
    delete forward_effect;
    delete forward_pass;
}