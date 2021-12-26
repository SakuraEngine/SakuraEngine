#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "vulkan_utils.h"
#include "vulkan/vulkan_core.h"
#include "cgpu/shader-reflections/spirv/spirv_reflect.h"
#include "../common/common_utils.h"
#include <string.h>

const CGpuProcTable tbl_vk = {
    // Instance APIs
    .create_instance = &cgpu_create_instance_vulkan,
    .query_instance_features = &cgpu_query_instance_features_vulkan,
    .free_instance = &cgpu_free_instance_vulkan,

    // Adapter APIs
    .enum_adapters = &cgpu_enum_adapters_vulkan,
    .query_adapter_detail = &cgpu_query_adapter_detail_vulkan,
    .query_queue_count = &cgpu_query_queue_count_vulkan,

    // Device APIs
    .create_device = &cgpu_create_device_vulkan,
    .free_device = &cgpu_free_device_vulkan,

    // API Object APIs
    .create_fence = &cgpu_create_fence_vulkan,
    .wait_fences = &cgpu_wait_fences_vulkan,
    .free_fence = &cgpu_free_fence_vulkan,
    .create_root_signature = &cgpu_create_root_signature_vulkan,
    .free_root_signature = &cgpu_free_root_signature_vulkan,
    .create_descriptor_set = &cgpu_create_descriptor_set_vulkan,
    .update_descriptor_set = &cgpu_update_descriptor_set_vulkan,
    .free_descriptor_set = &cgpu_free_descriptor_set_vulkan,
    .create_compute_pipeline = &cgpu_create_compute_pipeline_vulkan,
    .free_compute_pipeline = &cgpu_free_compute_pipeline_vulkan,
    .create_render_pipeline = &cgpu_create_render_pipeline_vulkan,
    .free_render_pipeline = &cgpu_free_render_pipeline_vulkan,

    // Queue APIs
    .get_queue = &cgpu_get_queue_vulkan,
    .submit_queue = &cgpu_submit_queue_vulkan,
    .wait_queue_idle = &cgpu_wait_queue_idle_vulkan,
    .queue_present = &cgpu_queue_present_vulkan,
    .free_queue = &cgpu_free_queue_vulkan,

    // Command APIs
    .create_command_pool = &cgpu_create_command_pool_vulkan,
    .create_command_buffer = &cgpu_create_command_buffer_vulkan,
    .reset_command_pool = &cgpu_reset_command_pool_vulkan,
    .free_command_buffer = &cgpu_free_command_buffer_vulkan,
    .free_command_pool = &cgpu_free_command_pool_vulkan,

    // Shader APIs
    .create_shader_library = &cgpu_create_shader_library_vulkan,
    .free_shader_library = &cgpu_free_shader_library_vulkan,

    // Buffer APIs
    .create_buffer = &cgpu_create_buffer_vulkan,
    .map_buffer = &cgpu_map_buffer_vulkan,
    .unmap_buffer = &cgpu_unmap_buffer_vulkan,
    .free_buffer = &cgpu_free_buffer_vulkan,

    // Texture/TextureView APIs
    .create_texture = &cgpu_create_texture_vulkan,
    .free_texture = &cgpu_free_texture_vulkan,
    .create_texture_view = &cgpu_create_texture_view_vulkan,
    .free_texture_view = &cgpu_free_texture_view_vulkan,

    // Sampler APIs
    .create_sampler = &cgpu_create_sampler_vulkan,
    .free_sampler = &cgpu_free_sampler_vulkan,

    // Swapchain APIs
    .create_swapchain = &cgpu_create_swapchain_vulkan,
    .acquire_next_image = &cgpu_acquire_next_image_vulkan,
    .free_swapchain = &cgpu_free_swapchain_vulkan,

    // CMDs
    .cmd_begin = &cgpu_cmd_begin_vulkan,
    .cmd_transfer_buffer_to_buffer = &cgpu_cmd_transfer_buffer_to_buffer_vulkan,
    .cmd_transfer_buffer_to_texture = &cgpu_cmd_transfer_buffer_to_texture_vulkan,
    .cmd_resource_barrier = &cgpu_cmd_resource_barrier_vulkan,
    .cmd_end = &cgpu_cmd_end_vulkan,

    // Compute CMDs
    .cmd_begin_compute_pass = &cgpu_cmd_begin_compute_pass_vulkan,
    .compute_encoder_bind_descriptor_set = &cgpu_compute_encoder_bind_descriptor_set_vulkan,
    .compute_encoder_bind_pipeline = &cgpu_compute_encoder_bind_pipeline_vulkan,
    .compute_encoder_dispatch = &cgpu_compute_encoder_dispatch_vulkan,
    .cmd_end_compute_pass = &cgpu_cmd_end_compute_pass_vulkan,

    // Render CMDs
    .cmd_begin_render_pass = &cgpu_cmd_begin_render_pass_vulkan,
    .render_encoder_bind_descriptor_set = cgpu_render_encoder_bind_descriptor_set_vulkan,
    .render_encoder_set_viewport = &cgpu_render_encoder_set_viewport_vulkan,
    .render_encoder_set_scissor = &cgpu_render_encoder_set_scissor_vulkan,
    .render_encoder_bind_pipeline = &cgpu_render_encoder_bind_pipeline_vulkan,
    .render_encoder_draw = &cgpu_render_encoder_draw_vulkan,
    .cmd_end_render_pass = &cgpu_cmd_end_render_pass_vulkan
};
const CGpuProcTable* CGPU_VulkanProcTable() { return &tbl_vk; }

static void VkUtil_FindOrCreateFrameBuffer(const CGpuDevice_Vulkan* D, const struct VkUtil_FramebufferDesc* pDesc, VkFramebuffer* ppFramebuffer)
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
        .attachmentCount = pDesc->mColorAttachmentCount,
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
FORCEINLINE static void VkUtil_FreeFramebuffer(CGpuDevice_Vulkan* D, VkFramebuffer pFramebuffer)
{
    D->mVkDeviceTable.vkDestroyFramebuffer(D->pVkDevice, pFramebuffer, GLOBAL_VkAllocationCallbacks);
}

// Render Pass Utils
// TODO: recycle cached render passes
FORCEINLINE static void VkUtil_FreeRenderPass(CGpuDevice_Vulkan* D, VkRenderPass pRenderPass)
{
    D->mVkDeviceTable.vkDestroyRenderPass(D->pVkDevice, pRenderPass, GLOBAL_VkAllocationCallbacks);
}

