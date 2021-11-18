#define DLL_IMPLEMENTATION
#include "vulkan_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>
#include <EASTL/string.h>

#ifdef CGPU_USE_VULKAN

const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

class VkUtil_Blackboard
{
public:
    VkUtil_Blackboard(CGpuInstanceDescriptor const* desc)
    {
        const CGpuVulkanInstanceDescriptor* exts_desc = (const CGpuVulkanInstanceDescriptor*)desc->chained;
        // Merge All Parameters into one blackboard
        if (exts_desc != CGPU_NULLPTR) // Extensions
        {
            if (exts_desc->backend != ECGPUBackEnd_VULKAN)
            {
                assert(exts_desc->backend == ECGPUBackEnd_VULKAN && "Chained Instance Descriptor must have a vulkan backend!");
                exts_desc = CGPU_NULLPTR;
            }
            else
            {
                messenger_info_ptr = exts_desc->pDebugUtilsMessenger;
                // Merge Instance Extension Names
                if (exts_desc->ppInstanceExtensions != NULL && exts_desc->mInstanceExtensionCount != 0)
                {
                    instance_extensions.insert(instance_extensions.end(),
                        exts_desc->ppInstanceExtensions, exts_desc->ppInstanceExtensions + exts_desc->mInstanceExtensionCount);
                }
                // Merge Instance Layer Names
                if (exts_desc->ppInstanceLayers != NULL && exts_desc->mInstanceLayerCount != 0)
                {
                    instance_layers.insert(instance_layers.end(),
                        exts_desc->ppInstanceLayers, exts_desc->ppInstanceLayers + exts_desc->mInstanceLayerCount);
                }
                // Merge Device Extension Names
                if (exts_desc->ppDeviceExtensions != NULL && exts_desc->mDeviceExtensionCount != 0)
                {
                    device_extensions.insert(device_extensions.end(),
                        exts_desc->ppDeviceExtensions,
                        exts_desc->ppDeviceExtensions + exts_desc->mDeviceExtensionCount);
                }
                if (desc->enableDebugLayer)
                    instance_layers.push_back(validation_layer_name);
            }
        }
        // from desc
        if (desc->enableDebugLayer)
        {
            instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
    }
    const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr = CGPU_NULLPTR;
    eastl::vector<const char*> instance_extensions = {
    #if defined(_WIN32) || defined(_WIN64)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif defined(_MACOS)
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
    #endif
        VK_KHR_SURFACE_EXTENSION_NAME
    };
    eastl::vector<const char*> instance_layers;
    eastl::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME,
        VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME,
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    #ifdef USE_EXTERNAL_MEMORY_EXTENSIONS
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
        #if defined(VK_USE_PLATFORM_WIN32_KHR)
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
        #endif
    #endif
    // Debug marker extension in case debug utils is not supported
    #ifndef ENABLE_DEBUG_UTILS_EXTENSION
        VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
    #endif
    #if defined(VK_USE_PLATFORM_GGP)
        VK_GGP_FRAME_TOKEN_EXTENSION_NAME,
    #endif

    #if VK_KHR_draw_indirect_count
        VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
    #endif
    // Fragment shader interlock extension to be used for ROV type functionality in Vulkan
    #if VK_EXT_fragment_shader_interlock
        VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
    #endif
    /************************************************************************/
    // NVIDIA Specific Extensions
    /************************************************************************/
    #ifdef USE_NV_EXTENSIONS
        VK_NVX_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME,
    #endif
        /************************************************************************/
        // AMD Specific Extensions
        /************************************************************************/
        VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
        VK_AMD_SHADER_BALLOT_EXTENSION_NAME,
        VK_AMD_GCN_SHADER_EXTENSION_NAME,
    /************************************************************************/
    // Multi GPU Extensions
    /************************************************************************/
    #if VK_KHR_device_group
        VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
    #endif
        /************************************************************************/
        // Bindless & None Uniform access Extensions
        /************************************************************************/
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    #if VK_KHR_maintenance3 // descriptor indexing depends on this
        VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    #endif
        /************************************************************************/
        // Descriptor Update Template Extension for efficient descriptor set updates
        /************************************************************************/
        VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
    /************************************************************************/
    // Raytracing
    /************************************************************************/
    #ifdef ENABLE_RAYTRACING
        VK_NV_RAY_TRACING_EXTENSION_NAME,
    #endif
    /************************************************************************/
    // YCbCr format support
    /************************************************************************/
    #if VK_KHR_bind_memory2
        // Requirement for VK_KHR_sampler_ycbcr_conversion
        VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
    #endif
    #if VK_KHR_sampler_ycbcr_conversion
        VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
    #endif
    /************************************************************************/
    // Nsight Aftermath
    /************************************************************************/
    #ifdef ENABLE_NSIGHT_AFTERMATH
        VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
        VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME,
    #endif
        /************************************************************************/
        /************************************************************************/
    };
};

