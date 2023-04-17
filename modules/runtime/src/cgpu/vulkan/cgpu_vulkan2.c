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
            isaInfos[i].stage = VkUtil_TranslateShaderUsages(descs[i].entry.stage);
            if (i >= 1) // HACK
            {
                isaInfos[i - 1].nextStage = isaInfos[i].stage;
            }
            isaInfos[i].pName = descs[i].entry.entry;
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
            linked->pVkShaders[i] = outShaders[i];
            linked->pStages[i] = descs[i].entry.stage;
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

// StateBuffer APIs

CGPUStateBufferId cgpu_create_state_buffer_vulkan(CGPUCommandBufferId cmd, const struct CGPUStateBufferDescriptor* desc)
{
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUStateBuffer_Vulkan* sb = cgpu_calloc(1, sizeof(CGPUStateBuffer_Vulkan));
    sb->super.device = CB->super.device;
    sb->super.cmd = cmd;
    return &sb->super;
}

void cgpu_render_encoder_bind_state_buffer_vulkan(CGPURenderPassEncoderId encoder, CGPUStateBufferId sb)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)sb;
    S->pREncoder = encoder;
}

void cgpu_compute_encoder_bind_state_buffer_vulkan(CGPUComputePassEncoderId encoder, CGPUStateBufferId sb)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)sb;
    S->pCEncoder = encoder;
}

void cgpu_free_state_buffer_vulkan(CGPUStateBufferId sb)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)sb;
    cgpu_free(S);
}

// raster state encoder APIs

CGPURasterStateEncoderId cgpu_open_raster_state_encoder_vulkan(CGPUStateBufferId sb, CGPURenderPassEncoderId encoder)
{
    return (CGPURasterStateEncoderId)sb;
}

void cgpu_close_raster_state_encoder_vulkan(CGPURasterStateEncoderId encoder)
{
    return;
}

// dynamic_state
void cgpu_raster_state_encoder_set_viewport_vulkan(CGPURasterStateEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    VkViewport viewport = {
        .x = x,
        .y = y + height,
        .width = width,
        .height = -height,
        .minDepth = min_depth,
        .maxDepth = max_depth
    };

    D->mVkDeviceTable.vkCmdSetViewport(CB->pVkCmdBuf, 0, 1, &viewport);
}

void cgpu_raster_state_encoder_set_scissor_vulkan(CGPURasterStateEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    VkRect2D scissor = { .offset = { x, y }, .extent = { width, height } };

    D->mVkDeviceTable.vkCmdSetScissor(CB->pVkCmdBuf, 0, 1, &scissor);
}

void cgpu_raster_state_encoder_set_cull_mode_vulkan(CGPURasterStateEncoderId encoder, ECGPUCullMode cull_mode)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    VkCullModeFlags vkcull_mode = gVkCullModeTranslator[cull_mode];

    D->mVkDeviceTable.vkCmdSetCullModeEXT(CB->pVkCmdBuf, vkcull_mode);
}

void cgpu_raster_state_encoder_set_front_face_vulkan(CGPURasterStateEncoderId encoder, ECGPUFrontFace front_face)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    VkFrontFace vkfront_face = gVkFrontFaceTranslator[front_face];

    D->mVkDeviceTable.vkCmdSetFrontFaceEXT(CB->pVkCmdBuf, vkfront_face);
}

void cgpu_raster_state_encoder_set_primitive_topology_vulkan(CGPURasterStateEncoderId encoder, ECGPUPrimitiveTopology topology)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    VkPrimitiveTopology vktopology = VkUtil_TranslateTopology(topology);

    D->mVkDeviceTable.vkCmdSetPrimitiveTopologyEXT(CB->pVkCmdBuf, vktopology);
}

void cgpu_raster_state_encoder_set_depth_test_enabled_vulkan(CGPURasterStateEncoderId encoder, bool enabled)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    D->mVkDeviceTable.vkCmdSetDepthTestEnableEXT(CB->pVkCmdBuf, enabled);
}

void cgpu_raster_state_encoder_set_depth_write_enabled_vulkan(CGPURasterStateEncoderId encoder, bool enabled)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    D->mVkDeviceTable.vkCmdSetDepthWriteEnableEXT(CB->pVkCmdBuf, enabled);
}

void cgpu_raster_state_encoder_set_depth_compare_op_vulkan(CGPURasterStateEncoderId encoder, ECGPUCompareMode compare_op)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    D->mVkDeviceTable.vkCmdSetDepthCompareOpEXT(CB->pVkCmdBuf, (VkCompareOp)compare_op);
}

void cgpu_raster_state_encoder_set_stencil_test_enabled_vulkan(CGPURasterStateEncoderId encoder, bool enabled)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    D->mVkDeviceTable.vkCmdSetStencilTestEnableEXT(CB->pVkCmdBuf, enabled);
}