static void VkUtil_FindOrCreateRenderPass(const CGpuDevice_Vulkan* D, const VkUtil_RenderPassDesc* pDesc, VkRenderPass* ppRenderPass)
{
    VkRenderPass found = VkUtil_RenderPassTableTryFind(D->pPassTable, pDesc);
    if (found != VK_NULL_HANDLE)
    {
        *ppRenderPass = found;
        return;
    }
    cgpu_assert(VK_NULL_HANDLE != D->pVkDevice);
    uint32_t colorAttachmentCount = pDesc->mColorAttachmentCount;
    uint32_t depthAttachmentCount = (pDesc->mDepthStencilFormat != PF_UNDEFINED) ? 1 : 0;
    VkAttachmentDescription attachments[MAX_MRT_COUNT + 1] = { 0 };
    VkAttachmentReference color_attachment_refs[MAX_MRT_COUNT] = { 0 };
    VkAttachmentReference depth_stencil_attachment_ref[1] = { 0 };
    VkSampleCountFlagBits sample_count = VkUtil_SampleCountTranslateToVk(pDesc->mSampleCount);
    // Fill out attachment descriptions and references
    {
        // Color
        for (uint32_t i = 0; i < colorAttachmentCount; ++i)
        {
            const uint32_t ssidx = i;
            // descriptions
            attachments[ssidx].flags = 0;
            attachments[ssidx].format = (VkFormat)VkUtil_FormatTranslateToVk(pDesc->pColorFormats[i]);
            attachments[ssidx].samples = sample_count;
            attachments[ssidx].loadOp = gVkAttachmentLoadOpTranslator[pDesc->pLoadActionsColor[i]];
            attachments[ssidx].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[ssidx].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[ssidx].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[ssidx].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments[ssidx].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // references
            color_attachment_refs[i].attachment = ssidx; //-V522
            color_attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }
    // Depth stencil
    if (depthAttachmentCount > 0)
    {
        uint32_t idx = colorAttachmentCount;
        attachments[idx].flags = 0;
        attachments[idx].format = (VkFormat)VkUtil_FormatTranslateToVk(pDesc->mDepthStencilFormat);
        attachments[idx].samples = sample_count;
        attachments[idx].loadOp = gVkAttachmentLoadOpTranslator[pDesc->mLoadActionDepth];
        attachments[idx].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[idx].stencilLoadOp = gVkAttachmentLoadOpTranslator[pDesc->mLoadActionStencil];
        attachments[idx].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[idx].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[idx].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_stencil_attachment_ref[0].attachment = idx; //-V522
        depth_stencil_attachment_ref[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    uint32_t attachment_count = colorAttachmentCount;
    attachment_count += depthAttachmentCount;
    void* render_pass_next = NULL;
    // Fill Description
    VkSubpassDescription subpass = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = colorAttachmentCount,
        .pColorAttachments = color_attachment_refs,
        .pResolveAttachments = NULL,
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

void cgpu_query_instance_features_vulkan(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
    features->specialization_constant = true;
}

void cgpu_enum_adapters_vulkan(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)instance;
    *adapters_num = I->mPhysicalDeviceCount;
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++)
        {
            adapters[i] = &I->pVulkanAdapters[i].super;
        }
    }
}

const CGpuAdapterDetail* cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter)
{
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)adapter;
    return &A->adapter_detail;
}

uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)adapter;
    uint32_t count = 0;
    switch (type)
    {
        case ECGpuQueueType_Graphics: {
            for (uint32_t i = 0; i < A->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &A->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    count += prop->queueCount;
                }
            }
        }
        break;
        case ECGpuQueueType_Compute: {
            for (uint32_t i = 0; i < A->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &A->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    if (!(prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    {
                        count += prop->queueCount;
                    }
                }
            }
        }
        break;
        case ECGpuQueueType_Transfer: {
            for (uint32_t i = 0; i < A->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &A->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    if (!(prop->queueFlags & VK_QUEUE_COMPUTE_BIT))
                    {
                        if (!(prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
                        {
                            count += prop->queueCount;
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
CGpuFenceId cgpu_create_fence_vulkan(CGpuDeviceId device)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuFence_Vulkan* F = (CGpuFence_Vulkan*)cgpu_calloc(1, sizeof(CGpuFence_Vulkan));
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

void cgpu_wait_fences_vulkan(const CGpuFenceId* fences, uint32_t fence_count)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)fences[0]->device;
    DECLARE_ZERO_VLA(VkFence, vfences, fence_count)
    uint32_t numValidFences = 0;
    for (uint32_t i = 0; i < fence_count; ++i)
    {
        CGpuFence_Vulkan* Fence = (CGpuFence_Vulkan*)fences[i];
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
        CGpuFence_Vulkan* Fence = (CGpuFence_Vulkan*)fences[i];
        Fence->mSubmitted = false;
    }
}

void cgpu_free_fence_vulkan(CGpuFenceId fence)
{
    CGpuFence_Vulkan* F = (CGpuFence_Vulkan*)fence;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)fence->device;
    D->mVkDeviceTable.vkDestroyFence(D->pVkDevice, F->pVkFence, GLOBAL_VkAllocationCallbacks);
    cgpu_free(F);
}

CGpuRootSignatureId cgpu_create_root_signature_vulkan(CGpuDeviceId device,
    const struct CGpuRootSignatureDescriptor* desc)
{
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)cgpu_calloc(1, sizeof(CGpuRootSignature_Vulkan));
    DECLARE_ZERO(CGpuUtil_RSBlackboard, bb)
    CGpuUtil_InitRSBlackboardAndParamTables((CGpuRootSignature*)RS, &bb, desc);
    // Collect Shader Resources
    if (bb.set_count * bb.max_binding > 0)
    {
        RS->set_layouts = (SetLayout_Vulkan*)cgpu_calloc(bb.set_count, sizeof(SetLayout_Vulkan));
        // Create Vk Objects
        for (uint32_t i_set = 0; i_set < RS->super.table_count; i_set++)
        {
            SetLayout_Vulkan* set_to_record = &RS->set_layouts[i_set];
            CGpuParameterTable* param_table = &RS->super.tables[i_set];
            VkDescriptorSetLayoutBinding* vkbindings = (VkDescriptorSetLayoutBinding*)cgpu_calloc(param_table->resources_count, sizeof(VkDescriptorSetLayoutBinding));
            for (uint32_t i_binding = 0; i_binding < param_table->resources_count; i_binding++)
            {
                vkbindings[i_binding].binding = param_table->resources[i_binding].binding;
                vkbindings[i_binding].stageFlags = VkUtil_TranslateShaderUsages(param_table->resources[i_binding].stages);
                vkbindings[i_binding].descriptorType = VkUtil_TranslateResourceType(param_table->resources[i_binding].type);
                vkbindings[i_binding].descriptorCount = param_table->resources[i_binding].size;
                // TODO: Support static samplers
                vkbindings[i_binding].pImmutableSamplers = CGPU_NULL;
            }
            VkDescriptorSetLayoutCreateInfo set_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .pBindings = vkbindings,
                .bindingCount = param_table->resources_count
            };
            CHECK_VKRESULT(D->mVkDeviceTable.vkCreateDescriptorSetLayout(D->pVkDevice, &set_info, GLOBAL_VkAllocationCallbacks, &set_to_record->layout));
            cgpu_free(vkbindings);
        }
    }
    // Record Descriptor Sets
    VkDescriptorSetLayout* set_layouts = cgpu_calloc(RS->super.table_count + 1, sizeof(VkDescriptorSetLayout));
    for (uint32_t i_set = 0; i_set < RS->super.table_count; i_set++)
    {
        SetLayout_Vulkan* set_to_record = (SetLayout_Vulkan*)&RS->set_layouts[i_set];
        set_layouts[i_set] = set_to_record->layout;
    }
    // Create Pipeline Layout
    VkPipelineLayoutCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = RS->super.table_count,
        .pSetLayouts = set_layouts,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreatePipelineLayout(D->pVkDevice, &pipeline_info, GLOBAL_VkAllocationCallbacks, &RS->pipeline_layout));
    // Create Update Templates
    for (uint32_t i_set = 0; i_set < RS->super.table_count; i_set++)
    {
        CGpuParameterTable* param_table = &RS->super.tables[i_set];
        SetLayout_Vulkan* set_to_record = &RS->set_layouts[i_set];
        VkDescriptorUpdateTemplateEntry* template_entries = (VkDescriptorUpdateTemplateEntry*)cgpu_calloc(
            param_table->resources_count, sizeof(VkDescriptorUpdateTemplateEntry));
        for (uint32_t i_binding = 0; i_binding < param_table->resources_count; i_binding++)
        {
            VkDescriptorUpdateTemplateEntry* this_entry = template_entries + i_binding;
            this_entry->descriptorCount = param_table->resources[i_binding].size;
            this_entry->descriptorType = VkUtil_TranslateResourceType(param_table->resources[i_binding].type);
            this_entry->dstBinding = param_table->resources[i_binding].binding;
            this_entry->dstArrayElement = 0;
            this_entry->stride = sizeof(VkDescriptorUpdateData);
            this_entry->offset = this_entry->dstBinding * this_entry->stride;
        }
        VkDescriptorUpdateTemplateCreateInfo template_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
            .pNext = NULL,
            .templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET_KHR,
            .pipelineLayout = RS->pipeline_layout,
            .pipelineBindPoint = gPipelineBindPoint[bb.pipelineType],
            .descriptorSetLayout = set_to_record->layout,
            .set = i_set,
            .pDescriptorUpdateEntries = template_entries,
            .descriptorUpdateEntryCount = param_table->resources_count
        };
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateDescriptorUpdateTemplate(D->pVkDevice,
            &template_info, GLOBAL_VkAllocationCallbacks, &set_to_record->update_template));
    }
    // Free Temporal Memory
    CGpuUtil_FreeRSBlackboard(&bb);
    cgpu_free(set_layouts);
    return &RS->super;
}

void cgpu_free_root_signature_vulkan(CGpuRootSignatureId signature)
{
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)signature;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)signature->device;
    // Free Reflection Data
    CGpuUtil_FreeRSParamTables((CGpuRootSignature*)signature);
    // Free Vk Objects
    for (uint32_t i_set = 0; i_set < RS->super.table_count; i_set++)
    {
        SetLayout_Vulkan* set_to_free = &RS->set_layouts[i_set];
        if (set_to_free->layout != VK_NULL_HANDLE)
            D->mVkDeviceTable.vkDestroyDescriptorSetLayout(D->pVkDevice, set_to_free->layout, GLOBAL_VkAllocationCallbacks);
        if (set_to_free->update_template != VK_NULL_HANDLE)
            D->mVkDeviceTable.vkDestroyDescriptorUpdateTemplate(D->pVkDevice, set_to_free->update_template, GLOBAL_VkAllocationCallbacks);
    }
    cgpu_free(RS->set_layouts);
    D->mVkDeviceTable.vkDestroyPipelineLayout(D->pVkDevice, RS->pipeline_layout, GLOBAL_VkAllocationCallbacks);
    cgpu_free(RS);
}

CGpuDescriptorSetId cgpu_create_descriptor_set_vulkan(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc)
{
    size_t totalSize = sizeof(CGpuDescriptorSet_Vulkan);
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)desc->root_signature;
    SetLayout_Vulkan* SetLayout = &RS->set_layouts[desc->set_index];
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    const size_t UpdateTemplateSize = RS->super.tables[desc->set_index].resources_count * sizeof(VkDescriptorUpdateData);
    totalSize += UpdateTemplateSize;
    CGpuDescriptorSet_Vulkan* Set = cgpu_calloc_aligned(1, totalSize, _Alignof(CGpuDescriptorSet_Vulkan));
    char8_t* pMem = (char8_t*)(Set + 1);
    // Allocate Descriptor Set
    VkUtil_ConsumeDescriptorSets(D->pDescriptorPool, &SetLayout->layout, &Set->pVkDescriptorSet, 1);
    // Fill Update Template Data
    Set->pUpdateData = (VkDescriptorUpdateData*)pMem;
    memset(Set->pUpdateData, 0, UpdateTemplateSize);
    return &Set->super;
}

