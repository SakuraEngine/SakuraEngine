#include "vulkan_utils.h"
#include "cgpu/drivers/cgpu_ags.h"
#include "cgpu/drivers/cgpu_nvapi.h"
#include <assert.h>
#include <stdio.h>

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
    CGpuInstance_Vulkan* I, const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr)
{
    const VkDebugUtilsMessengerCreateInfoEXT* messengerInfoPtr = CGPU_NULLPTR;
    DECLARE_ZERO(VkDebugUtilsMessengerCreateInfoEXT, messengerInfo)
    if (messenger_info_ptr != CGPU_NULLPTR)
    {
        messengerInfoPtr = messenger_info_ptr;
    }
    else
    {
        messengerInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.pfnUserCallback = VkUtil_DebugCallback;
        messengerInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerInfo.flags = 0;
        messengerInfo.pUserData = NULL;
        messengerInfoPtr = &messengerInfo;
    }
    assert(vkCreateDebugUtilsMessengerEXT &&
           "Load vkCreateDebugUtilsMessengerEXT failed!");
    VkResult res = vkCreateDebugUtilsMessengerEXT(I->pVkInstance,
        messengerInfoPtr, CGPU_NULLPTR,
        &(I->pVkDebugUtilsMessenger));
    if (VK_SUCCESS != res)
    {
        assert(0 && "vkCreateDebugUtilsMessengerEXT failed - disabling Vulkan "
                    "debug callbacks");
    }
}

inline static void __VkUtil_SelectQueueIndices(CGpuAdapter_Vulkan* VkAdapter)
{
    // Query Queue Information.
    vkGetPhysicalDeviceQueueFamilyProperties(
        VkAdapter->pPhysicalDevice, &VkAdapter->mQueueFamilyPropertiesCount,
        CGPU_NULLPTR);
    VkAdapter->pQueueFamilyProperties = (VkQueueFamilyProperties*)cgpu_calloc(
        VkAdapter->mQueueFamilyPropertiesCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(VkAdapter->pPhysicalDevice,
        &VkAdapter->mQueueFamilyPropertiesCount, VkAdapter->pQueueFamilyProperties);

    for (uint32_t j = 0; j < VkAdapter->mQueueFamilyPropertiesCount; j++)
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

inline static void __VkUtil_QueryPhysicalDeviceExtensionProperties(struct CGpuAdapter_Vulkan* VkAdapter,
    const char** device_extensions, uint32_t device_extension_count)
{
    const char* layer_name = NULL; // Query Vulkan implementation or by implicitly enabled layers
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(VkAdapter->pPhysicalDevice, layer_name, &count, NULL);
    if (count > 0)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            for (uint32_t j = 0; j < device_extension_count; j++)
            {
            }
        }
    }
    return;
}

void VkUtil_QueryAllAdapters(CGpuInstance_Vulkan* I,
    const char** device_extensions, uint32_t device_extension_count)
{
    assert((I->mPhysicalDeviceCount == 0) && "VkUtil_QueryAllAdapters should only be called once!");

    vkEnumeratePhysicalDevices(I->pVkInstance, &I->mPhysicalDeviceCount, CGPU_NULLPTR);
    if (I->mPhysicalDeviceCount != 0)
    {
        I->pVulkanAdapters =
            (CGpuAdapter_Vulkan*)cgpu_calloc(I->mPhysicalDeviceCount, sizeof(CGpuAdapter_Vulkan));
        VkPhysicalDevice* pysicalDevices =
            (VkPhysicalDevice*)cgpu_calloc(I->mPhysicalDeviceCount, sizeof(VkPhysicalDevice));
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
            vkGetPhysicalDeviceFeatures(pysicalDevices[i], &VkAdapter->mPhysicalDeviceFeatures);
            // Query Physical Device Extension Properties
            __VkUtil_QueryPhysicalDeviceExtensionProperties(
                VkAdapter, device_extensions, device_extension_count);
            // Select Queue Indices
            __VkUtil_SelectQueueIndices(VkAdapter);
        }
        cgpu_free(pysicalDevices);
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

// API Helpers
VkBufferUsageFlags
VkUtil_DescriptorTypesToBufferUsage(CGpuDescriptorTypes descriptors,
    bool texel)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (descriptors & DT_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (descriptors & DT_RW_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (descriptors & DT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (descriptors & DT_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (descriptors & DT_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (descriptors & DT_INDIRECT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
#ifdef ENABLE_RAYTRACING
    if (descriptors & DT_RAY_TRACING)
    {
        result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    }
#endif
    return result;
}
