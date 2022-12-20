#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "utils/log.hpp"
#include "cgpu/api.h"
#include "platform/memory.h"
#include "ecs/callback.hpp"
#include "ecs/dual.h"
#include "ecs/type_builder.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"
#include "SkrScene/scene.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_viewport.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/render_effect.h"
#include "SkrRenderer/render_group.h"
#include "SkrAnim/components/skin_component.h"
#include "SkrAnim/components/skeleton_component.h"
#include "cube.hpp"
#include "platform/vfs.h"
#include <platform/filesystem.hpp>

#include "utils/parallel_for.hpp"

#include "resource/resource_system.h"
#include "GameRuntime/game_animation.h"

#include "tracy/Tracy.hpp"

const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT_S8_UINT;

skr_render_pass_name_t forward_pass_name = "ForwardPass";
struct RenderPassForward : public IPrimitiveRenderPass {
    dual::query_t skin_query;
    dual::query_t anim_query;
    void on_update(const skr_primitive_pass_context_t* context) override
    {
        namespace rg = skr::render_graph;
        ZoneScopedN("ForwardPass Update");

        auto renderer = context->renderer;
        auto render_graph = context->render_graph;
        auto storage = context->storage;

        if (!skin_query)
        {
            auto sig = "[in]skr_render_mesh_comp_t, [in]skr_render_anim_comp_t, [in]skr_render_skel_comp_t, [in]skr_render_skin_comp_t";
            *skin_query = dualQ_from_literal(storage, sig);
        }
        if (!anim_query)
        {
            auto sig = "[in]skr_render_mesh_comp_t, [in]skr_render_anim_comp_t";
            *anim_query = dualQ_from_literal(storage, sig);
        }

        auto ensureBuffers = [&](dual_chunk_view_t* r_cv) {
            const auto cgpu_device = renderer->get_render_device()->get_cgpu_device();
            const auto meshes = dual::get_component_ro<skr_render_mesh_comp_t>(r_cv);
            auto anims = dual::get_owned_rw<skr_render_anim_comp_t>(r_cv);
            auto skins = dual::get_owned_rw<skr_render_skin_comp_t>(r_cv);
            auto skels = dual::get_owned_rw<skr_render_skel_comp_t>(r_cv);

            {
                ZoneScopedN("InitializeComponents");
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    auto mesh_resource = meshes[i].mesh_resource.get_resolved();
                    if(!mesh_resource)
                        continue;
                    if(skins[i].joint_remaps.empty())
                    {
                        auto skin_resource = skins[i].skin_resource.get_resolved();
                        auto skel_resource = skels[i].skeleton.get_resolved();
                        if(skel_resource && skin_resource)
                            skr_init_skin_component(&skins[i], skel_resource);
                    }
                    if(anims[i].buffers.empty())
                    {
                        auto skel_resource = skels[i].skeleton.get_resolved();
                        if(skel_resource)
                            skr_init_anim_component(&anims[i], mesh_resource, skel_resource);
                    }
                }
            }

            for (uint32_t i = 0; i < r_cv->count; i++)
            {
                auto mesh_resource = meshes[i].mesh_resource.get_resolved();
                if(!mesh_resource)
                    continue;
                auto* anim = anims + i;
                for (size_t j = 0u; j < anim->buffers.size(); j++)
                {
                    const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                    if (!anim->vbs[j])
                    {
                        ZoneScopedN("CreateVB");

                        skr::string name = mesh_resource->name;
                        auto vb_name = name + skr::to_string(i);

                        auto vb_desc = make_zeroed<CGPUBufferDescriptor>();
                        vb_desc.name = vb_name.c_str();
                        vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
                        vb_desc.flags = anim->use_dynamic_buffer ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE;
                        vb_desc.memory_usage = anim->use_dynamic_buffer ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY;
                        vb_desc.prefer_on_device = true;
                        vb_desc.size = anim->buffers[j].size;
                        SKR_ASSERT(vb_desc.size > 0);
                        anim->vbs[j] = cgpu_create_buffer(cgpu_device, &vb_desc);
                        auto renderMesh = mesh_resource->render_mesh;
                        anim->views.reserve(renderMesh->vertex_buffer_views.size());
                        for(size_t k=0; k < anim->primitives.size(); ++k)
                        {
                            auto& prim = anim->primitives[k];
                            auto vbv_start = anim->views.size();
                            for(size_t z=0; z < renderMesh->primitive_commands[k].vbvs.size(); ++z)
                            {
                                auto& vbv = renderMesh->primitive_commands[k].vbvs[z];
                                auto attr = mesh_resource->primitives[k].vertex_buffers[z].attribute;
                                if(attr == SKR_VERT_ATTRIB_POSITION)
                                {
                                    auto& view = anim->views.emplace_back();
                                    view.buffer = anim->vbs[j];
                                    view.offset = prim.position.offset;
                                    view.stride = prim.position.stride;
                                }
                                else if(attr == SKR_VERT_ATTRIB_NORMAL)
                                {
                                    auto& view = anim->views.emplace_back();
                                    view.buffer = anim->vbs[j];
                                    view.offset = prim.normal.offset;
                                    view.stride = prim.normal.stride;
                                }
                                else if(attr == SKR_VERT_ATTRIB_TANGENT)
                                {
                                    auto& view = anim->views.emplace_back();
                                    view.buffer = anim->vbs[j];
                                    view.offset = prim.tangent.offset;
                                    view.stride = prim.tangent.stride;
                                }
                                else
                                    anim->views.push_back(vbv);
                            }
                            prim.views = skr::span(anim->views.data() + vbv_start, renderMesh->primitive_commands[k].vbvs.size());
                        }
                    }
                    const auto vertex_size = anim->buffers[j].size;
                    if (use_dynamic_buffer)
                    {                        
                        ZoneScopedN("CVVUpdateVB");

                        void* vtx_dst = anim->vbs[j]->cpu_mapped_address;
                        memcpy(vtx_dst, anim->buffers[j].bytes, vertex_size);
                    }
                }
            }
        };
        {
            ZoneScopedN("PrepareMeshResource");
            // prepare skin mesh resources for rendering
            // TODO: skin_query -> anim_query
            dualQ_get_views(*skin_query, DUAL_LAMBDA(ensureBuffers));
        }
        {
            uint64_t vertices_size = 0; 
            {
                ZoneScopedN("CalculateSkinMeshSize");

                auto calcUploadBufferSize = [&](dual_chunk_view_t* r_cv) {
                    auto anims = dual::get_owned_rw<skr_render_anim_comp_t>(r_cv);
                    for (uint32_t i = 0; i < r_cv->count; i++)
                    {
                        auto* anim = anims + i;
                        const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                        if (use_dynamic_buffer) continue;
                        for (size_t j = 0u; j < anim->buffers.size(); j++)
                        {
                            vertices_size += anim->buffers[j].size;
                        }
                    }
                };
                dualQ_get_views(*anim_query, DUAL_LAMBDA(calcUploadBufferSize));
            }

            if (vertices_size != 0)
            {
                auto upload_buffer_handle = render_graph->create_buffer(
                    [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                        builder.set_name("SkinMeshUploadBuffer")
                            .size(vertices_size)
                            .with_tags(kRenderGraphDefaultResourceTag)
                            .as_upload_buffer();
                });

                render_graph->add_copy_pass(
                [=](rg::RenderGraph& g, rg::CopyPassBuilder& builder) {
                    builder.set_name("CopySkinMesh")
                        .from_buffer(upload_buffer_handle.range(0, vertices_size))
                        .can_be_lone();
                },
                [this, upload_buffer_handle](rg::RenderGraph& g, rg::CopyPassContext& context) {
                    ZoneScopedN("CopySkinMesh");

                    auto uploadVertices = [&](dual_chunk_view_t* r_cv) {
                        skr_render_anim_comp_t* anims = nullptr;
                        {
                            ZoneScopedN("FetchAnims");

                            // duel to dependency, anims fetch here may block a bit, waiting CPU skinning job done
                            anims = dual::get_owned_rw<skr_render_anim_comp_t>(r_cv);
                        }

                        auto upload_buffer = context.resolve(upload_buffer_handle);
                        auto mapped = (uint8_t*)upload_buffer->cpu_mapped_address;

                        // barrier from vb to copy dest
                        {
                            ZoneScopedN("Barriers");
                            CGPUResourceBarrierDescriptor barrier_desc = {};
                            eastl::vector<CGPUBufferBarrier> barriers;
                            for (uint32_t i = 0; i < r_cv->count; i++)
                            {
                                auto* anim = anims + i;
                                for (size_t j = 0u; j < anim->buffers.size(); j++)
                                {
                                    const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                                    if (anim->vbs[j] && !use_dynamic_buffer)
                                    {
                                        CGPUBufferBarrier& barrier = barriers.emplace_back();
                                        barrier.buffer = anim->vbs[j];
                                        barrier.src_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                                        barrier.dst_state = CGPU_RESOURCE_STATE_COPY_DEST;
                                    }
                                }
                            }
                            barrier_desc.buffer_barriers = barriers.data();
                            barrier_desc.buffer_barriers_count = (uint32_t)barriers.size();
                            cgpu_cmd_resource_barrier(context.cmd, &barrier_desc);
                        }

                        // upload
                        {
                            ZoneScopedN("MemCopies");
                            uint64_t cursor = 0;
                            for (uint32_t i = 0; i < r_cv->count; i++)
                            {
                                auto* anim = anims + i;
                                const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                                if (use_dynamic_buffer) continue;
                                for (size_t j = 0u; j < anim->buffers.size(); j++)
                                {
                                    // memcpy
                                    memcpy(mapped + cursor, anim->buffers[j].bytes, anim->buffers[j].size);

                                    // queue cpy
                                    CGPUBufferToBufferTransfer b2b = {};
                                    b2b.src = upload_buffer;
                                    b2b.src_offset = cursor;
                                    b2b.dst = anim->vbs[j];
                                    b2b.dst_offset = 0;
                                    b2b.size = anim->buffers[j].size;
                                    cgpu_cmd_transfer_buffer_to_buffer(context.cmd, &b2b);

                                    cursor += anim->buffers[j].size;
                                }
                            }
                        }
                    };
                    dualQ_get_views(*anim_query, DUAL_LAMBDA(uploadVertices));
                });   
            }
        }
    }
    
    void post_update(const skr_primitive_pass_context_t* context) override
    {

    }

    ECGPUShadingRate shading_rate = CGPU_SHADING_RATE_FULL;
    void execute(const skr_primitive_pass_context_t* context, skr_primitive_draw_list_view_t drawcalls) override
    {
        auto renderGraph = context->render_graph;
        // TODO: multi-viewport
        auto viewport_manager = context->renderer->get_viewport_manager();
        auto viewport = viewport_manager->find_viewport(0u); // the main viewport

        auto depth = renderGraph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                builder.set_name("depth")
                    .extent(viewport->viewport_width, viewport->viewport_height)
                    .format(depth_format)
                    .owns_memory()
                    .allow_depth_stencil();
            });(void)depth;
        if (drawcalls.count)
        {
            // IMGUI control shading rate
            {
                const char* shadingRateNames[] = {
                    "1x1", "2x2", "4x4", "1x2", "2x1", "2x4", "4x2"
                };
                ImGui::Begin(u8"ShadingRate");
                if (ImGui::Button(fmt::format("SwitchShadingRate-{}", shadingRateNames[shading_rate]).c_str()))
                {
                    if (shading_rate != CGPU_SHADING_RATE_COUNT - 1)
                        shading_rate = (ECGPUShadingRate)(shading_rate + 1);
                    else
                        shading_rate = CGPU_SHADING_RATE_FULL;
                }
                ImGui::End();
            }
            auto cbuffer = renderGraph->create_buffer(
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::BufferBuilder& builder) {
                    builder.set_name("forward_cbuffer")
                        .size(sizeof(skr_float4x4_t))
                        .memory_usage(CGPU_MEM_USAGE_CPU_TO_GPU)
                        .with_flags(CGPU_BCF_PERSISTENT_MAP_BIT)
                        .with_tags(kRenderGraphDynamicResourceTag)
                        .prefer_on_device()
                        .as_uniform_buffer();
                });
            // barrier skin vbs
            renderGraph->add_copy_pass(
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::CopyPassBuilder& builder) {
                    builder.set_name("BarrierSkinVertexBuffers")
                        .can_be_lone();
                },
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::CopyPassContext& context){
                    ZoneScopedN("BarrierSkinMeshes");
                    CGPUResourceBarrierDescriptor barrier_desc = {};
                    eastl::vector<CGPUBufferBarrier> barriers;
                    auto barrierVertices = [&](dual_chunk_view_t* r_cv) {
                        skr_render_anim_comp_t* anims = nullptr;
                        {
                            ZoneScopedN("FetchAnims");

                            // duel to dependency, anims fetch here may block a bit, waiting CPU skinning job done
                            anims = dual::get_owned_rw<skr_render_anim_comp_t>(r_cv);
                        }

                        for (uint32_t i = 0; i < r_cv->count; i++)
                        {
                            auto* anim = anims + i;
                            for (size_t j = 0u; j < anim->buffers.size(); j++)
                            {
                                const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                                if (anim->vbs[j] && !use_dynamic_buffer)
                                {
                                    CGPUBufferBarrier& barrier = barriers.emplace_back();
                                    barrier.buffer = anim->vbs[j];
                                    barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                                    barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                                }
                            }
                        }

                        {
                            ZoneScopedN("RecordBarrier");
                            
                            barrier_desc.buffer_barriers = barriers.data();
                            barrier_desc.buffer_barriers_count = (uint32_t)barriers.size();
                            cgpu_cmd_resource_barrier(context.cmd, &barrier_desc);
                        }
                    };
                    dualQ_get_views(*anim_query, DUAL_LAMBDA(barrierVertices));
                });
            renderGraph->add_render_pass(
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                    const auto out_color = renderGraph->get_texture("backbuffer");
                    const auto depth_buffer = renderGraph->get_texture("depth");
                    builder.set_name("forward_pass")
                        // we know that the drawcalls always have a same pipeline
                        .set_pipeline(drawcalls.drawcalls->pipeline)
                        .read("ForwardRenderConstants", cbuffer.range(0, sizeof(skr_float4x4_t)))
                        .write(0, out_color, CGPU_LOAD_ACTION_CLEAR)
                        .set_depth_stencil(depth_buffer.clear_depth(1.f));
                },
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& stack) {
                    auto cb = stack.resolve(cbuffer);
                    SKR_ASSERT(cb && "cbuffer not found");
                    ::memcpy(cb->cpu_mapped_address, &viewport->view_projection, sizeof(viewport->view_projection));
                    cgpu_render_encoder_set_viewport(stack.encoder,
                        0.0f, 0.0f,
                        (float)viewport->viewport_width, (float)viewport->viewport_height,
                        0.f, 1.f);
                    cgpu_render_encoder_set_scissor(stack.encoder,
                        0, 0, 
                        viewport->viewport_width, viewport->viewport_height);
                    
                    {
                    ZoneScopedN("DrawCalls");
                    for (uint32_t i = 0; i < drawcalls.count; i++)
                    {
                        // ZoneScopedN("DrawCall");

                        auto&& dc = drawcalls.drawcalls[i];
                        {
                            // ZoneScopedN("BindGeometry");
                            cgpu_render_encoder_bind_index_buffer(stack.encoder, 
                                dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
                            CGPUBufferId vertex_buffers[16] = { 0 };
                            uint32_t strides[16] = { 0 };
                            uint32_t offsets[16] = { 0 };
                            for (size_t i = 0; i < dc.vertex_buffer_count; i++)
                            {
                                vertex_buffers[i] = dc.vertex_buffers[i].buffer;
                                strides[i] = dc.vertex_buffers[i].stride;
                                offsets[i] = dc.vertex_buffers[i].offset;
                            }
                            cgpu_render_encoder_bind_vertex_buffers(stack.encoder, dc.vertex_buffer_count, vertex_buffers, strides, offsets);
                        }
                        {
                            // ZoneScopedN("PushConstants");
                            cgpu_render_encoder_push_constants(stack.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                        }
                        cgpu_render_encoder_set_shading_rate(stack.encoder, shading_rate, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH);
                        {
                            // ZoneScopedN("DrawIndexed");
                            cgpu_render_encoder_draw_indexed_instanced(stack.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
                        }
                    }
                    }
                });
        }
    }

    skr_render_pass_name_t identity() const override
    {
        return forward_pass_name;
    }
};
RenderPassForward* forward_pass = nullptr;