void cgpu_update_descriptor_set_vulkan(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count)
{
    CGpuDescriptorSet_Vulkan* Set = (CGpuDescriptorSet_Vulkan*)set;
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)set->root_signature;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)set->root_signature->device;
    VkDescriptorUpdateData* pUpdateData = Set->pUpdateData;
    bool dirty = false;
    SetLayout_Vulkan* SetLayout = &RS->set_layouts[set->index];
    CGpuParameterTable* ParamTable = &RS->super.tables[set->index];
    for (uint32_t i = 0; i < count; i++)
    {
        // Descriptor Info
        const CGpuDescriptorData* pParam = datas + i;
        CGpuShaderResource* ResData = CGPU_NULLPTR;
        size_t argNameHash = cgpu_hash(pParam->name, strlen(pParam->name), *(size_t*)&D);
        for (uint32_t p = 0; p < ParamTable->resources_count; p++)
        {
            if (ParamTable->resources[p].name_hash == argNameHash)
            {
                ResData = ParamTable->resources + p;
            }
        }
        // Update Info
        const uint32_t arrayCount = cgpu_max(1U, pParam->count);
        switch (ResData->type)
        {
            case RT_TEXTURE: {
                cgpu_assert(pParam->textures && "cgpu_assert: Binding NULL texture(s)");
                CGpuTextureView_Vulkan** TextureViews = (CGpuTextureView_Vulkan**)pParam->textures;
                for (uint32_t arr = 0; arr < arrayCount; ++arr)
                {
                    // TODO: Stencil support
                    cgpu_assert(pParam->textures[arr] && "cgpu_assert: Binding NULL texture!");
                    VkDescriptorUpdateData* Data = &pUpdateData[ResData->binding + arr];
                    Data->mImageInfo.imageView = TextureViews[arr]->pVkSRVDescriptor;
                    Data->mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    Data->mImageInfo.sampler = VK_NULL_HANDLE;
                    dirty = true;
                }
                break;
            }
            case RT_SAMPLER: {
                cgpu_assert(pParam->samplers && "cgpu_assert: Binding NULL Sampler(s)");
                CGpuSampler_Vulkan** Samplers = (CGpuSampler_Vulkan**)pParam->samplers;
                for (uint32_t arr = 0; arr < arrayCount; ++arr)
                {
                    cgpu_assert(pParam->samplers[arr] && "cgpu_assert: Binding NULL Sampler!");
                    VkDescriptorUpdateData* Data = &pUpdateData[ResData->binding + arr];
                    Data->mImageInfo.sampler = Samplers[arr]->pVkSampler;
                    dirty = true;
                }
                break;
            }
            case RT_BUFFER:
            case RT_BUFFER_RAW:
            case RT_RW_BUFFER:
            case RT_RW_BUFFER_RAW: {
                cgpu_assert(pParam->buffers && "cgpu_assert: Binding NULL Buffer(s)!");
                CGpuBuffer_Vulkan** Buffers = (CGpuBuffer_Vulkan**)pParam->buffers;
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
        D->mVkDeviceTable.vkUpdateDescriptorSetWithTemplateKHR(D->pVkDevice, Set->pVkDescriptorSet, SetLayout->update_template, Set->pUpdateData);
    }
}

void cgpu_free_descriptor_set_vulkan(CGpuDescriptorSetId set)
{
    CGpuDescriptorSet_Vulkan* Set = (CGpuDescriptorSet_Vulkan*)set;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)set->root_signature->device;
    VkUtil_ReturnDescriptorSets(D->pDescriptorPool, &Set->pVkDescriptorSet, 1);
    cgpu_free(Set);
}

CGpuComputePipelineId cgpu_create_compute_pipeline_vulkan(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuComputePipeline_Vulkan* PPL = (CGpuComputePipeline_Vulkan*)cgpu_calloc(1, sizeof(CGpuComputePipeline_Vulkan));
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)desc->root_signature;
    CGpuShaderLibrary_Vulkan* SL = (CGpuShaderLibrary_Vulkan*)desc->compute_shader->library;
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
        .layout = RS->pipeline_layout,
        .basePipelineHandle = 0,
        .basePipelineIndex = 0
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateComputePipelines(D->pVkDevice,
        D->pPipelineCache, 1, &pipeline_info, GLOBAL_VkAllocationCallbacks, &PPL->pVkPipeline));
    return &PPL->super;
}

void cgpu_free_compute_pipeline_vulkan(CGpuComputePipelineId pipeline)
{
    CGpuComputePipeline_Vulkan* PPL = (CGpuComputePipeline_Vulkan*)pipeline;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)pipeline->device;
    D->mVkDeviceTable.vkDestroyPipeline(D->pVkDevice, PPL->pVkPipeline, GLOBAL_VkAllocationCallbacks);
    cgpu_free(PPL);
}

VkCullModeFlagBits gVkCullModeTranslator[CULL_MODE_COUNT] = {
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_BIT
};

VkPolygonMode gVkFillModeTranslator[FM_COUNT] = {
    VK_POLYGON_MODE_FILL,
    VK_POLYGON_MODE_LINE
};

