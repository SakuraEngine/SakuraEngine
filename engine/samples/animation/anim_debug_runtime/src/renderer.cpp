#include "AnimDebugRuntime/renderer.h"
#include "common/utils.h"
// #include "AnimDebugRuntime/cube_geometry.h"
#include "AnimDebugRuntime/bone_geometry.h"
#include <SkrAnim/ozz/skeleton.h>
#include <SkrAnim/ozz/base/io/archive.h>
#include <SkrAnim/ozz/skeleton_utils.h>
#include <SkrAnim/ozz/local_to_model_job.h>
#include <AnimDebugRuntime/util.h>
#include <SkrAnim/ozz/base/containers/vector.h>

namespace animd
{

LightingPushConstants Renderer::lighting_data = { 0, 0 };
LightingCSPushConstants Renderer::lighting_cs_data = { { 0, 0 }, { 0, 0 } };

Renderer::Renderer(skr::RenderDevice* render_device)
    : _render_device(render_device)
{
    // Initialize lighting data
    lighting_data.bFlipUVX = 0;
    lighting_data.bFlipUVY = 0;

    // Initialize lighting CS data
    lighting_cs_data.viewportSize = { 1280.0f, 720.0f };
    lighting_cs_data.viewportOrigin = { 0.0f, 0.0f };
}

void Renderer::create_skeleton(ozz::animation::Skeleton& skeleton)
{
    _instance_count = skeleton.num_joints();
    _instance_data.resize(_instance_count, skr::float4x4::identity());
}

void Renderer::update_anim(ozz::animation::Skeleton& skeleton, ozz::span<ozz::math::Float4x4> prealloc_models_)
{
    const auto t0 = skr::TransformF(skr::QuatF(skr::RotatorF()), skr::float3(0.0), skr::float3(0.001f));
    const auto m0 = skr::transpose(t0.to_matrix());

    for (int i = 0; i < _instance_count; ++i)
    {
        ozz::math::Float4x4 model_matrix = prealloc_models_[i];

        auto mat = animd::ozz2skr_float4x4(model_matrix);
        auto parent_id = skeleton.joint_parents()[i];
        if (parent_id < 0)
        {
            _instance_data[i] = *(skr_float4x4_t*)&m0;
            continue; // skip joints with very small bone length
        }
        auto parent_mat = animd::ozz2skr_float4x4(prealloc_models_[parent_id]);
        auto bone_dir = skr::float3(parent_mat.rows[0][3] - mat.rows[0][3], parent_mat.rows[1][3] - mat.rows[1][3], parent_mat.rows[2][3] - mat.rows[2][3]);
        auto bone_length = skr::length(bone_dir);
        if (bone_length < 0.01f)
        {
            _instance_data[i] = *(skr_float4x4_t*)&m0;
            continue; // skip joints with very small bone length
        }
        // resize according to bone length
        auto t = skr::TransformF(skr::QuatF(skr::RotatorF()), skr::float3(0.0f), float3(bone_length));
        auto m = skr::transpose(t.to_matrix());
        mat = mat * m; // apply scale to the matrix
        _instance_data[i] = *(skr_float4x4_t*)&mat;
    }
}

void Renderer::create_resources()
{
    auto _device = _render_device->get_cgpu_device();
    auto _gfx_queue = _render_device->get_gfx_queue();

    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = u8"UploadBuffer";
    upload_buffer_desc.flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT;
    upload_buffer_desc.usages = CGPU_BUFFER_USAGE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;

    upload_buffer_desc.size = sizeof(BoneGeometry) + sizeof(BoneGeometry::g_Indices) + _instance_count * sizeof(skr_float4x4_t);
    auto upload_buffer = cgpu_create_buffer(_device, &upload_buffer_desc);

    CGPUBufferDescriptor vb_desc = {};
    vb_desc.name = u8"VertexBuffer";
    vb_desc.flags = CGPU_BUFFER_FLAG_NONE;
    vb_desc.usages = CGPU_BUFFER_USAGE_VERTEX_BUFFER;
    vb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(BoneGeometry);

    _vertex_buffer = cgpu_create_buffer(_device, &vb_desc);

    CGPUBufferDescriptor ib_desc = {};
    ib_desc.name = u8"IndexBuffer";
    ib_desc.flags = CGPU_BUFFER_FLAG_NONE;
    ib_desc.usages = CGPU_BUFFER_USAGE_INDEX_BUFFER;
    ib_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    ib_desc.size = sizeof(BoneGeometry::g_Indices);

    _index_buffer = cgpu_create_buffer(_device, &ib_desc);

    CGPUBufferDescriptor inb_desc = {};
    inb_desc.name = u8"InstanceBuffer";
    inb_desc.flags = CGPU_BUFFER_FLAG_NONE;
    inb_desc.usages = CGPU_BUFFER_USAGE_VERTEX_BUFFER;
    inb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    inb_desc.size = _instance_count * sizeof(skr_float4x4_t);

    _instance_buffer = cgpu_create_buffer(_device, &inb_desc);

    auto pool_desc = CGPUCommandPoolDescriptor();
    auto cmd_pool = cgpu_create_command_pool(_gfx_queue, &pool_desc);
    auto cmd_desc = CGPUCommandBufferDescriptor();
    auto cpy_cmd = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = BoneGeometry();
        SKR_LOG_INFO(u8"BoneGeometry size is %zu", sizeof(BoneGeometry));
        memcpy(upload_buffer->info->cpu_mapped_address, &geom, sizeof(BoneGeometry));
    }
    cgpu_cmd_begin(cpy_cmd);
    CGPUBufferToBufferTransfer vb_cpy = {};
    vb_cpy.dst = _vertex_buffer;
    vb_cpy.dst_offset = 0;
    vb_cpy.src = upload_buffer;
    vb_cpy.src_offset = 0;
    // vb_cpy.size                       = sizeof(CubeGeometry);
    vb_cpy.size = sizeof(BoneGeometry);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &vb_cpy);

    // copy geometry data to upload buffer
    {
        memcpy((char8_t*)upload_buffer->info->cpu_mapped_address + sizeof(BoneGeometry), BoneGeometry::g_Indices, sizeof(BoneGeometry::g_Indices));
    }

    // data[offset:offset+size] to _index_buffer
    CGPUBufferToBufferTransfer ib_cpy = {};
    ib_cpy.dst = _index_buffer;
    ib_cpy.dst_offset = 0;
    ib_cpy.src = upload_buffer;
    // ib_cpy.src_offset                 = sizeof(CubeGeometry);
    // ib_cpy.size                       = sizeof(CubeGeometry::g_Indices);
    ib_cpy.src_offset = sizeof(BoneGeometry);
    ib_cpy.size = sizeof(BoneGeometry::g_Indices);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &ib_cpy);
    // wvp
    {
        memcpy((char8_t*)upload_buffer->info->cpu_mapped_address + sizeof(BoneGeometry) + sizeof(BoneGeometry::g_Indices), _instance_data.data(), sizeof(skr_float4x4_t) * _instance_count);
    }

    CGPUBufferToBufferTransfer istb_cpy = {};
    istb_cpy.dst = _instance_buffer;
    istb_cpy.dst_offset = 0;
    istb_cpy.src = upload_buffer;
    istb_cpy.src_offset = sizeof(BoneGeometry) + sizeof(BoneGeometry::g_Indices);
    istb_cpy.size = _instance_count * sizeof(skr_float4x4_t);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &istb_cpy);

    // barriers
    CGPUBufferBarrier barriers[3] = {};

    CGPUBufferBarrier& vb_barrier = barriers[0];
    vb_barrier.buffer = _vertex_buffer;
    vb_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

    CGPUBufferBarrier& ib_barrier = barriers[1];
    ib_barrier.buffer = _index_buffer;
    ib_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state = CGPU_RESOURCE_STATE_INDEX_BUFFER;

    CGPUBufferBarrier& ist_barrier = barriers[2];
    ist_barrier.buffer = _instance_buffer;
    ist_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    ist_barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

    CGPUResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers = barriers;
    barrier_desc.buffer_barriers_count = 3;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
    cgpu_cmd_end(cpy_cmd);

    // submit command to queue
    CGPUQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(_gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(_gfx_queue);
    cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cmd_pool);
}