struct CGpuVkDeviceExtensionsTable : public eastl::unordered_map<eastl::string, bool> //
{
    static void ConstructForAllAdapters(struct CGpuInstance_Vulkan* I, const VkUtil_Blackboard& blackboard)
    {
        // enum physical devices & store informations.
        auto wanted_device_extensions = blackboard.device_extensions.data();
        const auto wanted_device_extensions_count = (uint32_t)blackboard.device_extensions.size();
        // construct extensions table
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++)
        {
            auto& Adapter = I->pVulkanAdapters[i];
            Adapter.pExtensionTable = new CGpuVkDeviceExtensionsTable();
            auto& Table = *Adapter.pExtensionTable;
            for (uint32_t j = 0; j < wanted_device_extensions_count; j++)
            {
                Table[wanted_device_extensions[j]] = false;
            }
            for (uint32_t j = 0; j < Adapter.mExtensionsCount; j++)
            {
                Table[Adapter.pExtensionNames[j]] = true;
            }
        }
    }
};

CGpuInstanceId cgpu_create_instance_vulkan(CGpuInstanceDescriptor const* desc)
{
    // Merge All Parameters into one blackboard
    const VkUtil_Blackboard blackboard(desc);

    // Memory Alloc
    CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)cgpu_calloc(1, sizeof(CGpuInstance_Vulkan));
    ::memset(I, 0, sizeof(CGpuInstance_Vulkan));

    // Initialize Environment
    VkUtil_InitializeEnvironment(&I->super);

    // Create VkInstance.
    DECLARE_ZERO(VkApplicationInfo, appInfo)
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CGPU";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    // TODO: Select Instance Layers & Layer Extensions
    VkUtil_SelectInstanceLayers(I,
        blackboard.instance_layers.data(),
        (uint32_t)blackboard.instance_layers.size());
    // TODO: Select Instance Extensions

    DECLARE_ZERO(VkInstanceCreateInfo, createInfo)
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    // Instance Extensions
    createInfo.enabledExtensionCount = (uint32_t)blackboard.instance_extensions.size();
    createInfo.ppEnabledExtensionNames = blackboard.instance_extensions.data();

    // List Validation Features
    DECLARE_ZERO(VkValidationFeaturesEXT, validationFeaturesExt)
    VkValidationFeatureEnableEXT enabledValidationFeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
    };
    if (desc->enableGpuBasedValidation)
    {
        if (!desc->enableDebugLayer)
            printf("[Vulkan Warning]: GpuBasedValidation enabled while ValidationLayer is closed, there'll be no effect.");
    #if VK_HEADER_VERSION >= 108
        validationFeaturesExt.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeaturesExt.enabledValidationFeatureCount = 1u;
        validationFeaturesExt.pEnabledValidationFeatures = enabledValidationFeatures;
        createInfo.pNext = &validationFeaturesExt;
    #else
        printf("[Vulkan Warning]: GpuBasedValidation enabled but VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT is not supported!\n");
    #endif
    }

    createInfo.enabledLayerCount = (uint32_t)blackboard.instance_layers.size();
    createInfo.ppEnabledLayerNames = blackboard.instance_layers.data();
    if (vkCreateInstance(&createInfo, GLOBAL_VkAllocationCallbacks, &I->pVkInstance) != VK_SUCCESS)
    {
        assert(0 && "Vulkan: failed to create instance!");
    }

    #if defined(NX64)
    loadExtensionsNX(result->pVkInstance);
    #else
    // Load Vulkan instance functions
    volkLoadInstance(I->pVkInstance);
    #endif

    // enum physical devices & store informations.
    const char* const* wanted_device_extensions = blackboard.device_extensions.data();
    const auto wanted_device_extensions_count = (uint32_t)blackboard.device_extensions.size();
    VkUtil_QueryAllAdapters(I, wanted_device_extensions, wanted_device_extensions_count);
    // construct extensions table
    CGpuVkDeviceExtensionsTable::ConstructForAllAdapters(I, blackboard);

    // Open validation layer.
    if (desc->enableDebugLayer)
    {
        VkUtil_EnableValidationLayer(I, blackboard.messenger_info_ptr);
    }

    return &(I->super);
}