VkFrontFace gVkFrontFaceTranslator[] = {
    VK_FRONT_FACE_COUNTER_CLOCKWISE,
    VK_FRONT_FACE_CLOCKWISE
};
VkBlendFactor gVkBlendConstantTranslator[BC_COUNT] = {
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_SRC_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    VK_BLEND_FACTOR_SRC_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    VK_BLEND_FACTOR_DST_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
    VK_BLEND_FACTOR_CONSTANT_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
};
VkBlendOp gVkBlendOpTranslator[BM_COUNT] = {
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_REVERSE_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX,
};
/* clang-format off */
CGpuRenderPipelineId cgpu_create_render_pipeline_vulkan(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)desc->root_signature;
    CGpuRenderPipeline_Vulkan* RP = (CGpuRenderPipeline_Vulkan*)cgpu_calloc(1, sizeof(CGpuRenderPipeline_Vulkan));
    // TODO: Shader spec
    const VkSpecializationInfo* specializationInfo = VK_NULL_HANDLE;
    // Vertex input state
    uint32_t input_binding_count = 0;
	DECLARE_ZERO(VkVertexInputBindingDescription, input_bindings[MAX_VERTEX_BINDINGS]) 
	uint32_t  input_attribute_count = 0;
	DECLARE_ZERO(VkVertexInputAttributeDescription, input_attributes[MAX_VERTEX_BINDINGS]) 
    // Make sure there's attributes
    if (desc->vertex_layout != NULL)
    {
        // Ignore everything that's beyond max_vertex_attribs
        uint32_t attrib_count = desc->vertex_layout->attribute_count > MAX_VERTEX_ATTRIBS ? MAX_VERTEX_ATTRIBS : desc->vertex_layout->attribute_count;
        uint32_t binding_value = UINT32_MAX;
        // Initial values
        for (uint32_t i = 0; i < attrib_count; ++i)
        {
            const CGpuVertexAttribute* attrib = &(desc->vertex_layout->attributes[i]);
            if (binding_value != attrib->binding)
            {
                binding_value = attrib->binding;
                ++input_binding_count;
            }
            input_bindings[input_binding_count - 1].binding = binding_value;
            if (attrib->rate == VAR_INSTANCE)
            {
                input_bindings[input_binding_count - 1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            }
            else
            {
                input_bindings[input_binding_count - 1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            }
            input_bindings[input_binding_count - 1].stride += FormatUtil_BitSizeOfBlock(attrib->format) / 8;
            input_attributes[input_attribute_count].location = attrib->location;
            input_attributes[input_attribute_count].binding = attrib->binding;
            input_attributes[input_attribute_count].format = VkUtil_FormatTranslateToVk(attrib->format);
            input_attributes[input_attribute_count].offset = attrib->offset;
            ++input_attribute_count;
        }
    }
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
        ECGpuShaderStage stage_mask = (ECGpuShaderStage)(1 << i);
        shaderStages[stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[stage_count].pNext = NULL;
        shaderStages[stage_count].flags = 0;
        shaderStages[stage_count].pSpecializationInfo = specializationInfo;
        switch (stage_mask)
        {
            case SS_VERT:
            {
                if(desc->vertex_shader)
                {
                    shaderStages[stage_count].pName = desc->vertex_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStages[stage_count].module = ((CGpuShaderLibrary_Vulkan*)desc->vertex_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case SS_TESC:
            {
                if(desc->tesc_shader)
                {
                    shaderStages[stage_count].pName = desc->tesc_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    shaderStages[stage_count].module = ((CGpuShaderLibrary_Vulkan*)desc->tesc_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case SS_TESE:
            {
                if(desc->tese_shader)
                {
                    shaderStages[stage_count].pName = desc->tese_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    shaderStages[stage_count].module = ((CGpuShaderLibrary_Vulkan*)desc->tese_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case SS_GEOM:
            {
                if(desc->geom_shader)
                {
                    shaderStages[stage_count].pName = desc->geom_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                    shaderStages[stage_count].module = ((CGpuShaderLibrary_Vulkan*)desc->geom_shader->library)->mShaderModule;
                    ++stage_count;
                }
            }
            break;
            case SS_FRAG:
            {
                if(desc->fragment_shader)
                {
                    shaderStages[stage_count].pName = desc->fragment_shader->entry;
                    shaderStages[stage_count].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStages[stage_count].module = ((CGpuShaderLibrary_Vulkan*)desc->fragment_shader->library)->mShaderModule;
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
    
	VkDynamicState dyn_states[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    };
    VkPipelineDynamicStateCreateInfo dys = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = sizeof(dyn_states) / sizeof(dyn_states[0]),
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
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (desc->prim_topology)
    {
        case TOPO_POINT_LIST: topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        case TOPO_LINE_LIST: topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case TOPO_LINE_STRIP: topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
        case TOPO_TRI_STRIP: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
        case TOPO_PATCH_LIST: topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST; break;
        case TOPO_TRI_LIST: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        default:  cgpu_assert(false && "Primitive Topo not supported!"); break;
    }
    VkPipelineInputAssemblyStateCreateInfo ia = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = topology,
        .primitiveRestartEnable = VK_FALSE
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
    DECLARE_ZERO(VkPipelineColorBlendAttachmentState, cb_attachments[MAX_MRT_COUNT])
	int blendDescIndex = 0;
    for (int i = 0; i < MAX_MRT_COUNT; ++i)
	{
        const CGpuBlendStateDescriptor* pDesc = desc->blend_state;
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
        rp_desc.pColorFormats[i] = desc->color_formats[i];
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
        // TODO: Depth stencil state
        .pDepthStencilState = VK_NULL_HANDLE,
        .pColorBlendState = &cbs,
        .layout = RS->pipeline_layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreateGraphicsPipelines(D->pVkDevice,
        D->pPipelineCache, 1, &pipelineInfo, GLOBAL_VkAllocationCallbacks, &RP->pVkPipeline));
    return &RP->super;
}
/* clang-format on */

void cgpu_free_render_pipeline_vulkan(CGpuRenderPipelineId pipeline)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)pipeline->device;
    CGpuRenderPipeline_Vulkan* RP = (CGpuRenderPipeline_Vulkan*)pipeline;
    D->mVkDeviceTable.vkDestroyPipeline(D->pVkDevice, RP->pVkPipeline, GLOBAL_VkAllocationCallbacks);
    cgpu_free(RP);
}

// Queue APIs
CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    cgpu_assert(device && "CGPU VULKAN: NULL DEVICE!");
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;

    CGpuQueue_Vulkan Q = { .super = { .device = &D->super, .index = index, .type = type } };
    D->mVkDeviceTable.vkGetDeviceQueue(D->pVkDevice, (uint32_t)A->mQueueFamilyIndices[type], index, &Q.pVkQueue);
    Q.mVkQueueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[type];
    Q.mTimestampPeriod = A->mPhysicalDeviceProps.properties.limits.timestampPeriod;

    CGpuQueue_Vulkan* RQ = (CGpuQueue_Vulkan*)cgpu_calloc(1, sizeof(CGpuQueue_Vulkan));
    memcpy(RQ, &Q, sizeof(Q));
    return &RQ->super;
}

void cgpu_submit_queue_vulkan(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc)
{
    uint32_t CmdCount = desc->cmds_count;
    CGpuCommandBuffer_Vulkan** Cmds = (CGpuCommandBuffer_Vulkan**)desc->cmds;
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)queue;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)queue->device;
    CGpuFence_Vulkan* F = (CGpuFence_Vulkan*)desc->signal_fence;

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

    // TODO: Add Necessary Semaphores
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = 0,
        .commandBufferCount = CmdCount,
        .pCommandBuffers = vkCmds,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = NULL,
    };
    // TODO: Thread Safety ?
    VkResult res = D->mVkDeviceTable.vkQueueSubmit(Q->pVkQueue, 1, &submit_info, F ? F->pVkFence : VK_NULL_HANDLE);
    CHECK_VKRESULT(res);
    if (F) F->mSubmitted = true;
}

void cgpu_wait_queue_idle_vulkan(CGpuQueueId queue)
{
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)queue;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)queue->device;
    D->mVkDeviceTable.vkQueueWaitIdle(Q->pVkQueue);
}

void cgpu_queue_present_vulkan(CGpuQueueId queue, const struct CGpuQueuePresentDescriptor* desc)
{
    CGpuSwapChain_Vulkan* SC = (CGpuSwapChain_Vulkan*)desc->swapchain;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)queue->device;
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)queue;
    if (SC)
    {
        uint32_t presentIndex = desc->index;
        // TODO: Wait semaphores
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = VK_NULL_HANDLE,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = VK_NULL_HANDLE,
            .swapchainCount = 1,
            .pSwapchains = &SC->pVkSwapChain,
            .pImageIndices = &presentIndex,
            .pResults = VK_NULL_HANDLE
        };
        VkResult vk_res = D->mVkDeviceTable.vkQueuePresentKHR(Q->pVkQueue, &present_info);
        if (vk_res != VK_SUCCESS && vk_res != VK_SUBOPTIMAL_KHR &&
            vk_res != VK_ERROR_OUT_OF_DATE_KHR)
        {
            cgpu_assert(0 && "Present failed!");
        }
    }
}

void cgpu_free_queue_vulkan(CGpuQueueId queue)
{
    cgpu_free((void*)queue);
}

VkCommandPool allocate_transient_command_pool(CGpuDevice_Vulkan* D, CGpuQueueId queue)
{
    VkCommandPool P = VK_NULL_HANDLE;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)queue->device->adapter;

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

void free_transient_command_pool(CGpuDevice_Vulkan* D, VkCommandPool pool)
{
    D->mVkDeviceTable.vkDestroyCommandPool(D->pVkDevice, pool, GLOBAL_VkAllocationCallbacks);
}

CGpuCommandPoolId cgpu_create_command_pool_vulkan(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)queue->device;
    CGpuCommandPool_Vulkan* P = (CGpuCommandPool_Vulkan*)cgpu_calloc(1, sizeof(CGpuCommandPool_Vulkan));
    P->pVkCmdPool = allocate_transient_command_pool(D, queue);
    return &P->super;
}

CGpuCommandBufferId cgpu_create_command_buffer_vulkan(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc)
{
    cgpu_assert(pool);
    CGpuCommandPool_Vulkan* P = (CGpuCommandPool_Vulkan*)pool;
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)P->super.queue;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)Q->super.device;
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cgpu_calloc_aligned(
        1, sizeof(CGpuCommandBuffer_Vulkan), _Alignof(CGpuCommandBuffer_Vulkan));
    cgpu_assert(Cmd);

    Cmd->mType = Q->super.type;
    Cmd->mNodeIndex = SINGLE_GPU_NODE_MASK;

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

void cgpu_reset_command_pool_vulkan(CGpuCommandPoolId pool)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)pool->queue->device;
    CGpuCommandPool_Vulkan* P = (CGpuCommandPool_Vulkan*)pool;
    CHECK_VKRESULT(D->mVkDeviceTable.vkResetCommandPool(D->pVkDevice, P->pVkCmdPool, 0));
}

