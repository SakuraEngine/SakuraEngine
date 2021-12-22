#include "vulkan_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>
#include <EASTL/string.h>

class VkUtil_Blackboard
{
public:
    VkUtil_Blackboard(CGpuInstanceDescriptor const* desc)
    {
        const CGpuVulkanInstanceDescriptor* exts_desc = (const CGpuVulkanInstanceDescriptor*)desc->chained;
        // default
        device_extensions.insert(device_extensions.end(),
            eastl::begin(cgpu_wanted_device_exts), eastl::end(cgpu_wanted_device_exts));
        instance_extensions.insert(instance_extensions.end(),
            eastl::begin(cgpu_wanted_instance_exts), eastl::end(cgpu_wanted_instance_exts));
        // from desc
        if (desc->enable_debug_layer)
        {
            instance_layers.push_back(validation_layer_name);
            instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        // Merge All Parameters into one blackboard
        if (exts_desc != CGPU_NULLPTR) // Extensions
        {
            if (exts_desc->backend != ECGpuBackend_VULKAN)
            {
                assert(exts_desc->backend == ECGpuBackend_VULKAN && "Chained Instance Descriptor must have a vulkan backend!");
                exts_desc = CGPU_NULLPTR;
            }
            else
            {
                messenger_info_ptr = exts_desc->pDebugUtilsMessenger;
                report_info_ptr = exts_desc->pDebugReportMessenger;
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
            }
        }
    }
    const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr = CGPU_NULLPTR;
    const VkDebugReportCallbackCreateInfoEXT* report_info_ptr = CGPU_NULLPTR;
    eastl::vector<const char*> instance_extensions;
    eastl::vector<const char*> instance_layers;
    eastl::vector<const char*> device_extensions;
    eastl::vector<const char*> device_layers;
};

struct CGpuCachedRenderPass {
    VkRenderPass pass;
    size_t timestamp;
};

struct CGpuCachedFramebuffer {
    VkFramebuffer framebuffer;
    size_t timestamp;
};

struct CGpuVkPassTable //
{
    eastl::unordered_map<size_t, CGpuCachedRenderPass> cached_renderpasses;
    eastl::unordered_map<size_t, CGpuCachedFramebuffer> cached_framebuffers;
};

VkFramebuffer VkUtil_FramebufferTableTryFind(struct CGpuVkPassTable* table, const VkUtil_FramebufferDesc* desc)
{
    size_t hash = cgpu_hash(desc, sizeof(*desc), *(size_t*)&table);
    const auto& iter = table->cached_framebuffers.find(hash);
    if (iter != table->cached_framebuffers.end())
    {
        return iter->second.framebuffer;
    }
    return VK_NULL_HANDLE;
}

void VkUtil_FramebufferTableAdd(struct CGpuVkPassTable* table, const struct VkUtil_FramebufferDesc* desc, VkFramebuffer framebuffer)
{
    size_t hash = cgpu_hash(desc, sizeof(*desc), *(size_t*)&table);
    const auto& iter = table->cached_framebuffers.find(hash);
    if (iter != table->cached_framebuffers.end())
    {
        cgpu_warn("Vulkan Framebuffer with this desc already exists!");
    }
    // TODO: Add timestamp
    CGpuCachedFramebuffer new_fb = { framebuffer, 0 };
    table->cached_framebuffers[hash] = new_fb;
}

VkRenderPass VkUtil_RenderPassTableTryFind(struct CGpuVkPassTable* table, const struct VkUtil_RenderPassDesc* desc)
{
    size_t hash = cgpu_hash(desc, sizeof(*desc), *(size_t*)&table);
    const auto& iter = table->cached_renderpasses.find(hash);
    if (iter != table->cached_renderpasses.end())
    {
        return iter->second.pass;
    }
    return VK_NULL_HANDLE;
}

void VkUtil_RenderPassTableAdd(struct CGpuVkPassTable* table, const struct VkUtil_RenderPassDesc* desc, VkRenderPass pass)
{
    size_t hash = cgpu_hash(desc, sizeof(*desc), *(size_t*)&table);
    const auto& iter = table->cached_renderpasses.find(hash);
    if (iter != table->cached_renderpasses.end())
    {
        cgpu_warn("Vulkan Pass with this desc already exists!");
    }
    // TODO: Add timestamp
    CGpuCachedRenderPass new_pass = { pass, 0 };
    table->cached_renderpasses[hash] = new_pass;
}

struct CGpuVkExtensionsTable : public eastl::unordered_map<eastl::string, bool> //
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
            Adapter.pExtensionsTable = new CGpuVkExtensionsTable();
            auto& Table = *Adapter.pExtensionsTable;
            for (uint32_t j = 0; j < wanted_device_extensions_count; j++)
            {
                Table[wanted_device_extensions[j]] = false;
            }
            for (uint32_t j = 0; j < Adapter.mExtensionsCount; j++)
            {
                Table[Adapter.pExtensionNames[j]] = true;
            }
            // Cache
            {
                Adapter.debug_marker = Table[VK_EXT_DEBUG_MARKER_EXTENSION_NAME];
                Adapter.dedicated_allocation = Table[VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME];
                Adapter.memory_req2 = Table[VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME];
                Adapter.external_memory = Table[VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME];
#ifdef _WINDOWS
                Adapter.external_memory_win32 = Table[VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME];
                Adapter.external_memory &= Adapter.external_memory_win32;
#endif
                Adapter.draw_indirect_count = Table[VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME];
                Adapter.amd_draw_indirect_count = Table[VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME];
                Adapter.amd_gcn_shader = Table[VK_AMD_GCN_SHADER_EXTENSION_NAME];
                Adapter.descriptor_indexing = Table[VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME];
                Adapter.sampler_ycbcr = Table[VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME];
                Adapter.nv_diagnostic_checkpoints = Table[VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME];
                Adapter.nv_diagnostic_config = Table[VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME];
            }
        }
    }
    static void ConstructForInstance(struct CGpuInstance_Vulkan* I, const VkUtil_Blackboard& blackboard)
    {
        // enum physical devices & store informations.
        auto wanted_instance_extensions = blackboard.instance_extensions.data();
        const auto wanted_instance_extensions_count = (uint32_t)blackboard.instance_extensions.size();
        // construct extensions table
        I->pExtensionsTable = new CGpuVkExtensionsTable();
        auto& Table = *I->pExtensionsTable;
        for (uint32_t j = 0; j < wanted_instance_extensions_count; j++)
        {
            Table[wanted_instance_extensions[j]] = false;
        }
        for (uint32_t j = 0; j < I->mExtensionsCount; j++)
        {
            Table[I->pExtensionNames[j]] = true;
        }
        // Cache
        {
            I->device_group_creation = Table[VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME]; // Linked GPU
            I->debug_utils = Table[VK_EXT_DEBUG_UTILS_EXTENSION_NAME];
            I->debug_report = !I->debug_utils && Table[VK_EXT_DEBUG_UTILS_EXTENSION_NAME];
        }
    }
};

