#include "vulkan_utils.h"
#include "cgpu/drivers/cgpu_ags.h"
#include "cgpu/drivers/cgpu_nvapi.h"
#include "vulkan/vulkan_core.h"
#include <stdio.h>
#include <string.h>

// Debug Callback
VKAPI_ATTR VkBool32 VKAPI_CALL
VkUtil_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            printf("[verbose]");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            printf("[info]");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            printf("[warning]");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            printf("[error]");
            break;
        default:
            return VK_TRUE;
    }
    printf(" validation layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL
VkUtil_DebugReportCallback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    switch (flags)
    {
        case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
            printf("[perf-warn]");
            break;
        case VK_DEBUG_REPORT_WARNING_BIT_EXT:
            printf("[warning]");
            break;
        case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
            printf("[debug]");
            break;
        case VK_DEBUG_REPORT_ERROR_BIT_EXT:
            printf("[error]");
            break;
        default:
            return VK_TRUE;
    }
    printf(" validation layer: %s\n", pMessage);
    return VK_FALSE;
}

bool VkUtil_InitializeEnvironment(struct CGpuInstance* Inst)
{
    // AGS
    bool AGS_started = false;
    AGS_started = (cgpu_ags_init(Inst) == CGPU_AGS_SUCCESS);
    (void)AGS_started;
    // NVAPI
    bool NVAPI_started = false;
    NVAPI_started = (cgpu_nvapi_init(Inst) == CGPU_NVAPI_OK);
    (void)NVAPI_started;
    // VOLK
#if !defined(NX64)
    VkResult volkInit = volkInitialize();
    if (volkInit != VK_SUCCESS)
    {
        assert((volkInit == VK_SUCCESS) && "Volk Initialize Failed!");
        return false;
    }
#endif
    return true;
}

void VkUtil_DeInitializeEnvironment(struct CGpuInstance* Inst)
{
    cgpu_ags_exit();
    Inst->ags_status = CGPU_AGS_NONE;
    cgpu_nvapi_exit();
    Inst->nvapi_status = CGPU_NVAPI_NONE;
}

// Instance APIs
void VkUtil_EnableValidationLayer(
    CGpuInstance_Vulkan* I,
    const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr,
    const VkDebugReportCallbackCreateInfoEXT* report_info_ptr)
{
    if (I->debug_utils)
    {
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pfnUserCallback = VkUtil_DebugCallback,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .flags = 0,
            .pUserData = NULL
        };
        const VkDebugUtilsMessengerCreateInfoEXT* messengerInfoPtr =
            (messenger_info_ptr != CGPU_NULLPTR) ? messenger_info_ptr : &messengerInfo;

        assert(vkCreateDebugUtilsMessengerEXT && "Load vkCreateDebugUtilsMessengerEXT failed!");
        VkResult res = vkCreateDebugUtilsMessengerEXT(I->pVkInstance,
            messengerInfoPtr, GLOBAL_VkAllocationCallbacks,
            &(I->pVkDebugUtilsMessenger));
        if (VK_SUCCESS != res)
        {
            assert(0 && "vkCreateDebugUtilsMessengerEXT failed - disabling Vulkan debug callbacks");
        }
    }
    else if (I->debug_report)
    {
        VkDebugReportCallbackCreateInfoEXT reportInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
            .pNext = NULL,
            .pfnCallback = VkUtil_DebugReportCallback,
            .flags =
#if defined(NX64) || defined(__ANDROID__)
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | // Performance warnings are not very vaild on desktop
#endif
                VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT /* | VK_DEBUG_REPORT_INFORMATION_BIT_EXT*/
        };
        const VkDebugReportCallbackCreateInfoEXT* reportInfoPtr =
            (report_info_ptr != CGPU_NULLPTR) ? report_info_ptr : &reportInfo;
        VkResult res = vkCreateDebugReportCallbackEXT(I->pVkInstance,
            reportInfoPtr, GLOBAL_VkAllocationCallbacks,
            &(I->pVkDebugReport));
        assert(vkCreateDebugUtilsMessengerEXT && "Load vkCreateDebugReportCallbackEXT failed!");
        if (VK_SUCCESS != res)
        {
            assert(0 && "vkCreateDebugReportCallbackEXT failed - disabling Vulkan debug callbacks");
        }
    }
}
#ifdef __clang__
#endif
void VkUtil_QueryAllAdapters(CGpuInstance_Vulkan* I,
    const char* const* device_layers, uint32_t device_layers_count,
    const char* const* device_extensions, uint32_t device_extension_count)
{
    assert((I->mPhysicalDeviceCount == 0) && "VkUtil_QueryAllAdapters should only be called once!");

    vkEnumeratePhysicalDevices(I->pVkInstance, &I->mPhysicalDeviceCount, CGPU_NULLPTR);
    if (I->mPhysicalDeviceCount != 0)
    {
        I->pVulkanAdapters =
            (CGpuAdapter_Vulkan*)cgpu_calloc(I->mPhysicalDeviceCount, sizeof(CGpuAdapter_Vulkan));
        DECLARE_ZERO_VLA(VkPhysicalDevice, pysicalDevices, I->mPhysicalDeviceCount)
        vkEnumeratePhysicalDevices(I->pVkInstance, &I->mPhysicalDeviceCount,
            pysicalDevices);
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++)
        {
            // Alloc & Zero Adapter
            CGpuAdapter_Vulkan* VkAdapter = &I->pVulkanAdapters[i];
            for (uint32_t q = 0; q < ECGpuQueueType_Count; q++)
            {
                VkAdapter->mQueueFamilyIndices[q] = -1;
            }
            VkAdapter->super.instance = &I->super;
            VkAdapter->pPhysicalDevice = pysicalDevices[i];
            // Query Physical Device Properties
            VkAdapter->mSubgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
            VkAdapter->mSubgroupProperties.pNext = NULL;
            VkAdapter->mPhysicalDeviceProps.pNext = &VkAdapter->mSubgroupProperties;
            VkAdapter->mPhysicalDeviceProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
            vkGetPhysicalDeviceProperties2(pysicalDevices[i], &VkAdapter->mPhysicalDeviceProps);
            // Query Physical Device Features
            VkAdapter->mPhysicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
#ifndef NX64
            vkGetPhysicalDeviceFeatures2KHR(pysicalDevices[i], &VkAdapter->mPhysicalDeviceFeatures);
#else
            vkGetPhysicalDeviceFeatures2(pysicalDevices[i], &VkAdapter->mPhysicalDeviceFeatures);
#endif
            // Enumerate Format Supports
            VkUtil_EnumFormatSupports(VkAdapter);
            // Query Physical Device Layers Properties
            VkUtil_SelectPhysicalDeviceLayers(VkAdapter, device_layers, device_layers_count);
            // Query Physical Device Extension Properties
            VkUtil_SelectPhysicalDeviceExtensions(VkAdapter, device_extensions, device_extension_count);
            // Select Queue Indices
            VkUtil_SelectQueueIndices(VkAdapter);
            // Record Adapter Detail
            VkUtil_RecordAdapterDetail(VkAdapter);
        }
    }
    else
    {
        assert(0 && "Vulkan: 0 physical device avalable!");
    }
}