void cgpu_free_command_buffer_vulkan(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    CGpuCommandPool_Vulkan* P = (CGpuCommandPool_Vulkan*)cmd->pool;
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)P->super.queue;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)Q->super.device;
    D->mVkDeviceTable.vkFreeCommandBuffers(D->pVkDevice, P->pVkCmdPool, 1, &(Cmd->pVkCmdBuf));
    cgpu_free(Cmd);
}

void cgpu_free_command_pool_vulkan(CGpuCommandPoolId pool)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)pool->queue->device;
    CGpuCommandPool_Vulkan* P = (CGpuCommandPool_Vulkan*)pool;
    free_transient_command_pool(D, P->pVkCmdPool);
    cgpu_free(P);
}

// CMDs
void cgpu_cmd_begin_vulkan(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cmd->device;
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkBeginCommandBuffer(Cmd->pVkCmdBuf, &begin_info));
    Cmd->pBoundPipelineLayout = CGPU_NULL;
}

void cgpu_cmd_resource_barrier_vulkan(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cmd->device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)cmd->device->adapter;
    VkAccessFlags srcAccessFlags = 0;
    VkAccessFlags dstAccessFlags = 0;

    DECLARE_ZERO_VLA(VkBufferMemoryBarrier, BBs, desc->buffer_barriers_count)
    uint32_t bufferBarrierCount = 0;
    for (uint32_t i = 0; i < desc->buffer_barriers_count; i++)
    {
        const CGpuBufferBarrier* buffer_barrier = &desc->buffer_barriers[i];
        CGpuBuffer_Vulkan* B = (CGpuBuffer_Vulkan*)buffer_barrier->buffer;
        VkBufferMemoryBarrier* pBufferBarrier = NULL;

        if (RS_UNORDERED_ACCESS == buffer_barrier->src_state && RS_UNORDERED_ACCESS == buffer_barrier->dst_state)
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
                pBufferBarrier->dstQueueFamilyIndex = A->mQueueFamilyIndices[Cmd->mType];
                pBufferBarrier->srcQueueFamilyIndex = A->mQueueFamilyIndices[buffer_barrier->queue_type];
            }
            else if (buffer_barrier->queue_release)
            {
                pBufferBarrier->srcQueueFamilyIndex = A->mQueueFamilyIndices[Cmd->mType];
                pBufferBarrier->dstQueueFamilyIndex = A->mQueueFamilyIndices[buffer_barrier->queue_type];
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
        const CGpuTextureBarrier* texture_barrier = &desc->texture_barriers[i];
        CGpuTexture_Vulkan* T = (CGpuTexture_Vulkan*)texture_barrier->texture;
        VkImageMemoryBarrier* pImageBarrier = NULL;
        if (RS_UNORDERED_ACCESS == texture_barrier->src_state && RS_UNORDERED_ACCESS == texture_barrier->dst_state)
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
            pImageBarrier->subresourceRange.aspectMask = (VkImageAspectFlags)T->super.aspect_mask;
            pImageBarrier->subresourceRange.baseMipLevel = texture_barrier->subresource_barrier ? texture_barrier->mip_level : 0;
            pImageBarrier->subresourceRange.levelCount = texture_barrier->subresource_barrier ? 1 : VK_REMAINING_MIP_LEVELS;
            pImageBarrier->subresourceRange.baseArrayLayer = texture_barrier->subresource_barrier ? texture_barrier->array_layer : 0;
            pImageBarrier->subresourceRange.layerCount = texture_barrier->subresource_barrier ? 1 : VK_REMAINING_ARRAY_LAYERS;

            if (texture_barrier->queue_acquire && texture_barrier->src_state != RS_UNDEFINED)
            {
                pImageBarrier->dstQueueFamilyIndex = A->mQueueFamilyIndices[Cmd->mType];
                pImageBarrier->srcQueueFamilyIndex = A->mQueueFamilyIndices[texture_barrier->queue_type];
            }
            else if (texture_barrier->queue_release && texture_barrier->src_state != RS_UNDEFINED)
            {
                pImageBarrier->srcQueueFamilyIndex = A->mQueueFamilyIndices[Cmd->mType];
                pImageBarrier->dstQueueFamilyIndex = A->mQueueFamilyIndices[texture_barrier->queue_type];
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
        VkUtil_DeterminePipelineStageFlags(A, srcAccessFlags, (ECGpuQueueType)Cmd->mType);
    VkPipelineStageFlags dstStageMask =
        VkUtil_DeterminePipelineStageFlags(A, dstAccessFlags, (ECGpuQueueType)Cmd->mType);
    if (bufferBarrierCount || imageBarrierCount)
    {
        D->mVkDeviceTable.vkCmdPipelineBarrier(Cmd->pVkCmdBuf,
            srcStageMask, dstStageMask, 0,
            0, NULL,
            bufferBarrierCount, BBs,
            imageBarrierCount, TBs);
    }
}

void cgpu_cmd_end_vulkan(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cmd->device;
    CHECK_VKRESULT(D->mVkDeviceTable.vkEndCommandBuffer(Cmd->pVkCmdBuf));
}

// Compute CMDs
CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass_vulkan(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc)
{
    // DO NOTHING NOW
    return (CGpuComputePassEncoderId)cmd;
}

void cgpu_compute_encoder_bind_descriptor_set_vulkan(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId set)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    const CGpuDescriptorSet_Vulkan* Set = (CGpuDescriptorSet_Vulkan*)set;
    const CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)set->root_signature;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)set->root_signature->device;

    // TODO: VK Must Fill All DescriptorSetLayouts at first dispach/draw.
    // Example: If shader uses only set 2, we still have to bind empty sets for set=0 and set=1

    D->mVkDeviceTable.vkCmdBindDescriptorSets(Cmd->pVkCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, RS->pipeline_layout,
        Set->super.index, 1, &Set->pVkDescriptorSet,
        // TODO: Dynamic Offset
        0, NULL);
}

