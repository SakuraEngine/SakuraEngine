#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "vulkan_utils.h"
#include "../common/common_utils.h"

static const char* kVkShaderISAMemoryPoolName = "vk::shader_isa";

void cgpu_create_shader_objs_vulkan_impl(CGPURootSignatureId signature, 
    const struct CGPUCompiledShaderDescriptor* descs, uint32_t count, VkShaderEXT* outShaders)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)signature->device;
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)signature;
    if (D->mVkDeviceTable.vkCreateShadersEXT)
    {
        VkShaderCreateInfoEXT isaInfos[CGPU_SHADER_STAGE_COUNT];
        VkShaderCreateInfoEXT isaInfoCommon = {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
            .pNext = NULL,
            .flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
            .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
            .setLayoutCount = RS->mSetLayoutCount,
            .pSetLayouts = RS->pVkSetLayouts,
            .pushConstantRangeCount = RS->super.push_constant_count,
            .pPushConstantRanges = RS->pPushConstRanges
        };
        for (uint32_t i = 0; i < count; i++)
        {
            isaInfos[i] = isaInfoCommon;
            isaInfos[i].stage = VkUtil_TranslateShaderUsages(descs[i].stage);
            if (i >= 1) // HACK
            {
                isaInfos[i - 1].nextStage = isaInfos[i].stage;
            }
            isaInfos[i].pName = descs[i].entry;
            isaInfos[i].pCode = descs[i].shader_code;
            isaInfos[i].codeSize = descs[i].code_size;
        }
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateShadersEXT(D->pVkDevice, count, isaInfos, GLOBAL_VkAllocationCallbacks, outShaders));
#ifdef TRACY_ENABLE
        if (D->mVkDeviceTable.vkGetShaderBinaryDataEXT)
        {
            for (uint32_t i = 0; i < count; i++)
            {
                size_t size;
                VkResult res = D->mVkDeviceTable.vkGetShaderBinaryDataEXT(D->pVkDevice, outShaders[i], &size, NULL);
                // uint8_t* ISA = sakura_calloc(1, size);
                // D->mVkDeviceTable.vkGetShaderBinaryDataEXT(D->pVkDevice, outShaders[i], &size, ISA);
                if ((res == VK_SUCCESS) && (size > 0))
                {
                    TracyCAllocN(outShaders[i], size, kVkShaderISAMemoryPoolName)
                }
            }
        }
#endif
    }
}

CGPULinkedShaderId cgpu_compile_and_link_shaders_vulkan(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)signature->device;
    // CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)signature;
    if (D->mVkDeviceTable.vkCreateShadersEXT)
    {
        CGPULinkedShader_Vulkan* linked = count ? cgpu_calloc(1, sizeof(CGPULinkedShader_Vulkan)) : CGPU_NULLPTR;
        VkShaderEXT outShaders[CGPU_SHADER_STAGE_COUNT];
        cgpu_create_shader_objs_vulkan_impl(signature, descs, count, outShaders);
        for (uint32_t i = 0; i < count; i++)
        {
            linked->pVkShaders[descs[i].stage] = outShaders[i];
        }
        linked->super.device = signature->device;
        linked->super.root_signature = signature;
        return &linked->super;
    }
    else
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return CGPU_NULLPTR;
    }
}

void cgpu_compile_shaders_vulkan(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count, CGPUCompiledShaderId* out_isas)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)signature->device;
    // CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)signature;
    if (D->mVkDeviceTable.vkCreateShadersEXT)
    {
        VkShaderEXT* outShaders = cgpu_calloc(count, sizeof(VkShaderEXT));
        cgpu_create_shader_objs_vulkan_impl(signature, descs, count, outShaders);
        for (uint32_t i = 0; i < count; i++)
        {
            out_isas = cgpu_calloc(1, sizeof(CGPUCompiledShader_Vulkan));
            ((CGPUCompiledShader_Vulkan*)out_isas)->pVkShader = outShaders[i];
            ((CGPUCompiledShader_Vulkan*)out_isas)->super.device = signature->device;
            ((CGPUCompiledShader_Vulkan*)out_isas)->super.root_signature = signature;
        }
        cgpu_free(outShaders);
    }
    else
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
}

void cgpu_free_compiled_shader_vulkan(CGPUCompiledShaderId shader)
{
    const CGPUCompiledShader_Vulkan* S = (CGPUCompiledShader_Vulkan*)shader;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)shader->device;
    if (S)
    {
#ifdef TRACY_ENABLE
        if (D->mVkDeviceTable.vkGetShaderBinaryDataEXT)
        {
            void* data; size_t size;
            D->mVkDeviceTable.vkGetShaderBinaryDataEXT(D->pVkDevice, S->pVkShader, &size, &data);
            TracyCFreeN(data, kVkShaderISAMemoryPoolName)
        }
#endif
        if (D->mVkDeviceTable.vkDestroyShaderEXT)
        {
            D->mVkDeviceTable.vkDestroyShaderEXT(D->pVkDevice, S->pVkShader, GLOBAL_VkAllocationCallbacks);
        }
        cgpu_free((void*)S);
    }
}

void cgpu_free_linked_shader_vulkan(CGPULinkedShaderId shader)
{
    const CGPULinkedShader_Vulkan* S = (CGPULinkedShader_Vulkan*)shader;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)shader->device;
    if (S)
    {
        for (uint32_t i = 0; i < CGPU_SHADER_STAGE_COUNT; i++)
        {
#ifdef TRACY_ENABLE
            if (D->mVkDeviceTable.vkGetShaderBinaryDataEXT)
            {
                void* data; size_t size;
                D->mVkDeviceTable.vkGetShaderBinaryDataEXT(D->pVkDevice, S->pVkShaders[i], &size, &data);
                TracyCFreeN(data, kVkShaderISAMemoryPoolName)
            }
#endif
            if (D->mVkDeviceTable.vkDestroyShaderEXT)
            {
                D->mVkDeviceTable.vkDestroyShaderEXT(D->pVkDevice, S->pVkShaders[i], GLOBAL_VkAllocationCallbacks);
            }
        }
        
        cgpu_free((void*)S);
    }
}