#include "rtm/quatf.h"
#include "rtm/scalarf.h"
#include "rtm/qvvf.h"
#include "rtm/rtmx.h"

typedef struct forward_effect_identity_t {
    dual_entity_t game_entity;
} forward_effect_identity_t;
skr_render_effect_name_t forward_effect_name = "ForwardEffect";
struct RenderEffectForward : public IRenderEffectProcessor {
    RenderEffectForward(skr_vfs_t* resource_vfs)
        :resource_vfs(resource_vfs) {}
    ~RenderEffectForward() = default;
    dual::type_builder_t type_builder;
    dual_type_set_t typeset;
    skr_vfs_t* resource_vfs;

    void on_register(SRendererId renderer, dual_storage_t* storage) override
    {
        // make identity component type
        {
            auto guid = make_zeroed<skr_guid_t>();
            dual_make_guid(&guid);
            auto desc = make_zeroed<dual_type_description_t>();
            desc.name = "forward_render_identity";
            desc.size = sizeof(forward_effect_identity_t);
            desc.guid = guid;
            desc.alignment = alignof(forward_effect_identity_t);
            identity_type = dualT_register_type(&desc);
            type_builder.with(identity_type);
            type_builder.with<skr_render_mesh_comp_t>();
            type_builder.with<skr_render_group_t>();
            typeset = type_builder.build();
        }
        initialize_queries(storage);
        // prepare render resources
        prepare_pipeline(renderer);
        prepare_geometry_resources(renderer);
    }