void cgpu_render_encoder_bind_descriptor_set_vulkan(CGpuRenderPassEncoderId encoder, CGpuDescriptorSetId set)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    const CGpuDescriptorSet_Vulkan* Set = (CGpuDescriptorSet_Vulkan*)set;
    const CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)set->root_signature;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)set->root_signature->device;

    // TODO: VK Must Fill All DescriptorSetLayouts at first dispach/draw.
    // Example: If shader uses only set 2, we still have to bind empty sets for set=0 and set=1
    D->mVkDeviceTable.vkCmdBindDescriptorSets(Cmd->pVkCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, RS->pipeline_layout,
        Set->super.index, 1, &Set->pVkDescriptorSet,
        // TODO: Dynamic Offset
        0, NULL);
}

void cgpu_compute_encoder_bind_pipeline_vulkan(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    CGpuComputePipeline_Vulkan* PPL = (CGpuComputePipeline_Vulkan*)pipeline;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)pipeline->device;
    D->mVkDeviceTable.vkCmdBindPipeline(Cmd->pVkCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, PPL->pVkPipeline);
}

void cgpu_compute_encoder_dispatch_vulkan(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)Cmd->super.device;
    D->mVkDeviceTable.vkCmdDispatch(Cmd->pVkCmdBuf, X, Y, Z);
}

void cgpu_cmd_end_compute_pass_vulkan(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder)
{
    // DO NOTHING NOW
}

// Render CMDs
CGpuRenderPassEncoderId cgpu_cmd_begin_render_pass_vulkan(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cmd->device;
    // Find or create render pass
    uint32_t Width, Height;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    {
        VkUtil_RenderPassDesc rpdesc = {
            .mColorAttachmentCount = desc->render_target_count,
            .mDepthStencilFormat = desc->depth_stencil ? desc->depth_stencil->view->info.format : PF_UNDEFINED,
            .mSampleCount = desc->sample_count,
            .mLoadActionDepth = desc->depth_stencil ? desc->depth_stencil->depth_load_action : LA_DONTCARE,
            .mLoadActionStencil = desc->depth_stencil ? desc->depth_stencil->stencil_load_action : LA_DONTCARE
        };
        for (uint32_t i = 0; i < desc->render_target_count; i++)
        {
            rpdesc.pColorFormats[i] = desc->color_attachments[i].view->info.format;
            rpdesc.pLoadActionsColor[i] = desc->color_attachments[i].load_action;
            Width = desc->color_attachments[i].view->info.texture->width;
            Height = desc->color_attachments[i].view->info.texture->height;
        }
        VkUtil_FindOrCreateRenderPass(D, &rpdesc, &render_pass);
    }
    // Find or create framebuffer
    VkFramebuffer pFramebuffer = VK_NULL_HANDLE;
    {
        VkUtil_FramebufferDesc fbDesc = {
            .pRenderPass = render_pass,
            .mColorAttachmentCount = desc->render_target_count,
            .mWidth = Width,
            .mHeight = Height,
            .mLayers = 1
        };
        for (uint32_t i = 0; i < desc->render_target_count; i++)
        {
            CGpuTextureView_Vulkan* TVV = (CGpuTextureView_Vulkan*)desc->color_attachments[i].view;
            fbDesc.pImageViews[i] = TVV->pVkRTVDescriptor;
            fbDesc.mLayers = TVV->super.info.array_layer_count;
        }
        if (desc->depth_stencil != CGPU_NULLPTR)
        {
            CGpuTextureView_Vulkan* TVV = (CGpuTextureView_Vulkan*)desc->depth_stencil->view;
            fbDesc.pImageViews[desc->render_target_count] = TVV->pVkRTVDescriptor;
            fbDesc.mLayers = TVV->super.info.array_layer_count;
        }
        if (desc->render_target_count)
            cgpu_assert(fbDesc.mLayers == 1 && "MRT pass supports only one layer!");
        VkUtil_FindOrCreateFrameBuffer(D, &fbDesc, &pFramebuffer);
    }
    // Cmd begin render pass
    VkClearValue clearValues[MAX_MRT_COUNT + 1];
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        CGpuClearValue clearValue = desc->color_attachments[i].clear_color;
        clearValues[i].color.float32[0] = clearValue.r;
        clearValues[i].color.float32[1] = clearValue.g;
        clearValues[i].color.float32[2] = clearValue.b;
        clearValues[i].color.float32[3] = clearValue.a;
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
        // TODO: Support depth render target clear
        .clearValueCount = desc->render_target_count,
        .pClearValues = clearValues
    };
    D->mVkDeviceTable.vkCmdBeginRenderPass(Cmd->pVkCmdBuf, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    Cmd->pRenderPass = render_pass;
    return (CGpuRenderPassEncoderId)cmd;
}

void cgpu_render_encoder_set_viewport_vulkan(CGpuRenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)encoder->device;
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

