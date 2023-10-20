#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "cgpu/cgpu_config.h"
#include "vulkan_utils.h"
#include "cgpu/shader-reflections/spirv/spirv_reflect.h"
#include "../common/common_utils.h"

#include <string.h>

static void VkUtil_FindOrCreateFrameBuffer(const CGPUDevice_Vulkan* D, const struct VkUtil_FramebufferDesc* pDesc, VkFramebuffer* ppFramebuffer)
{
    VkFramebuffer found = VkUtil_FramebufferTableTryFind(D->pPassTable, pDesc);
    if (found != VK_NULL_HANDLE)
    {
        *ppFramebuffer = found;
        return;
    }
    cgpu_assert(VK_NULL_HANDLE != D->pVkDevice);
    VkFramebufferCreateInfo add_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = pDesc->mAttachmentCount,
        .pAttachments = pDesc->pImageViews,
        .width = pDesc->mWidth,
        .height = pDesc->mHeight,
        .layers = pDesc->mLayers,
        .renderPass = pDesc->pRenderPass
    };
    CHECK_VKRESULT(vkCreateFramebuffer(D->pVkDevice, &add_info, GLOBAL_VkAllocationCallbacks, ppFramebuffer));
    VkUtil_FramebufferTableAdd(D->pPassTable, pDesc, *ppFramebuffer);
}

// TODO: recycle cached render passes
FORCEINLINE static void VkUtil_FreeFramebuffer(CGPUDevice_Vulkan* D, VkFramebuffer pFramebuffer)
{
    D->mVkDeviceTable.vkDestroyFramebuffer(D->pVkDevice, pFramebuffer, GLOBAL_VkAllocationCallbacks);
}

// Render Pass Utils
// TODO: recycle cached render passes
FORCEINLINE static void VkUtil_FreeRenderPass(CGPUDevice_Vulkan* D, VkRenderPass pRenderPass)
{
    D->mVkDeviceTable.vkDestroyRenderPass(D->pVkDevice, pRenderPass, GLOBAL_VkAllocationCallbacks);
}

static void VkUtil_FindOrCreateRenderPass(const CGPUDevice_Vulkan* D, const VkUtil_RenderPassDesc* pDesc, VkRenderPass* ppRenderPass)
{
    VkRenderPass found = VkUtil_RenderPassTableTryFind(D->pPassTable, pDesc);
    if (found != VK_NULL_HANDLE)
    {
        *ppRenderPass = found;
        return;
    }
    cgpu_assert(VK_NULL_HANDLE != D->pVkDevice);
    uint32_t colorAttachmentCount = pDesc->mColorAttachmentCount;
    uint32_t colorResolveAttachmentCount = 0;
    uint32_t depthAttachmentCount = (pDesc->mDepthStencilFormat != CGPU_FORMAT_UNDEFINED) ? 1 : 0;
    VkAttachmentDescription attachments[2 * CGPU_MAX_MRT_COUNT + 1] = { 0 };
    VkAttachmentReference color_attachment_refs[CGPU_MAX_MRT_COUNT] = { 0 };
    VkAttachmentReference color_resolve_attachment_refs[CGPU_MAX_MRT_COUNT] = { 0 };
    VkAttachmentReference depth_stencil_attachment_ref[1] = { 0 };
    VkSampleCountFlagBits sample_count = VkUtil_SampleCountTranslateToVk(pDesc->mSampleCount);
    // Fill out attachment descriptions and references
    uint32_t ssidx = 0;
    // Color
    for (uint32_t i = 0; i < colorAttachmentCount; i++)
    {
        // descriptions
        attachments[ssidx].flags = 0;
        attachments[ssidx].format = (VkFormat)VkUtil_FormatTranslateToVk(pDesc->pColorFormats[i]);
        attachments[ssidx].samples = sample_count;
        attachments[ssidx].loadOp = gVkAttachmentLoadOpTranslator[pDesc->pLoadActionsColor[i]];
        attachments[ssidx].storeOp = gVkAttachmentStoreOpTranslator[pDesc->pStoreActionsColor[i]];
        attachments[ssidx].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments[ssidx].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // references
        color_attachment_refs[i].attachment = ssidx;
        color_attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ssidx++;
    }
    // Color Resolve
    for (uint32_t i = 0; i < colorAttachmentCount; i++)
    {
        if (pDesc->pResolveMasks[i])
        {
            attachments[ssidx].flags = 0;
            attachments[ssidx].format = (VkFormat)VkUtil_FormatTranslateToVk(pDesc->pColorFormats[i]);
            attachments[ssidx].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[ssidx].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[ssidx].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[ssidx].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[ssidx].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // references
            color_resolve_attachment_refs[i].attachment = ssidx;
            color_resolve_attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ssidx++;
            colorResolveAttachmentCount++;
        }
        else
        {
            color_resolve_attachment_refs[i].attachment = VK_ATTACHMENT_UNUSED;
        }
    }
    // Depth stencil
    if (depthAttachmentCount > 0)
    {
        attachments[ssidx].flags = 0;
        attachments[ssidx].format = (VkFormat)VkUtil_FormatTranslateToVk(pDesc->mDepthStencilFormat);
        attachments[ssidx].samples = sample_count;
        attachments[ssidx].loadOp = gVkAttachmentLoadOpTranslator[pDesc->mLoadActionDepth];
        attachments[ssidx].storeOp = gVkAttachmentStoreOpTranslator[pDesc->mStoreActionDepth];
        attachments[ssidx].stencilLoadOp = gVkAttachmentLoadOpTranslator[pDesc->mLoadActionStencil];
        attachments[ssidx].stencilStoreOp = gVkAttachmentStoreOpTranslator[pDesc->mStoreActionStencil];
        attachments[ssidx].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[ssidx].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_stencil_attachment_ref[0].attachment = ssidx;
        depth_stencil_attachment_ref[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        ssidx++;
    }
    uint32_t attachment_count = colorAttachmentCount;
    attachment_count += depthAttachmentCount;
    attachment_count += colorResolveAttachmentCount;
    void* render_pass_next = NULL;
    // Fill Description
    VkSubpassDescription subpass = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = colorAttachmentCount,
        .pColorAttachments = color_attachment_refs,
        .pResolveAttachments = colorResolveAttachmentCount ? color_resolve_attachment_refs : VK_NULL_HANDLE,
        .pDepthStencilAttachment = (depthAttachmentCount > 0) ? depth_stencil_attachment_ref : VK_NULL_HANDLE,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL
    };
    VkRenderPassCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = render_pass_next,
        .flags = 0,
        .attachmentCount = attachment_count,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = NULL
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateRenderPass(D->pVkDevice, &create_info, GLOBAL_VkAllocationCallbacks, ppRenderPass));
    VkUtil_RenderPassTableAdd(D->pPassTable, pDesc, *ppRenderPass);
}

void cgpu_query_instance_features_vulkan(CGPUInstanceId instance, struct CGPUInstanceFeatures* features)
{
    features->specialization_constant = true;
}

void cgpu_enum_adapters_vulkan(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num)
{
    CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)instance;
    *adapters_num = I->mPhysicalDeviceCount;
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++)
        {
            adapters[i] = &I->pVulkanAdapters[i].super;
        }
    }
}

const CGPUAdapterDetail* cgpu_query_adapter_detail_vulkan(const CGPUAdapterId adapter)
{
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)adapter;
    return &A->adapter_detail;
}

uint32_t cgpu_query_queue_count_vulkan(const CGPUAdapterId adapter, const ECGPUQueueType type)
{
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)adapter;
    uint32_t count = 0;
    switch (type)
    {
        case CGPU_QUEUE_TYPE_GRAPHICS: {
            for (uint32_t i = 0; i < A->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &A->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    return prop->queueCount;
                }
            }
        }
        break;
        case CGPU_QUEUE_TYPE_COMPUTE: {
            for (uint32_t i = 0; i < A->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &A->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    if (!(prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    {
                        return prop->queueCount;
                    }
                }
            }
        }
        break;
        case CGPU_QUEUE_TYPE_TRANSFER: {
            for (uint32_t i = 0; i < A->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &A->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    if (!(prop->queueFlags & VK_QUEUE_COMPUTE_BIT))
                    {
                        if (!(prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
                        {
                            return prop->queueCount;
                        }
                    }
                }
            }
        }
        break;
        case CGPU_QUEUE_TYPE_TILE_MAPPING: {
            for (uint32_t i = 0; i < A->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &A->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                {
                    if (!(prop->queueFlags & VK_QUEUE_TRANSFER_BIT))
                    {
                        if (!(prop->queueFlags & VK_QUEUE_COMPUTE_BIT))
                        {
                            if (!(prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
                            {
                                return prop->queueCount;
                            }
                        }
                    }
                }
            }
        }
        break;
        default:
            cgpu_assert(0 && "CGPU VULKAN: ERROR Queue Type!");
    }
    return count;
}

// API Objects APIs
CGPUFenceId cgpu_create_fence_vulkan(CGPUDeviceId device)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUFence_Vulkan* F = (CGPUFence_Vulkan*)cgpu_calloc(1, sizeof(CGPUFence_Vulkan));
    cgpu_assert(F);
    VkFenceCreateInfo add_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
    };

    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateFence(
    D->pVkDevice, &add_info, GLOBAL_VkAllocationCallbacks, &F->pVkFence));
    F->mSubmitted = false;
    return &F->super;
}

void cgpu_wait_fences_vulkan(const CGPUFenceId* fences, uint32_t fence_count)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)fences[0]->device;
    DECLARE_ZERO_VLA(VkFence, vfences, fence_count)
    uint32_t numValidFences = 0;
    for (uint32_t i = 0; i < fence_count; ++i)
    {
        CGPUFence_Vulkan* Fence = (CGPUFence_Vulkan*)fences[i];
        if (Fence->mSubmitted)
            vfences[numValidFences++] = Fence->pVkFence;
    }
    if (numValidFences)
    {
        D->mVkDeviceTable.vkWaitForFences(D->pVkDevice, numValidFences, vfences, VK_TRUE, UINT64_MAX);
        D->mVkDeviceTable.vkResetFences(D->pVkDevice, numValidFences, vfences);
    }
    for (uint32_t i = 0; i < fence_count; ++i)
    {
        CGPUFence_Vulkan* Fence = (CGPUFence_Vulkan*)fences[i];
        Fence->mSubmitted = false;
    }
}

ECGPUFenceStatus cgpu_query_fence_status_vulkan(CGPUFenceId fence)
{
    ECGPUFenceStatus status = CGPU_FENCE_STATUS_COMPLETE;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)fence->device;
    CGPUFence_Vulkan* F = (CGPUFence_Vulkan*)fence;
    if (F->mSubmitted)
    {
        VkResult vkRes = vkGetFenceStatus(D->pVkDevice, F->pVkFence);
        /*
        if (vkRes == VK_SUCCESS)
        {
            D->mVkDeviceTable.vkResetFences(D->pVkDevice, 1, &F->pVkFence);
            F->mSubmitted = false;
        }
        */
        status = vkRes == VK_SUCCESS ? CGPU_FENCE_STATUS_COMPLETE : CGPU_FENCE_STATUS_INCOMPLETE;
    }
    else
    {
        status = CGPU_FENCE_STATUS_NOTSUBMITTED;
    }
    return status;
}

void cgpu_free_fence_vulkan(CGPUFenceId fence)
{
    CGPUFence_Vulkan* F = (CGPUFence_Vulkan*)fence;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)fence->device;
    D->mVkDeviceTable.vkDestroyFence(D->pVkDevice, F->pVkFence, GLOBAL_VkAllocationCallbacks);
    cgpu_free(F);
}

CGPUSemaphoreId cgpu_create_semaphore_vulkan(CGPUDeviceId device)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUSemaphore_Vulkan* Semaphore = (CGPUSemaphore_Vulkan*)cgpu_calloc(1, sizeof(CGPUSemaphore_Vulkan));
    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateSemaphore(D->pVkDevice,
    &semaphore_info, GLOBAL_VkAllocationCallbacks, &(Semaphore->pVkSemaphore)));
    Semaphore->mSignaled = false;
    return &Semaphore->super;
}

