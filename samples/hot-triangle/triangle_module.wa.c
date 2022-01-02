#include "../../thirdparty/mimalloc/include/mimalloc.h"
#include "cgpu/api.h"

void raster_cmd_record(
    CGpuCommandBufferId cmd, CGpuRenderPipelineId pipeline,
    CGpuRenderPassEncoderId rp_encoder, uint32_t width, uint32_t height)
{
    cgpu_render_encoder_set_viewport(rp_encoder,
        0.0f, 0.0f,
        (float)width, (float)height,
        0.f, 1.f);
    cgpu_render_encoder_set_scissor(rp_encoder, 0, 0, width, height);
    cgpu_render_encoder_bind_pipeline(rp_encoder, pipeline);
    cgpu_render_encoder_draw(rp_encoder, 3, 0);
}