    void initialize_queries(dual_storage_t* storage)
    {
        // initialize queries
        mesh_query = dualQ_from_literal(storage, "[in]forward_render_identity, [in]skr_render_mesh_comp_t");
        draw_mesh_query = dualQ_from_literal(storage, "[in]forward_render_identity, [in]skr_render_mesh_comp_t, [out]skr_render_group_t");
    }

    void release_queries()
    {
        dualQ_release(mesh_query);
        dualQ_release(draw_mesh_query);
    }

    void on_unregister(SRendererId renderer, dual_storage_t* storage) override
    {
        auto sweepFunction = [&](dual_chunk_view_t* r_cv) {
            auto resource_system = skr::resource::GetResourceSystem();
            auto meshes = dual::get_owned_rw<skr_render_mesh_comp_t>(r_cv);
            for (uint32_t i = 0; i < r_cv->count; i++)
            {
                auto status = meshes[i].mesh_resource.get_status();
                if (status == ESkrLoadingStatus::SKR_LOADING_STATUS_INSTALLED)
                {
                    auto mesh_resource = (skr_mesh_resource_id)meshes[i].mesh_resource.get_ptr();
                    SKR_LOG_TRACE("Mesh Loaded: name - %s", mesh_resource->name.c_str());
                    resource_system->UnloadResource(meshes[i].mesh_resource);
                    resource_system->Update();
                    while (meshes[i].mesh_resource.get_status() != SKR_LOADING_STATUS_UNLOADED)
                    {
                        resource_system->Update();
                    }
                }
            }
        };
        dualQ_get_views(mesh_query, DUAL_LAMBDA(sweepFunction));
        release_queries();
        free_pipeline(renderer);
        free_geometry_resources(renderer);
    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        *set = typeset;
    }