// Device APIs
void VkUtil_CreatePipelineCache(CGpuDevice_Vulkan* D)
{
    assert((D->pPipelineCache == VK_NULL_HANDLE) && "VkUtil_CreatePipelineCache should be called only once!");

    // TODO: serde
    VkPipelineCacheCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .pNext = NULL,
        .initialDataSize = 0,
        .pInitialData = NULL
    };
    D->mVkDeviceTable.vkCreatePipelineCache(D->pVkDevice,
        &info, GLOBAL_VkAllocationCallbacks, &D->pPipelineCache);
}

void VkUtil_CreateVMAAllocator(CGpuInstance_Vulkan* I, CGpuAdapter_Vulkan* A,
    CGpuDevice_Vulkan* D)
{
    VmaVulkanFunctions vulkanFunctions = {
        .vkAllocateMemory = vkAllocateMemory,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkCreateBuffer = vkCreateBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkDestroyImage = vkDestroyImage,
        .vkFreeMemory = vkFreeMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkCmdCopyBuffer = vkCmdCopyBuffer
    };

    VmaAllocatorCreateInfo vmaInfo = {
        .device = D->pVkDevice,
        .physicalDevice = A->pPhysicalDevice,
        .instance = I->pVkInstance,
        .pVulkanFunctions = &vulkanFunctions,
        .pAllocationCallbacks = GLOBAL_VkAllocationCallbacks
    };
    if (A->dedicated_allocation)
    {
        vmaInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    }
    if (vmaCreateAllocator(&vmaInfo, &D->pVmaAllocator) != VK_SUCCESS)
    {
        assert(0);
    }
}

void VkUtil_FreeVMAAllocator(CGpuInstance_Vulkan* I, CGpuAdapter_Vulkan* A, CGpuDevice_Vulkan* D)
{
    vmaDestroyAllocator(D->pVmaAllocator);
}

void VkUtil_FreePipelineCache(CGpuInstance_Vulkan* I, CGpuAdapter_Vulkan* A, CGpuDevice_Vulkan* D)
{
    if (D->pPipelineCache != VK_NULL_HANDLE)
    {
        D->mVkDeviceTable.vkDestroyPipelineCache(
            D->pVkDevice, D->pPipelineCache, GLOBAL_VkAllocationCallbacks);
    }
}

