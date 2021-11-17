#define DLL_IMPLEMENTATION
#include "vulkan_utils.h"
#include <cassert>
#include <stdlib.h>
#include <vector>

#ifdef CGPU_USE_VULKAN

const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

CGpuInstanceId cgpu_vulkan_create_instance(CGpuInstanceDescriptor const* desc,
	CGpuVulkanInstanceDescriptor const* exts_desc)
{	
	static auto volkInit = volkInitialize();
	(void)volkInit;
	assert((volkInit == VK_SUCCESS) && "Volk Initialize Failed!");

	CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)cgpu_calloc(1, sizeof(CGpuInstance_Vulkan));
	::memset(I, 0, sizeof(CGpuInstance_Vulkan));
    DECLARE_ZERO(VkApplicationInfo, appInfo)
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CGPU";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

	// Create VkInstance.
    DECLARE_ZERO(VkInstanceCreateInfo, createInfo)
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	std::vector<const char*> exts = {
#if defined(_WIN32) || defined(_WIN64)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined (_MACOS)
		VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#endif
		VK_KHR_SURFACE_EXTENSION_NAME
	};
	if(desc->enableDebugLayer)
	{
		exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	if(exts_desc != CGPU_NULLPTR) // Extensions
	{
		exts.insert(exts.end(), exts_desc->ppInstanceExtensions, exts_desc->ppInstanceExtensions + exts_desc->mInstanceExtensionCount);
	}
	createInfo.enabledExtensionCount = (uint32_t)exts.size();
	createInfo.ppEnabledExtensionNames = exts.data();
	
	DECLARE_ZERO(VkValidationFeaturesEXT, validationFeaturesExt)
	VkValidationFeatureEnableEXT enabledValidationFeatures[] =
	{
		VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
	};
	if(desc->enableGpuBasedValidation)
	{
		if(!desc->enableDebugLayer)
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

    std::vector<const char*> layers;
	if (exts_desc != CGPU_NULLPTR) // Layers
	{
        layers.insert(layers.end(), 
            exts_desc->ppInstanceLayers, exts_desc->ppInstanceLayers + exts_desc->mInstanceLayerCount);

        if(desc->enableDebugLayer) 
            layers.push_back(validation_layer_name);
		
	}
	createInfo.enabledLayerCount = (uint32_t)layers.size();
	createInfo.ppEnabledLayerNames = layers.data();
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
	VkUtil_QueryAllAdapters(I);
	
    // Open validation layer.
    if(desc->enableDebugLayer)
    {
		VkUtil_EnableValidationLayer(I, exts_desc);
    }

	return &(I->super);
}

void cgpu_free_instance_vulkan(CGpuInstanceId instance)
{
    CGpuInstance_Vulkan* to_destroy = (CGpuInstance_Vulkan*)instance;
	if(to_destroy->pVkDebugUtilsMessenger) {
		assert(vkDestroyDebugUtilsMessengerEXT && "Load vkDestroyDebugUtilsMessengerEXT failed!");
		vkDestroyDebugUtilsMessengerEXT(to_destroy->pVkInstance, to_destroy->pVkDebugUtilsMessenger, nullptr);
	}

	vkDestroyInstance(to_destroy->pVkInstance, VK_NULL_HANDLE);
	cgpu_free(to_destroy->pVulkanAdapters);
	cgpu_free(to_destroy);
}

const float queuePriorities[] = {
	1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,
	1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,
	1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,
	1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,  1.f, 1.f, 1.f, 1.f,
};
const std::vector<const char*> deviceExtensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
	CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)adapter->instance;
	CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)cgpu_calloc(1, sizeof(CGpuDevice_Vulkan));
	CGpuAdapter_Vulkan* A = (CGpuAdapter_Vulkan*)adapter;

	*const_cast<CGpuAdapterId*>(&D->super.adapter) = adapter;
	
	// Prepare Create Queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.resize(desc->queueGroupCount);
	for(uint32_t i = 0; i < desc->queueGroupCount; i++)
	{
		VkDeviceQueueCreateInfo& info = queueCreateInfos[i];
		CGpuQueueGroupDescriptor& descriptor = desc->queueGroups[i];
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.queueCount = descriptor.queueCount;
		info.queueFamilyIndex = (uint32_t)A->mQueueFamilyIndices[descriptor.queueType];
        info.pQueuePriorities = queuePriorities;

		assert(cgpu_query_queue_count_vulkan(adapter, descriptor.queueType) >= descriptor.queueCount 
			&& "allocated too many queues!");
	}
	// Create Device
	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
	createInfo.enabledExtensionCount = (uint32_t)deviceExtensionNames.size();
	// Enable Validation Layer
	if (I->pVkDebugUtilsMessenger) {
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &validation_layer_name;
	} else {
		createInfo.enabledLayerCount = 0;
	}
	if (vkCreateDevice(A->pPhysicalDevice, &createInfo, CGPU_NULLPTR, &D->pVkDevice) != VK_SUCCESS) {
		assert(0 && "failed to create logical device!");
	}

	// Single Device Only.
	volkLoadDeviceTable(&D->mVkDeviceTable, D->pVkDevice);
	
	// Create Pipeline Cache
	if(desc->disable_pipeline_cache)
	{
		D->pPipelineCache = CGPU_NULLPTR;
	}
	else
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