void cgpu_free_semaphore_vulkan(CGPUSemaphoreId semaphore)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)semaphore->device;
    CGPUSemaphore_Vulkan* Semaphore = (CGPUSemaphore_Vulkan*)semaphore;
    D->mVkDeviceTable.vkDestroySemaphore(D->pVkDevice, Semaphore->pVkSemaphore, GLOBAL_VkAllocationCallbacks);
    cgpu_free(Semaphore);
}

uint32_t get_set_count(uint32_t set_index_mask)
{
    uint32_t set_count = 0;
    while (set_index_mask != 0)
    {
        if (set_index_mask & 1)
        {
            set_count++;
        }
        set_index_mask >>= 1;
    }
    return set_count;
}

CGPURootSignatureId cgpu_create_root_signature_vulkan(CGPUDeviceId device,const struct CGPURootSignatureDescriptor* desc)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)cgpu_calloc(1, sizeof(CGPURootSignature_Vulkan));
    CGPUUtil_InitRSParamTables((CGPURootSignature*)RS, desc);
    // [RS POOL] ALLOCATION
    if (desc->pool)
    {
        CGPURootSignature_Vulkan* poolSig =
        (CGPURootSignature_Vulkan*)CGPUUtil_TryAllocateSignature(desc->pool, &RS->super, desc);
        if (poolSig != CGPU_NULLPTR)
        {
            RS->pPipelineLayout = poolSig->pPipelineLayout;
            RS->pSetLayouts = poolSig->pSetLayouts;
            RS->mSetLayoutCount = poolSig->mSetLayoutCount;
            RS->pPushConstRanges = poolSig->pPushConstRanges;
            RS->super.pool = desc->pool;
            RS->super.pool_sig = &poolSig->super;
            return &RS->super;
        }
    }
    // [RS POOL] END ALLOCATION
    // set index mask. set(0, 1, 2, 3) -> 0000...1111
    uint32_t set_index_mask = 0;
    // tables
    for (uint32_t i = 0; i < RS->super.table_count; i++)
    {
        set_index_mask |= (1 << RS->super.tables[i].set_index);
    }
    // static samplers
    for (uint32_t i = 0; i < RS->super.static_sampler_count; i++)
    {
        set_index_mask |= (1 << RS->super.static_samplers[i].set);
    }
    // parse
    const uint32_t set_count = get_set_count(set_index_mask);
    RS->pSetLayouts = (SetLayout_Vulkan*)cgpu_calloc(set_count, sizeof(SetLayout_Vulkan));
    RS->mSetLayoutCount = set_count;
    uint32_t set_index = 0;
    while (set_index_mask != 0)
    {
        if (set_index_mask & 1)
        {
            CGPUParameterTable* param_table = CGPU_NULLPTR;
            for (uint32_t i = 0; i < RS->super.table_count; i++)
            {
                if (RS->super.tables[i].set_index == set_index)
                {
                    param_table = &RS->super.tables[i];
                    break;
                }
            }
            uint32_t bindings_count = param_table ? param_table->resources_count + desc->static_sampler_count : 0 + desc->static_sampler_count;
            VkDescriptorSetLayoutBinding* vkbindings = (VkDescriptorSetLayoutBinding*)cgpu_calloc(
            bindings_count, sizeof(VkDescriptorSetLayoutBinding));
            uint32_t i_binding = 0;
            // bindings
            if (param_table)
            {
                for (i_binding = 0; i_binding < param_table->resources_count; i_binding++)
                {
                    vkbindings[i_binding].binding = param_table->resources[i_binding].binding;
                    vkbindings[i_binding].stageFlags = VkUtil_TranslateShaderUsages(param_table->resources[i_binding].stages);
                    vkbindings[i_binding].descriptorType = VkUtil_TranslateResourceType(param_table->resources[i_binding].type);
                    vkbindings[i_binding].descriptorCount = param_table->resources[i_binding].size;
                }
            }
            // static samplers
            for (uint32_t i_ss = 0; i_ss < desc->static_sampler_count; i_ss++)
            {
                if (RS->super.static_samplers[i_ss].set == set_index)
                {
                    CGPUSampler_Vulkan* immutableSampler = (CGPUSampler_Vulkan*)desc->static_samplers[i_ss];
                    vkbindings[i_binding].pImmutableSamplers = &immutableSampler->pVkSampler;
                    vkbindings[i_binding].binding = RS->super.static_samplers[i_ss].binding;
                    vkbindings[i_binding].stageFlags = VkUtil_TranslateShaderUsages(RS->super.static_samplers[i_ss].stages);
                    vkbindings[i_binding].descriptorType = VkUtil_TranslateResourceType(RS->super.static_samplers[i_ss].type);
                    vkbindings[i_binding].descriptorCount = RS->super.static_samplers[i_ss].size;
                    i_binding++;
                }
            }
            VkDescriptorSetLayoutCreateInfo setLayoutInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .pBindings = vkbindings,
                .bindingCount = i_binding
            };
            CHECK_VKRESULT(D->mVkDeviceTable.vkCreateDescriptorSetLayout(D->pVkDevice,
                &setLayoutInfo, GLOBAL_VkAllocationCallbacks, &RS->pSetLayouts[set_index].layout));
            VkUtil_ConsumeDescriptorSets(D->pDescriptorPool, &RS->pSetLayouts[set_index].layout,
                &RS->pSetLayouts[set_index].pEmptyDescSet, 1);

            if (bindings_count) cgpu_free(vkbindings);
        }
        set_index++;
        set_index_mask >>= 1;
    }
    // Push constants
    // Collect push constants count
    if (RS->super.push_constant_count > 0)
    {
        RS->pPushConstRanges = (VkPushConstantRange*)cgpu_calloc(RS->super.push_constant_count, sizeof(VkPushConstantRange));
        // Create Vk Objects
        for (uint32_t i_const = 0; i_const < RS->super.push_constant_count; i_const++)
        {
            RS->pPushConstRanges[i_const].stageFlags =
            VkUtil_TranslateShaderUsages(RS->super.push_constants[i_const].stages);
            RS->pPushConstRanges[i_const].size = RS->super.push_constants[i_const].size;
            RS->pPushConstRanges[i_const].offset = RS->super.push_constants[i_const].offset;
        }
    }
    // Record Descriptor Sets
    RS->pVkSetLayouts = cgpu_calloc(set_count, sizeof(VkDescriptorSetLayout));
    for (uint32_t i_set = 0; i_set < set_count; i_set++)
    {
        SetLayout_Vulkan* set_to_record = (SetLayout_Vulkan*)&RS->pSetLayouts[i_set];
        RS->pVkSetLayouts[i_set] = set_to_record->layout;
    }
    // Create Pipeline Layout
    VkPipelineLayoutCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = set_count,
        .pSetLayouts = RS->pVkSetLayouts,
        .pushConstantRangeCount = RS->super.push_constant_count,
        .pPushConstantRanges = RS->pPushConstRanges
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreatePipelineLayout(D->pVkDevice, &pipeline_info, GLOBAL_VkAllocationCallbacks, &RS->pPipelineLayout));
    // Create Update Templates
    for (uint32_t i_table = 0; i_table < RS->super.table_count; i_table++)
    {
        CGPUParameterTable* param_table = &RS->super.tables[i_table];
        SetLayout_Vulkan* set_to_record = &RS->pSetLayouts[param_table->set_index];
        uint32_t update_entry_count = param_table->resources_count;
        VkDescriptorUpdateTemplateEntry* template_entries = (VkDescriptorUpdateTemplateEntry*)cgpu_calloc(
            param_table->resources_count, sizeof(VkDescriptorUpdateTemplateEntry));
        for (uint32_t i_iter = 0; i_iter < param_table->resources_count; i_iter++)
        {
            uint32_t i_binding = param_table->resources[i_iter].binding;
            VkDescriptorUpdateTemplateEntry* this_entry = template_entries + i_iter;
            this_entry->descriptorCount = param_table->resources[i_iter].size;
            this_entry->descriptorType = VkUtil_TranslateResourceType(param_table->resources[i_iter].type);
            this_entry->dstBinding = i_binding;
            this_entry->dstArrayElement = 0;
            this_entry->stride = sizeof(VkDescriptorUpdateData);
            this_entry->offset = this_entry->dstBinding * this_entry->stride;
        }
        if (update_entry_count > 0)
        {
            VkDescriptorUpdateTemplateCreateInfo template_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
                .pNext = NULL,
                .templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET_KHR,
                .pipelineLayout = RS->pPipelineLayout,
                .pipelineBindPoint = gPipelineBindPoint[RS->super.pipeline_type],
                .descriptorSetLayout = set_to_record->layout,
                .set = param_table->set_index,
                .pDescriptorUpdateEntries = template_entries,
                .descriptorUpdateEntryCount = update_entry_count
            };
            set_to_record->mUpdateEntriesCount = update_entry_count;
            CHECK_VKRESULT(D->mVkDeviceTable.vkCreateDescriptorUpdateTemplate(D->pVkDevice,
                &template_info, GLOBAL_VkAllocationCallbacks, &set_to_record->pUpdateTemplate));
        }
        cgpu_free(template_entries);
    }
    // [RS POOL] INSERTION
    if (desc->pool)
    {
        CGPURootSignatureId result = CGPUUtil_AddSignature(desc->pool, &RS->super, desc);
        cgpu_assert(result && "Root signature pool insertion failed!");
        return result;
    }
    // [RS POOL] END INSERTION
    return &RS->super;
}

void cgpu_free_root_signature_vulkan(CGPURootSignatureId signature)
{
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)signature;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)signature->device;
    // [RS POOL] FREE
    if (signature->pool)
    {
        CGPUUtil_PoolFreeSignature(signature->pool, signature);
        return;
    }
    // [RS POOL] END FREE
    // Free Reflection Data
    CGPUUtil_FreeRSParamTables((CGPURootSignature*)signature);
    // Free Vk Objects
    for (uint32_t i_set = 0; i_set < RS->mSetLayoutCount; i_set++)
    {
        SetLayout_Vulkan* set_to_free = &RS->pSetLayouts[i_set];
        if (set_to_free->layout != VK_NULL_HANDLE)
            D->mVkDeviceTable.vkDestroyDescriptorSetLayout(D->pVkDevice, set_to_free->layout, GLOBAL_VkAllocationCallbacks);
        if (set_to_free->pUpdateTemplate != VK_NULL_HANDLE)
            D->mVkDeviceTable.vkDestroyDescriptorUpdateTemplate(D->pVkDevice, set_to_free->pUpdateTemplate, GLOBAL_VkAllocationCallbacks);
    }
    cgpu_free(RS->pVkSetLayouts);
    cgpu_free(RS->pSetLayouts);
    cgpu_free(RS->pPushConstRanges);
    D->mVkDeviceTable.vkDestroyPipelineLayout(D->pVkDevice, RS->pPipelineLayout, GLOBAL_VkAllocationCallbacks);
    cgpu_free(RS);
}

CGPURootSignaturePoolId cgpu_create_root_signature_pool_vulkan(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc)
{
    return CGPUUtil_CreateRootSignaturePool(desc);
}

void cgpu_free_root_signature_pool_vulkan(CGPURootSignaturePoolId pool)
{
    CGPUUtil_FreeRootSignaturePool(pool);
}

CGPUDescriptorSetId cgpu_create_descriptor_set_vulkan(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc)
{
    size_t totalSize = sizeof(CGPUDescriptorSet_Vulkan);
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)desc->root_signature;
    uint32_t table_index = 0;
    for (uint32_t i = 0; i < RS->super.table_count; i++)
    {
        if (RS->super.tables[i].set_index == desc->set_index)
        {
            table_index = i;
        }
    }
    SetLayout_Vulkan* SetLayout = &RS->pSetLayouts[desc->set_index];
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    const size_t UpdateTemplateSize = RS->super.tables[table_index].resources_count * sizeof(VkDescriptorUpdateData);
    totalSize += UpdateTemplateSize;
    CGPUDescriptorSet_Vulkan* Set = cgpu_calloc_aligned(1, totalSize, _Alignof(CGPUDescriptorSet_Vulkan));
    char8_t* pMem = (char8_t*)(Set + 1);
    // Allocate Descriptor Set
    VkUtil_ConsumeDescriptorSets(D->pDescriptorPool, &SetLayout->layout, &Set->pVkDescriptorSet, 1);
    // Fill Update Template Data
    Set->pUpdateData = (VkDescriptorUpdateData*)pMem;
    memset(Set->pUpdateData, 0, UpdateTemplateSize);
    return &Set->super;
}