void cgpu_raster_state_encoder_set_stencil_compare_op_vulkan(CGPURasterStateEncoderId encoder, CGPUStencilFaces faces, ECGPUStencilOp failOp, ECGPUStencilOp passOp, ECGPUStencilOp depthFailOp, ECGPUCompareMode compareOp)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    VkStencilFaceFlags vkFaces = 0;
    vkFaces |= (faces & CGPU_STENCIL_FACE_FRONT) ? VK_STENCIL_FACE_FRONT_BIT : 0;
    vkFaces |= (faces & CGPU_STENCIL_FACE_BACK) ? VK_STENCIL_FACE_BACK_BIT : 0;

    D->mVkDeviceTable.vkCmdSetStencilOp(CB->pVkCmdBuf,
        vkFaces, 
        gVkStencilOpTranslator[failOp], gVkStencilOpTranslator[passOp], 
        gVkStencilOpTranslator[depthFailOp], gVkComparisonFuncTranslator[compareOp]);
}
// dynamic_state2

// dynamic_state3

void cgpu_raster_state_encoder_set_fill_mode_vulkan(CGPURasterStateEncoderId encoder, ECGPUFillMode fill_mode)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    D->mVkDeviceTable.vkCmdSetPolygonModeEXT(CB->pVkCmdBuf, gVkFillModeTranslator[fill_mode]);
}

void cgpu_raster_state_encoder_set_sample_count_vulkan(CGPURasterStateEncoderId encoder, ECGPUSampleCount sample_count)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    D->mVkDeviceTable.vkCmdSetRasterizationSamplesEXT(CB->pVkCmdBuf, VkUtil_SampleCountTranslateToVk(sample_count));
}

// shader state encoder APIs

CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_r_vulkan(CGPUStateBufferId sb, CGPURenderPassEncoderId encoder)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)sb;
    S->pREncoder = encoder;
    return (CGPUShaderStateEncoderId)sb;
}

CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_c_vulkan(CGPUStateBufferId sb, CGPUComputePassEncoderId encoder)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)sb;
    S->pCEncoder = encoder;
    return (CGPUShaderStateEncoderId)sb;
}

void cgpu_shader_state_encoder_bind_shaders_vulkan(CGPUShaderStateEncoderId encoder, uint32_t stage_count, const ECGPUShaderStage* stages, const CGPUCompiledShaderId* shaders)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = S->pCEncoder ? (CGPUCommandBuffer_Vulkan*)S->pCEncoder : (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    VkShaderStageFlagBits vkStages[CGPU_SHADER_STAGE_COUNT];
    VkShaderEXT vkShaders[CGPU_SHADER_STAGE_COUNT];
    for (uint32_t i = 0; i < stage_count; i++)
    {
        vkStages[i] = VkUtil_TranslateShaderUsages(stages[i]);
        vkShaders[i] = ((CGPUCompiledShader_Vulkan*)shaders[i])->pVkShader;
    }

    D->mVkDeviceTable.vkCmdBindShadersEXT(CB->pVkCmdBuf, stage_count, vkStages, vkShaders);
}

void cgpu_shader_state_encoder_bind_linked_shader_vulkan(CGPUShaderStateEncoderId encoder, CGPULinkedShaderId linked)
{
    CGPUStateBuffer_Vulkan* S = (CGPUStateBuffer_Vulkan*)encoder;
    CGPUCommandBuffer_Vulkan* CB = S->pCEncoder ? (CGPUCommandBuffer_Vulkan*)S->pCEncoder : (CGPUCommandBuffer_Vulkan*)S->pREncoder;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;
    CGPULinkedShader_Vulkan* L = (CGPULinkedShader_Vulkan*)linked;

    uint32_t stage_count = 0;
    VkShaderStageFlagBits vkStages[CGPU_SHADER_STAGE_COUNT];
    VkShaderEXT vkShaders[CGPU_SHADER_STAGE_COUNT];
    for (uint32_t i = 0; i < CGPU_SHADER_STAGE_COUNT; i++)
    {
        if (L->pVkShaders[i])
        {
            vkStages[stage_count] = VkUtil_TranslateShaderUsages(L->pStages[i]);
            vkShaders[stage_count] = L->pVkShaders[i];
            stage_count++;
        }
    }

    D->mVkDeviceTable.vkCmdBindShadersEXT(CB->pVkCmdBuf, stage_count, vkStages, vkShaders);
}

void cgpu_close_shader_state_encoder_vulkan(CGPUShaderStateEncoderId encoder)
{
    return;
}

// user state encoder APIs

CGPUUserStateEncoderId cgpu_open_user_state_encoder_vulkan(CGPUStateBufferId sb, CGPURenderPassEncoderId encoder)
{
    return (CGPUUserStateEncoderId)sb;
}

