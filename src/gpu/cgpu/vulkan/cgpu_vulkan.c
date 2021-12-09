#include "cgpu/api.h"
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "cgpu/cgpu_config.h"
#include "cgpu/flags.h"
#include "platform/configure.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_utils.h"
#include "cgpu/shader-reflections/spirv/spirv_reflect.h"
#include <stdint.h>
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
    .free_fence = &cgpu_free_fence_vulkan,
    .create_root_signature = &cgpu_create_root_signature_vulkan,
    .free_root_signature = &cgpu_free_root_signature_vulkan,
    .create_descriptor_set = &cgpu_create_descriptor_set_vulkan,
    .update_descriptor_set = &cgpu_update_descriptor_set_vulkan,
    .free_descriptor_set = &cgpu_free_descriptor_set_vulkan,
    .create_compute_pipeline = &cgpu_create_compute_pipeline_vulkan,
    .free_compute_pipeline = &cgpu_free_compute_pipeline_vulkan,

    // Queue APIs
    .get_queue = &cgpu_get_queue_vulkan,
    .submit_queue = &cgpu_submit_queue_vulkan,
    .wait_queue_idle = &cgpu_wait_queue_idle_vulkan,
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

    // Swapchain APIs
    .create_swapchain = &cgpu_create_swapchain_vulkan,
    .free_swapchain = &cgpu_free_swapchain_vulkan,

    // CMDs
    .cmd_begin = &cgpu_cmd_begin_vulkan,
    .cmd_update_buffer = &cgpu_cmd_update_buffer_vulkan,
    .cmd_resource_barrier = &cgpu_cmd_resource_barrier_vulkan,
    .cmd_end = &cgpu_cmd_end_vulkan,
    .cmd_begin_compute_pass = &cgpu_cmd_begin_compute_pass_vulkan,
    .compute_encoder_bind_descriptor_set = &cgpu_compute_encoder_bind_descriptor_set_vulkan,
    .compute_encoder_bind_pipeline = &cgpu_compute_encoder_bind_pipeline_vulkan,
    .compute_encoder_dispatch = &cgpu_compute_encoder_dispatch_vulkan,
    .cmd_end_compute_pass = &cgpu_cmd_end_compute_pass_vulkan
};

const CGpuProcTable* CGPU_VulkanProcTable() { return &tbl_vk; }

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
    CGpuAdapter_Vulkan* a = (CGpuAdapter_Vulkan*)adapter;
    return &a->adapter_detail;
}

uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    CGpuAdapter_Vulkan* a = (CGpuAdapter_Vulkan*)adapter;
    uint32_t count = 0;
    switch (type)
    {
        case ECGpuQueueType_Graphics: {
            for (uint32_t i = 0; i < a->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &a->pQueueFamilyProperties[i];
                if (prop->queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    count += prop->queueCount;
                }
            }
        }
        break;
        case ECGpuQueueType_Compute: {
            for (uint32_t i = 0; i < a->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &a->pQueueFamilyProperties[i];
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
            for (uint32_t i = 0; i < a->mQueueFamiliesCount; i++)
            {
                const VkQueueFamilyProperties* prop = &a->pQueueFamilyProperties[i];
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
            assert(0 && "CGPU VULKAN: ERROR Queue Type!");
    }
    return count;
}

// API Objects APIs
CGpuFenceId cgpu_create_fence_vulkan(CGpuDeviceId device)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuFence_Vulkan* F = (CGpuFence_Vulkan*)cgpu_calloc(1, sizeof(CGpuFence_Vulkan));
    assert(F);
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
    DECLARE_ZERO_VLA(CGpuShaderReflection*, entry_reflections, desc->shaders_count)
    // pick shader reflection data
    for (uint32_t i = 0; i < desc->shaders_count; i++)
    {
        const CGpuPipelineShaderDescriptor* shader_desc = &desc->shaders[i];
        // find shader refl
        for (uint32_t j = 0; j < shader_desc->library->entrys_count; j++)
        {
            CGpuShaderReflection* temp_entry_reflcetion = &shader_desc->library->entry_reflections[j];
            if (strcmp(shader_desc->entry, temp_entry_reflcetion->entry_name) == 0)
            {
                entry_reflections[i] = temp_entry_reflcetion;
                break;
            }
        }
        if (entry_reflections[i] == CGPU_NULLPTR)
        {
            assert(0 && "Shader Entry Not Found!");
        }
    }

    // Count Sets and Binding Slots
    uint32_t set_count = 0;
    uint32_t max_binding = 0;
    ECGpuPipelineType pipelineType = PT_NONE;
    for (uint32_t i = 0; i < desc->shaders_count; i++)
    {
        CGpuShaderReflection* reflection = entry_reflections[i];
        for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
        {
            CGpuShaderResource* resource = &reflection->shader_resources[j];
            set_count = set_count > resource->set + 1 ? set_count : resource->set + 1;
            max_binding = max_binding > resource->binding + 1 ? max_binding : resource->binding + 1;
        }
        // Pipeline Type
        if (reflection->stage & SS_COMPUTE)
            pipelineType = PT_COMPUTE;
#ifdef ENABLE_RAYTRACING
        else if (reflection->stage & SS_RAYTRACING)
            pipelineType = PT_RAYTRACING;
#endif
        else
            pipelineType = PT_GRAPHICS;
    }
    RS->set_count = set_count;

    // Collect Shader Resources
    if (set_count * max_binding > 0)
    {
        // Record On Temporal Memory
        CGpuShaderResource* sig_reflections = (CGpuShaderResource*)cgpu_calloc(set_count * max_binding, sizeof(CGpuShaderResource));
        uint32_t* valid_bindings = (uint32_t*)cgpu_calloc(set_count, sizeof(uint32_t));
        for (uint32_t i = 0; i < desc->shaders_count; i++)
        {
            CGpuShaderReflection* reflection = entry_reflections[i];
            for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
            {
                CGpuShaderResource* resource = &reflection->shader_resources[j];
                memcpy(&sig_reflections[resource->set * resource->binding], resource, sizeof(CGpuShaderResource));
                valid_bindings[resource->set] =
                    valid_bindings[resource->set] > resource->binding + 1 ? valid_bindings[resource->set] : resource->binding + 1;
            }
        }
        // Collect Secure Set Count
        RS->parameter_sets = (ParameterSet_Vulkan*)cgpu_calloc(set_count, sizeof(ParameterSet_Vulkan));
        for (uint32_t i_set = 0; i_set < RS->set_count; i_set++)
        {
            ParameterSet_Vulkan* set_to_record = &RS->parameter_sets[i_set];
            set_to_record->resources_count = valid_bindings[i_set];
            set_to_record->resources = cgpu_calloc(valid_bindings[i_set], sizeof(CGpuShaderResource));
            for (uint32_t i_binding = 0; i_binding < valid_bindings[i_set]; i_binding++)
            {
                const size_t source_len = strlen(sig_reflections[i_set * i_binding + i_binding].name);
                set_to_record->resources[i_binding].name = (char8_t*)cgpu_malloc(sizeof(char8_t) * (1 + source_len));
#ifdef _WIN32
                strcpy_s((char8_t*)set_to_record->resources[i_binding].name, source_len + 1, sig_reflections[i_set * i_binding + i_binding].name);
#else
                strcpy((char8_t*)set_to_record->resources[i_binding].name, sig_reflections[i_set * i_binding + i_binding].name);
#endif
                set_to_record->resources[i_binding].type = sig_reflections[i_set * i_binding + i_binding].type;
                set_to_record->resources[i_binding].set = sig_reflections[i_set * i_binding + i_binding].set;
                set_to_record->resources[i_binding].binding = sig_reflections[i_set * i_binding + i_binding].binding;
                set_to_record->resources[i_binding].size = sig_reflections[i_set * i_binding + i_binding].size;
                set_to_record->resources[i_binding].stages = sig_reflections[i_set * i_binding + i_binding].stages;
                set_to_record->resources[i_binding].name_hash = sig_reflections[i_set * i_binding + i_binding].name_hash;
            }
        }
        // Create Vk Objects
        for (uint32_t i_set = 0; i_set < RS->set_count; i_set++)
        {
            ParameterSet_Vulkan* set_to_record = &RS->parameter_sets[i_set];
            VkDescriptorSetLayoutBinding* vkbindings = (VkDescriptorSetLayoutBinding*)cgpu_calloc(set_to_record->resources_count, sizeof(VkDescriptorSetLayoutBinding));
            for (uint32_t i_binding = 0; i_binding < set_to_record->resources_count; i_binding++)
            {
                vkbindings[i_binding].binding = set_to_record->resources[i_binding].binding;
                vkbindings[i_binding].stageFlags = VkUtil_TranslateShaderUsages(set_to_record->resources[i_binding].stages);
                vkbindings[i_binding].descriptorType = VkUtil_TranslateResourceType(set_to_record->resources[i_binding].type);
                vkbindings[i_binding].descriptorCount = set_to_record->resources[i_binding].size;
            }
            VkDescriptorSetLayoutCreateInfo set_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .pBindings = vkbindings,
                .bindingCount = set_to_record->resources_count
            };
            CHECK_VKRESULT(D->mVkDeviceTable.vkCreateDescriptorSetLayout(D->pVkDevice, &set_info, GLOBAL_VkAllocationCallbacks, &set_to_record->layout));
            cgpu_free(vkbindings);
        }
        // Free Temporal Memory
        cgpu_free(sig_reflections);
        cgpu_free(valid_bindings);
    }
    // Record Descriptor Sets
    VkDescriptorSetLayout* set_layouts = cgpu_calloc(RS->set_count + 1, sizeof(VkDescriptorSetLayout));
    for (uint32_t i_set = 0; i_set < RS->set_count; i_set++)
    {
        ParameterSet_Vulkan* set_to_record = &RS->parameter_sets[i_set];
        set_layouts[i_set] = set_to_record->layout;
    }
    // Create Pipeline Layout
    VkPipelineLayoutCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = RS->set_count,
        .pSetLayouts = set_layouts,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };
    CHECK_VKRESULT(D->mVkDeviceTable.vkCreatePipelineLayout(D->pVkDevice,
        &pipeline_info, GLOBAL_VkAllocationCallbacks, &RS->pipeline_layout));

    // Create Update Templates
    for (uint32_t i_set = 0; i_set < RS->set_count; i_set++)
    {
        ParameterSet_Vulkan* set_to_record = &RS->parameter_sets[i_set];
        VkDescriptorUpdateTemplateEntry* template_entries = (VkDescriptorUpdateTemplateEntry*)cgpu_calloc(
            set_to_record->resources_count, sizeof(VkDescriptorUpdateTemplateEntry));
        for (uint32_t i_binding = 0; i_binding < set_to_record->resources_count; i_binding++)
        {
            template_entries[i_binding].descriptorCount = set_to_record->resources[i_binding].size;
            template_entries[i_binding].descriptorType = VkUtil_TranslateResourceType(set_to_record->resources[i_binding].type);
            template_entries[i_binding].dstArrayElement = 0;
            template_entries[i_binding].dstBinding = set_to_record->resources[i_binding].binding;
        }
        VkDescriptorUpdateTemplateCreateInfo template_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
            .pNext = NULL,
            .templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET_KHR,
            .pipelineLayout = RS->pipeline_layout,
            .pipelineBindPoint = gPipelineBindPoint[pipelineType],
            .descriptorSetLayout = set_to_record->layout,
            .set = i_set,
            .pDescriptorUpdateEntries = template_entries,
            .descriptorUpdateEntryCount = set_to_record->resources_count
        };
        CHECK_VKRESULT(D->mVkDeviceTable.vkCreateDescriptorUpdateTemplate(D->pVkDevice,
            &template_info, GLOBAL_VkAllocationCallbacks, &set_to_record->update_template));
    }
    // Free Temporal Memory
    cgpu_free(set_layouts);
    return &RS->super;
}

