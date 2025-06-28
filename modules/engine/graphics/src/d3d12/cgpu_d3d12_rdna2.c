#include "SkrGraphics/backend/d3d12/cgpu_d3d12.h"
#include "d3d12_utils.h"

void cgpu_render_encoder_set_shading_rate_d3d12(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate)
{
#ifdef __ID3D12GraphicsCommandList5_INTERFACE_DEFINED__
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    CGPUAdapter_D3D12* Adapter = (CGPUAdapter_D3D12*)Cmd->super.device->adapter;
    if (Adapter->adapter_detail.support_shading_rate)
    {
        D3D12_SHADING_RATE_COMBINER combiners[2] = { 
            D3D12Util_TranslateShadingRateCombiner(post_rasterizer_rate), 
            D3D12Util_TranslateShadingRateCombiner(final_rate) };

        ID3D12GraphicsCommandList5* CmdList5 = CGPU_NULLPTR;
        CHECK_HRESULT(COM_CALL(QueryInterface, Cmd->pDxCmdList, IID_REF(ID3D12GraphicsCommandList5), (void**)&CmdList5));
        COM_CALL(RSSetShadingRate, CmdList5, D3D12Util_TranslateShadingRate(shading_rate), combiners);
        SAFE_RELEASE(CmdList5);
    }
    // TODO: VRS Tier2
    if (Adapter->adapter_detail.support_shading_rate_mask)
    {

    }
#endif
}