void cgpu_update_descriptor_set_vulkan(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count)
{
    CGPUDescriptorSet_Vulkan* Set = (CGPUDescriptorSet_Vulkan*)set;
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)set->root_signature;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)set->root_signature->device;
    uint32_t table_index = 0;
    for (uint32_t i = 0; i < RS->super.table_count; i++)
    {
        if (RS->super.tables[i].set_index == set->index)
        {
            table_index = i;
        }
    }
    SetLayout_Vulkan* SetLayout = &RS->pSetLayouts[set->index];
    const CGPUParameterTable* ParamTable = &RS->super.tables[table_index];
    VkDescriptorUpdateData* pUpdateData = Set->pUpdateData;
    memset(pUpdateData, 0, count * sizeof(VkDescriptorUpdateData));
    bool dirty = false;
    for (uint32_t i = 0; i < count; i++)
    {
        // Descriptor Info
        const CGPUDescriptorData* pParam = datas + i;
        const CGPUShaderResource* ResData = CGPU_NULLPTR;
        if (pParam->name != CGPU_NULLPTR)
        {
            size_t argNameHash = cgpu_name_hash(pParam->name, strlen(pParam->name));
            for (uint32_t p = 0; p < ParamTable->resources_count; p++)
            {
                if (ParamTable->resources[p].name_hash == argNameHash)
                {
                    ResData = ParamTable->resources + p;
                }
            }
        }
        else
        {
            for (uint32_t p = 0; p < ParamTable->resources_count; p++)
            {
                if (ParamTable->resources[p].binding == pParam->binding)
                {
                    ResData = ParamTable->resources + p;
                }
            }
        }
        // Update Info
        const uint32_t arrayCount = cgpu_max(1U, pParam->count);
        const ECGPUResourceType resourceType = (ECGPUResourceType)ResData->type;
        switch (resourceType)
        {
            case CGPU_RESOURCE_TYPE_RW_TEXTURE:
            case CGPU_RESOURCE_TYPE_TEXTURE: {
                cgpu_assert(pParam->textures && "cgpu_assert: Binding NULL texture(s)");
                CGPUTextureView_Vulkan** TextureViews = (CGPUTextureView_Vulkan**)pParam->textures;
                for (uint32_t arr = 0; arr < arrayCount; ++arr)
                {
                    // TODO: Stencil support
                    cgpu_assert(pParam->textures[arr] && "cgpu_assert: Binding NULL texture!");
                    VkDescriptorUpdateData* Data = &pUpdateData[ResData->binding + arr];
                    Data->mImageInfo.imageView =
                        ResData->type == CGPU_RESOURCE_TYPE_RW_TEXTURE ?
                        TextureViews[arr]->pVkUAVDescriptor :
                        TextureViews[arr]->pVkSRVDescriptor;
                    Data->mImageInfo.imageLayout =
                    ResData->type == CGPU_RESOURCE_TYPE_RW_TEXTURE ?
                        VK_IMAGE_LAYOUT_GENERAL :
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    Data->mImageInfo.sampler = VK_NULL_HANDLE;
                    dirty = true;
                }
                break;
            }
            case CGPU_RESOURCE_TYPE_SAMPLER: {
                cgpu_assert(pParam->samplers && "cgpu_assert: Binding NULL Sampler(s)");
                CGPUSampler_Vulkan** Samplers = (CGPUSampler_Vulkan**)pParam->samplers;
                for (uint32_t arr = 0; arr < arrayCount; ++arr)
                {
                    cgpu_assert(pParam->samplers[arr] && "cgpu_assert: Binding NULL Sampler!");
                    VkDescriptorUpdateData* Data = &pUpdateData[ResData->binding + arr];
                    Data->mImageInfo.sampler = Samplers[arr]->pVkSampler;
                    dirty = true;
                }
                break;
            }
            case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER:
            case CGPU_RESOURCE_TYPE_BUFFER:
            case CGPU_RESOURCE_TYPE_BUFFER_RAW:
            case CGPU_RESOURCE_TYPE_RW_BUFFER:
            case CGPU_RESOURCE_TYPE_RW_BUFFER_RAW: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                CGPUBuffer_Vulkan** Buffers = (CGPUBuffer_Vulkan**)pParam->buffers;
                for (uint32_t arr = 0; arr < arrayCount; ++arr)
                {
                    cgpu_assert(pParam->buffers[arr] && "cgpu_assert: Binding NULL Buffer!");
                    VkDescriptorUpdateData* Data = &pUpdateData[ResData->binding + arr];
                    Data->mBufferInfo.buffer = Buffers[arr]->pVkBuffer;
                    Data->mBufferInfo.offset = Buffers[arr]->mOffset;
                    Data->mBufferInfo.range = VK_WHOLE_SIZE;
                    if (pParam->buffers_params.offsets)
                    {
                        Data->mBufferInfo.offset = pParam->buffers_params.offsets[arr];
                        Data->mBufferInfo.range = pParam->buffers_params.sizes[arr];
                    }
                    dirty = true;
                }
                break;
            }
            default:
                assert(0 && ResData->type && "Descriptor Type not supported!");
                break;
        }
    }
    if (dirty)
    {
        D->mVkDeviceTable.vkUpdateDescriptorSetWithTemplateKHR(D->pVkDevice, Set->pVkDescriptorSet, SetLayout->pUpdateTemplate, Set->pUpdateData);
    }
}

void cgpu_free_descriptor_set_vulkan(CGPUDescriptorSetId set)
{
    CGPUDescriptorSet_Vulkan* Set = (CGPUDescriptorSet_Vulkan*)set;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)set->root_signature->device;
    VkUtil_ReturnDescriptorSets(D->pDescriptorPool, &Set->pVkDescriptorSet, 1);
    cgpu_free_aligned(Set, _Alignof(CGPUDescriptorSet_Vulkan));
}

CGPUComputePipelineId cgpu_create_compute_pipeline_vulkan(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUComputePipeline_Vulkan* PPL = (CGPUComputePipeline_Vulkan*)cgpu_calloc(1, sizeof(CGPUComputePipeline_Vulkan));
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)desc->root_signature;
    CGPUShaderLibrary_Vulkan* SL = (CGPUShaderLibrary_Vulkan*)desc->compute_shader->library;
    VkPipelineShaderStageCreateInfo cs_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = SL->mShaderModule,
        .pName = desc->compute_shader->entry,
        .pSpecializationInfo = NULL
    };
    VkComputePipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = cs_stage_info,
        .layout = RS->pPipelineLayout,
        .basePipelineHandle = 0,
        .basePipelineIndex = 0
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateComputePipelines(D->pVkDevice,
    D->pPipelineCache, 1, &pipeline_info, GLOBAL_VkAllocationCallbacks, &PPL->pVkPipeline));
    return &PPL->super;
}

void cgpu_free_compute_pipeline_vulkan(CGPUComputePipelineId pipeline)
{
    CGPUComputePipeline_Vulkan* PPL = (CGPUComputePipeline_Vulkan*)pipeline;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pipeline->device;
    D->mVkDeviceTable.vkDestroyPipeline(D->pVkDevice, PPL->pVkPipeline, GLOBAL_VkAllocationCallbacks);
    cgpu_free(PPL);
}