void cgpu_free_root_signature_vulkan(CGpuRootSignatureId signature)
{
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)signature;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)signature->device;
    // Free Reflection Data
    if (RS->parameter_sets != CGPU_NULLPTR)
    {
        for (uint32_t i_set = 0; i_set < RS->set_count; i_set++)
        {
            ParameterSet_Vulkan* set_to_free = &RS->parameter_sets[i_set];
            if (set_to_free->resources != CGPU_NULLPTR)
            {
                for (uint32_t i_binding = 0; i_binding < set_to_free->resources_count; i_binding++)
                {
                    CGpuShaderResource* binding_to_free = &set_to_free->resources[i_binding];
                    if (binding_to_free->name != CGPU_NULLPTR)
                    {
                        cgpu_free((char8_t*)binding_to_free->name);
                    }
                }
                if (set_to_free->layout != VK_NULL_HANDLE)
                    D->mVkDeviceTable.vkDestroyDescriptorSetLayout(D->pVkDevice, set_to_free->layout, GLOBAL_VkAllocationCallbacks);
                if (set_to_free->update_template != VK_NULL_HANDLE)
                    D->mVkDeviceTable.vkDestroyDescriptorUpdateTemplate(D->pVkDevice, set_to_free->update_template, GLOBAL_VkAllocationCallbacks);
                cgpu_free(RS->parameter_sets[i_set].resources);
            }
        }
        cgpu_free(RS->parameter_sets);
    }
    D->mVkDeviceTable.vkDestroyPipelineLayout(D->pVkDevice, RS->pipeline_layout, GLOBAL_VkAllocationCallbacks);
    cgpu_free(RS);
}