void cgpu_render_encoder_set_scissor_vulkan(CGpuRenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)encoder->device;
    VkRect2D scissor = {
        .offset.x = x,
        .offset.y = y,
        .extent.width = width,
        .extent.height = height
    };
    D->mVkDeviceTable.vkCmdSetScissor(Cmd->pVkCmdBuf, 0, 1, &scissor);
}

void cgpu_render_encoder_bind_pipeline_vulkan(CGpuRenderPassEncoderId encoder, CGpuRenderPipelineId pipeline)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    CGpuComputePipeline_Vulkan* PPL = (CGpuComputePipeline_Vulkan*)pipeline;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)pipeline->device;
    D->mVkDeviceTable.vkCmdBindPipeline(Cmd->pVkCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, PPL->pVkPipeline);
}

void cgpu_render_encoder_draw_vulkan(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)encoder->device;
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)encoder;
    D->mVkDeviceTable.vkCmdDraw(Cmd->pVkCmdBuf, vertex_count, 1, first_vertex, 0);
}

void cgpu_cmd_end_render_pass_vulkan(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cmd->device;
    D->mVkDeviceTable.vkCmdEndRenderPass(Cmd->pVkCmdBuf);
    Cmd->pRenderPass = VK_NULL_HANDLE;
}

// SwapChain APIs
#define clamp(x, min, max) (x) < (min) ? (min) : ((x) > (max) ? (max) : (x));
// TODO: Handle multi-queue presenting
CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    // CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)device->adapter->instance;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)desc->presentQueues[0];

    VkSurfaceKHR vkSurface = (VkSurfaceKHR)desc->surface;

    VkSurfaceCapabilitiesKHR caps = { 0 };
    CHECK_VKRESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(A->pPhysicalDevice, vkSurface, &caps));
    if ((caps.maxImageCount > 0) && (desc->imageCount > caps.maxImageCount))
    {
        ((CGpuSwapChainDescriptor*)desc)->imageCount = caps.maxImageCount;
    }
    else if (desc->imageCount < caps.minImageCount)
    {
        ((CGpuSwapChainDescriptor*)desc)->imageCount = caps.minImageCount;
    }

    // Surface format
    // Select a surface format, depending on whether HDR is available.
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
        VkColorSpaceKHR requested_color_space = requested_format == hdrSurfaceFormat.format ? hdrSurfaceFormat.colorSpace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
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
    uint32_t preferredModeStartIndex = desc->enableVsync ? 1 : 0;
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
    uint32_t presentQueueFamilyIndex = -1;
    // Check Queue Present Support.
    {
        VkBool32 sup = VK_FALSE;
        VkResult res =
            vkGetPhysicalDeviceSurfaceSupportKHR(A->pPhysicalDevice, Q->mVkQueueFamilyIndex, vkSurface, &sup);
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
            vkGetPhysicalDeviceQueueFamilyProperties(
                A->pPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties);

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
        .minImageCount = desc->imageCount,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = sharing_mode,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &presentQueueFamilyIndex,
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
    CGpuSwapChain_Vulkan* S = (CGpuSwapChain_Vulkan*)cgpu_calloc(1,
        sizeof(CGpuSwapChain_Vulkan) + sizeof(CGpuTexture_Vulkan) * buffer_count + sizeof(CGpuTextureId) * buffer_count);
    S->pVkSwapChain = new_chain;
    S->super.buffer_count = buffer_count;
    DECLARE_ZERO_VLA(VkImage, vimages, S->super.buffer_count)
    CHECK_VKRESULT(D->mVkDeviceTable.vkGetSwapchainImagesKHR(D->pVkDevice, S->pVkSwapChain, &S->super.buffer_count, vimages));
    CGpuTexture_Vulkan* Ts = (CGpuTexture_Vulkan*)(S + 1);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Ts[i].pVkImage = vimages[i];
        Ts[i].image_type = VK_IMAGE_TYPE_2D;
        // TODO: Create default SRV descriptor
        // Ts[i].pVkSRVDescriptor
        // TODO: Create default UAV descriptor
        Ts[i].super.is_cube = false;
        // Ts[i].pVkUAVDescriptors
        Ts[i].super.array_size_minus_one = 0;
        Ts[i].super.device = &D->super;
        Ts[i].super.format = VkUtil_FormatTranslateToCGPU(surface_format.format);
        Ts[i].super.aspect_mask = VkUtil_DeterminAspectMask(Ts[i].super.format, false);
        Ts[i].super.depth = 1;
        Ts[i].super.width = extent.width;
        Ts[i].super.height = extent.height;
        Ts[i].super.mip_levels = 1;
        Ts[i].super.node_index = SINGLE_GPU_NODE_INDEX;
        Ts[i].super.owns_image = false;
    }
    CGpuTextureId* Vs = (CGpuTextureId*)(Ts + buffer_count);
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        Vs[i] = &Ts[i].super;
    }
    S->super.back_buffers = Vs;
    S->pVkSurface = vkSurface;
    return &S->super;
}

uint32_t cgpu_acquire_next_image_vulkan(CGpuSwapChainId swapchain, const struct CGpuAcquireNextDescriptor* desc)
{
    CGpuFence_Vulkan* Fence = (CGpuFence_Vulkan*)desc->fence;
    CGpuSwapChain_Vulkan* SC = (CGpuSwapChain_Vulkan*)swapchain;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)swapchain->device;

    VkResult vk_res = VK_SUCCESS;
    uint32_t idx;

    VkSemaphore vsemaphore = VK_NULL_HANDLE;
    VkFence vfence = Fence ? Fence->pVkFence : VK_NULL_HANDLE;

    vk_res = vkAcquireNextImageKHR(
        D->pVkDevice, SC->pVkSwapChain,
        UINT64_MAX,
        vsemaphore, // sem
        vfence,     // fence
        &idx);

    if (Fence) Fence->mSubmitted = true;
    // If swapchain is out of date, let caller know by setting image index to -1
    if (vk_res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        idx = -1;
    }
    return idx;
}

void cgpu_free_swapchain_vulkan(CGpuSwapChainId swapchain)
{
    CGpuSwapChain_Vulkan* S = (CGpuSwapChain_Vulkan*)swapchain;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)swapchain->device;

    D->mVkDeviceTable.vkDestroySwapchainKHR(D->pVkDevice, S->pVkSwapChain, GLOBAL_VkAllocationCallbacks);

    cgpu_free((void*)swapchain);
}

// exts
#include "cgpu/extensions/cgpu_vulkan_exts.h"