/* clang-format off */
static const char* kVkPSOMemoryPoolName = "cgpu::vk_pso";
CGPURenderPipelineId cgpu_create_render_pipeline_vulkan(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)device->adapter;
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)desc->root_signature;
    
    uint32_t input_binding_count = 0;
	uint32_t input_attribute_count = 0;
    VkUtil_GetVertexInputBindingAttrCount(desc->vertex_layout, &input_binding_count, &input_attribute_count);
    uint64_t dsize = sizeof(CGPURenderPipeline_Vulkan);
    const uint64_t input_elements_offset = dsize;
    dsize += (sizeof(VkVertexInputBindingDescription) * input_binding_count);
    const uint64_t input_attrs_offset = dsize;
    dsize += (sizeof(VkVertexInputAttributeDescription) * input_attribute_count);

    uint8_t* ptr = (uint8_t*)cgpu_callocN(1, dsize, kVkPSOMemoryPoolName);
    CGPURenderPipeline_Vulkan* RP = (CGPURenderPipeline_Vulkan*)ptr;
    VkVertexInputBindingDescription* input_bindings = (VkVertexInputBindingDescription*)(ptr + input_elements_offset);
    VkVertexInputAttributeDescription* input_attributes = (VkVertexInputAttributeDescription*)(ptr + input_attrs_offset);
    // Vertex input state
    if (desc->vertex_layout != NULL)
    {
        // Ignore everything that's beyond CGPU_MAX_VERTEX_ATTRIBS
        uint32_t attrib_count = desc->vertex_layout->attribute_count > CGPU_MAX_VERTEX_ATTRIBS ? CGPU_MAX_VERTEX_ATTRIBS : desc->vertex_layout->attribute_count;
        uint32_t attr_slot = 0;
        // Initial values
        for (uint32_t i = 0; i < attrib_count; ++i)
        {
            const CGPUVertexAttribute* attrib = &(desc->vertex_layout->attributes[i]);
            const uint32_t array_size = attrib->array_size ? attrib->array_size : 1;

            VkVertexInputBindingDescription* current_binding = &input_bindings[attrib->binding];
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

    // TODO: const spec
    const VkSpecializationInfo* specializationInfo = VK_NULL_HANDLE;

    VkPipelineVertexInputStateCreateInfo vi = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = input_binding_count,
		.pVertexBindingDescriptions = input_bindings,
		.vertexAttributeDescriptionCount = input_attribute_count,
		.pVertexAttributeDescriptions = input_attributes
    };
    // Shader stages
    DECLARE_ZERO(VkPipelineShaderStageCreateInfo, shaderStages[5])
    uint32_t stage_count = 0;
    for (uint32_t i = 0; i < 5; ++i)
    {
        ECGPUShaderStage stage_mask = (ECGPUShaderStage)(1 << i);
        shaderStages[stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[stage_count].pNext = NULL;
        shaderStages[stage_count].flags = 0;
        shaderStages[stage_count].pSpecializationInfo = specializationInfo;
        switch (stage_mask)
        {
            case CGPU_SHADER_STAGE_VERT:
            {
                if(desc->vertex_shader)
                {
                    shaderStages[stage_count].pName = desc->vertex_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStages[stage_count].module = ((CGPUShaderLibrary_Vulkan*)desc->vertex_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case CGPU_SHADER_STAGE_TESC:
            {
                if(desc->tesc_shader)
                {
                    shaderStages[stage_count].pName = desc->tesc_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    shaderStages[stage_count].module = ((CGPUShaderLibrary_Vulkan*)desc->tesc_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case CGPU_SHADER_STAGE_TESE:
            {
                if(desc->tese_shader)
                {
                    shaderStages[stage_count].pName = desc->tese_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    shaderStages[stage_count].module = ((CGPUShaderLibrary_Vulkan*)desc->tese_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case CGPU_SHADER_STAGE_GEOM:
            {
                if(desc->geom_shader)
                {
                    shaderStages[stage_count].pName = desc->geom_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                    shaderStages[stage_count].module = ((CGPUShaderLibrary_Vulkan*)desc->geom_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case CGPU_SHADER_STAGE_FRAG:
            {
                if(desc->fragment_shader)
                {
                    shaderStages[stage_count].pName = desc->fragment_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStages[stage_count].module = ((CGPUShaderLibrary_Vulkan*)desc->fragment_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            default: cgpu_assert(false && "Shader Stage not supported!"); break;
        }
    }
    // Viewport state
    VkPipelineViewportStateCreateInfo vps = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            // we are using dynamic viewports but we must set the count to 1
            .viewportCount = 1,
            .pViewports = NULL,
            .scissorCount = 1,
            .pScissors = NULL
    };
    uint32_t dyn_state_count = 0;
    VkUitl_QueryDynamicPipelineStates(A, &dyn_state_count, CGPU_NULLPTR);
    VkDynamicState* dyn_states = cgpu_callocN(dyn_state_count, sizeof(VkDynamicState), kVkPSOMemoryPoolName);
    VkUitl_QueryDynamicPipelineStates(A, &dyn_state_count, dyn_states);
    VkPipelineDynamicStateCreateInfo dys = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = dyn_state_count,
		.pDynamicStates = dyn_states
    };
    // Multi-sampling
    VkPipelineMultisampleStateCreateInfo ms = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.rasterizationSamples = VkUtil_SampleCountTranslateToVk(desc->sample_count),
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 0.0f,
		.pSampleMask = 0,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE
    };
    // IA stage
    VkPrimitiveTopology topology = VkUtil_TranslateTopology(desc->prim_topology);
    VkPipelineInputAssemblyStateCreateInfo ia = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = topology,
        .primitiveRestartEnable = VK_FALSE
    };
    // Depth stencil state
    VkPipelineDepthStencilStateCreateInfo dss = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        
        .depthTestEnable = desc->depth_state->depth_test ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = desc->depth_state->depth_write ? VK_TRUE : VK_FALSE,
        .depthCompareOp = gVkComparisonFuncTranslator[desc->depth_state->depth_func],
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = desc->depth_state->stencil_test ? VK_TRUE : VK_FALSE,

        .front.failOp = gVkStencilOpTranslator[desc->depth_state->stencil_front_fail],
        .front.passOp = gVkStencilOpTranslator[desc->depth_state->stencil_front_pass],
        .front.depthFailOp = gVkStencilOpTranslator[desc->depth_state->depth_front_fail],
        .front.compareOp = gVkComparisonFuncTranslator[desc->depth_state->stencil_front_func],
        .front.compareMask = desc->depth_state->stencil_read_mask,
        .front.writeMask = desc->depth_state->stencil_write_mask,
        .front.reference = 0,

        .back.failOp = gVkStencilOpTranslator[desc->depth_state->stencil_back_fail],
        .back.passOp = gVkStencilOpTranslator[desc->depth_state->stencil_back_pass],
        .back.depthFailOp = gVkStencilOpTranslator[desc->depth_state->depth_back_fail],
        .back.compareOp = gVkComparisonFuncTranslator[desc->depth_state->stencil_back_func],
        .back.compareMask = desc->depth_state->stencil_read_mask,
        .back.writeMask = desc->depth_state->stencil_write_mask,
        .back.reference = 0,

        .minDepthBounds = 0,
        .maxDepthBounds = 1
    };
    // Rasterizer state
    const float depth_bias = desc->rasterizer_state ? desc->rasterizer_state->depth_bias : 0.f;
    const VkCullModeFlagBits cullMode = !desc->rasterizer_state ? VK_CULL_MODE_BACK_BIT : gVkCullModeTranslator[desc->rasterizer_state->cull_mode];
    const VkPolygonMode polygonMode = !desc->rasterizer_state ? VK_POLYGON_MODE_FILL : gVkFillModeTranslator[desc->rasterizer_state->fill_mode];
    const VkFrontFace frontFace = !desc->rasterizer_state ? VK_FRONT_FACE_COUNTER_CLOCKWISE : gVkFrontFaceTranslator[desc->rasterizer_state->front_face];
    const float slope_scaled_depth_bias = desc->rasterizer_state ? desc->rasterizer_state->slope_scaled_depth_bias : 0.f;
    const VkBool32 enable_depth_clamp = desc->rasterizer_state ? 
        (desc->rasterizer_state->enable_depth_clamp ? VK_TRUE : VK_FALSE) :
        VK_FALSE;
    VkPipelineRasterizationStateCreateInfo rs = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthClampEnable = enable_depth_clamp,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = polygonMode,
        .cullMode = cullMode,
        .frontFace = frontFace,
        .depthBiasEnable = (depth_bias != 0) ? VK_TRUE : VK_FALSE,
        .depthBiasConstantFactor = depth_bias,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = slope_scaled_depth_bias,
        .lineWidth = 1.f
    };
    // Color blending state
    DECLARE_ZERO(VkPipelineColorBlendAttachmentState, cb_attachments[CGPU_MAX_MRT_COUNT])
	int blendDescIndex = 0;
    const CGPUBlendStateDescriptor* pDesc = desc->blend_state;
    for (int i = 0; i < CGPU_MAX_MRT_COUNT; ++i)
	{
        VkBool32 blendEnable =
            (gVkBlendConstantTranslator[pDesc->src_factors[blendDescIndex]] != VK_BLEND_FACTOR_ONE ||
                gVkBlendConstantTranslator[pDesc->dst_factors[blendDescIndex]] != VK_BLEND_FACTOR_ZERO ||
                gVkBlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]] != VK_BLEND_FACTOR_ONE ||
                gVkBlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]] != VK_BLEND_FACTOR_ZERO);

        cb_attachments[i].blendEnable = blendEnable;
        cb_attachments[i].colorWriteMask = pDesc->masks[blendDescIndex];
        cb_attachments[i].srcColorBlendFactor = gVkBlendConstantTranslator[pDesc->src_factors[blendDescIndex]];
        cb_attachments[i].dstColorBlendFactor = gVkBlendConstantTranslator[pDesc->dst_factors[blendDescIndex]];
        cb_attachments[i].colorBlendOp = gVkBlendOpTranslator[pDesc->blend_modes[blendDescIndex]];
        cb_attachments[i].srcAlphaBlendFactor = gVkBlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]];
        cb_attachments[i].dstAlphaBlendFactor = gVkBlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]];
        cb_attachments[i].alphaBlendOp = gVkBlendOpTranslator[pDesc->blend_alpha_modes[blendDescIndex]];

		if (desc->blend_state->independent_blend)
			++blendDescIndex;
	}
    VkPipelineColorBlendStateCreateInfo cbs = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = desc->render_target_count,
        .pAttachments = cb_attachments,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f
    };
    // Create a stub render pass
    VkRenderPass render_pass = VK_NULL_HANDLE;
    cgpu_assert(desc->render_target_count >= 0);
    VkUtil_RenderPassDesc rp_desc = {
        .mColorAttachmentCount = desc->render_target_count,
        .mSampleCount = desc->sample_count,
        .mDepthStencilFormat = desc->depth_stencil_format
    };
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        rp_desc.pColorFormats[i] = desc->color_formats[i];
        if(desc->color_resolve_disable_mask & (CGPU_SLOT_0 << i))
        {
            rp_desc.pResolveMasks[i] = false;
        }
        else if(rp_desc.mSampleCount != CGPU_SAMPLE_COUNT_1)
        {
            rp_desc.pResolveMasks[i] = true;
        }
    }
    VkUtil_FindOrCreateRenderPass(D, &rp_desc, &render_pass);
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = stage_count,
        .pStages = shaderStages,
        .pVertexInputState = &vi,
        .pInputAssemblyState = &ia,
        .pDynamicState = &dys,
        .pViewportState = &vps,
        .pRasterizationState = &rs,
        .pMultisampleState = &ms,
        .pDepthStencilState = &dss,
        .pColorBlendState = &cbs,
        .layout = RS->pPipelineLayout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };
    VkResult createResult = D->mVkDeviceTable.vkCreateGraphicsPipelines(D->pVkDevice,
        D->pPipelineCache, 1, &pipelineInfo, GLOBAL_VkAllocationCallbacks, &RP->pVkPipeline);
    cgpu_freeN(dyn_states, kVkPSOMemoryPoolName);
    if (createResult != VK_SUCCESS)
    {
        cgpu_fatal("CGPU VULKAN: Failed to create Graphics Pipeline! Error Code: %d", createResult);
    }
    return &RP->super;
}
/* clang-format on */

void cgpu_free_render_pipeline_vulkan(CGPURenderPipelineId pipeline)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pipeline->device;
    CGPURenderPipeline_Vulkan* RP = (CGPURenderPipeline_Vulkan*)pipeline;
    D->mVkDeviceTable.vkDestroyPipeline(D->pVkDevice, RP->pVkPipeline, GLOBAL_VkAllocationCallbacks);
    cgpu_freeN(RP, kVkPSOMemoryPoolName);
}

VkQueryType VkUtil_ToVkQueryType(ECGPUQueryType type)
{
    switch (type)
    {
        case CGPU_QUERY_TYPE_TIMESTAMP:
            return VK_QUERY_TYPE_TIMESTAMP;
        case CGPU_QUERY_TYPE_PIPELINE_STATISTICS:
            return VK_QUERY_TYPE_PIPELINE_STATISTICS;
        case CGPU_QUERY_TYPE_OCCLUSION:
            return VK_QUERY_TYPE_OCCLUSION;
        default:
            cgpu_assert(false && "Invalid query heap type");
            return VK_QUERY_TYPE_MAX_ENUM;
    }
}
CGPUQueryPoolId cgpu_create_query_pool_vulkan(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUQueryPool_Vulkan* P = (CGPUQueryPool_Vulkan*)cgpu_calloc(1, sizeof(CGPUQueryPool_Vulkan));
    P->mType = VkUtil_ToVkQueryType(desc->type);
    P->super.count = desc->query_count;
    DECLARE_ZERO(VkQueryPoolCreateInfo, createInfo)
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.queryCount = desc->query_count;
    createInfo.queryType = P->mType;
    createInfo.flags = 0;
    createInfo.pipelineStatistics = 0;
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateQueryPool(
    D->pVkDevice, &createInfo, GLOBAL_VkAllocationCallbacks, &P->pVkQueryPool));
    return &P->super;
}

void cgpu_free_query_pool_vulkan(CGPUQueryPoolId pool)
{
    CGPUQueryPool_Vulkan* P = (CGPUQueryPool_Vulkan*)pool;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pool->device;
    D->mVkDeviceTable.vkDestroyQueryPool(
    D->pVkDevice, P->pVkQueryPool, GLOBAL_VkAllocationCallbacks);
    cgpu_free(P);
}

CGPUMemoryPoolId cgpu_create_memory_pool_vulkan(CGPUDeviceId device, const struct CGPUMemoryPoolDescriptor* desc)
{
    VmaPool vmaPool;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    VmaPoolCreateInfo poolInfo = {
        .minBlockCount = desc->min_block_count,
        .maxBlockCount = desc->max_block_count,
        .minAllocationAlignment = desc->min_alloc_alignment,
        .blockSize = desc->block_size,
        .memoryTypeIndex = 0,
        .flags = 0
    };
    vmaCreatePool(D->pVmaAllocator, &poolInfo, &vmaPool);
    return NULL;
}

void cgpu_free_memory_pool_vulkan(CGPUMemoryPoolId pool)
{
}

// Queue APIs
CGPUQueueId cgpu_get_queue_vulkan(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    cgpu_assert(device && "CGPU VULKAN: NULL DEVICE!");
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)device->adapter;

    CGPUQueue_Vulkan Q = {
        .super = {
        .device = device,
        .index = index,
        .type = type }
    };
    D->mVkDeviceTable.vkGetDeviceQueue(D->pVkDevice, (uint32_t)A->mQueueFamilyIndices[type], index, &Q.pVkQueue);
    Q.mVkQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[type];

    CGPUQueue_Vulkan* RQ = (CGPUQueue_Vulkan*)cgpu_calloc(1, sizeof(CGPUQueue_Vulkan));
    memcpy(RQ, &Q, sizeof(Q));
    CGPUCommandPoolDescriptor pool_desc = {
        .name = "InternalCmdPool"
    };
    RQ->pInnerCmdPool = cgpu_create_command_pool(&RQ->super, &pool_desc);
    CGPUCommandBufferDescriptor cmd_desc = {
        .is_secondary = false
    };
    RQ->pInnerCmdBuffer = cgpu_create_command_buffer(RQ->pInnerCmdPool, &cmd_desc);
    RQ->pInnerFence = cgpu_create_fence(device);
#ifdef CGPU_THREAD_SAFETY
    RQ->pMutex = (SMutex*)cgpu_calloc(1, sizeof(SMutex));
    skr_init_mutex_recursive(RQ->pMutex);
#endif
    return &RQ->super;
}

