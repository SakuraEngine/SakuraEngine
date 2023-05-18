#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "utils/log.hpp"
#include "platform/memory.h"

#include "cgpu/api.h"
#include "cgpu/cgpux.h"


#include "ecs/dual.h"

#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrScene/scene.h"

#include "forward_pass.hpp"
#include "rfx_mesh.hpp"
#include "rfx_skmesh.hpp"

#include "containers/string.hpp"

#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrRenderer/render_group.h"
#include "SkrAnim/components/skin_component.h"
#include "SkrAnim/components/skeleton_component.h"

#include "cube.hpp"
#include "platform/vfs.h"
#include <platform/filesystem.hpp>

#include "utils/parallel_for.hpp"

#include "resource/resource_system.h"

#include "tracy/Tracy.hpp"

#include "math/rtm/quatf.h"
#include "math/rtm/scalarf.h"
#include "math/rtm/qvvf.h"
#include "math/rtm/rtmx.h"
#include "math/transform.h"

void RenderEffectForward::on_register(SRendererId renderer, dual_storage_t* storage)
{
    // make identity component type
    {
        auto guid = make_zeroed<skr_guid_t>();
        dual_make_guid(&guid);
        auto desc = make_zeroed<dual_type_description_t>();
        desc.name = u8"forward_render_identity";
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

void RenderEffectForward::initialize_queries(dual_storage_t* storage)
{
    // initialize queries
    mesh_query = dualQ_from_literal(storage, "[in]forward_render_identity, [in]skr_render_mesh_comp_t");
    draw_mesh_query = dualQ_from_literal(storage, "[in]forward_render_identity, [in]skr_render_mesh_comp_t, [out]skr_render_group_t");
}

void RenderEffectForward::release_queries()
{
    dualQ_release(mesh_query);
    dualQ_release(draw_mesh_query);
}

void RenderEffectForward::on_unregister(SRendererId renderer, dual_storage_t* storage)
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

void RenderEffectForward::get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set)
{
    *set = typeset;
}

dual_type_index_t RenderEffectForward::get_identity_type()
{
    return identity_type;
}

void RenderEffectForward::initialize_data(SRendererId renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv)
{
    auto game_ents = dualV_get_entities(game_cv);
    auto identities = (forward_effect_identity_t*)dualV_get_owned_ro(render_cv, identity_type);
    for (uint32_t i = 0u; i < game_cv->count; ++i)
    {
        identities[i].game_entity = game_ents[i];
    }
}

skr_primitive_draw_packet_t RenderEffectForward::produce_draw_packets(const skr_primitive_draw_context_t* context)
{
    auto pass = context->pass;
    auto storage = context->storage;

    skr_primitive_draw_packet_t packet = {};
    // 0. only produce for forward pass
    if (strcmp((const char*)pass->identity(), (const char*)forward_pass_name) != 0) return {};
    
    // 1. calculate primitive count
    uint32_t primitiveCount = 0;
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
                primitiveCount += (uint32_t)renderMesh->primitive_commands.size();
            }
            else
            {
                primitiveCount++;
            }
        }
    };
    dualQ_get_views(mesh_query, DUAL_LAMBDA(counterF));

    // 2. resize data buffers
    model_matrices.clear();
    push_constants.clear();
    mesh_drawcalls.clear();
    model_matrices.resize(primitiveCount);
    push_constants.reserve(primitiveCount);
    mesh_drawcalls.reserve(primitiveCount);

    // 3. fill draw packets
    auto r_effect_callback = [&](dual_chunk_view_t* r_cv) {
        uint32_t r_idx = 0;
        uint32_t dc_idx = 0;

        auto identities = (forward_effect_identity_t*)dualV_get_owned_ro(r_cv, identity_type);
        auto unbatched_g_ents = (dual_entity_t*)identities;
        const skr_render_mesh_comp_t* meshes = nullptr;
        const skr_render_anim_comp_t* anims = nullptr;
        {
            ZoneScopedN("FetchRenderMeshes");
            meshes = dual::get_component_ro<skr_render_mesh_comp_t>(r_cv);
        }
        {
            ZoneScopedN("FetchAnimComps");
            anims = dual::get_component_ro<skr_render_anim_comp_t>(r_cv);
        }
        if (!unbatched_g_ents) return;

        auto gBatchCallback = [&](dual_chunk_view_t* g_cv) {
            ZoneScopedN("BatchedEnts");

            //SKR_LOG_DEBUG("batch: %d -> %d", g_cv->start, g_cv->count);
            const auto l2ws = dual::get_component_ro<skr_transform_comp_t>(g_cv);
            const auto translations = dual::get_component_ro<skr_translation_comp_t>(g_cv);
            const auto rotations = dual::get_component_ro<skr_rotation_comp_t>(g_cv);(void)rotations;
            const auto scales = dual::get_component_ro<skr_scale_comp_t>(g_cv);
            // 3.1 calculate model matrices
            {
                ZoneScopedN("ComputeModelMatrices");
#ifdef NDEBUG
                const size_t batch_size = 256u;
#else
                const size_t batch_size = 64u;
#endif
                skr::parallel_for(translations, translations + g_cv->count, batch_size, 
                [translations, rotations, scales, l2ws, this] (auto&& begin, auto&& end){
                    ZoneScopedN("ModelMatrixJob");
                
                    const auto base_cursor = begin - translations;
                    uint32_t i = 0;
                    for (auto iter = begin; iter < end; iter++, i++)
                    {
                        auto g_idx = base_cursor + i;
                        
                        // Model Matrix
                        skr_float4x4_t& model_matrix = model_matrices[g_idx];
                        if(l2ws)
                        {
                            const rtm::qvvf transform = skr::math::load(l2ws[g_idx].value);
                            const rtm::matrix4x4f matrix = rtm::matrix_cast(rtm::matrix_from_qvv(transform));
                            model_matrix = *(skr_float4x4_t*)&matrix;
                        }
                        else
                        {                                    
                            const auto quat = skr::math::load(rotations[g_idx].euler);
                            const rtm::vector4f translation = skr::math::load(translations[g_idx].value);
                            const rtm::vector4f scale = skr::math::load(scales[g_idx].value);
                            const rtm::qvvf transform = rtm::qvv_set(quat, translation, scale);
                            const rtm::matrix4x4f matrix = rtm::matrix_cast(rtm::matrix_from_qvv(transform));
                            model_matrix = *(skr_float4x4_t*)&matrix;
                        }
                    }
                }, 9u); // if we are under 8 * 64/256 ents, use inplace sync compute
            }
            // 3.2 fill draw calls
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

                    // early resolve all materials
                    for (auto& material : resourcePtr->materials)
                    {
                        material.resolve(true, nullptr);
                    }
                    bool materials_ready = true;
                    for (const auto& material : resourcePtr->materials)
                    {
                        if (!material.is_resolved())
                        {
                            materials_ready = false;
                            break;
                        } 
                    }
                    if (!materials_ready) continue;

                    // record draw calls
                    const auto& cmds = renderMesh->primitive_commands;
                    const auto& materials = resourcePtr->materials;
                    if(anims && !anims->vbs.empty())
                    {
                        for (size_t i = 0; i < resourcePtr->primitives.size(); ++i)
                        {
                            auto& cmd = cmds[i];
                            CGPURenderPipelineId proper_pipeline = pipeline;
                            CGPUXBindTableId proper_bind_table = nullptr;
                            if (materials.size())
                            {
                                const auto material = materials[cmd.material_index].get_ptr();
                                // TODO: Add multi-pass
                                const auto& pass = material->installed_passes[0];
                                SKR_ASSERT(pass.pso && "Material not ready! (no PSO)");
                                proper_pipeline = pass.pso;
                                proper_bind_table = pass.bind_table;
                            }
                            auto& push_const = push_constants.emplace_back();
                            push_const.model = model_matrix;
                            auto& drawcall = mesh_drawcalls.emplace_back();
                            drawcall.pipeline = proper_pipeline;
                            drawcall.bind_table = proper_bind_table;
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
                            CGPURenderPipelineId proper_pipeline = pipeline;
                            CGPUXBindTableId proper_bind_table = nullptr;
                            if (materials.size())
                            {
                                const auto material = materials[cmd.material_index].get_ptr();
                                // TODO: Add multi-pass
                                const auto& pass = material->installed_passes[0];
                                SKR_ASSERT(pass.pso && "Material not ready! (no PSO)");
                                proper_pipeline = pass.pso;
                                proper_bind_table = pass.bind_table;
                            }
                            auto& push_const = push_constants.emplace_back();
                            push_const.model = model_matrix;
                            auto& drawcall = mesh_drawcalls.emplace_back();
                            drawcall.pipeline = proper_pipeline;
                            drawcall.bind_table = proper_bind_table;
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
        dualS_batch(storage, unbatched_g_ents, r_cv->count, DUAL_LAMBDA(gBatchCallback));
    };
    dualQ_get_views(draw_mesh_query, DUAL_LAMBDA(r_effect_callback));

    // 4. return packet info
    mesh_draw_list.drawcalls = mesh_drawcalls.data();
    mesh_draw_list.count = (uint32_t)mesh_drawcalls.size();
    packet.count = 1;
    packet.lists = &mesh_draw_list;
    return packet;
}

void RenderEffectForward::prepare_geometry_resources(SRendererId renderer)
{
    const auto render_device = renderer->get_render_device();
    const auto device = render_device->get_cgpu_device();
    const auto gfx_queue = render_device->get_gfx_queue();
    // upload
    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = u8"UploadBuffer";
    upload_buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
    auto upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGPUBufferDescriptor vb_desc = {};
    vb_desc.name = u8"VertexBuffer";
    vb_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(CubeGeometry);
    vertex_buffer = cgpu_create_buffer(device, &vb_desc);
    CGPUBufferDescriptor ib_desc = {};
    ib_desc.name = u8"IndexBuffer";
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
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? u8".dxil" : u8".spv");
    auto vsfile = skr_vfs_fopen(resource_vfs, vsname.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _vs_length = (uint32_t)skr_vfs_fsize(vsfile);
    uint32_t* _vs_bytes = (uint32_t*)sakura_malloc(_vs_length);
    skr_vfs_fread(vsfile, _vs_bytes, 0, _vs_length);
    skr_vfs_fclose(vsfile);

    skr::string fsname = u8"shaders/Game/gbuffer_fs";
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? u8".dxil" : u8".spv");
    auto fsfile = skr_vfs_fopen(resource_vfs, fsname.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _fs_length = (uint32_t)skr_vfs_fsize(fsfile);
    uint32_t* _fs_bytes = (uint32_t*)sakura_malloc(_fs_length);
    skr_vfs_fread(fsfile, _fs_bytes, 0, _fs_length);
    skr_vfs_fclose(fsfile);

    // create default deferred material
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = u8"gbuffer_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = _vs_bytes;
    vs_desc.code_size = _vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = u8"gbuffer_pixel_shader";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = _fs_bytes;
    fs_desc.code_size = _fs_length;
    CGPUShaderLibraryId _vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId _fs = cgpu_create_shader_library(device, &fs_desc);
    sakura_free(_vs_bytes);
    sakura_free(_fs_bytes);

    CGPUShaderEntryDescriptor ppl_shaders[2];
    CGPUShaderEntryDescriptor& vs = ppl_shaders[0];
    vs.library = _vs;
    vs.stage = CGPU_SHADER_STAGE_VERT;
    vs.entry = u8"main";
    CGPUShaderEntryDescriptor& ps = ppl_shaders[1];
    ps.library = _fs;
    ps.stage = CGPU_SHADER_STAGE_FRAG;
    ps.entry = u8"main";

    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    rs_desc.pool = render_device->get_root_signature_pool();
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { u8"POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { u8"TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { u8"TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 2, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { u8"NORMAL", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 3, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { u8"TANGENT", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 4, 0, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
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

// skin effect impl

void RenderEffectForwardSkin::on_register(SRendererId renderer, dual_storage_t* storage)
{
    // make identity component type
    {
        auto guid = make_zeroed<skr_guid_t>();
        dual_make_guid(&guid);
        auto desc = make_zeroed<dual_type_description_t>();
        desc.name = u8"forward_skin_render_identity";
        desc.size = sizeof(forward_effect_identity_t);
        desc.guid = guid;
        desc.alignment = alignof(forward_effect_identity_t);
        identity_type = dualT_register_type(&desc);
        type_builder.with(identity_type)
            .with<skr_render_mesh_comp_t>()
            .with<skr_render_group_t>()
            .with<skr_render_anim_comp_t>()
            .with<skr_render_skel_comp_t>()
            .with<skr_render_skin_comp_t>();
        typeset = type_builder.build();
    }
    initialize_queries(storage);
    // prepare render resources
    prepare_pipeline(renderer);
    prepare_geometry_resources(renderer);
}

void RenderEffectForwardSkin::initialize_queries(dual_storage_t* storage)
{
    mesh_query = dualQ_from_literal(storage, "[in]forward_skin_render_identity, [in]skr_render_mesh_comp_t");
    draw_mesh_query = dualQ_from_literal(storage, "[in]forward_skin_render_identity, [in]skr_render_mesh_comp_t, [out]skr_render_group_t");
    install_query = dualQ_from_literal(storage, "[in]forward_skin_render_identity, [in]skr_render_anim_comp_t, [in]skr_render_skel_comp_t, [in]skr_render_skin_comp_t");
}

void RenderEffectForwardSkin::release_queries()
{
    dualQ_release(mesh_query);
    dualQ_release(draw_mesh_query);
    dualQ_release(install_query);
}

void RenderEffectForwardSkin::on_unregister(SRendererId renderer, dual_storage_t* storage)
{
    free_pipeline(renderer);
    free_geometry_resources(renderer);
    release_queries();
}

RenderEffectForwardSkin* forward_effect_skin = nullptr;
RenderPassForward* forward_pass = nullptr;
RenderEffectForward* forward_effect = nullptr;

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