// Select Helpers
void VkUtil_RecordAdapterDetail(CGpuAdapter_Vulkan* VkAdapter)
{
    CGpuAdapterDetail* adapter_detail = &VkAdapter->adapter_detail;
    VkPhysicalDeviceProperties* prop = &VkAdapter->mPhysicalDeviceProps.properties;
    // Vendor Info
    adapter_detail->vendor_preset.device_id = prop->deviceID;
    adapter_detail->vendor_preset.vendor_id = prop->vendorID;
    adapter_detail->vendor_preset.driver_version = prop->driverVersion;
    const char* device_name = prop->deviceName;
    memcpy(adapter_detail->vendor_preset.gpu_name, device_name, strlen(device_name));

    // Some Features
    adapter_detail->uniform_buffer_alignment =
        (uint32_t)prop->limits.minUniformBufferOffsetAlignment;
    adapter_detail->upload_buffer_texture_alignment =
        (uint32_t)prop->limits.optimalBufferCopyOffsetAlignment;
    adapter_detail->upload_buffer_texture_row_alignment =
        (uint32_t)prop->limits.optimalBufferCopyRowPitchAlignment;
    adapter_detail->max_vertex_input_bindings = prop->limits.maxVertexInputBindings;
    adapter_detail->multidraw_indirect = prop->limits.maxDrawIndirectCount > 1;
    adapter_detail->wave_lane_count = VkAdapter->mSubgroupProperties.subgroupSize;
}