void Renderer::create_render_pipeline()
{
    create_gbuffer_pipeline();
    create_lighting_pipeline();
    create_blit_pipeline();
}

void Renderer::build_render_graph(skr::render_graph::RenderGraph* graph, skr::render_graph::TextureHandle back_buffer)
{
    namespace rg = skr::render_graph;

    rg::TextureHandle composite_buffer = graph->create_texture(
        [this](rg::RenderGraph& g, rg::TextureBuilder& builder) {
            builder.set_name(u8"composite_buffer")
                .extent(this->_width, this->_height)
                .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                .heap_dedicated()
                .allow_render_target();
        });
    auto gbuffer_color = graph->create_texture(
        [this](rg::RenderGraph& g, rg::TextureBuilder& builder) {
            builder.set_name(u8"gbuffer_color")
                .extent(this->_width, this->_height)
                .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                .heap_dedicated()
                .allow_render_target();
        });
    auto gbuffer_depth = graph->create_texture(
        [this](rg::RenderGraph& g, rg::TextureBuilder& builder) {
            builder.set_name(u8"gbuffer_depth")
                .extent(this->_width, this->_height)
                .format(CGPU_FORMAT_D32_SFLOAT)
                .heap_dedicated()
                .allow_depth_stencil();
        });
    auto gbuffer_normal = graph->create_texture(
        [this](rg::RenderGraph& g, rg::TextureBuilder& builder) {
            builder.set_name(u8"gbuffer_normal")
                .extent(this->_width, this->_height)
                .format(CGPU_FORMAT_R16G16B16A16_SNORM)
                .heap_dedicated()
                .allow_render_target();
        });

    // update camera view and projection matrices
    if (!mp_camera)
    {
        SKR_LOG_ERROR(u8"Camera is not set.");
        return;
    }
    auto view = skr::float4x4::view_at(
        mp_camera->position,
        mp_camera->position + mp_camera->front,
        mp_camera->up);

    auto proj = skr::float4x4::perspective_fov(
        skr::camera_fov_y_from_x(mp_camera->fov, mp_camera->aspect),
        mp_camera->aspect,
        mp_camera->near_plane,
        mp_camera->far_plane);
    auto _view_proj = skr::mul(view, proj);
    auto view_proj = skr::transpose(_view_proj);
    // render to g-buffer
    graph->add_render_pass(
        [this, gbuffer_color, gbuffer_normal, gbuffer_depth](rg::RenderGraph& g, rg::RenderPassBuilder& builder) {
            builder.set_name(u8"gbuffer_pass")
                .set_pipeline(_gbuffer_pipeline)
                .write(0, gbuffer_color, CGPU_LOAD_ACTION_CLEAR)
                .write(1, gbuffer_normal, CGPU_LOAD_ACTION_CLEAR)
                .set_depth_stencil(gbuffer_depth.clear_depth(1.f));
        },
        [this, view_proj](rg::RenderGraph& g, rg::RenderPassContext& stack) {
            cgpu_render_encoder_set_viewport(stack.encoder, 0.0f, 0.0f, (float)this->_width, (float)this->_height, 0.f, 1.f);
            cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, this->_width, this->_height);

            CGPUBufferId vertex_buffers[5] = {
                _vertex_buffer, _vertex_buffer, _vertex_buffer, _vertex_buffer, _instance_buffer
            };

            const uint32_t strides[5] = {
                sizeof(float3), sizeof(float2), sizeof(uint32_t), sizeof(uint32_t), sizeof(skr_float4x4_t)
            };
            const uint32_t offsets[5] = {
                offsetof(BoneGeometry, g_Positions), offsetof(BoneGeometry, g_TexCoords), offsetof(BoneGeometry, g_Normals), offsetof(BoneGeometry, g_Tangents), 0
            };
            cgpu_render_encoder_bind_index_buffer(stack.encoder, _index_buffer, sizeof(uint32_t), 0);
            cgpu_render_encoder_bind_vertex_buffers(stack.encoder, 5, vertex_buffers, strides, offsets);
            cgpu_render_encoder_push_constants(stack.encoder, _gbuffer_pipeline->root_signature, u8"push_constants", &view_proj);

            cgpu_render_encoder_draw_indexed_instanced(stack.encoder, 24, 0, _instance_count, 0, 0);
        }

    );
    // screen space lighting with fragment shader

    // pass by value, so that we can use when graph executing
    graph->add_render_pass(
        [this, gbuffer_color, gbuffer_normal, gbuffer_depth, composite_buffer](rg::RenderGraph& g, rg::RenderPassBuilder& builder) {
            builder.set_name(u8"light_pass_fs")
                .set_pipeline(_lighting_pipeline)
                .read(u8"gbuffer_color", gbuffer_color.read_mip(0, 1))
                .read(u8"gbuffer_normal", gbuffer_normal)
                .read(u8"gbuffer_depth", gbuffer_depth)
                .write(0, composite_buffer, CGPU_LOAD_ACTION_CLEAR);
        },
        [this](rg::RenderGraph& g, rg::RenderPassContext& stack) {
            cgpu_render_encoder_set_viewport(stack.encoder, 0.0f, 0.0f, (float)this->_width, (float)this->_height, 0.f, 1.f);
            cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, this->_width, this->_height);
            cgpu_render_encoder_push_constants(stack.encoder, _lighting_pipeline->root_signature, u8"push_constants", &lighting_data);
            cgpu_render_encoder_draw(stack.encoder, 3, 0);
        });

    graph->add_render_pass(
        [back_buffer, composite_buffer, this](rg::RenderGraph& g, rg::RenderPassBuilder& builder) {
            builder.set_name(u8"final_blit")
                .set_pipeline(this->_blit_pipeline)
                .read(u8"input_color", composite_buffer)
                .write(0, back_buffer, CGPU_LOAD_ACTION_CLEAR);
        },
        [this](rg::RenderGraph& g, rg::RenderPassContext& stack) {
            cgpu_render_encoder_set_viewport(stack.encoder, 0.0f, 0.0f, (float)this->_width, (float)this->_height, 0.f, 1.f);
            cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, this->_width, this->_height);
            cgpu_render_encoder_draw(stack.encoder, 3, 0);
        });

} // namespace skr::render_graph

void Renderer::finalize()
{
    // Free cgpu objects
    cgpu_free_buffer(_index_buffer);
    cgpu_free_buffer(_vertex_buffer);
    cgpu_free_buffer(_instance_buffer);
    free_pipeline_and_signature(_gbuffer_pipeline);
    free_pipeline_and_signature(_lighting_pipeline);
    free_pipeline_and_signature(_blit_pipeline);
}

} // namespace animd