void cgpu_close_user_state_encoder_vulkan(CGPUUserStateEncoderId encoder)
{
    return;
}

// EXPERIMENTAL binder APIs

CGPUBinderId cgpu_create_binder_vulkan(CGPUCommandBufferId cmd)
{    
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUBinder_Vulkan* bdr = cgpu_calloc(1, sizeof(CGPUBinder_Vulkan));
    bdr->super.device = CB->super.device;
    bdr->super.cmd = cmd;
    return &bdr->super;
}

void cgpu_binder_bind_vertex_layout_vulkan(CGPUBinderId binder, const struct CGPUVertexLayout* layout)
{
    CGPUBinder_Vulkan* bdr = (CGPUBinder_Vulkan*)binder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)bdr->super.cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;

    uint32_t input_binding_count = 0;
	uint32_t input_attribute_count = 0;
    VkUtil_GetVertexInputBindingAttrCount(layout, &input_binding_count, &input_attribute_count);
    
    uint64_t dsize = 0u;
    dsize += (sizeof(VkVertexInputBindingDescription2EXT) * input_binding_count);
    const uint64_t input_attrs_offset = dsize;
    dsize += (sizeof(VkVertexInputAttributeDescription2EXT) * input_attribute_count);
    
    uint8_t* data = cgpu_calloc(1, dsize);
    VkVertexInputBindingDescription2EXT* input_bindings = (VkVertexInputBindingDescription2EXT*)data;
    VkVertexInputAttributeDescription2EXT* input_attributes = (VkVertexInputAttributeDescription2EXT*)(data + input_attrs_offset);
    {
        // Ignore everything that's beyond CGPU_MAX_VERTEX_ATTRIBS
        uint32_t attrib_count = layout->attribute_count > CGPU_MAX_VERTEX_ATTRIBS ? CGPU_MAX_VERTEX_ATTRIBS : layout->attribute_count;
        uint32_t attr_slot = 0;
        // Initial values
        for (uint32_t i = 0; i < attrib_count; ++i)
        {
            const CGPUVertexAttribute* attrib = &(layout->attributes[i]);
            const uint32_t array_size = attrib->array_size ? attrib->array_size : 1;

            VkVertexInputBindingDescription2EXT* current_binding = &input_bindings[attrib->binding];
            current_binding->binding = attrib->binding;
            if (attrib->rate == CGPU_INPUT_RATE_INSTANCE)
                current_binding->inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            else
                current_binding->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            current_binding->stride += attrib->elem_stride;
            
            for(uint32_t j = 0; j < array_size; j++)
            {
                input_attributes[attr_slot].location = attr_slot;
                input_attributes[attr_slot].binding = attrib->binding;
                input_attributes[attr_slot].format = VkUtil_FormatTranslateToVk(attrib->format);
                input_attributes[attr_slot].offset = attrib->offset + (j * FormatUtil_BitSizeOfBlock(attrib->format) / 8);
                ++attr_slot;
            }
        }
    }

    D->mVkDeviceTable.vkCmdSetVertexInputEXT(CB->pVkCmdBuf, input_binding_count, input_bindings, input_attribute_count, input_attributes);

    cgpu_free(data);
}

void cgpu_binder_bind_vertex_buffer_vulkan(CGPUBinderId binder, uint32_t first_binding, uint32_t binding_count, const CGPUBufferId* buffers, const uint64_t* offsets, const uint64_t* sizes, const uint64_t* strides)
{
    CGPUBinder_Vulkan* bdr = (CGPUBinder_Vulkan*)binder;
    CGPUCommandBuffer_Vulkan* CB = (CGPUCommandBuffer_Vulkan*)bdr->super.cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)CB->super.device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)D->super.adapter;
    const CGPUBuffer_Vulkan** Buffers = (const CGPUBuffer_Vulkan**)buffers;
    const uint32_t final_buffer_count = cgpu_min(binding_count, A->mPhysicalDeviceProps.properties.limits.maxVertexInputBindings);

    DECLARE_ZERO(VkBuffer, vkBuffers[64]);
    DECLARE_ZERO(VkDeviceSize, vkOffsets[64]);

    for (uint32_t i = 0; i < final_buffer_count; ++i)
    {
        vkBuffers[i] = Buffers[i]->pVkBuffer;
        vkOffsets[i] = (offsets ? offsets[i] : 0);
    }

    D->mVkDeviceTable.vkCmdBindVertexBuffers2EXT(CB->pVkCmdBuf, first_binding, binding_count, vkBuffers, vkOffsets, sizes, strides);
}

void cgpu_free_binder_vulkan(CGPUBinderId binder)
{
    CGPUBinder_Vulkan* Bdr = (CGPUBinder_Vulkan*)binder;
    cgpu_free(Bdr);
}