void cgpu_submit_queue_vulkan(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc)
{
    uint32_t CmdCount = desc->cmds_count;
    CGPUCommandBuffer_Vulkan** Cmds = (CGPUCommandBuffer_Vulkan**)desc->cmds;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)queue;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)queue->device;
    CGPUFence_Vulkan* F = (CGPUFence_Vulkan*)desc->signal_fence;

    // cgpu_assert that given cmd list and given params are valid
    cgpu_assert(CmdCount > 0);
    cgpu_assert(Cmds);
    // execute given command list
    cgpu_assert(Q->pVkQueue != VK_NULL_HANDLE);

    DECLARE_ZERO_VLA(VkCommandBuffer, vkCmds, CmdCount);
    for (uint32_t i = 0; i < CmdCount; ++i)
    {
        vkCmds[i] = Cmds[i]->pVkCmdBuf;
    }
    // Set wait semaphores
    DECLARE_ZERO_VLA(VkSemaphore, wait_semaphores, desc->wait_semaphore_count + 1)
    DECLARE_ZERO_VLA(VkPipelineStageFlags, wait_stages, desc->wait_semaphore_count + 1)
    uint32_t waitCount = 0;
    CGPUSemaphore_Vulkan** WaitSemaphores = (CGPUSemaphore_Vulkan**)desc->wait_semaphores;
    for (uint32_t i = 0; i < desc->wait_semaphore_count; ++i)
    {
        if (WaitSemaphores[i]->mSignaled)
        {
            wait_semaphores[waitCount] = WaitSemaphores[i]->pVkSemaphore;
            wait_stages[waitCount] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;            
            WaitSemaphores[i]->mSignaled = false;
            ++waitCount;
        }
    }
    // Set signal semaphores
    DECLARE_ZERO_VLA(VkSemaphore, signal_semaphores, desc->signal_semaphore_count + 1)
    uint32_t signalCount = 0;
    CGPUSemaphore_Vulkan** SignalSemaphores = (CGPUSemaphore_Vulkan**)desc->signal_semaphores;
    for (uint32_t i = 0; i < desc->signal_semaphore_count; ++i)
    {
        if (!SignalSemaphores[i]->mSignaled)
        {
            signal_semaphores[signalCount] = SignalSemaphores[i]->pVkSemaphore;
            SignalSemaphores[i]->mSignaled = true;
            ++signalCount;
        }
    }
    // Submit
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .pWaitDstStageMask = 0,
        .commandBufferCount = CmdCount,
        .pCommandBuffers = vkCmds,
        .waitSemaphoreCount = waitCount,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .signalSemaphoreCount = signalCount,
        .pSignalSemaphores = signal_semaphores,
    };
#ifdef CGPU_THREAD_SAFETY
    if (Q->pMutex) skr_mutex_acquire(Q->pMutex);
#endif
    VkResult res = D->mVkDeviceTable.vkQueueSubmit(Q->pVkQueue, 1, &submit_info, F ? F->pVkFence : VK_NULL_HANDLE);
    if(res != VK_SUCCESS)
    {
        SKR_LOG_FATAL(u8"CGPU VULKAN: Failed to submit queue! Error code: %d", res);
        if (res == VK_ERROR_DEVICE_LOST)
        {
            ((CGPUDevice*)queue->device)->is_lost = true;
        }
        else
        {
            SKR_ASSERT("Unhandled VK ERROR!");
        }
    };
    if (F) F->mSubmitted = true;
#ifdef CGPU_THREAD_SAFETY
    if (Q->pMutex) skr_mutex_release(Q->pMutex);
#endif
}

void cgpu_wait_queue_idle_vulkan(CGPUQueueId queue)
{
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)queue;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)queue->device;
    D->mVkDeviceTable.vkQueueWaitIdle(Q->pVkQueue);
}

void cgpu_queue_present_vulkan(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc)
{
    CGPUSwapChain_Vulkan* SC = (CGPUSwapChain_Vulkan*)desc->swapchain;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)queue->device;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)queue;
    if (SC)
    {
        // Set semaphores
        DECLARE_ZERO_VLA(VkSemaphore, wait_semaphores, desc->wait_semaphore_count + 1)
        uint32_t waitCount = 0;
        CGPUSemaphore_Vulkan** Semaphores = (CGPUSemaphore_Vulkan**)desc->wait_semaphores;
        for (uint32_t i = 0; i < desc->wait_semaphore_count; ++i)
        {
            if (Semaphores[i]->mSignaled)
            {
                wait_semaphores[waitCount] = Semaphores[i]->pVkSemaphore;
                Semaphores[i]->mSignaled = false;
                ++waitCount;
            }
        }
        // Present
        uint32_t presentIndex = desc->index;
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = VK_NULL_HANDLE,
            .waitSemaphoreCount = waitCount,
            .pWaitSemaphores = wait_semaphores,
            .swapchainCount = 1,
            .pSwapchains = &SC->pVkSwapChain,
            .pImageIndices = &presentIndex,
            .pResults = VK_NULL_HANDLE
        };
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_mutex_acquire(Q->pMutex);
#endif
        VkResult vk_res = D->mVkDeviceTable.vkQueuePresentKHR(Q->pVkQueue, &present_info);
#ifdef CGPU_THREAD_SAFETY
        if (Q->pMutex) skr_mutex_release(Q->pMutex);
#endif
        if (vk_res != VK_SUCCESS && vk_res != VK_SUBOPTIMAL_KHR &&
            vk_res != VK_ERROR_OUT_OF_DATE_KHR)
        {
            cgpu_assert(0 && "Present failed!");
        }
    }
}

float cgpu_queue_get_timestamp_period_ns_vulkan(CGPUQueueId queue)
{
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)queue->device->adapter;
    return A->mPhysicalDeviceProps.properties.limits.timestampPeriod;
}

void cgpu_free_queue_vulkan(CGPUQueueId queue)
{
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)queue;
    if (Q->pInnerCmdBuffer) cgpu_free_command_buffer(Q->pInnerCmdBuffer);
    if (Q->pInnerCmdPool) cgpu_free_command_pool(Q->pInnerCmdPool);
    if (Q->pInnerFence) cgpu_free_fence(Q->pInnerFence);
#ifdef CGPU_THREAD_SAFETY
    if (Q->pMutex)
    {
        skr_destroy_mutex(Q->pMutex);
        cgpu_free(Q->pMutex);
    }
#endif
    cgpu_free((void*)queue);
}

VkCommandPool allocate_transient_command_pool(CGPUDevice_Vulkan* D, CGPUQueueId queue)
{
    VkCommandPool P = VK_NULL_HANDLE;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)queue->device->adapter;

    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        // transient.
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[queue->type]
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateCommandPool(
        D->pVkDevice, &create_info, GLOBAL_VkAllocationCallbacks, &P));
        
    return P;
}

void free_transient_command_pool(CGPUDevice_Vulkan* D, VkCommandPool pool)
{
    D->mVkDeviceTable.vkDestroyCommandPool(D->pVkDevice, pool, GLOBAL_VkAllocationCallbacks);
}

CGPUCommandPoolId cgpu_create_command_pool_vulkan(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)queue->device;
    CGPUCommandPool_Vulkan* P = (CGPUCommandPool_Vulkan*)cgpu_calloc(1, sizeof(CGPUCommandPool_Vulkan));
    P->pVkCmdPool = allocate_transient_command_pool(D, queue);
    VkUtil_OptionalSetObjectName(D, (uint64_t)P->pVkCmdPool, VK_OBJECT_TYPE_COMMAND_POOL, desc ? desc->name : CGPU_NULLPTR);
    return &P->super;
}

CGPUCommandBufferId cgpu_create_command_buffer_vulkan(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc)
{
    cgpu_assert(pool);
    CGPUCommandPool_Vulkan* P = (CGPUCommandPool_Vulkan*)pool;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)P->super.queue;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Q->super.device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cgpu_calloc_aligned(
    1, sizeof(CGPUCommandBuffer_Vulkan), _Alignof(CGPUCommandBuffer_Vulkan));
    cgpu_assert(Cmd);

    Cmd->mType = Q->super.type;
    Cmd->mNodeIndex = CGPU_SINGLE_GPU_NODE_MASK;

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = P->pVkCmdPool,
        .level = desc->is_secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkAllocateCommandBuffers(D->pVkDevice, &alloc_info, &(Cmd->pVkCmdBuf)));
    return &Cmd->super;
}

void cgpu_reset_command_pool_vulkan(CGPUCommandPoolId pool)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pool->queue->device;
    CGPUCommandPool_Vulkan* P = (CGPUCommandPool_Vulkan*)pool;
    CHECK_VKRESULT(D->mVkDeviceTable.vkResetCommandPool(D->pVkDevice, P->pVkCmdPool, 0));
}

void cgpu_free_command_buffer_vulkan(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUCommandPool_Vulkan* P = (CGPUCommandPool_Vulkan*)cmd->pool;
    CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)P->super.queue;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Q->super.device;
    D->mVkDeviceTable.vkFreeCommandBuffers(D->pVkDevice, P->pVkCmdPool, 1, &(Cmd->pVkCmdBuf));
    cgpu_free_aligned(Cmd, _Alignof(CGPUCommandBuffer_Vulkan));
}

void cgpu_free_command_pool_vulkan(CGPUCommandPoolId pool)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pool->queue->device;
    CGPUCommandPool_Vulkan* P = (CGPUCommandPool_Vulkan*)pool;
    free_transient_command_pool(D, P->pVkCmdPool);
    cgpu_free(P);
}

// CMDs
void cgpu_cmd_begin_vulkan(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkBeginCommandBuffer(Cmd->pVkCmdBuf, &begin_info));
    Cmd->pBoundPipelineLayout = CGPU_NULL;
}

