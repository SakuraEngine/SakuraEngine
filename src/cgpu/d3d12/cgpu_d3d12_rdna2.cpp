#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"
#include "../common/common_utils.h"

void cgpu_render_encoder_set_shading_rate_d3d12(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate)
{
#ifdef D3D12_HEADER_SUPPORT_VRS
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    auto Adapter = (CGPUAdapter_D3D12*)Cmd->super.device->adapter;
    if (Adapter->adapter_detail.support_shading_rate)
    {
        D3D12_SHADING_RATE_COMBINER combiners[2] = { 
            D3D12Util_TranslateShadingRateCombiner(post_rasterizer_rate), 
            D3D12Util_TranslateShadingRateCombiner(final_rate) };
        ((ID3D12GraphicsCommandList5*)Cmd->pDxCmdList)->RSSetShadingRate(
            D3D12Util_TranslateShadingRate(shading_rate), combiners);
    }
    // TODO: VRS Tier2
    if (Adapter->adapter_detail.support_shading_rate_mask)
    {

    }
#endif
}