struct CGpuVkLayersTable : public eastl::unordered_map<eastl::string, bool> //
{
    static void ConstructForAllAdapters(struct CGpuInstance_Vulkan* I, const VkUtil_Blackboard& blackboard)
    {
        // enum physical devices & store informations.
        auto wanted_device_layers = blackboard.device_layers.data();
        const auto wanted_device_layers_count = (uint32_t)blackboard.device_layers.size();
        // construct layers table
        for (uint32_t i = 0; i < I->mPhysicalDeviceCount; i++)
        {
            auto& Adapter = I->pVulkanAdapters[i];
            Adapter.pLayersTable = new CGpuVkLayersTable();
            auto& Table = *Adapter.pLayersTable;
            for (uint32_t j = 0; j < wanted_device_layers_count; j++)
            {
                Table[wanted_device_layers[j]] = false;
            }
            for (uint32_t j = 0; j < Adapter.mLayersCount; j++)
            {
                Table[Adapter.pLayerNames[j]] = true;
            }
        }
    }
    static void ConstructForInstance(struct CGpuInstance_Vulkan* I, const VkUtil_Blackboard& blackboard)
    {
        // enum physical devices & store informations.
        auto wanted_instance_layers = blackboard.instance_layers.data();
        const auto wanted_instance_layers_count = (uint32_t)blackboard.instance_layers.size();
        // construct layers table
        I->pLayersTable = new CGpuVkLayersTable();
        auto& Table = *I->pLayersTable;
        for (uint32_t j = 0; j < wanted_instance_layers_count; j++)
        {
            Table[wanted_instance_layers[j]] = false;
        }
        for (uint32_t j = 0; j < I->mLayersCount; j++)
        {
            Table[I->pLayerNames[j]] = true;
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

    // Select Instance Layers & Layer Extensions
    VkUtil_SelectInstanceLayers(I,
        blackboard.instance_layers.data(),
        (uint32_t)blackboard.instance_layers.size());
    // Select Instance Extensions
    VkUtil_SelectInstanceExtensions(I,
        blackboard.instance_extensions.data(),
        (uint32_t)blackboard.instance_extensions.size());

    DECLARE_ZERO(VkInstanceCreateInfo, createInfo)
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    // Instance Layers
    createInfo.enabledLayerCount = (uint32_t)blackboard.instance_layers.size();
    createInfo.ppEnabledLayerNames = blackboard.instance_layers.data();
    // Instance Extensions
    createInfo.enabledExtensionCount = I->mExtensionsCount;
    createInfo.ppEnabledExtensionNames = I->pExtensionNames;

    // List Validation Features
    DECLARE_ZERO(VkValidationFeaturesEXT, validationFeaturesExt)
    VkValidationFeatureEnableEXT enabledValidationFeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
    };
    if (desc->enable_gpu_based_validation)
    {
        if (!desc->enable_debug_layer)
            cgpu_warn("Vulkan GpuBasedValidation enabled while ValidationLayer is closed, there'll be no effect.");
#if VK_HEADER_VERSION >= 108
        validationFeaturesExt.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeaturesExt.enabledValidationFeatureCount = sizeof(enabledValidationFeatures) / sizeof(VkValidationFeatureEnableEXT);
        validationFeaturesExt.pEnabledValidationFeatures = enabledValidationFeatures;
        createInfo.pNext = &validationFeaturesExt;
#else
        cgpu_warn("Vulkan GpuBasedValidation enabled but VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT is not supported!\n");
#endif
    }

    if (vkCreateInstance(&createInfo, GLOBAL_VkAllocationCallbacks, &I->pVkInstance) != VK_SUCCESS)
    {
        assert(0 && "Vulkan: failed to create instance!");
    }
    CGpuVkLayersTable::ConstructForInstance(I, blackboard);
    CGpuVkExtensionsTable::ConstructForInstance(I, blackboard);

#if defined(NX64)
    loadExtensionsNX(result->pVkInstance);
#else
    // Load Vulkan instance functions
    volkLoadInstance(I->pVkInstance);
#endif

    // enum physical devices & store informations.
    const char* const* wanted_device_extensions = blackboard.device_extensions.data();
    const auto wanted_device_extensions_count = (uint32_t)blackboard.device_extensions.size();
    const char* const* wanted_device_layers = blackboard.device_layers.data();
    const auto wanted_device_layers_count = (uint32_t)blackboard.device_layers.size();
    VkUtil_QueryAllAdapters(I,
        wanted_device_layers, wanted_device_layers_count,
        wanted_device_extensions, wanted_device_extensions_count);
    // construct extensions table
    CGpuVkLayersTable::ConstructForAllAdapters(I, blackboard);
    CGpuVkExtensionsTable::ConstructForAllAdapters(I, blackboard);

    // Open validation layer.
    if (desc->enable_debug_layer)
    {
        VkUtil_EnableValidationLayer(I, blackboard.messenger_info_ptr, blackboard.report_info_ptr);
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
        // free extensions cache
        delete Adapter.pExtensionsTable;
        cgpu_free(Adapter.pExtensionNames);
        cgpu_free(Adapter.pExtensionProperties);

        // free layers cache
        delete Adapter.pLayersTable;
        cgpu_free(Adapter.pLayerNames);
        cgpu_free(Adapter.pLayerProperties);
    }
    // free extensions cache
    delete to_destroy->pExtensionsTable;
    cgpu_free(to_destroy->pExtensionNames);
    cgpu_free(to_destroy->pExtensionProperties);
    // free layers cache
    delete to_destroy->pLayersTable;
    cgpu_free(to_destroy->pLayerNames);
    cgpu_free(to_destroy->pLayerProperties);

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
    createInfo.pNext = &A->mPhysicalDeviceFeatures;
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = NULL;
    createInfo.enabledExtensionCount = A->mExtensionsCount;
    createInfo.ppEnabledExtensionNames = A->pExtensionNames;
    createInfo.enabledLayerCount = A->mLayersCount;
    createInfo.ppEnabledLayerNames = A->pLayerNames;

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
    // Create Descriptor Heap
    D->pDescriptorPool = VkUtil_CreateDescriptorPool(D);
    // Create pass table
    D->pPassTable = cgpu_new<CGpuVkPassTable>();
    return &D->super;
}

void cgpu_free_device_vulkan(CGpuDeviceId device)
{
    CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
    CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)device->adapter;
    CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)device->adapter->instance;

    for (auto& iter : D->pPassTable->cached_renderpasses)
    {
        D->mVkDeviceTable.vkDestroyRenderPass(D->pVkDevice, iter.second.pass, GLOBAL_VkAllocationCallbacks);
    }
    for (auto& iter : D->pPassTable->cached_framebuffers)
    {
        D->mVkDeviceTable.vkDestroyFramebuffer(D->pVkDevice, iter.second.framebuffer, GLOBAL_VkAllocationCallbacks);
    }
    cgpu_delete(D->pPassTable);

    VkUtil_FreeVMAAllocator(I, A, D);
    VkUtil_FreeDescriptorPool(D->pDescriptorPool);
    VkUtil_FreePipelineCache(I, A, D);
    vkDestroyDevice(D->pVkDevice, nullptr);
    cgpu_free(D);
}