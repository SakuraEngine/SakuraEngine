#include "cgpu/backend/vulkan/cgpu_vulkan2.h"
#include "vulkan_utils.h"
#include "../common/common_utils.h"


// Compiled/Linked ISA APIs

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

// StateStream APIs

CGPUStateStreamId cgpu_create_state_stream_vulkan(CGPUDeviceId device, const struct CGPUStateStreamDescriptor* desc)
{
    CGPUStateStream_Vulkan* stream = cgpu_calloc(1, sizeof(CGPUStateStream_Vulkan));
    stream->super.device = device;
    return &stream->super;
}

void cgpu_render_encoder_bind_state_stream_vulkan(CGPURenderPassEncoderId encoder, CGPUStateStreamId stream)
{
    CGPUStateStream_Vulkan* S = (CGPUStateStream_Vulkan*)stream;
    cgpu_free(S);
}

void cgpu_compute_encoder_bind_state_stream_vulkan(CGPUComputePassEncoderId encoder, CGPUStateStreamId stream)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_free_state_stream_vulkan(CGPUStateStreamId stream)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// raster state encoder APIs

CGPURasterStateEncoderId cgpu_open_raster_state_encoder_vulkan(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return CGPU_NULLPTR;
}

void cgpu_close_raster_state_encoder_vulkan(CGPURasterStateEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// dynamic_state
void cgpu_raster_state_encoder_set_viewport_vulkan(CGPURasterStateEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_scissor_vulkan(CGPURasterStateEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_cull_mode_vulkan(CGPURasterStateEncoderId encoder, ECGPUCullMode cull_mode)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_front_face_vulkan(CGPURasterStateEncoderId encoder, ECGPUFrontFace front_face)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_primitive_topology_vulkan(CGPURasterStateEncoderId encoder, ECGPUPrimitiveTopology topology)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_depth_test_enabled_vulkan(CGPURasterStateEncoderId encoder, bool enabled)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_depth_write_enabled_vulkan(CGPURasterStateEncoderId encoder, bool enabled)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_depth_compare_op_vulkan(CGPURasterStateEncoderId encoder, ECGPUCompareMode compare_op)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_stencil_test_enabled_vulkan(CGPURasterStateEncoderId encoder, bool enabled)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_stencil_compare_op_vulkan(CGPURasterStateEncoderId encoder, CGPUStencilFaces faces, ECGPUStencilOp failOp, ECGPUStencilOp passOp, ECGPUStencilOp depthFailOp, ECGPUCompareMode compareOp)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
// dynamic_state2

// dynamic_state3

void cgpu_raster_state_encoder_set_fill_mode_vulkan(CGPURasterStateEncoderId encoder, ECGPUFillMode fill_mode)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_raster_state_encoder_set_sample_count_vulkan(CGPURasterStateEncoderId encoder, ECGPUSampleCount sample_count)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// shader state encoder APIs

CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_r_vulkan(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return CGPU_NULLPTR;
}

CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_c_vulkan(CGPUStateStreamId stream, CGPUComputePassEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return CGPU_NULLPTR;
}

void cgpu_shader_state_encoder_bind_shaders_vulkan(CGPUShaderStateEncoderId encoder, uint32_t stage_count, const ECGPUShaderStage* stages, const CGPUCompiledShaderId* shaders)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_shader_state_encoder_bind_linked_shader_vulkan(CGPUShaderStateEncoderId encoder, CGPULinkedShaderId linked)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_close_shader_state_encoder_vulkan(CGPUShaderStateEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// user state encoder APIs

CGPUUserStateEncoderId cgpu_open_user_state_encoder_vulkan(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return CGPU_NULLPTR;
}

void cgpu_close_user_state_encoder_vulkan(CGPUUserStateEncoderId encoder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// EXPERIMENTAL binder APIs

CGPUBinderId cgpu_create_binder_vulkan(CGPURootSignatureId root_signature)
{    
    SKR_UNIMPLEMENTED_FUNCTION();
    return CGPU_NULLPTR;
}

void cgpu_binder_bind_vertex_layout_vulkan(CGPUBinderId binder, const struct CGPUVertexLayout* layout)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_binder_bind_vertex_buffer_vulkan(CGPUBinderId binder, uint32_t first_binding, uint32_t binding_count, const CGPUBufferId* buffers, const uint64_t* offsets, const uint64_t* sizes, const uint64_t* strides)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_free_binder_vulkan(CGPUBinderId binder)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}