CGpuDescriptorSetId cgpu_create_descriptor_set_vulkan(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc)
{
    size_t totalSize = sizeof(CGpuDescriptorSet_Vulkan);
    CGpuRootSignature_Vulkan* RS = (CGpuRootSignature_Vulkan*)desc->root_signature;
    const CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;

    const size_t UpdateTemplateSize = RS->set_count * sizeof(VkDescriptorUpdateData);
    totalSize += UpdateTemplateSize;
    CGpuDescriptorSet_Vulkan* Set = cgpu_calloc_aligned(1, totalSize, _Alignof(CGpuDescriptorSet_Vulkan));
    char8_t* pMem = (char8_t*)(Set + 1);
    // Allocate Descriptor Set
    VkUtil_ConsumeDescriptorSets(D->pDescriptorPool, &RS->parameter_sets[desc->set_index].layout, &Set->pVkDescriptorSet, 1);
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
    ParameterSet_Vulkan* SetLayout = RS->parameter_sets + set->index;
    for (uint32_t i = 0; i < count; i++)
    {
        // Descriptor Info
        const CGpuDescriptorData* ArgData = datas + i;
        CGpuShaderResource* ResData = CGPU_NULLPTR;
        size_t argNameHash = cgpu_hash(ArgData->name, strlen(ArgData->name), (size_t)D);
        for (uint32_t p = 0; p < SetLayout->resources_count; p++)
        {
            if (SetLayout->resources[p].name_hash == argNameHash)
            {
                ResData = SetLayout->resources + p;
            }
        }
        // Update Info
        const CGpuDescriptorData* pParam = datas + i;
        const uint32_t arrayCount = cgpu_max(1U, pParam->count);
        switch (ResData->type)
        {
            case RT_BUFFER:
            case RT_BUFFER_RAW:
            case RT_RW_BUFFER:
            case RT_RW_BUFFER_RAW: {
                assert(ArgData->buffers && "ASSERT: Binding NULL Buffer(s)!");
                CGpuBuffer_Vulkan** Buffers = (CGpuBuffer_Vulkan**)ArgData->buffers;
                for (uint32_t arr = 0; arr < arrayCount; ++arr)
                {
                    assert(ArgData->buffers[arr] && "ASSERT: Binding NULL Buffer!");
                    VkDescriptorUpdateData* Data = &pUpdateData[ResData->binding + arr];
                    Data->mBufferInfo.buffer = Buffers[arr]->pVkBuffer;
                    Data->mBufferInfo.offset = Buffers[arr]->mOffset;
                    Data->mBufferInfo.range = VK_WHOLE_SIZE;
                    if (ArgData->buffers_params.offsets)
                    {
                        Data->mBufferInfo.offset = ArgData->buffers_params.offsets[arr];
                        Data->mBufferInfo.range = ArgData->buffers_params.sizes[arr];
                    }
                    dirty = true;
                }
                break;
            }
            default:
                break;
        }
        if (dirty)
        {
            D->mVkDeviceTable.vkUpdateDescriptorSetWithTemplateKHR(D->pVkDevice, Set->pVkDescriptorSet, SetLayout->update_template, pUpdateData);
        }
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

// Queue APIs
CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    assert(device && "CGPU VULKAN: NULL DEVICE!");
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

    // ASSERT that given cmd list and given params are valid
    assert(CmdCount > 0);
    assert(Cmds);
    // execute given command list
    assert(Q->pVkQueue != VK_NULL_HANDLE);

    DECLARE_ZERO_VLA(VkCommandBuffer, vkcmds, CmdCount);
    for (uint32_t i = 0; i < CmdCount; ++i)
    {
        vkcmds[i] = Cmds[i]->pVkCmdBuf;
    }

    // TODO: Add Necessary Semaphores
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = 0,
        .commandBufferCount = CmdCount,
        .pCommandBuffers = vkcmds,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = NULL,
    };
    // TODO: Thread Safety ?
    CHECK_VKRESULT(D->mVkDeviceTable.vkQueueSubmit(
        Q->pVkQueue, 1, &submit_info, F ? F->pVkFence : VK_NULL_HANDLE));
    if (F)
        F->mSubmitted = true;
}

