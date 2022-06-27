#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "cgpu/flags.h"
#include "utils/log.h"
#include "vulkan_utils.h"
#include "vulkan/vulkan_core.h"
#include "../common/common_utils.h"
#ifdef CGPU_THREAD_SAFETY
    #include "platform/thread.h"
#endif

void cgpu_render_encoder_set_shading_rate_vulkan(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate)
{
#if VK_KHR_fragment_shading_rate
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Cmd->super.device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)D->super.adapter;
    if (A->mPhysicalDeviceFragmentShadingRateFeatures.pipelineFragmentShadingRate)
    {
        const VkExtent2D fragmentSize = {
            .width = VkUtil_GetShadingRateX(shading_rate),
            .height = VkUtil_GetShadingRateY(shading_rate)
        };
        VkFragmentShadingRateCombinerOpKHR combinerOps[] = {
            VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR, VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR
        };
        if (A->mPhysicalDeviceFragmentShadingRateFeatures.primitiveFragmentShadingRate)
            combinerOps[0] = VkUtil_TranslateShadingRateCombiner(post_rasterizer_rate);
        if (A->mPhysicalDeviceFragmentShadingRateFeatures.attachmentFragmentShadingRate)
            combinerOps[1] = VkUtil_TranslateShadingRateCombiner(final_rate);
        D->mVkDeviceTable.vkCmdSetFragmentShadingRateKHR(Cmd->pVkCmdBuf, &fragmentSize, combinerOps);
    }
    // TODO: VRS Tier2
#endif
}