void VkUtil_SelectQueueIndices(CGpuAdapter_Vulkan* VkAdapter)
{
    // Query Queue Information.
    vkGetPhysicalDeviceQueueFamilyProperties(
        VkAdapter->pPhysicalDevice, &VkAdapter->mQueueFamiliesCount,
        CGPU_NULLPTR);
    VkAdapter->pQueueFamilyProperties = (VkQueueFamilyProperties*)cgpu_calloc(
        VkAdapter->mQueueFamiliesCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(VkAdapter->pPhysicalDevice,
        &VkAdapter->mQueueFamiliesCount, VkAdapter->pQueueFamilyProperties);

    for (uint32_t j = 0; j < VkAdapter->mQueueFamiliesCount; j++)
    {
        const VkQueueFamilyProperties* prop = &VkAdapter->pQueueFamilyProperties[j];
        if ((VkAdapter->mQueueFamilyIndices[ECGpuQueueType_Graphics] == -1) &&
            (prop->queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            VkAdapter->mQueueFamilyIndices[ECGpuQueueType_Graphics] = j;
        }
        else if ((VkAdapter->mQueueFamilyIndices[ECGpuQueueType_Compute] == -1) &&
                 (prop->queueFlags & VK_QUEUE_COMPUTE_BIT))
        {
            VkAdapter->mQueueFamilyIndices[ECGpuQueueType_Compute] = j;
        }
        else if ((VkAdapter->mQueueFamilyIndices[ECGpuQueueType_Transfer] == -1) &&
                 (prop->queueFlags & VK_QUEUE_TRANSFER_BIT))
        {
            VkAdapter->mQueueFamilyIndices[ECGpuQueueType_Transfer] = j;
        }
    }
}

void VkUtil_EnumFormatSupports(CGpuAdapter_Vulkan* VkAdapter)
{
    for (uint32_t i = 0; i < PF_Count; ++i)
    {
        VkFormatProperties formatSupport;
        CGpuAdapterDetail* adapter_detail = (CGpuAdapterDetail*)&VkAdapter->adapter_detail;
        adapter_detail->format_supports[i].shader_read = 0;
        adapter_detail->format_supports[i].shader_write = 0;
        adapter_detail->format_supports[i].render_target_write = 0;
        VkFormat fmt = (VkFormat)pf_translate_to_vulkan((ECGpuPixelFormat)i);
        if (fmt == VK_FORMAT_UNDEFINED) continue;

        vkGetPhysicalDeviceFormatProperties(VkAdapter->pPhysicalDevice, fmt, &formatSupport);
        adapter_detail->format_supports[i].shader_read =
            (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
        adapter_detail->format_supports[i].shader_write =
            (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
        adapter_detail->format_supports[i].render_target_write =
            (formatSupport.optimalTilingFeatures &
                (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) != 0;
    }
    return;
}

void VkUtil_SelectInstanceLayers(struct CGpuInstance_Vulkan* vkInstance,
    const char* const* instance_layers, uint32_t instance_layers_count)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    if (count != 0)
    {
        vkInstance->pLayerNames = cgpu_calloc(instance_layers_count, sizeof(const char*));
        vkInstance->pLayerProperties = cgpu_calloc(instance_layers_count, sizeof(VkLayerProperties));

        DECLARE_ZERO_VLA(VkLayerProperties, layer_props, count)
        vkEnumerateInstanceLayerProperties(&count, layer_props);
        uint32_t filled_exts = 0;
        for (uint32_t i = 0; i < count; i++)
        {
            for (uint32_t j = 0; j < instance_layers_count; j++)
            {
                if (strcmp(layer_props[i].layerName, instance_layers[j]) == 0)
                {
                    vkInstance->pLayerProperties[filled_exts] = layer_props[i];
                    vkInstance->pLayerNames[filled_exts] = vkInstance->pLayerProperties[filled_exts].layerName;
                    filled_exts++;
                    break;
                }
            }
        }
        vkInstance->mLayersCount = filled_exts;
    }
    return;
}

void VkUtil_SelectInstanceExtensions(struct CGpuInstance_Vulkan* VkInstance,
    const char* const* instance_extensions, uint32_t instance_extension_count)
{
    const char* layer_name = NULL; // Query Vulkan implementation or by implicitly enabled layers
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(layer_name, &count, NULL);
    if (count > 0)
    {
        VkInstance->pExtensionProperties = cgpu_calloc(instance_extension_count, sizeof(VkExtensionProperties));
        VkInstance->pExtensionNames = cgpu_calloc(instance_extension_count, sizeof(const char*));

        DECLARE_ZERO_VLA(VkExtensionProperties, ext_props, count)
        vkEnumerateInstanceExtensionProperties(layer_name, &count, ext_props);
        uint32_t filled_exts = 0;
        for (uint32_t i = 0; i < count; i++)
        {
            for (uint32_t j = 0; j < instance_extension_count; j++)
            {
                if (strcmp(ext_props[i].extensionName, instance_extensions[j]) == 0)
                {
                    VkInstance->pExtensionProperties[filled_exts] = ext_props[i];
                    VkInstance->pExtensionNames[filled_exts] = VkInstance->pExtensionProperties[filled_exts].extensionName;
                    filled_exts++;
                    break;
                }
            }
        }
        VkInstance->mExtensionsCount = filled_exts;
    }
    return;
}

void VkUtil_SelectPhysicalDeviceLayers(struct CGpuAdapter_Vulkan* VkAdapter,
    const char* const* device_layers, uint32_t device_layers_count)
{
    uint32_t count;
    vkEnumerateDeviceLayerProperties(VkAdapter->pPhysicalDevice, &count, NULL);
    if (count != 0)
    {
        VkAdapter->pLayerNames = cgpu_calloc(device_layers_count, sizeof(const char*));
        VkAdapter->pLayerProperties = cgpu_calloc(device_layers_count, sizeof(VkLayerProperties));

        DECLARE_ZERO_VLA(VkLayerProperties, layer_props, count)
        vkEnumerateInstanceLayerProperties(&count, layer_props);
        uint32_t filled_exts = 0;
        for (uint32_t i = 0; i < count; i++)
        {
            for (uint32_t j = 0; j < device_layers_count; j++)
            {
                if (strcmp(layer_props[i].layerName, device_layers[j]) == 0)
                {
                    VkAdapter->pLayerProperties[filled_exts] = layer_props[i];
                    VkAdapter->pLayerNames[filled_exts] = VkAdapter->pLayerProperties[filled_exts].layerName;
                    filled_exts++;
                    break;
                }
            }
        }
        VkAdapter->mLayersCount = filled_exts;
    }
    return;
}

void VkUtil_SelectPhysicalDeviceExtensions(struct CGpuAdapter_Vulkan* VkAdapter,
    const char* const* device_extensions, uint32_t device_extension_count)
{
    const char* layer_name = NULL; // Query Vulkan implementation or by implicitly enabled layers
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(VkAdapter->pPhysicalDevice, layer_name, &count, NULL);
    if (count > 0)
    {
        VkAdapter->pExtensionProperties = cgpu_calloc(device_extension_count, sizeof(VkExtensionProperties));
        VkAdapter->pExtensionNames = cgpu_calloc(device_extension_count, sizeof(const char*));

        DECLARE_ZERO_VLA(VkExtensionProperties, ext_props, count)
        vkEnumerateDeviceExtensionProperties(VkAdapter->pPhysicalDevice, layer_name, &count, ext_props);
        uint32_t filled_exts = 0;
        for (uint32_t i = 0; i < count; i++)
        {
            for (uint32_t j = 0; j < device_extension_count; j++)
            {
                if (strcmp(ext_props[i].extensionName, device_extensions[j]) == 0)
                {
                    VkAdapter->pExtensionProperties[filled_exts] = ext_props[i];
                    VkAdapter->pExtensionNames[filled_exts] = VkAdapter->pExtensionProperties[filled_exts].extensionName;
                    filled_exts++;
                    continue;
                }
            }
        }
        VkAdapter->mExtensionsCount = filled_exts;
    }
    return;
}