void cgpu_cmd_resource_barrier_vulkan(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)cmd->device->adapter;
    VkAccessFlags srcAccessFlags = 0;
    VkAccessFlags dstAccessFlags = 0;

    DECLARE_ZERO_VLA(VkBufferMemoryBarrier, BBs, desc->buffer_barriers_count)
    uint32_t bufferBarrierCount = 0;
    for (uint32_t i = 0; i < desc->buffer_barriers_count; i++)
    {
        const CGPUBufferBarrier* buffer_barrier = &desc->buffer_barriers[i];
        CGPUBuffer_Vulkan* B = (CGPUBuffer_Vulkan*)buffer_barrier->buffer;
        VkBufferMemoryBarrier* pBufferBarrier = NULL;

        if (CGPU_RESOURCE_STATE_UNORDERED_ACCESS == buffer_barrier->src_state &&
            CGPU_RESOURCE_STATE_UNORDERED_ACCESS == buffer_barrier->dst_state)
        {
            pBufferBarrier = &BBs[bufferBarrierCount++];                     //-V522
            pBufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER; //-V522
            pBufferBarrier->pNext = NULL;

            pBufferBarrier->srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            pBufferBarrier->dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        }
        else
        {
            pBufferBarrier = &BBs[bufferBarrierCount++];
            pBufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            pBufferBarrier->pNext = NULL;

            pBufferBarrier->srcAccessMask = VkUtil_ResourceStateToVkAccessFlags(buffer_barrier->src_state);
            pBufferBarrier->dstAccessMask = VkUtil_ResourceStateToVkAccessFlags(buffer_barrier->dst_state);
        }

        if (pBufferBarrier)
        {
            pBufferBarrier->buffer = B->pVkBuffer;
            pBufferBarrier->size = VK_WHOLE_SIZE;
            pBufferBarrier->offset = 0;

            if (buffer_barrier->queue_acquire)
            {
                pBufferBarrier->dstQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[Cmd->mType];
                pBufferBarrier->srcQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[buffer_barrier->queue_type];
            }
            else if (buffer_barrier->queue_release)
            {
                pBufferBarrier->srcQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[Cmd->mType];
                pBufferBarrier->dstQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[buffer_barrier->queue_type];
            }
            else
            {
                pBufferBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                pBufferBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }
            srcAccessFlags |= pBufferBarrier->srcAccessMask;
            dstAccessFlags |= pBufferBarrier->dstAccessMask;
        }
    }

    DECLARE_ZERO_VLA(VkImageMemoryBarrier, TBs, desc->texture_barriers_count)
    uint32_t imageBarrierCount = 0;
    for (uint32_t i = 0; i < desc->texture_barriers_count; i++)
    {
        const CGPUTextureBarrier* texture_barrier = &desc->texture_barriers[i];
        CGPUTexture_Vulkan* T = (CGPUTexture_Vulkan*)texture_barrier->texture;
        const CGPUTextureInfo* pInfo = T->super.info;
        VkImageMemoryBarrier* pImageBarrier = NULL;
        if (CGPU_RESOURCE_STATE_UNORDERED_ACCESS == texture_barrier->src_state &&
            CGPU_RESOURCE_STATE_UNORDERED_ACCESS == texture_barrier->dst_state)
        {
            pImageBarrier = &TBs[imageBarrierCount++];
            pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            pImageBarrier->pNext = NULL;

            pImageBarrier->srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            pImageBarrier->dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
            pImageBarrier->oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            pImageBarrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else
        {
            pImageBarrier = &TBs[imageBarrierCount++];
            pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            pImageBarrier->pNext = NULL;

            pImageBarrier->srcAccessMask = VkUtil_ResourceStateToVkAccessFlags(texture_barrier->src_state);
            pImageBarrier->dstAccessMask = VkUtil_ResourceStateToVkAccessFlags(texture_barrier->dst_state);
            pImageBarrier->oldLayout = VkUtil_ResourceStateToImageLayout(texture_barrier->src_state);
            pImageBarrier->newLayout = VkUtil_ResourceStateToImageLayout(texture_barrier->dst_state);
        }

        if (pImageBarrier)
        {
            pImageBarrier->image = T->pVkImage;
            pImageBarrier->subresourceRange.aspectMask = (VkImageAspectFlags)pInfo->aspect_mask;
            pImageBarrier->subresourceRange.baseMipLevel = texture_barrier->subresource_barrier ? texture_barrier->mip_level : 0;
            pImageBarrier->subresourceRange.levelCount = texture_barrier->subresource_barrier ? 1 : VK_REMAINING_MIP_LEVELS;
            pImageBarrier->subresourceRange.baseArrayLayer = texture_barrier->subresource_barrier ? texture_barrier->array_layer : 0;
            pImageBarrier->subresourceRange.layerCount = texture_barrier->subresource_barrier ? 1 : VK_REMAINING_ARRAY_LAYERS;

            if (texture_barrier->queue_acquire &&
                texture_barrier->src_state != CGPU_RESOURCE_STATE_UNDEFINED)
            {
                pImageBarrier->dstQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[Cmd->mType];
                pImageBarrier->srcQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[texture_barrier->queue_type];
            }
            else if (texture_barrier->queue_release &&
                     texture_barrier->src_state != CGPU_RESOURCE_STATE_UNDEFINED)
            {
                pImageBarrier->srcQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[Cmd->mType];
                pImageBarrier->dstQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[texture_barrier->queue_type];
            }
            else
            {
                pImageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                pImageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }

            srcAccessFlags |= pImageBarrier->srcAccessMask;
            dstAccessFlags |= pImageBarrier->dstAccessMask;
        }
    }

    // Commit barriers
    VkPipelineStageFlags srcStageMask =
    VkUtil_DeterminePipelineStageFlags(A, srcAccessFlags, (ECGPUQueueType)Cmd->mType);
    VkPipelineStageFlags dstStageMask =
    VkUtil_DeterminePipelineStageFlags(A, dstAccessFlags, (ECGPUQueueType)Cmd->mType);
    if (bufferBarrierCount || imageBarrierCount)
    {
        D->mVkDeviceTable.vkCmdPipelineBarrier(Cmd->pVkCmdBuf,
            srcStageMask, dstStageMask, 0,
            0, NULL,
            bufferBarrierCount, BBs,
            imageBarrierCount, TBs);
    }
}

VkPipelineStageFlagBits VkUtil_ShaderStagesToPipelineStage(ECGPUShaderStage stage)
{
    if (stage == CGPU_SHADER_STAGE_ALL_GRAPHICS) return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    if (stage == CGPU_SHADER_STAGE_NONE) return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    if (stage == CGPU_SHADER_STAGE_VERT) return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if (stage == CGPU_SHADER_STAGE_TESC) return VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    if (stage == CGPU_SHADER_STAGE_TESE) return VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    if (stage == CGPU_SHADER_STAGE_GEOM) return VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    if (stage == CGPU_SHADER_STAGE_FRAG) return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (stage == CGPU_SHADER_STAGE_COMPUTE) return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    if (stage == CGPU_SHADER_STAGE_RAYTRACING) return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

void cgpu_cmd_begin_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUQueryPool_Vulkan* P = (CGPUQueryPool_Vulkan*)pool;
    switch (P->mType)
    {
        case VK_QUERY_TYPE_TIMESTAMP:
            D->mVkDeviceTable.vkCmdWriteTimestamp(
            Cmd->pVkCmdBuf,
            VkUtil_ShaderStagesToPipelineStage(desc->stage),
            P->pVkQueryPool, desc->index);
            break;
        case VK_QUERY_TYPE_PIPELINE_STATISTICS:
            break;
        case VK_QUERY_TYPE_OCCLUSION:
            break;
        default:
            break;
    }
}

void cgpu_cmd_reset_query_pool_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, uint32_t start_query, uint32_t query_count)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pool->device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUQueryPool_Vulkan* P = (CGPUQueryPool_Vulkan*)pool;
    D->mVkDeviceTable.vkCmdResetQueryPool(Cmd->pVkCmdBuf, P->pVkQueryPool, start_query, query_count);
}

void cgpu_cmd_end_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    cgpu_cmd_begin_query(cmd, pool, desc);
}

void cgpu_cmd_resolve_query_vulkan(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count)
{
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pool->device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUQueryPool_Vulkan* P = (CGPUQueryPool_Vulkan*)pool;
    CGPUBuffer_Vulkan* B = (CGPUBuffer_Vulkan*)readback;
    VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT;
#ifdef ANDROID
    flags |= VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
#else
    flags |= VK_QUERY_RESULT_WAIT_BIT;
#endif
    D->mVkDeviceTable.vkCmdCopyQueryPoolResults(
    Cmd->pVkCmdBuf, P->pVkQueryPool,
    start_query, query_count, B->pVkBuffer, 0,
    sizeof(uint64_t), flags);
}

void cgpu_cmd_end_vulkan(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CHECK_VKRESULT(D->mVkDeviceTable.vkEndCommandBuffer(Cmd->pVkCmdBuf));
}

// Events & Markser
void cgpu_cmd_begin_event_vulkan(CGPUCommandBufferId cmd, const CGPUEventInfo* event)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)cmd->device->adapter->instance;
    if (I->debug_report)
    {
        DECLARE_ZERO(VkDebugMarkerMarkerInfoEXT, markerInfo)
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        markerInfo.pMarkerName = event->name;
        markerInfo.color[0] = event->color[0];
        markerInfo.color[1] = event->color[1];
        markerInfo.color[2] = event->color[2];
        markerInfo.color[3] = event->color[3];
        D->mVkDeviceTable.vkCmdDebugMarkerBeginEXT(Cmd->pVkCmdBuf, &markerInfo);
    }
    if (I->debug_utils)
    {
        DECLARE_ZERO(VkDebugUtilsLabelEXT, markerInfo)
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        markerInfo.pLabelName = event->name;
        markerInfo.color[0] = event->color[0];
        markerInfo.color[1] = event->color[1];
        markerInfo.color[2] = event->color[2];
        markerInfo.color[3] = event->color[3];
        vkCmdBeginDebugUtilsLabelEXT(Cmd->pVkCmdBuf, &markerInfo);
    }
}

void cgpu_cmd_set_marker_vulkan(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)cmd->device->adapter->instance;
    if (I->debug_utils)
    {
        DECLARE_ZERO(VkDebugUtilsLabelEXT, label)
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = marker->name;
        label.color[0] = marker->color[0];
        label.color[1] = marker->color[1];
        label.color[2] = marker->color[2];
        label.color[3] = marker->color[3];
        vkCmdInsertDebugUtilsLabelEXT(Cmd->pVkCmdBuf, &label);
    }
    if (I->debug_report)
    {
        DECLARE_ZERO(VkDebugMarkerMarkerInfoEXT, info)
        info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        info.pMarkerName = marker->name;
        info.color[0] = marker->color[0];
        info.color[1] = marker->color[1];
        info.color[2] = marker->color[2];
        info.color[3] = marker->color[3];
        D->mVkDeviceTable.vkCmdDebugMarkerInsertEXT(Cmd->pVkCmdBuf, &info);
    }
}

void cgpu_cmd_end_event_vulkan(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)cmd->device->adapter->instance;
    if (I->debug_report)
        D->mVkDeviceTable.vkCmdDebugMarkerEndEXT(Cmd->pVkCmdBuf);
    if (I->debug_utils)
        vkCmdEndDebugUtilsLabelEXT(Cmd->pVkCmdBuf);
}

// Compute CMDs
CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_vulkan(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc)
{
    // DO NOTHING NOW
    return (CGPUComputePassEncoderId)cmd;
}

void cgpu_compute_encoder_bind_descriptor_set_vulkan(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    const CGPUDescriptorSet_Vulkan* Set = (CGPUDescriptorSet_Vulkan*)set;
    const CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)set->root_signature;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)set->root_signature->device;

    // VK Must Fill All DescriptorSetLayouts at first dispach/draw.
    // Example: If shader uses only set 2, we still have to bind empty sets for set=0 and set=1
    if (Cmd->pBoundPipelineLayout != RS->pPipelineLayout)
    {
        Cmd->pBoundPipelineLayout = RS->pPipelineLayout;
        for (uint32_t i = 0; i < RS->mSetLayoutCount; i++)
        {
            if (RS->pSetLayouts[i].pEmptyDescSet != VK_NULL_HANDLE &&
                Set->super.index != i)
            {
                D->mVkDeviceTable.vkCmdBindDescriptorSets(Cmd->pVkCmdBuf,
                VK_PIPELINE_BIND_POINT_COMPUTE, RS->pPipelineLayout, i,
                1, &RS->pSetLayouts[i].pEmptyDescSet, 0, NULL);
            }
        }
    }
    D->mVkDeviceTable.vkCmdBindDescriptorSets(Cmd->pVkCmdBuf,
    VK_PIPELINE_BIND_POINT_COMPUTE, RS->pPipelineLayout,
    Set->super.index, 1, &Set->pVkDescriptorSet,
    // TODO: Dynamic Offset
    0, NULL);
}

void cgpu_render_encoder_bind_descriptor_set_vulkan(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    const CGPUDescriptorSet_Vulkan* Set = (CGPUDescriptorSet_Vulkan*)set;
    const CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)set->root_signature;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)set->root_signature->device;

    // VK Must Fill All DescriptorSetLayouts at first dispach/draw.
    // Example: If shader uses only set 2, we still have to bind empty sets for set=0 and set=1
    if (Cmd->pBoundPipelineLayout != RS->pPipelineLayout)
    {
        Cmd->pBoundPipelineLayout = RS->pPipelineLayout;
        for (uint32_t i = 0; i < RS->mSetLayoutCount; i++)
        {
            if (RS->pSetLayouts[i].pEmptyDescSet != VK_NULL_HANDLE &&
                Set->super.index != i)
            {
                D->mVkDeviceTable.vkCmdBindDescriptorSets(Cmd->pVkCmdBuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS, RS->pPipelineLayout, i,
                1, &RS->pSetLayouts[i].pEmptyDescSet, 0, NULL);
            }
        }
    }
    D->mVkDeviceTable.vkCmdBindDescriptorSets(Cmd->pVkCmdBuf,
    VK_PIPELINE_BIND_POINT_GRAPHICS, RS->pPipelineLayout,
    Set->super.index, 1, &Set->pVkDescriptorSet,
    // TODO: Dynamic Offset
    0, NULL);
}

void cgpu_compute_encoder_push_constants_vulkan(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)rs;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)rs->device;
    D->mVkDeviceTable.vkCmdPushConstants(Cmd->pVkCmdBuf, RS->pPipelineLayout,
    RS->pPushConstRanges[0].stageFlags, 0,
    RS->pPushConstRanges[0].size, data);
}

