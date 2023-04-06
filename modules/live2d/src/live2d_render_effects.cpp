#include "platform/memory.h"
#include "platform/vfs.h"
#include "platform/time.h"
#include "platform/guid.hpp"
#include "platform/thread.h"
#include "utils/make_zeroed.hpp"

#include "ecs/type_builder.hpp"
#include "ecs/callback.hpp"

#include "SkrLive2D/skr_live2d.h"
#include "SkrLive2D/l2d_render_effect.h"
#include "SkrLive2D/l2d_render_model.h"

#include "Framework/Math/CubismMatrix44.hpp"
#include "Framework/Math/CubismViewMatrix.hpp"

#include "SkrRenderGraph/api.h"

#include "SkrRenderer/primitive_draw.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_effect.h"

#include "live2d_model_pass.hpp"
#include "live2d_mask_pass.hpp"
#include "live2d_clipping.hpp"

#include <EASTL/fixed_vector.h>

#include "rtm/matrix4x4f.h"

#include "tracy/Tracy.hpp"

static struct RegisterComponentskr_live2d_render_model_comp_tHelper
{
    RegisterComponentskr_live2d_render_model_comp_tHelper()
    {
        using namespace skr::guid::literals;

        dual_type_description_t desc = make_zeroed<dual_type_description_t>();
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

SKR_LIVE2D_API dual_type_index_t dual_id_of<skr_live2d_render_model_comp_t>::get()
{
    return _RegisterComponentskr_live2d_render_model_comp_tHelper.type;
}

typedef struct live2d_effect_identity_t {
    dual_entity_t game_entity;
} live2d_effect_identity_t;
skr_render_effect_name_t live2d_effect_name = "Live2DEffect";
struct RenderEffectLive2D : public IRenderEffectProcessor {
    skr_vfs_t* resource_vfs = nullptr;
    const char* push_constants_name = "push_constants";
    // this is a view object, later we will expose it to the world
    live2d_render_view_t view_;

    dual_query_t* effect_query = nullptr;
    dual::type_builder_t type_builder;
    dual_type_index_t identity_type = {};

    void initialize(SRendererId renderer, dual_storage_t* storage)
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
        // prepare render resources
        prepare_pipeline_settings();
        prepare_pipeline(renderer);
        prepare_mask_pipeline(renderer);
        // prepare_geometry_resources(renderer);
        skr_live2d_render_view_reset(&view_);
    }

    void finalize(SRendererId renderer)
    {
        auto sweepFunction = [&](dual_chunk_view_t* r_cv) {
        auto meshes = dual::get_owned_rw<skr_live2d_render_model_comp_t>(r_cv);
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
    }

    void on_register(SRendererId, dual_storage_t*) override
    {

    }

    void on_unregister(SRendererId, dual_storage_t*) override
    {

    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        *set = type_builder.build();
    }

    dual_type_index_t get_identity_type() override
    {
        return identity_type;
    }

    void initialize_data(SRendererId renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
    {

    }

    eastl::vector_map<skr_live2d_render_model_id, skr::span<const uint32_t>> sorted_drawable_list;
    eastl::vector_map<skr_live2d_render_model_id, eastl::fixed_vector<uint32_t, 4>> sorted_mask_drawable_lists;
    const float kMotionFramesPerSecond = 240.0f;
    eastl::vector_map<skr_live2d_render_model_id, STimer> motion_timers;
    uint32_t last_ms = 0;
    const bool use_high_precision_mask = false;

    eastl::vector<skr_primitive_draw_t> model_drawcalls;
    skr_primitive_draw_list_view_t model_draw_list;
    inline CGPURenderPipelineId get_pipeline() const
    {
        switch ((uint32_t)sample_count)
        {
        case 1: return pipeline;
        case 2: return msaa2_pipeline;
        case 4: return msaa4_pipeline;
        case 8: return msaa8_pipeline;
        default: return pipeline;
        }
        return pipeline;
    }
    inline CGPURenderPipelineId get_mask_pipeline() const
    {
        switch ((uint32_t)sample_count)
        {
        case 1: return mask_pipeline;
        case 2: return msaa2_mask_pipeline;
        case 4: return msaa4_mask_pipeline;
        case 8: return msaa8_mask_pipeline;
        default: return mask_pipeline;
        }
        return mask_pipeline;
    }

    void produce_model_drawcall(const skr_primitive_draw_context_t* context, dual_storage_t* storage) 
    {
        CubismMatrix44 projection;
        // TODO: Correct Projection
        projection.Scale(static_cast<float>(100.f) / static_cast<float>(100.f), 1.0f);
        // TODO: View Matrix
        model_drawcalls.resize(0);
        auto counterF = [&](dual_chunk_view_t* r_cv) {
            auto models = dual::get_owned_rw<skr_live2d_render_model_comp_t>(r_cv);
            const auto proper_pipeline = get_pipeline();
            for (uint32_t i = 0; i < r_cv->count; i++)
            {
                if (models[i].vram_request.is_ready())
                {
                    auto&& render_model = models[i].vram_request.render_model;
                    const auto& cmds = render_model->primitive_commands;
                    push_constants[render_model].resize(0);

                    auto&& model_resource = models[i].ram_request.model_resource;
                    const auto list = skr_live2d_model_get_sorted_drawable_list(model_resource);
                    if(!list) continue;
                    auto drawable_list = sorted_drawable_list[render_model] = { list , render_model->index_buffer_views.size() };
                    push_constants[render_model].resize(drawable_list.size());
                    // record constant parameters
                    auto clipping_manager = render_model->clipping_manager;
                    if (auto clipping_list = clipping_manager->GetClippingContextListForDraw())
                    {
                        for (auto drawable : drawable_list)
                        {
                            const auto& cmd = cmds[drawable];
                            auto& push_const = push_constants[render_model][drawable];
                            CubismClippingContext* clipping_context = (*clipping_list)[drawable];
                            push_const.use_mask = clipping_context && clipping_context->_isUsing;
                            if (push_const.use_mask)
                            {
                                const auto clip_mat = rtm::matrix_set(
                                    rtm::vector_load( &clipping_context->_matrixForDraw.GetArray()[4 * 0] ),
                                    rtm::vector_load( &clipping_context->_matrixForDraw.GetArray()[4 * 1] ),
                                    rtm::vector_load( &clipping_context->_matrixForDraw.GetArray()[4 * 2] ),
                                    rtm::vector_load( &clipping_context->_matrixForDraw.GetArray()[4 * 3] )
                                );
                                push_const.clip_matrix = rtm::matrix_transpose(clip_mat);
                                const csmInt32 channelNo = clipping_context->_layoutChannelNo;
                                CubismRenderer::CubismTextureColor* colorChannel = clipping_context->GetClippingManager()->GetChannelFlagAsColor(channelNo);
                                push_const.channel_flag = { colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A };
                            }
                            const auto proj_mat = rtm::matrix_set(
                                rtm::vector_load( &projection.GetArray()[4 * 0] ),
                                rtm::vector_load( &projection.GetArray()[4 * 1] ),
                                rtm::vector_load( &projection.GetArray()[4 * 2] ),
                                rtm::vector_load( &projection.GetArray()[4 * 3] )
                            );
                            push_const.projection_matrix = rtm::matrix_transpose(proj_mat);
                            skr_live2d_model_get_drawable_colors(render_model->model_resource_id, drawable,
                                &push_const.multiply_color,
                                &push_const.screen_color);
                            push_const.base_color = { 1.f, 1.f, 1.f, push_const.multiply_color.w };

                            auto visibility = skr_live2d_model_get_drawable_is_visible(render_model->model_resource_id, drawable);
                            auto& drawcall = model_drawcalls.emplace_back();
                            if (!visibility)
                            {
                                drawcall.desperated = true;
                                drawcall.pipeline = proper_pipeline;
                            }
                            else
                            {
                                drawcall.pipeline = proper_pipeline;
                                drawcall.push_const_name = push_constants_name;
                                drawcall.push_const = (const uint8_t*)(push_constants[render_model].data() + drawable);
                                drawcall.index_buffer = *cmd.ibv;
                                drawcall.vertex_buffers = cmd.vbvs.data();
                                drawcall.vertex_buffer_count = (uint32_t)cmd.vbvs.size();
                                {
                                    auto texture_view = skr_live2d_render_model_get_texture_view(render_model, drawable);
                                    drawcall.bind_table = render_model->bind_tables[texture_view];
                                }
                            }
                        }
                    }
                }
            }
        };
        dualQ_get_views(effect_query, DUAL_LAMBDA(counterF));
    }

    eastl::vector<skr_primitive_draw_t> mask_drawcalls;
    skr_primitive_draw_list_view_t mask_draw_list;
    void produce_mask_drawcall(const skr_primitive_draw_context_t* context, dual_storage_t* storage) 
    {
        {
            ZoneScopedN("FrameCleanUp");

            mask_drawcalls.resize(0);
            sorted_mask_drawable_lists.resize(0);
        }
        auto updateMaskF = [&](dual_chunk_view_t* r_cv) {
            ZoneScopedN("UpdateMaskF");

            const auto proper_pipeline = get_mask_pipeline();
            auto models = dual::get_owned_rw<skr_live2d_render_model_comp_t>(r_cv);
            for (uint32_t i = 0; i < r_cv->count; i++)
            {
                if (models[i].vram_request.is_ready())
                {
                    auto&& render_model = models[i].vram_request.render_model;
                    auto&& model_resource = models[i].ram_request.model_resource;
                    mask_push_constants[render_model].resize(0);

                    // TODO: move this to (some manager?) other than update morph/phys in a render pass
                    updateModelMotion(context->render_graph, render_model);
                    updateTexture(render_model);
                    // record constant parameters
                    if (auto clipping_manager = render_model->clipping_manager)
                    {
                        auto mask_list = clipping_manager->GetClippingContextListForMask();
                        if (!mask_list) continue;
                        uint32_t valid_masks = 0;
                        for (uint32_t j = 0; j < mask_list->GetSize(); j++)
                        {
                            CubismClippingContext* clipping_context = (clipping_manager != NULL)
                                ? (*mask_list)[j]
                                : NULL;
                            if (clipping_context && !use_high_precision_mask && clipping_context->_isUsing)
                            {
                                const csmInt32 clipDrawCount = clipping_context->_clippingIdCount;
                                for (csmInt32 ctx = 0; ctx < clipDrawCount; ctx++)
                                {
                                    const csmInt32 clipDrawIndex = clipping_context->_clippingIdList[ctx];
                                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                                    if (!model_resource->model->GetModel()->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                                    {
                                        continue;
                                    }
                                    valid_masks++;
                                }
                            }
                        }
                        sorted_mask_drawable_lists.reserve(valid_masks);
                        mask_push_constants.reserve(valid_masks);
                        for (uint32_t j = 0; j < mask_list->GetSize(); j++)
                        {
                            CubismClippingContext* clipping_context = (clipping_manager != NULL)
                                ? (*mask_list)[j]
                                : NULL;
                            if (clipping_context && !use_high_precision_mask && clipping_context->_isUsing)
                            {
                                const csmInt32 clipDrawCount = clipping_context->_clippingIdCount;
                                for (csmInt32 ctx = 0; ctx < clipDrawCount; ctx++)
                                {
                                    const csmInt32 clipDrawIndex = clipping_context->_clippingIdList[ctx];
                                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                                    if (!model_resource->model->GetModel()->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                                    {
                                        continue;
                                    }
                                    sorted_mask_drawable_lists[render_model].emplace_back(clipDrawIndex);
                                    auto&& push_const = mask_push_constants[render_model].emplace_back();
                                    const auto proj_mat = rtm::matrix_set(
                                        rtm::vector_load( &clipping_context->_matrixForMask.GetArray()[4 * 0] ),
                                        rtm::vector_load( &clipping_context->_matrixForMask.GetArray()[4 * 1] ),
                                        rtm::vector_load( &clipping_context->_matrixForMask.GetArray()[4 * 2] ),
                                        rtm::vector_load( &clipping_context->_matrixForMask.GetArray()[4 * 3] )
                                    );
                                    push_const.projection_matrix = rtm::matrix_transpose(proj_mat);
                                    const csmInt32 channelNo = clipping_context->_layoutChannelNo;
                                    CubismRenderer::CubismTextureColor* colorChannel = clipping_context->GetClippingManager()->GetChannelFlagAsColor(channelNo);
                                    push_const.channel_flag = { colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A };
                                    
                                    skr_live2d_model_get_drawable_colors(render_model->model_resource_id, clipDrawIndex,
                                        &push_const.multiply_color,
                                        &push_const.screen_color);
                                    csmRectF* rect = clipping_context->_layoutBounds;
                                    push_const.base_color = { rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f };
                                
                                    const auto& cmds = render_model->primitive_commands;
                                    const auto& cmd = cmds[clipDrawIndex];
                                    auto visibility = skr_live2d_model_get_drawable_is_visible(render_model->model_resource_id, clipDrawIndex);
                                    auto& drawcall = mask_drawcalls.emplace_back();
                                    if (!visibility)
                                    {
                                        drawcall.desperated = true;
                                        drawcall.pipeline = proper_pipeline;
                                    }
                                    else
                                    {
                                        drawcall.pipeline = proper_pipeline;
                                        drawcall.push_const_name = push_constants_name;
                                        drawcall.push_const = (const uint8_t*)(&push_const);
                                        drawcall.index_buffer = *cmd.ibv;
                                        drawcall.vertex_buffers = cmd.vbvs.data();
                                        drawcall.vertex_buffer_count = (uint32_t)cmd.vbvs.size();
                                        {
                                            auto texture_view = skr_live2d_render_model_get_texture_view(render_model, clipDrawIndex);
                                            drawcall.bind_table = render_model->mask_bind_tables[texture_view];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        };
        dualQ_get_views(effect_query, DUAL_LAMBDA(updateMaskF));
    }

    double sample_count = 1.0;
    uint64_t frame_count = 0;
    uint64_t async_slot_index = 0;
    skr_primitive_draw_packet_t produce_draw_packets(const skr_primitive_draw_context_t* context) override
    {
        auto pass = context->pass;
        auto storage = context->storage;
        
        frame_count++;
        async_slot_index = frame_count % RG_MAX_FRAME_IN_FLIGHT;

        skr_primitive_draw_packet_t packet = {};
        if (strcmp(pass->identity(), live2d_mask_pass_name) == 0)
        {
            ZoneScopedN("ProduceMaskDrawPackets");

            produce_mask_drawcall(context, storage);
            mask_draw_list.drawcalls = mask_drawcalls.data();
            mask_draw_list.count = (uint32_t)mask_drawcalls.size();
            packet.count = 1;
            packet.lists = &mask_draw_list;
        }
        if (strcmp(pass->identity(), live2d_pass_name) == 0)
        {
            ZoneScopedN("ProduceModelDrawPackets");

            produce_model_drawcall(context, storage);
            model_draw_list.drawcalls = model_drawcalls.data();
            model_draw_list.count = (uint32_t)model_drawcalls.size();
            packet.count = 1;
            packet.lists = &model_draw_list;
        }
        return packet;
    }

protected:
    const char* color_texture_name = "color_texture";
    void updateTexture(skr_live2d_render_model_id render_model)
    {
        ZoneScopedN("Live2D::updateTexture");

        // create descriptor sets if not existed
        const auto ib_c = render_model->index_buffer_views.size();
        for (uint32_t j = 0; j < ib_c; j++)
        {
            auto texture_view = skr_live2d_render_model_get_texture_view(render_model, j);
            {
                auto iter = render_model->bind_tables.find(texture_view);
                if (iter == render_model->bind_tables.end())
                {
                    ZoneScopedN("Live2D::createBindTable");

                    CGPUXBindTableDescriptor bind_table_desc = {};
                    bind_table_desc.root_signature = pipeline->root_signature;
                    bind_table_desc.names = &color_texture_name;
                    bind_table_desc.names_count = 1;
                    auto bind_table = cgpux_create_bind_table(pipeline->device, &bind_table_desc);
                    render_model->bind_tables[texture_view] = bind_table;

                    CGPUDescriptorData datas[1] = {};
                    datas[0] = make_zeroed<CGPUDescriptorData>();
                    datas[0].name = color_texture_name;
                    datas[0].count = 1;
                    datas[0].textures = &texture_view;
                    datas[0].binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
                    cgpux_bind_table_update(bind_table, datas, 1);
                }
            }
            {
                auto iter = render_model->mask_bind_tables.find(texture_view);
                if (iter == render_model->mask_bind_tables.end())
                {
                    ZoneScopedN("Live2D::createBindTable");

                    CGPUXBindTableDescriptor bind_table_desc = {};
                    bind_table_desc.root_signature = mask_pipeline->root_signature;
                    bind_table_desc.names = &color_texture_name;
                    bind_table_desc.names_count = 1;
                    auto bind_table = cgpux_create_bind_table(pipeline->device, &bind_table_desc);
                    render_model->mask_bind_tables[texture_view] = bind_table;
                    
                    CGPUDescriptorData datas[1] = {};
                    datas[0] = make_zeroed<CGPUDescriptorData>();
                    datas[0].name = color_texture_name;
                    datas[0].count = 1;
                    datas[0].textures = &texture_view;
                    datas[0].binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
                    cgpux_bind_table_update(bind_table, datas, 1);
                }
            }
        }
    }

    static const void* getVBData(skr_live2d_render_model_id render_model, uint32_t index, uint32_t& out_vcount)
    {
        const auto model_resource = render_model->model_resource_id;
        const void* pSrc = nullptr;
        // pos-uv-pos-uv...
        if (index % 2 == 0)
        {
            pSrc = skr_live2d_model_get_drawable_vertex_positions(
                model_resource, index / 2, &out_vcount);
        }
        else
        {
            pSrc = skr_live2d_model_get_drawable_vertex_uvs(
                model_resource, (index - 1) / 2, &out_vcount);
        }
        return pSrc;
    }

    void updateModelMotion(skr::render_graph::RenderGraph* render_graph, skr_live2d_render_model_id render_model)
    {
        ZoneScopedN("Live2D::updateModelMotion");

        const auto model_resource = render_model->model_resource_id;
        last_ms = skr_timer_get_msec(&motion_timers[render_model], true);
        static float delta_sum = 0.f;
        delta_sum += ((float)last_ms / 1000.f);
        if (delta_sum > (1.f / kMotionFramesPerSecond))
        {
            skr_live2d_model_update(model_resource, delta_sum);
            delta_sum = 0.f;
            const auto vb_c = render_model->vertex_buffer_views.size();
            // update buffer
            if (render_model->use_dynamic_buffer && vb_c) // direct copy vertices to CVV buffer
            {
                for (uint32_t j = 0; j < vb_c; j++)
                {
                    auto& view = render_model->vertex_buffer_views[j];
                    uint32_t vcount = 0;
                    const void* pSrc = getVBData(render_model, j, vcount);
                    if (render_model->use_dynamic_buffer) // direct copy vertices to CVV buffer
                    {
                        memcpy((uint8_t*)view.buffer->cpu_mapped_address + view.offset, pSrc, vcount * view.stride);
                    }
                }
            }
            else if (vb_c)
            {
                uint64_t totalVertexSize = 0;
                eastl::vector<skr::render_graph::BufferHandle> imported_vbs;
                eastl::vector<uint64_t> vb_sizes;
                eastl::vector<uint64_t> vb_offsets;
                if (!render_model->use_dynamic_buffer)
                {
                    imported_vbs.resize(vb_c);
                    vb_sizes.resize(vb_c);
                    vb_offsets.resize(vb_c);
                }
                for (uint32_t j = 0; j < vb_c; j++)
                {
                    auto& view = render_model->vertex_buffer_views[j];
                    uint32_t vcount = 0;
                    const void* pSrc = getVBData(render_model, j, vcount); (void)pSrc;
                    imported_vbs[j] = render_graph->create_buffer(
                            [=](skr::render_graph::RenderGraph& g, skr::render_graph::BufferBuilder& builder) {
                            skr::string name = "live2d_vb-";
                            name.append(skr::to_string((uint64_t)render_model).c_str());
                            name.append(skr::to_string(j).c_str());
                            builder.set_name(name.c_str())
                                    .import(view.buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
                            });
                    vb_sizes[j] = vcount * view.stride;
                    vb_offsets[j] = view.offset;
                    totalVertexSize += vcount * view.stride;
                }
                if (totalVertexSize)
                {
                namespace rg = skr::render_graph;

                auto upload_buffer = render_graph->create_buffer(
                    [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                    ZoneScopedN("ConstructUploadPass");

                    skr::string name = "live2d_upload-";
                    name.append(skr::to_string((uint64_t)render_model).c_str());
                    builder.set_name(name.c_str())
                            .size(totalVertexSize)
                            .with_tags(kRenderGraphDefaultResourceTag)
                            .as_upload_buffer();
                    });
                render_graph->add_copy_pass(
                [=](rg::RenderGraph& g, rg::CopyPassBuilder& builder) {
                    ZoneScopedN("ConstructCopyPass");
                    skr::string name = "live2d_copy-";
                    name.append(skr::to_string((uint64_t)render_model).c_str());
                    builder.set_name(name.c_str());
                    uint64_t range_cursor = 0;
                    for (uint32_t j = 0; j < vb_c; j++)
                    {
                        const auto vb_size = vb_sizes[j];
                        builder.buffer_to_buffer(
                            upload_buffer.range(range_cursor, range_cursor + vb_size),
                            imported_vbs[j].range(vb_offsets[j], vb_offsets[j] + vb_size));
                        range_cursor += vb_size;
                    }
                    },
                    [upload_buffer_hdl = upload_buffer, vb_c, render_model](rg::RenderGraph& g, rg::CopyPassContext& context){
                        auto upload_buffer = context.resolve(upload_buffer_hdl);
                        uint8_t* range_cursor =(uint8_t*)upload_buffer->cpu_mapped_address;
                        for (uint32_t j = 0; j < vb_c; j++)
                        {
                            auto& view = render_model->vertex_buffer_views[j];
                            uint32_t vcount = 0;
                            const void* pSrc = getVBData(render_model, j, vcount);
                            memcpy(range_cursor, pSrc, vcount * view.stride);
                            range_cursor += vcount * view.stride;
                        }
                    });
            }
            }
            if (auto clipping_manager = render_model->clipping_manager)
            {
                clipping_manager->SetupClippingContext(*model_resource->model->GetModel(), use_high_precision_mask);
            }
        }
    }

    void prepare_pipeline_settings();
    void prepare_pipeline(SRendererId renderer);
    void prepare_mask_pipeline(SRendererId renderer);
    void free_pipeline(SRendererId renderer);
    void free_mask_pipeline(SRendererId renderer);
    uint32_t* read_shader_bytes(SRendererId renderer, const char* name, uint32_t* out_length);
    CGPUShaderLibraryId create_shader_library(SRendererId renderer, const char* name, ECGPUShaderStage stage);

    struct PushConstants {
        rtm::matrix4x4f projection_matrix;
        rtm::matrix4x4f clip_matrix;
        skr_float4_t base_color;
        skr_float4_t multiply_color;
        skr_float4_t screen_color;
        skr_float4_t channel_flag;
        float use_mask;
        float pad0;
        float pad1;
        float pad2;
    };
    eastl::vector_map<skr_live2d_render_model_id, eastl::vector<PushConstants>> push_constants;
    eastl::vector_map<skr_live2d_render_model_id, eastl::vector<PushConstants>> mask_push_constants;

    CGPUVertexLayout vertex_layout = {};
    CGPURasterizerStateDescriptor rs_state = {};
    CGPUDepthStateDescriptor depth_state = {};

    CGPURenderPipelineId pipeline = nullptr;
    CGPURenderPipelineId msaa2_pipeline = nullptr;
    CGPURenderPipelineId msaa4_pipeline = nullptr;
    CGPURenderPipelineId msaa8_pipeline = nullptr;
    CGPURenderPipelineId mask_pipeline = nullptr;
    CGPURenderPipelineId msaa2_mask_pipeline = nullptr;
    CGPURenderPipelineId msaa4_mask_pipeline = nullptr;
    CGPURenderPipelineId msaa8_mask_pipeline = nullptr;
};
MaskPassLive2D* live2d_mask_pass = SkrNew<MaskPassLive2D>();
RenderPassLive2D* live2d_pass = SkrNew<RenderPassLive2D>();
RenderEffectLive2D* live2d_effect = SkrNew<RenderEffectLive2D>();

uint32_t* RenderEffectLive2D::read_shader_bytes(SRendererId renderer, const char* name, uint32_t* out_length)
{
    const auto render_device = renderer->get_render_device();
    const auto cgpu_device = render_device->get_cgpu_device();
    const auto backend = cgpu_device->adapter->instance->backend;
    skr::string shader_name = name;
    shader_name.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto shader_file = skr_vfs_fopen(resource_vfs, shader_name.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    const uint32_t shader_length = (uint32_t)skr_vfs_fsize(shader_file);
    auto shader_bytes = (uint32_t*)sakura_malloc(shader_length);
    skr_vfs_fread(shader_file, shader_bytes, 0, shader_length);
    skr_vfs_fclose(shader_file);
    if (out_length) *out_length = shader_length;
    return shader_bytes;
}

CGPUShaderLibraryId RenderEffectLive2D::create_shader_library(SRendererId renderer, const char* name, ECGPUShaderStage stage)
{
    const auto render_device = renderer->get_render_device();
    const auto cgpu_device = render_device->get_cgpu_device();
    uint32_t shader_length = 0;
    uint32_t* shader_bytes = read_shader_bytes(renderer, name, &shader_length);
    CGPUShaderLibraryDescriptor shader_desc = {};
    shader_desc.name = name;
    shader_desc.stage = stage;
    shader_desc.code = shader_bytes;
    shader_desc.code_size = shader_length;
    CGPUShaderLibraryId shader = cgpu_create_shader_library(cgpu_device, &shader_desc);
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

void RenderEffectLive2D::prepare_pipeline(SRendererId renderer)
{
    const auto render_device = renderer->get_render_device();
    const auto cgpu_device = render_device->get_cgpu_device();

    CGPUShaderLibraryId vs = create_shader_library(renderer, "shaders/live2d_vs", CGPU_SHADER_STAGE_VERT);
    CGPUShaderLibraryId ps = create_shader_library(renderer, "shaders/live2d_ps", CGPU_SHADER_STAGE_FRAG);

    CGPUShaderEntryDescriptor ppl_shaders[2];
    CGPUShaderEntryDescriptor& ppl_vs = ppl_shaders[0];
    ppl_vs.library = vs;
    ppl_vs.stage = CGPU_SHADER_STAGE_VERT;
    ppl_vs.entry = "main";
    CGPUShaderEntryDescriptor& ppl_ps = ppl_shaders[1];
    ppl_ps.library = ps;
    ppl_ps.stage = CGPU_SHADER_STAGE_FRAG;
    ppl_ps.entry = "main";

    const char* static_sampler_name = "color_sampler";
    auto static_sampler = render_device->get_linear_sampler();
    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = render_device->get_root_signature_pool();
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    auto root_sig = cgpu_create_root_signature(cgpu_device, &rs_desc);

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_vs;
    rp_desc.fragment_shader = &ppl_ps;
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &live2d_mask_format;
    rp_desc.depth_stencil_format = live2d_depth_format;

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
    pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);
    rp_desc.sample_count = CGPU_SAMPLE_COUNT_2;
    msaa2_pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);
    rp_desc.sample_count = CGPU_SAMPLE_COUNT_4;
    msaa4_pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);
    rp_desc.sample_count = CGPU_SAMPLE_COUNT_8;
    msaa8_pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);

    cgpu_free_shader_library(vs);
    cgpu_free_shader_library(ps);
}

void RenderEffectLive2D::free_pipeline(SRendererId renderer)
{
    auto sig_to_free = pipeline->root_signature;
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_render_pipeline(msaa2_pipeline);
    cgpu_free_render_pipeline(msaa4_pipeline);
    cgpu_free_render_pipeline(msaa8_pipeline);
    cgpu_free_root_signature(sig_to_free);
}

void RenderEffectLive2D::prepare_mask_pipeline(SRendererId renderer)
{
    const auto render_device = renderer->get_render_device();
    const auto cgpu_device = render_device->get_cgpu_device();
    
    CGPUShaderLibraryId vs = create_shader_library(renderer, "shaders/live2d_mask_vs", CGPU_SHADER_STAGE_VERT);
    CGPUShaderLibraryId ps = create_shader_library(renderer, "shaders/live2d_mask_ps", CGPU_SHADER_STAGE_FRAG);

    CGPUShaderEntryDescriptor ppl_shaders[2];
    CGPUShaderEntryDescriptor& ppl_vs = ppl_shaders[0];
    ppl_vs.library = vs;
    ppl_vs.stage = CGPU_SHADER_STAGE_VERT;
    ppl_vs.entry = "main";
    CGPUShaderEntryDescriptor& ppl_ps = ppl_shaders[1];
    ppl_ps.library = ps;
    ppl_ps.stage = CGPU_SHADER_STAGE_FRAG;
    ppl_ps.entry = "main";

    const char* static_sampler_name = "color_sampler";
    auto static_sampler = render_device->get_linear_sampler();
    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = render_device->get_root_signature_pool();
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    auto root_sig = cgpu_create_root_signature(cgpu_device, &rs_desc);

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_vs;
    rp_desc.fragment_shader = &ppl_ps;
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &live2d_mask_format;
    rp_desc.depth_stencil_format = live2d_depth_format;

    CGPUBlendStateDescriptor blend_state = {};
    blend_state.blend_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.masks[0] = CGPU_COLOR_MASK_ALL;
    blend_state.independent_blend = false;

    // Mask
    blend_state.src_factors[0] = CGPU_BLEND_CONST_ZERO;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_COLOR;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ZERO;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    // Multiply
    // blend_state.src_factors[0] = CGPU_BLEND_CONST_ONE;
    // blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    // blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    // blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;

    rp_desc.blend_state = &blend_state;
    rp_desc.rasterizer_state = &rs_state;
    rp_desc.depth_state = &depth_state;
    mask_pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);
    rp_desc.sample_count = CGPU_SAMPLE_COUNT_2;
    msaa2_mask_pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);
    rp_desc.sample_count = CGPU_SAMPLE_COUNT_4;
    msaa4_mask_pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);
    rp_desc.sample_count = CGPU_SAMPLE_COUNT_8;
    msaa8_mask_pipeline = cgpu_create_render_pipeline(cgpu_device, &rp_desc);

    cgpu_free_shader_library(vs);
    cgpu_free_shader_library(ps);
}

void RenderEffectLive2D::free_mask_pipeline(SRendererId renderer)
{
    auto sig_to_free = mask_pipeline->root_signature;
    cgpu_free_render_pipeline(mask_pipeline);
    cgpu_free_render_pipeline(msaa2_mask_pipeline);
    cgpu_free_render_pipeline(msaa4_mask_pipeline);
    cgpu_free_render_pipeline(msaa8_mask_pipeline);
    cgpu_free_root_signature(sig_to_free);
}

void skr_live2d_initialize_render_effects(live2d_renderer_t* renderer, live2d_render_graph_t* render_graph, struct skr_vfs_t* resource_vfs)
{
    live2d_effect->resource_vfs = resource_vfs;
    auto storage = renderer->get_dual_storage();
    live2d_effect->initialize(renderer, storage);
    skr_renderer_register_render_effect(renderer, live2d_effect_name, live2d_effect);
}

void skr_live2d_register_render_effects(live2d_renderer_t* renderer, live2d_render_graph_t* render_graph, uint32_t sample_count)
{
    live2d_effect->sample_count = (double)sample_count;
    auto& blackboard = render_graph->get_blackboard();
    blackboard.set_value("l2d_msaa", live2d_effect->sample_count);
    skr_renderer_register_render_pass(renderer, live2d_mask_pass_name, live2d_mask_pass);
    skr_renderer_register_render_pass(renderer, live2d_pass_name, live2d_pass);
}

void skr_live2d_finalize_render_effects(live2d_renderer_t* renderer, live2d_render_graph_t* render_graph, struct skr_vfs_t* resource_vfs)
{
    live2d_effect->finalize(renderer);
    SkrDelete(live2d_effect);
    SkrDelete(live2d_pass);
    SkrDelete(live2d_mask_pass);
}