void cgpu_free_instance_vulkan(CGpuInstanceId instance)
{
    CGpuInstance_Vulkan* to_destroy = (CGpuInstance_Vulkan*)instance;
    VkUtil_DeInitializeEnvironment(&to_destroy->super);
    if (to_destroy->pVkDebugUtilsMessenger)
    {
        assert(vkDestroyDebugUtilsMessengerEXT && "Load vkDestroyDebugUtilsMessengerEXT failed!");
        vkDestroyDebugUtilsMessengerEXT(to_destroy->pVkInstance, to_destroy->pVkDebugUtilsMessenger, nullptr);
    }

    vkDestroyInstance(to_destroy->pVkInstance, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < to_destroy->mPhysicalDeviceCount; i++)
    {
        auto& Adapter = to_destroy->pVulkanAdapters[i];
        cgpu_free(Adapter.pQueueFamilyProperties);
        cgpu_free(Adapter.pExtensionProperties);
        cgpu_free(Adapter.pExtensionNames);
        delete Adapter.pExtensionTable;
    }
    cgpu_free(to_destroy->pVulkanAdapters);
    cgpu_free(to_destroy);
}

const float queuePriorities[] = {
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
};
CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)adapter->instance;
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cgpu_calloc(1, sizeof(CGpuDevice_Vulkan));
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)adapter;

    *const_cast<CGpuAdapterId*>(&D->super.adapter) = adapter;

    // Prepare Create Queues
    eastl::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.resize(desc->queueGroupCount);
    for (uint32_t i = 0; i < desc->queueGroupCount; i++)
    {
        VkDeviceQueueCreateInfo& info = queueCreateInfos[i];
        CGpuQueueGroupDescriptor& descriptor = desc->queueGroups[i];
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueCount = descriptor.queueCount;
        info.queueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[descriptor.queueType];
        info.pQueuePriorities = queuePriorities;

        assert(cgpu_query_queue_count_vulkan(adapter, descriptor.queueType) >= descriptor.queueCount && "allocated too many queues!");
    }
    // Create Device
    DECLARE_ZERO(VkDeviceCreateInfo, createInfo)
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &A->mPhysicalDeviceFeatures;
    createInfo.enabledExtensionCount = A->mExtensionsCount;
    createInfo.ppEnabledExtensionNames = A->pExtensionNames;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = &validation_layer_name;

    if (vkCreateDevice(A->pPhysicalDevice, &createInfo, CGPU_NULLPTR, &D->pVkDevice) != VK_SUCCESS)
    {
        assert(0 && "failed to create logical device!");
    }

    // Single Device Only.
    volkLoadDeviceTable(&D->mVkDeviceTable, D->pVkDevice);

    // Create Pipeline Cache
    D->pPipelineCache = CGPU_NULLPTR;
    if (!desc->disable_pipeline_cache)
    {
        VkUtil_CreatePipelineCache(D);
    }

    // Create VMA Allocator
    VkUtil_CreateVMAAllocator(I, A, D);

    return &D->super;
}

void cgpu_free_device_vulkan(CGpuDeviceId device)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    vkDestroyDevice(D->pVkDevice, nullptr);
    cgpu_free(D);
}

#endif