    dual_type_index_t get_identity_type() override
    {
        return identity_type;
    }

    void initialize_data(SRendererId renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
    {
        auto game_ents = dualV_get_entities(game_cv);
        auto identities = (forward_effect_identity_t*)dualV_get_owned_ro(render_cv, identity_type);
        for (uint32_t i = 0u; i < game_cv->count; ++i)
        {
            identities[i].game_entity = game_ents[i];
        }
    }

    eastl::vector<skr_primitive_draw_t> mesh_drawcalls;
    skr_primitive_draw_list_view_t mesh_draw_list;
    skr_primitive_draw_packet_t produce_draw_packets(const skr_primitive_draw_context_t* context) override
    {
        auto pass = context->pass;
        auto storage = context->storage;

        skr_primitive_draw_packet_t packet = {};
        // query from identity component
        if (strcmp(pass->identity(), forward_pass_name) != 0)
            return {};
        uint32_t c = 0;
        auto counterF = [&](dual_chunk_view_t* r_cv) {
            ZoneScopedN("PreCalculateDrawCallCount");
            const skr_render_mesh_comp_t* meshes = nullptr;
            {
                ZoneScopedN("FetchRenderMeshes");
                meshes = dual::get_component_ro<skr_render_mesh_comp_t>(r_cv);
            }
            for (uint32_t i = 0; i < r_cv->count; i++)
            {
                auto status = meshes[i].mesh_resource.get_status();
                if (status == SKR_LOADING_STATUS_INSTALLED)
                {
                    auto resourcePtr = (skr_mesh_resource_t*)meshes[i].mesh_resource.get_ptr();
                    auto renderMesh = resourcePtr->render_mesh;
                    c += (uint32_t)renderMesh->primitive_commands.size();
                }
                else
                {
                    c++;
                }
            }
        };
        dualQ_get_views(mesh_query, DUAL_LAMBDA(counterF));

        model_matrices.clear();
        push_constants.clear();
        mesh_drawcalls.clear();
        model_matrices.resize(c);
        push_constants.reserve(c);
        mesh_drawcalls.reserve(c);
        // draw static meshess
        {
        auto r_effect_callback = [&](dual_chunk_view_t* r_cv) {
            uint32_t r_idx = 0;
            uint32_t dc_idx = 0;

            auto identities = (forward_effect_identity_t*)dualV_get_owned_rw(r_cv, identity_type);
            auto unbatched_g_ents = (dual_entity_t*)identities;
            const auto meshes = dual::get_component_ro<skr_render_mesh_comp_t>(r_cv);
            const auto anims = dual::get_component_ro<skr_render_anim_comp_t>(r_cv);
            if (unbatched_g_ents)
            {
                auto g_batch_callback = [&](dual_chunk_view_t* g_cv) {
                    ZoneScopedN("BatchedEnts");

                    //SKR_LOG_DEBUG("batch: %d -> %d", g_cv->start, g_cv->count);
                    const auto translations = dual::get_component_ro<skr_translation_comp_t>(g_cv);
                    const auto rotations = dual::get_component_ro<skr_rotation_comp_t>(g_cv);(void)rotations;
                    const auto scales = dual::get_component_ro<skr_scale_comp_t>(g_cv);
                    {
                        ZoneScopedN("ComputeModelMatrices");
#ifdef NDEBUG
                        const size_t batch_size = 256u;
#else
                        const size_t batch_size = 64u;
#endif
                        skr::parallel_for(translations, translations + g_cv->count, batch_size, 
                        [translations, rotations, scales, this] (auto&& begin, auto&& end){
                            ZoneScopedN("ModelMatrixJob");
                        
                            const auto base_cursor = begin - translations;
                            uint32_t i = 0;
                            for (auto iter = begin; iter < end; iter++, i++)
                            {
                                auto g_idx = base_cursor + i;
                                
                                // Model Matrix
                                skr_float4x4_t& model_matrix = model_matrices[g_idx];
                                {                                    
                                    const auto quat = rtm::quat_from_euler_rh(
                                        rtm::scalar_deg_to_rad(-rotations[g_idx].euler.pitch),
                                        rtm::scalar_deg_to_rad(rotations[g_idx].euler.yaw),
                                        rtm::scalar_deg_to_rad(rotations[g_idx].euler.roll));
                                    const rtm::vector4f translation = rtm::vector_set(
                                        translations[g_idx].value.x, translations[g_idx].value.y,
                                        translations[g_idx].value.z, 0.f);
                                    const rtm::vector4f scale = rtm::vector_set(
                                        scales[g_idx].value.x, scales[g_idx].value.y,
                                        scales[g_idx].value.z, 0.f
                                    );
                                    const rtm::qvvf transform = rtm::qvv_set(quat, translation, scale);
                                    const rtm::matrix4x4f matrix = rtm::matrix_cast(rtm::matrix_from_qvv(transform));
                                    model_matrix = *(skr_float4x4_t*)&matrix;
                                }
                            }
                        }, 9u); // if we are under 8 * 64/256 ents, use inplace sync compute
                    }

                    {
                    ZoneScopedN("RecordDrawList");
                    for (uint32_t g_idx = 0; g_idx < g_cv->count; g_idx++, r_idx++)
                    {
                        const auto& model_matrix = model_matrices[g_idx];
                        // drawcall
                        auto status = meshes[r_idx].mesh_resource.get_status();
                        if (status == SKR_LOADING_STATUS_INSTALLED)
                        {
                            auto resourcePtr = (skr_mesh_resource_t*)meshes[r_idx].mesh_resource.get_ptr();
                            auto renderMesh = resourcePtr->render_mesh;
                
                            const auto& cmds = renderMesh->primitive_commands;
                            if(anims && !anims->vbs.empty())
                            {
                                for (size_t i = 0; i < resourcePtr->primitives.size(); ++i)
                                {
                                    auto& cmd = cmds[i];
                                    auto& push_const = push_constants.emplace_back();
                                    push_const.model = model_matrix;
                                    auto& drawcall = mesh_drawcalls.emplace_back();
                                    drawcall.pipeline = pipeline;
                                    drawcall.push_const_name = push_constants_name;
                                    drawcall.push_const = (const uint8_t*)(&push_const);
                                    drawcall.index_buffer = *cmd.ibv;
                                    drawcall.vertex_buffers = anims[r_idx].primitives[i].views.data();
                                    drawcall.vertex_buffer_count = (uint32_t)anims[r_idx].primitives[i].views.size();
                                    dc_idx++;
                                }
                            }
                            else 
                            {
                                for (auto&& cmd : cmds)
                                {
                                    // resources may be ready after produce_drawcall, so we need to check it here
                                    if (push_constants.capacity() <= dc_idx) return;

                                    auto& push_const = push_constants.emplace_back();
                                    push_const.model = model_matrix;
                                    auto& drawcall = mesh_drawcalls.emplace_back();
                                    drawcall.pipeline = pipeline;
                                    drawcall.push_const_name = push_constants_name;
                                    drawcall.push_const = (const uint8_t*)(&push_const);
                                    drawcall.index_buffer = *cmd.ibv;
                                    drawcall.vertex_buffers = cmd.vbvs.data();
                                    drawcall.vertex_buffer_count = (uint32_t)cmd.vbvs.size();
                                    dc_idx++;
                                }
                            }
                        }
                        else
                        {
                            // resources may be ready after produce_drawcall, so we need to check it here
                            if (push_constants.capacity() <= dc_idx) return;

                            auto& push_const = push_constants.emplace_back();
                            push_const.model = model_matrix;
                            auto& drawcall = mesh_drawcalls.emplace_back();
                            drawcall.pipeline = pipeline;
                            drawcall.push_const_name = push_constants_name;
                            drawcall.push_const = (const uint8_t*)(&push_const);
                            drawcall.index_buffer = ibv;
                            drawcall.vertex_buffers = vbvs;
                            drawcall.vertex_buffer_count = 5;
                            dc_idx++;
                        }
                    }
                    }
                };
                dualS_batch(storage, unbatched_g_ents, r_cv->count, DUAL_LAMBDA(g_batch_callback));
            }
        };
        dualQ_get_views(draw_mesh_query, DUAL_LAMBDA(r_effect_callback));
        }

        mesh_draw_list.drawcalls = mesh_drawcalls.data();
        mesh_draw_list.count = (uint32_t)mesh_drawcalls.size();
        packet.count = 1;
        packet.lists = &mesh_draw_list;
        return packet;
    }

protected:
    // TODO: move these anywhere else
    void prepare_geometry_resources(SRendererId renderer);
    void free_geometry_resources(SRendererId renderer);
    void prepare_pipeline(SRendererId renderer);
    void free_pipeline(SRendererId renderer);
    // render resources
    skr_vertex_buffer_view_t vbvs[5];
    skr_index_buffer_view_t ibv;
    CGPUBufferId vertex_buffer;
    CGPUBufferId index_buffer;
    CGPURenderPipelineId pipeline;
    CGPURenderPipelineId skin_pipeline;
    // effect processor data
    const char* push_constants_name = "push_constants";
    dual_query_t* mesh_query = nullptr;
    dual_query_t* draw_mesh_query = nullptr;
    dual_query_t* draw_skin_query = nullptr;
    dual_type_index_t identity_type = {};
    struct PushConstants {
        skr_float4x4_t model;
    };
    eastl::vector<PushConstants> push_constants;
    eastl::vector<skr_float4x4_t> model_matrices;
};
RenderEffectForward* forward_effect = nullptr;

void RenderEffectForward::prepare_geometry_resources(SRendererId renderer)
{
    const auto render_device = renderer->get_render_device();
    const auto device = render_device->get_cgpu_device();
    const auto gfx_queue = render_device->get_gfx_queue();
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
        memcpy(upload_buffer->cpu_mapped_address, &geom, sizeof(CubeGeometry));
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
    // init vbvs & ibvs
    vbvs[0].buffer = vertex_buffer;
    vbvs[0].stride = sizeof(skr_float3_t);
    vbvs[0].offset = offsetof(CubeGeometry, g_Positions);
    vbvs[1].buffer = vertex_buffer;
    vbvs[1].stride = sizeof(skr_float2_t);
    vbvs[1].offset = offsetof(CubeGeometry, g_TexCoords);
    vbvs[2].buffer = vertex_buffer;
    vbvs[2].stride = sizeof(skr_float2_t);
    vbvs[2].offset = offsetof(CubeGeometry, g_TexCoords2);
    vbvs[3].buffer = vertex_buffer;
    vbvs[3].stride = sizeof(skr_float3_t);
    vbvs[3].offset = offsetof(CubeGeometry, g_Normals);
    vbvs[4].buffer = vertex_buffer;
    vbvs[4].stride = sizeof(skr_float4_t);
    vbvs[4].offset = offsetof(CubeGeometry, g_Tangents);
    ibv.buffer = index_buffer;
    ibv.index_count = 36;
    ibv.first_index = 0;
    ibv.offset = 0;
    ibv.stride = sizeof(uint32_t);
}

void RenderEffectForward::free_geometry_resources(SRendererId renderer)
{
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
}

void RenderEffectForward::prepare_pipeline(SRendererId renderer)
{
    const auto render_device = renderer->get_render_device();
    const auto device = render_device->get_cgpu_device();
    const auto backend = device->adapter->instance->backend;

    // read shaders
    skr::string vsname = u8"shaders/Game/gbuffer_vs";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto vsfile = skr_vfs_fopen(resource_vfs, vsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _vs_length = (uint32_t)skr_vfs_fsize(vsfile);
    uint32_t* _vs_bytes = (uint32_t*)sakura_malloc(_vs_length);
    skr_vfs_fread(vsfile, _vs_bytes, 0, _vs_length);
    skr_vfs_fclose(vsfile);

    skr::string fsname = u8"shaders/Game/gbuffer_fs";
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto fsfile = skr_vfs_fopen(resource_vfs, fsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _fs_length = (uint32_t)skr_vfs_fsize(fsfile);
    uint32_t* _fs_bytes = (uint32_t*)sakura_malloc(_fs_length);
    skr_vfs_fread(fsfile, _fs_bytes, 0, _fs_length);
    skr_vfs_fclose(fsfile);

    // create default deferred material
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "gbuffer_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = _vs_bytes;
    vs_desc.code_size = _vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "gbuffer_pixel_shader";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = _fs_bytes;
    fs_desc.code_size = _fs_length;
    CGPUShaderLibraryId _vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId _fs = cgpu_create_shader_library(device, &fs_desc);
    sakura_free(_vs_bytes);
    sakura_free(_fs_bytes);

    CGPUPipelineShaderDescriptor ppl_shaders[2];
    CGPUPipelineShaderDescriptor& vs = ppl_shaders[0];
    vs.library = _vs;
    vs.stage = CGPU_SHADER_STAGE_VERT;
    vs.entry = "main";
    CGPUPipelineShaderDescriptor& ps = ppl_shaders[1];
    ps.library = _fs;
    ps.stage = CGPU_SHADER_STAGE_FRAG;
    ps.entry = "main";

    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = render_device->get_root_signature_pool();
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 2, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "NORMAL", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 3, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { "TANGENT", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 4, 0, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 4;


    const auto fmt = CGPU_FORMAT_B8G8R8A8_UNORM;
    auto rp_desc = make_zeroed<CGPURenderPipelineDescriptor>();
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &vs;
    rp_desc.fragment_shader = &ps;
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &fmt;
    rp_desc.depth_stencil_format = depth_format;

    auto raster_desc = make_zeroed<CGPURasterizerStateDescriptor>();
    raster_desc.cull_mode = CGPU_CULL_MODE_BACK;
    raster_desc.depth_bias = 0;
    raster_desc.fill_mode = CGPU_FILL_MODE_SOLID;
    raster_desc.front_face = CGPU_FRONT_FACE_CCW;

    auto ds_desc = make_zeroed<CGPUDepthStateDescriptor>();
    ds_desc.depth_func = CGPU_CMP_LEQUAL;
    ds_desc.depth_write = true;
    ds_desc.depth_test = true;

    rp_desc.rasterizer_state = &raster_desc;
    rp_desc.depth_state = &ds_desc;
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);

    cgpu_free_shader_library(_vs);
    cgpu_free_shader_library(_fs);
}

void RenderEffectForward::free_pipeline(SRendererId renderer)
{
    auto sig_to_free = pipeline->root_signature;
    // cgpu_free_render_pipeline(skin_pipeline);
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(sig_to_free);
}

skr_render_effect_name_t forward_effect_skin_name = "ForwardEffectSkin";
struct RenderEffectForwardSkin : public RenderEffectForward
{
    RenderEffectForwardSkin(skr_vfs_t* resource_vfs)
        : RenderEffectForward(resource_vfs) {}

    void on_register(SRendererId renderer, dual_storage_t* storage) override
    {
        // make identity component type
        {
            auto guid = make_zeroed<skr_guid_t>();
            dual_make_guid(&guid);
            auto desc = make_zeroed<dual_type_description_t>();
            desc.name = "forward_skin_render_identity";
            desc.size = sizeof(forward_effect_identity_t);
            desc.guid = guid;
            desc.alignment = alignof(forward_effect_identity_t);
            identity_type = dualT_register_type(&desc);
            type_builder.with(identity_type);
            type_builder.with<skr_render_mesh_comp_t>();
            type_builder.with<skr_render_group_t>();
            type_builder.with<skr_render_anim_comp_t>();
            type_builder.with<skr_render_skel_comp_t>();
            type_builder.with<skr_render_skin_comp_t>();
            typeset = type_builder.build();
        }
        initialize_queries(storage);
        // prepare render resources
        prepare_pipeline(renderer);
        prepare_geometry_resources(renderer);
    }

    void initialize_queries(dual_storage_t* storage)
    {
        mesh_query = dualQ_from_literal(storage, "[in]forward_skin_render_identity, [in]skr_render_mesh_comp_t");
        draw_mesh_query = dualQ_from_literal(storage, "[in]forward_skin_render_identity, [in]skr_render_mesh_comp_t, [out]skr_render_group_t");
        install_query = dualQ_from_literal(storage, "[in]forward_skin_render_identity, [in]skr_render_anim_comp_t, [in]skr_render_skel_comp_t, [in]skr_render_skin_comp_t");
    }

    void release_queries()
    {
        dualQ_release(mesh_query);
        dualQ_release(draw_mesh_query);
        dualQ_release(install_query);
    }

    void on_unregister(SRendererId renderer, dual_storage_t* storage) override
    {
        free_pipeline(renderer);
        free_geometry_resources(renderer);
        release_queries();
    }

    dual_query_t* install_query = nullptr;
};
RenderEffectForwardSkin* forward_effect_skin = nullptr;

void game_initialize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* resource_vfs)
{
    forward_effect = new RenderEffectForward(resource_vfs);
    forward_effect_skin = new RenderEffectForwardSkin(resource_vfs);
    forward_pass = new RenderPassForward();
    skr_renderer_register_render_effect(renderer, forward_effect_name, forward_effect);
    skr_renderer_register_render_effect(renderer, forward_effect_skin_name, forward_effect_skin);
}

void game_register_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph)
{
    skr_renderer_register_render_pass(renderer, forward_pass_name, forward_pass);
}

void game_finalize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph)
{
    skr_renderer_remove_render_pass(renderer, forward_pass_name);
    skr_renderer_remove_render_effect(renderer, forward_effect_name);
    skr_renderer_remove_render_effect(renderer, forward_effect_skin_name);
    delete forward_effect;
    delete forward_effect_skin;
    delete forward_pass;
}