void cgpu_compute_encoder_bind_pipeline_vulkan(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    CGPUComputePipeline_Vulkan* PPL = (CGPUComputePipeline_Vulkan*)pipeline;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pipeline->device;
    D->mVkDeviceTable.vkCmdBindPipeline(Cmd->pVkCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, PPL->pVkPipeline);
}

void cgpu_compute_encoder_dispatch_vulkan(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Cmd->super.device;
    D->mVkDeviceTable.vkCmdDispatch(Cmd->pVkCmdBuf, X, Y, Z);
}

void cgpu_cmd_end_compute_pass_vulkan(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder)
{
    // DO NOTHING NOW
}

// Render CMDs
CGPURenderPassEncoderId cgpu_cmd_begin_render_pass_vulkan(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    // Find or create render pass
    uint32_t Width, Height;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    {
        VkUtil_RenderPassDesc rpdesc = {
            .mColorAttachmentCount = desc->render_target_count,
            .mDepthStencilFormat =
                desc->depth_stencil ?
                (desc->depth_stencil->view ? desc->depth_stencil->view->info.format : CGPU_FORMAT_UNDEFINED) :
                CGPU_FORMAT_UNDEFINED,
            .mSampleCount = desc->sample_count,
            .mLoadActionDepth =
                desc->depth_stencil ? desc->depth_stencil->depth_load_action : CGPU_LOAD_ACTION_DONTCARE,
            .mStoreActionDepth =
                desc->depth_stencil ? desc->depth_stencil->depth_store_action : CGPU_STORE_ACTION_STORE,
            .mLoadActionStencil =
                desc->depth_stencil ? desc->depth_stencil->stencil_load_action : CGPU_LOAD_ACTION_DONTCARE,
            .mStoreActionStencil =
                desc->depth_stencil ? desc->depth_stencil->stencil_store_action : CGPU_STORE_ACTION_STORE
        };
        for (uint32_t i = 0; i < desc->render_target_count; i++)
        {
            CGPUTextureId tex = desc->color_attachments[i].view->info.texture;
            const CGPUTextureInfo* info = tex->info;

            rpdesc.pResolveMasks[i] = (desc->sample_count != CGPU_SAMPLE_COUNT_1) &&
                                      (desc->color_attachments[i].resolve_view != NULL);
            rpdesc.pColorFormats[i] = desc->color_attachments[i].view->info.format;
            rpdesc.pLoadActionsColor[i] = desc->color_attachments[i].load_action;
            rpdesc.pStoreActionsColor[i] = desc->color_attachments[i].store_action;
            Width = (uint32_t)info->width;
            Height = (uint32_t)info->height;
        }
        VkUtil_FindOrCreateRenderPass(D, &rpdesc, &render_pass);
    }
    // Find or create framebuffer
    VkFramebuffer pFramebuffer = VK_NULL_HANDLE;
    {
        VkUtil_FramebufferDesc fbDesc = {
            .pRenderPass = render_pass,
            .mAttachmentCount = 0,
            .mWidth = Width,
            .mHeight = Height,
            .mLayers = 1
        };
        uint32_t idx = 0;
        for (uint32_t i = 0; i < desc->render_target_count; i++)
        {
            CGPUTextureView_Vulkan* TVV = (CGPUTextureView_Vulkan*)desc->color_attachments[i].view;
            fbDesc.pImageViews[idx] = TVV->pVkRTVDSVDescriptor;
            fbDesc.mLayers = TVV->super.info.array_layer_count;
            fbDesc.mAttachmentCount += 1;
            idx++;
        }
        for (uint32_t i = 0; i < desc->render_target_count; i++)
        {
            CGPUTextureView_Vulkan* TVV_Resolve = (CGPUTextureView_Vulkan*)desc->color_attachments[i].resolve_view;
            if (TVV_Resolve && (desc->sample_count != CGPU_SAMPLE_COUNT_1))
            {
                fbDesc.pImageViews[idx] = TVV_Resolve->pVkRTVDSVDescriptor;
                fbDesc.mAttachmentCount += 1;
                idx++;
            }
        }
        if (desc->depth_stencil != CGPU_NULLPTR && desc->depth_stencil->view != CGPU_NULLPTR)
        {
            CGPUTextureView_Vulkan* TVV = (CGPUTextureView_Vulkan*)desc->depth_stencil->view;
            fbDesc.pImageViews[idx] = TVV->pVkRTVDSVDescriptor;
            fbDesc.mLayers = TVV->super.info.array_layer_count;
            fbDesc.mAttachmentCount += 1;
            idx++;
        }
        if (desc->render_target_count)
            cgpu_assert(fbDesc.mLayers == 1 && "MRT pass supports only one layer!");
        VkUtil_FindOrCreateFrameBuffer(D, &fbDesc, &pFramebuffer);
    }
    // Cmd begin render pass
    VkClearValue clearValues[2 * CGPU_MAX_MRT_COUNT + 1] = { 0 };
    uint32_t clearCount = 0;
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        CGPUClearValue clearValue = desc->color_attachments[i].clear_color;
        if (desc->color_attachments[i].load_action == CGPU_LOAD_ACTION_CLEAR)
        {
            clearValues[i].color.float32[0] = clearValue.r;
            clearValues[i].color.float32[1] = clearValue.g;
            clearValues[i].color.float32[2] = clearValue.b;
            clearValues[i].color.float32[3] = clearValue.a;
            clearCount++;
        }
    }
    // clear msaa resolve targets
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        if (desc->color_attachments[i].load_action == CGPU_LOAD_ACTION_CLEAR)
            if (desc->color_attachments[i].resolve_view)
            {
                clearCount++;
            }
    }
    // depth stencil clear
    if (desc->depth_stencil)
    {
        if (desc->depth_stencil->depth_load_action == CGPU_LOAD_ACTION_CLEAR)
        {
            clearValues[clearCount].depthStencil.depth = desc->depth_stencil->clear_depth;
            clearValues[clearCount].depthStencil.stencil = desc->depth_stencil->clear_stencil;
            clearCount++;
        }
    }
    VkRect2D render_area = {
        .offset.x = 0,
        .offset.y = 0,
        .extent.width = Width,
        .extent.height = Height
    };
    VkRenderPassBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = VK_NULL_HANDLE,
        .renderPass = render_pass,
        .framebuffer = pFramebuffer,
        .renderArea = render_area,
        .clearValueCount = clearCount,
        .pClearValues = clearValues
    };
    D->mVkDeviceTable.vkCmdBeginRenderPass(Cmd->pVkCmdBuf, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    Cmd->pRenderPass = render_pass;
    return (CGPURenderPassEncoderId)cmd;
}

void cgpu_render_encoder_set_viewport_vulkan(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)encoder->device;
    VkViewport viewport = {
        .x = x,
        .y = y + height,
        .width = width,
        .height = -height,
        .minDepth = min_depth,
        .maxDepth = max_depth
    };
    D->mVkDeviceTable.vkCmdSetViewport(Cmd->pVkCmdBuf, 0, 1, &viewport);
}

void cgpu_render_encoder_set_scissor_vulkan(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)encoder->device;
    VkRect2D scissor = {
        .offset.x = x,
        .offset.y = y,
        .extent.width = width,
        .extent.height = height
    };
    D->mVkDeviceTable.vkCmdSetScissor(Cmd->pVkCmdBuf, 0, 1, &scissor);
}

void cgpu_render_encoder_bind_pipeline_vulkan(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    CGPURenderPipeline_Vulkan* PPL = (CGPURenderPipeline_Vulkan*)pipeline;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)pipeline->device;
    D->mVkDeviceTable.vkCmdBindPipeline(Cmd->pVkCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, PPL->pVkPipeline);
}

void cgpu_render_encoder_bind_vertex_buffers_vulkan(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)Cmd->super.device;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)D->super.adapter;
    const CGPUBuffer_Vulkan** Buffers = (const CGPUBuffer_Vulkan**)buffers;
    const uint32_t final_buffer_count = cgpu_min(buffer_count, A->mPhysicalDeviceProps.properties.limits.maxVertexInputBindings);

    DECLARE_ZERO(VkBuffer, vkBuffers[64]);
    DECLARE_ZERO(VkDeviceSize, vkOffsets[64]);

    for (uint32_t i = 0; i < final_buffer_count; ++i)
    {
        vkBuffers[i] = Buffers[i]->pVkBuffer;
        vkOffsets[i] = (offsets ? offsets[i] : 0);
    }

    D->mVkDeviceTable.vkCmdBindVertexBuffers(Cmd->pVkCmdBuf, 0, final_buffer_count, vkBuffers, vkOffsets);
}

void cgpu_render_encoder_bind_index_buffer_vulkan(CGPURenderPassEncoderId encoder, CGPUBufferId buffer,
uint32_t index_stride, uint64_t offset)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    const CGPUBuffer_Vulkan* Buffer = (const CGPUBuffer_Vulkan*)buffer;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)buffer->device;
    cgpu_assert(Cmd);
    cgpu_assert(buffer);
    cgpu_assert(VK_NULL_HANDLE != Cmd->pVkCmdBuf);
    cgpu_assert(VK_NULL_HANDLE != Buffer->pVkBuffer);

    VkIndexType vk_index_type =
    (sizeof(uint16_t) == index_stride) ?
    VK_INDEX_TYPE_UINT16 :
    ((sizeof(uint8_t) == index_stride) ? VK_INDEX_TYPE_UINT8_EXT : VK_INDEX_TYPE_UINT32);
    D->mVkDeviceTable.vkCmdBindIndexBuffer(Cmd->pVkCmdBuf, Buffer->pVkBuffer, offset, vk_index_type);
}

void cgpu_render_encoder_push_constants_vulkan(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    CGPURootSignature_Vulkan* RS = (CGPURootSignature_Vulkan*)rs;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)rs->device;
    D->mVkDeviceTable.vkCmdPushConstants(Cmd->pVkCmdBuf, RS->pPipelineLayout,
        RS->pPushConstRanges[0].stageFlags, 0,
        RS->pPushConstRanges[0].size, data);
}

void cgpu_render_encoder_draw_vulkan(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)encoder->device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    D->mVkDeviceTable.vkCmdDraw(Cmd->pVkCmdBuf, vertex_count, 1, first_vertex, 0);
}

void cgpu_render_encoder_draw_instanced_vulkan(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)encoder->device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    D->mVkDeviceTable.vkCmdDraw(Cmd->pVkCmdBuf, vertex_count, instance_count, first_vertex, first_instance);
}

void cgpu_render_encoder_draw_indexed_vulkan(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)encoder->device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    D->mVkDeviceTable.vkCmdDrawIndexed(Cmd->pVkCmdBuf, index_count, 1, first_index, first_vertex, 0);
}

void cgpu_render_encoder_draw_indexed_instanced_vulkan(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
{
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)encoder->device;
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)encoder;
    D->mVkDeviceTable.vkCmdDrawIndexed(Cmd->pVkCmdBuf, index_count, instance_count, first_index, first_vertex, first_instance);
}

void cgpu_cmd_end_render_pass_vulkan(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    const CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)cmd->device;
    D->mVkDeviceTable.vkCmdEndRenderPass(Cmd->pVkCmdBuf);
    Cmd->pRenderPass = VK_NULL_HANDLE;
}