void cgpu_wait_queue_idle_vulkan(CGpuQueueId queue)
{
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)queue;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)queue->device;
    D->mVkDeviceTable.vkQueueWaitIdle(Q->pVkQueue);
}

void cgpu_free_queue_vulkan(CGpuQueueId queue)
{
    cgpu_free((void*)queue);
}

VkCommandPool allocate_transient_command_pool(CGpuDevice_Vulkan* D, CGpuQueueId queue)
{
    VkCommandPool P = VK_NULL_HANDLE;
    // CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)queue;
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
    assert(pool);
    CGpuCommandPool_Vulkan* P = (CGpuCommandPool_Vulkan*)pool;
    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)P->super.queue;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)Q->super.device;
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cgpu_calloc_aligned(
        1, sizeof(CGpuCommandBuffer_Vulkan), _Alignof(CGpuCommandBuffer_Vulkan));
    assert(Cmd);

    Cmd->pCmdPool = P;
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
    DECLARE_ZERO_VLA(VkBufferMemoryBarrier, BBs, desc->buffer_barriers_count)
    VkAccessFlags srcAccessFlags = 0;
    VkAccessFlags dstAccessFlags = 0;
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
                pBufferBarrier->srcQueueFamilyIndex = A->mQueueFamilyIndices[buffer_barrier->queue_type];
                pBufferBarrier->dstQueueFamilyIndex = A->mQueueFamilyIndices[Cmd->mType];
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

        VkPipelineStageFlags srcStageMask =
            VkUtil_DeterminePipelineStageFlags(A, srcAccessFlags, (ECGpuQueueType)Cmd->mType);
        VkPipelineStageFlags dstStageMask =
            VkUtil_DeterminePipelineStageFlags(A, dstAccessFlags, (ECGpuQueueType)Cmd->mType);

        if (bufferBarrierCount)
        {
            D->mVkDeviceTable.vkCmdPipelineBarrier(Cmd->pVkCmdBuf,
                srcStageMask, dstStageMask, 0,
                0, NULL,
                bufferBarrierCount, BBs,
                0 /*TODO*/, NULL);
        }
    }
}

void cgpu_cmd_end_vulkan(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_Vulkan* Cmd = (CGpuCommandBuffer_Vulkan*)cmd;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cmd->device;
    if (Cmd->pVkActiveRenderPass)
    {
        vkCmdEndRenderPass(Cmd->pVkCmdBuf);
    }
    Cmd->pVkActiveRenderPass = VK_NULL_HANDLE;
    CHECK_VKRESULT(D->mVkDeviceTable.vkEndCommandBuffer(Cmd->pVkCmdBuf));
}

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

// SwapChain APIs
#define clamp(x, min, max) (x) < (min) ? (min) : ((x) > (max) ? (max) : (x));
CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;
    // CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)device->adapter->instance;
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

    const VkSurfaceFormatKHR hdrSurfaceFormat = {
        //
        VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        VK_COLOR_SPACE_HDR10_ST2084_EXT
        //
    };
    if ((1 == surfaceFormatCount) && (VK_FORMAT_UNDEFINED == formats[0].format))
    {
        surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        VkFormat requested_format = VkUtil_FormatTranslateToVk(desc->format);
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
    assert(VK_FORMAT_UNDEFINED != surface_format.format);

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
            if (modes[i] == mode)
            {
                break;
            }
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

    CGpuQueue_Vulkan* Q = (CGpuQueue_Vulkan*)desc->presentQueues[0];
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
                            assert(0);
                        }
                    }
                }
            }
        }
    }

    VkSurfaceTransformFlagBitsKHR pre_transform;
    // #TODO: Add more if necessary but identity should be enough for now
    if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        pre_transform = caps.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] = {
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
    assert(composite_alpha != VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR);

    CGpuSwapChain_Vulkan* S = (CGpuSwapChain_Vulkan*)cgpu_calloc(1, sizeof(CGpuSwapChain_Vulkan));
    S->super.device = device;
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
    VkResult res = D->mVkDeviceTable.vkCreateSwapchainKHR(
        D->pVkDevice, &swapChainCreateInfo, GLOBAL_VkAllocationCallbacks, &S->pVkSwapChain);
    if (VK_SUCCESS != res)
    {
        assert(0 && "fatal: vkCreateSwapchainKHR failed!");
    }
    S->pVkSurface = vkSurface;
    return &S->super;
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