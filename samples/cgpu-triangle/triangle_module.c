#include "cgpu/api.h"
#include "platform/thread.h"

void raster_cmd_record(
    CGpuCommandBufferId cmd, CGpuRenderPipelineId pipeline,
    CGpuTextureId back_buffer, CGpuTextureViewId back_buffer_view,
    uint32_t width, uint32_t height)
{
    cgpu_cmd_begin(cmd);
    CGpuColorAttachment screen_attachment = {
        .view = back_buffer_view,
        .load_action = LOAD_ACTION_CLEAR,
        .store_action = STORE_ACTION_STORE,
        .clear_color = fastclear_0000
    };
    CGpuRenderPassDescriptor rp_desc = {
        .render_target_count = 1,
        .sample_count = SAMPLE_COUNT_1,
        .color_attachments = &screen_attachment,
        .depth_stencil = CGPU_NULLPTR
    };
    CGpuTextureBarrier draw_barrier = {
        .texture = back_buffer,
        .src_state = RESOURCE_STATE_UNDEFINED,
        .dst_state = RESOURCE_STATE_RENDER_TARGET
    };
    CGpuResourceBarrierDescriptor barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc0);
    CGpuRenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);
    {
        cgpu_render_encoder_set_viewport(rp_encoder,
            0.0f, 0.0f,
            (float)width, (float)height,
            0.f, 1.f);
        cgpu_render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->width, back_buffer->height);
        cgpu_render_encoder_bind_pipeline(rp_encoder, pipeline);
        cgpu_render_encoder_draw(rp_encoder, 3, 0);
    }
    cgpu_cmd_end_render_pass(cmd, rp_encoder);
    CGpuTextureBarrier present_barrier = {
        .texture = back_buffer,
        .src_state = RESOURCE_STATE_RENDER_TARGET,
        .dst_state = RESOURCE_STATE_PRESENT
    };
    CGpuResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);
    cgpu_cmd_end(cmd);
}