// SwapChain APIs
#define clamp(x, min, max) (x) < (min) ? (min) : ((x) > (max) ? (max) : (x));
// TODO: Handle multi-queue presenting
CGPUSwapChainId cgpu_create_swapchain_vulkan_impl(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc, CGPUSwapChain_Vulkan* old)
{
    // CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)device->adapter->instance;
    CGPUAdapter_Vulkan* A = (CGPUAdapter_Vulkan*)device->adapter;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)device;

    VkSurfaceKHR vkSurface = (VkSurfaceKHR)desc->surface;

    VkSurfaceCapabilitiesKHR caps = { 0 };
    CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(A->pPhysicalDevice, vkSurface, &caps));
    if ((caps.maxImageCount > 0) && (desc->image_count > caps.maxImageCount))
    {
        ((CGPUSwapChainDescriptor*)desc)->image_count = caps.maxImageCount;
    }
    else if (desc->image_count < caps.minImageCount)
    {
        ((CGPUSwapChainDescriptor*)desc)->image_count = caps.minImageCount;
    }

    // Surface format
    DECLARE_ZERO(VkSurfaceFormatKHR, surface_format)
    surface_format.format = VK_FORMAT_UNDEFINED;
    uint32_t surfaceFormatCount = 0;
    CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
    A->pPhysicalDevice, vkSurface, &surfaceFormatCount, CGPU_NULLPTR));
    // Allocate and get surface formats
    DECLARE_ZERO_VLA(VkSurfaceFormatKHR, formats, surfaceFormatCount)
    CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
    A->pPhysicalDevice, vkSurface, &surfaceFormatCount, formats))

    // Only undefined format support found, force use B8G8R8A8
    if ((1 == surfaceFormatCount) && (VK_FORMAT_UNDEFINED == formats[0].format))
    {
        surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        VkFormat requested_format = VkUtil_FormatTranslateToVk(desc->format);
        // Handle hdr surface
        const VkSurfaceFormatKHR hdrSurfaceFormat = {
            VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            VK_COLOR_SPACE_HDR10_ST2084_EXT
        };
        VkColorSpaceKHR requested_color_space =
        requested_format == hdrSurfaceFormat.format ? hdrSurfaceFormat.colorSpace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        for (uint32_t i = 0; i < surfaceFormatCount; ++i)
        {
            if ((requested_format == formats[i].format) && (requested_color_space == formats[i].colorSpace))
            {
                surface_format.format = requested_format;
                surface_format.colorSpace = requested_color_space;
                break;
            }
        }
        // Default to VK_FORMAT_B8G8R8A8_UNORM if requested format isn't found
        if (VK_FORMAT_UNDEFINED == surface_format.format)
        {
            surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
            surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
    }
    cgpu_assert(VK_FORMAT_UNDEFINED != surface_format.format);

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t swapChainImageCount = 0;
    // Get present mode count
    CHECK_VKRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
    A->pPhysicalDevice, vkSurface, &swapChainImageCount, NULL));
    // Allocate and get present modes
    DECLARE_ZERO_VLA(VkPresentModeKHR, modes, swapChainImageCount)
    CHECK_VKRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
    A->pPhysicalDevice, vkSurface, &swapChainImageCount, modes));
    // Select Preferred Present Mode
    VkPresentModeKHR preferredModeList[] = {
        VK_PRESENT_MODE_IMMEDIATE_KHR,    // normal
        VK_PRESENT_MODE_MAILBOX_KHR,      // low latency
        VK_PRESENT_MODE_FIFO_RELAXED_KHR, // minimize stuttering
        VK_PRESENT_MODE_FIFO_KHR          // low power consumption
    };
    const uint32_t preferredModeCount = CGPU_ARRAY_LEN(preferredModeList);
    uint32_t preferredModeStartIndex = desc->enable_vsync ? 1 : 0;
    for (uint32_t j = preferredModeStartIndex; j < preferredModeCount; ++j)
    {
        VkPresentModeKHR mode = preferredModeList[j];
        uint32_t i = 0;
        for (i = 0; i < swapChainImageCount; ++i)
        {
            if (modes[i] == mode) break;
        }
        if (i < swapChainImageCount)
        {
            present_mode = mode;
            break;
        }
    }
    // Swapchain
    VkExtent2D extent;
    extent.width = clamp(desc->width, caps.minImageExtent.width, caps.maxImageExtent.width);
    extent.height = clamp(desc->height, caps.minImageExtent.height, caps.maxImageExtent.height);

    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    /*
    uint32_t presentQueueFamilyIndex = -1;
    // Check Queue Present Support.
    for (uint32_t i = 0; i < desc->present_queues_count; i++)
    {
        // CGPUQueue_Vulkan* Q = (CGPUQueue_Vulkan*)desc->present_queues[i];
        VkBool32 sup = VK_FALSE;
        VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(A->pPhysicalDevice, Q->mVkQueueFamilyIndex, vkSurface, &sup);
        if ((VK_SUCCESS == res) && (VK_TRUE == sup))
        {
            presentQueueFamilyIndex = Q->mVkQueueFamilyIndex;
        }
        else
        {
            // Get queue family properties
            uint32_t queueFamilyPropertyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(A->pPhysicalDevice, &queueFamilyPropertyCount, NULL);
            DECLARE_ZERO_VLA(VkQueueFamilyProperties, queueFamilyProperties, queueFamilyPropertyCount)
            vkGetPhysicalDeviceQueueFamilyProperties(A->pPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties);
            // Check if hardware provides dedicated present queue
            if (queueFamilyPropertyCount)
            {
                for (uint32_t index = 0; index < queueFamilyPropertyCount; ++index)
                {
                    VkBool32 supports_present = VK_FALSE;
                    VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(
                    A->pPhysicalDevice, index, vkSurface, &supports_present);
                    if ((VK_SUCCESS == res) && (VK_TRUE == supports_present) && Q->mVkQueueFamilyIndex != index)
                    {
                        presentQueueFamilyIndex = index;
                        break;
                    }
                }
                // If there is no dedicated present queue, just find the first available queue which supports
                // present
                if (presentQueueFamilyIndex == -1)
                {
                    for (uint32_t index = 0; index < queueFamilyPropertyCount; ++index)
                    {
                        VkBool32 supports_present = VK_FALSE;
                        VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(
                        A->pPhysicalDevice, index, vkSurface, &supports_present);
                        if ((VK_SUCCESS == res) && (VK_TRUE == supports_present))
                        {
                            presentQueueFamilyIndex = index;
                            break;
                        }
                        else
                        {
                            // No present queue family available. Something goes wrong.
                            cgpu_assert(0);
                        }
                    }
                }
            }
        }
    }
    */
    
    VkSurfaceTransformFlagBitsKHR pre_transform;
    // #TODO: Add more if necessary but identity should be enough for now
    if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        pre_transform = caps.currentTransform;

    const VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] = {
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    };
    VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
    for (uint32_t _i = 0; _i < CGPU_ARRAY_LEN(compositeAlphaFlags); _i++)
    {
        if (caps.supportedCompositeAlpha & compositeAlphaFlags[_i])
        {
            composite_alpha = compositeAlphaFlags[_i];
            break;
        }
    }
    cgpu_assert(composite_alpha != VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR);

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = vkSurface,
        .minImageCount = desc->image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = sharing_mode,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = CGPU_NULLPTR,
        .preTransform = pre_transform,
        .compositeAlpha = composite_alpha,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
    VkSwapchainKHR new_chain = VK_NULL_HANDLE;
    uint32_t buffer_count = 0;
    VkResult res = D->mVkDeviceTable.vkCreateSwapchainKHR(D->pVkDevice, &swapChainCreateInfo, GLOBAL_VkAllocationCallbacks, &new_chain);
    if (VK_SUCCESS != res)
    {
        cgpu_assert(0 && "fatal: vkCreateSwapchainKHR failed!");
    }

    // Get swapchain images
    CHECK_VKRESULT(D->mVkDeviceTable.vkGetSwapchainImagesKHR(D->pVkDevice, new_chain, &buffer_count, VK_NULL_HANDLE));
    CGPUSwapChain_Vulkan* S = old;
    if (!old)
    {
        S = (CGPUSwapChain_Vulkan*)cgpu_calloc_aligned(1,
            sizeof(CGPUSwapChain_Vulkan) + 
            (sizeof(CGPUTexture_Vulkan) + sizeof(CGPUTextureInfo)) * buffer_count + 
            sizeof(CGPUTextureId) * buffer_count, _Alignof(CGPUSwapChain_Vulkan));
    }
    S->pVkSwapChain = new_chain;
    S->super.buffer_count = buffer_count;
    DECLARE_ZERO_VLA(VkImage, vimages, S->super.buffer_count)
    CHECK_VKRESULT(D->mVkDeviceTable.vkGetSwapchainImagesKHR(D->pVkDevice, S->pVkSwapChain, &S->super.buffer_count, vimages));
    
    struct THeader
    {
        CGPUTexture_Vulkan T;
        CGPUTextureInfo I;
    };
    struct THeader* Ts = (struct THeader*)(S + 1);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Ts[i].T.pVkImage = vimages[i];
        Ts[i].T.super.device = &D->super;
        Ts[i].T.super.info = &Ts[i].I;

        Ts[i].I.is_cube = false;
        Ts[i].I.array_size_minus_one = 0;
        Ts[i].I.sample_count = CGPU_SAMPLE_COUNT_1; // TODO: ?
        Ts[i].I.format = VkUtil_FormatTranslateToCGPU(surface_format.format);
        Ts[i].I.aspect_mask = VkUtil_DeterminAspectMask(VkUtil_FormatTranslateToVk(Ts[i].I.format), false);
        Ts[i].I.depth = 1;
        Ts[i].I.width = extent.width;
        Ts[i].I.height = extent.height;
        Ts[i].I.mip_levels = 1;
        Ts[i].I.node_index = CGPU_SINGLE_GPU_NODE_INDEX;
        Ts[i].I.owns_image = false;
    }
    CGPUTextureId* Vs = (CGPUTextureId*)(Ts + buffer_count);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Vs[i] = &Ts[i].T.super;
    }
    S->super.back_buffers = Vs;
    S->pVkSurface = vkSurface;
    return &S->super;
}

void cgpu_free_swapchain_vulkan_impl(CGPUSwapChainId swapchain)
{
    CGPUSwapChain_Vulkan* S = (CGPUSwapChain_Vulkan*)swapchain;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)swapchain->device;

    D->mVkDeviceTable.vkDestroySwapchainKHR(D->pVkDevice, S->pVkSwapChain, GLOBAL_VkAllocationCallbacks);
}

CGPUSwapChainId cgpu_create_swapchain_vulkan(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc)
{
    return cgpu_create_swapchain_vulkan_impl(device, desc, CGPU_NULLPTR);
}

uint32_t cgpu_acquire_next_image_vulkan(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc)
{
    CGPUFence_Vulkan* Fence = (CGPUFence_Vulkan*)desc->fence;
    CGPUSemaphore_Vulkan* Semaphore = (CGPUSemaphore_Vulkan*)desc->signal_semaphore;
    CGPUSwapChain_Vulkan* SC = (CGPUSwapChain_Vulkan*)swapchain;
    CGPUDevice_Vulkan* D = (CGPUDevice_Vulkan*)swapchain->device;

    VkResult vk_res = VK_SUCCESS;
    uint32_t idx;

    VkSemaphore vsemaphore = Semaphore ? Semaphore->pVkSemaphore : VK_NULL_HANDLE;
    VkFence vfence = Fence ? Fence->pVkFence : VK_NULL_HANDLE;

    vk_res = vkAcquireNextImageKHR(D->pVkDevice, SC->pVkSwapChain,
        UINT64_MAX,
        vsemaphore, // sem
        vfence,     // fence
        &idx);

    // If swapchain is out of date, let caller know by setting image index to -1
    if (vk_res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        idx = -1;
        if (Fence)
        {
            Fence->mSubmitted = false;
            D->mVkDeviceTable.vkResetFences(D->pVkDevice, 1, &Fence->pVkFence);
        }
        if (Semaphore) Semaphore->mSignaled = false;
    }
    else if (vk_res == VK_SUCCESS)
    {
        if (Fence) Fence->mSubmitted = true;
        if (Semaphore) Semaphore->mSignaled = true;
    }
    return idx;
}

void cgpu_free_swapchain_vulkan(CGPUSwapChainId swapchain)
{
    cgpu_free_swapchain_vulkan_impl(swapchain);
    cgpu_free((void*)swapchain);
}

// exts
#include "cgpu/extensions/cgpu_vulkan_exts.h"

VkCommandBuffer cgpu_vulkan_get_command_buffer(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_Vulkan* Cmd = (CGPUCommandBuffer_Vulkan*)cmd;
    return Cmd->pVkCmdBuf;
}

VkBuffer cgpu_vulkan_get_buffer(CGPUBufferId buffer)
{
    CGPUBuffer_Vulkan* Buf = (CGPUBuffer_Vulkan*)buffer;
    return Buf->pVkBuffer;
}
