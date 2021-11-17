#define DLL_IMPLEMENTATION
#include "cgpu/extensions/cgpu_vulkan_exts.h"
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include <cassert>
#include <stdlib.h>
#include <vector>
#ifdef CGPU_USE_VULKAN

const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
	switch(messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			printf("[verbose]");break; 
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: 
			printf("[info]");break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			printf("[warning]"); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT :
			printf("[error]"); break;
		default:
			return VK_TRUE;
	}
	printf(" validation layer: %s\n", pCallbackData->pMessage); 
    return VK_FALSE;
}

CGpuInstanceId cgpu_vulkan_create_instance(CGpuInstanceDescriptor const* desc,
	CGpuVulkanInstanceDescriptor const* exts_desc)
{	
	static auto volkInit = volkInitialize();
	assert((volkInit == VK_SUCCESS) && "Volk Initialize Failed!");

	CGpuInstance_Vulkan* result = (CGpuInstance_Vulkan*)malloc(sizeof(CGpuInstance_Vulkan));
	::memset(result, 0, sizeof(CGpuInstance_Vulkan));
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
	
	if(desc->enableGpuBasedValidation)
	{
		if(!desc->enableDebugLayer)
			printf("[Vulkan Warning]: GpuBasedValidation enabled while ValidationLayer is closed, there'll be no effect.");
#if VK_HEADER_VERSION >= 108
		VkValidationFeatureEnableEXT enabledValidationFeatures[] =
		{
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
		};
		DECLARE_ZERO(VkValidationFeaturesEXT, validationFeaturesExt)
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
	if (vkCreateInstance(&createInfo, GLOBAL_VkAllocationCallbacks, &result->pVkInstance) != VK_SUCCESS)
	{
		assert(0 && "Vulkan: failed to create instance!");
	}

#if defined(NX64)
	loadExtensionsNX(result->pVkInstance);
#else
	// Load Vulkan instance functions
	volkLoadInstance(result->pVkInstance);
#endif

	// enum physical devices & store informations.
	vkEnumeratePhysicalDevices(result->pVkInstance, &result->mPhysicalDeviceCount, CGPU_NULLPTR);
	if(result->mPhysicalDeviceCount != 0)
	{
		result->pVulkanAdapters = (CGpuAdapter_Vulkan*)malloc(sizeof(CGpuAdapter_Vulkan) * result->mPhysicalDeviceCount);
		VkPhysicalDevice* pysicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * result->mPhysicalDeviceCount);
		vkEnumeratePhysicalDevices(result->pVkInstance, &result->mPhysicalDeviceCount, pysicalDevices);
		for(uint32_t i = 0; i < result->mPhysicalDeviceCount; i++)
		{
			auto& VkAdapter = result->pVulkanAdapters[i];
			for(uint32_t q = 0; q < ECGpuQueueType_Count; q++)
			{
				VkAdapter.mQueueFamilyIndices[q] = -1;
			}
			VkAdapter.super.instance = &result->super;
			VkAdapter.pPhysicalDevice = pysicalDevices[i];
			VkAdapter.mSubgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
			VkAdapter.mSubgroupProperties.pNext = NULL;
			VkAdapter.mPhysicalDeviceProps.pNext = &VkAdapter.mSubgroupProperties;
			VkAdapter.mPhysicalDeviceProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
			vkGetPhysicalDeviceProperties2(pysicalDevices[i], &VkAdapter.mPhysicalDeviceProps);
			vkGetPhysicalDeviceFeatures(pysicalDevices[i], &VkAdapter.mPhysicalDeviceFeatures);

			// Query Queue Information.
			vkGetPhysicalDeviceQueueFamilyProperties(pysicalDevices[i],
				&VkAdapter.mQueueFamilyPropertiesCount, nullptr);
			VkAdapter.pQueueFamilyProperties = (VkQueueFamilyProperties*)malloc(
				sizeof(VkQueueFamilyProperties) * VkAdapter.mQueueFamilyPropertiesCount);
			vkGetPhysicalDeviceQueueFamilyProperties(pysicalDevices[i],
				&VkAdapter.mQueueFamilyPropertiesCount, VkAdapter.pQueueFamilyProperties);
			// 
			for(uint32_t j = 0; j < VkAdapter.mQueueFamilyPropertiesCount; j++)
			{
				const VkQueueFamilyProperties& prop =  VkAdapter.pQueueFamilyProperties[j];
				if( (VkAdapter.mQueueFamilyIndices[ECGpuQueueType_Graphics] == -1) 
					&&
					(prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) )
				{
					VkAdapter.mQueueFamilyIndices[ECGpuQueueType_Graphics] = j;
				} 
				else if( (VkAdapter.mQueueFamilyIndices[ECGpuQueueType_Compute] == -1) 
					&&
					(prop.queueFlags & VK_QUEUE_COMPUTE_BIT) )
				{
					VkAdapter.mQueueFamilyIndices[ECGpuQueueType_Compute] = j;
				} 
				else if( (VkAdapter.mQueueFamilyIndices[ECGpuQueueType_Transfer] == -1) 
					&&
					(prop.queueFlags & VK_QUEUE_TRANSFER_BIT) )
				{
					VkAdapter.mQueueFamilyIndices[ECGpuQueueType_Transfer] = j;
				}
			}
		}
		free(pysicalDevices);
	} else {
		assert(0 && "Vulkan: 0 physical device avalable!");
	}

    // open validation layer.
    if(desc->enableDebugLayer)
    {
        const VkDebugUtilsMessengerCreateInfoEXT* messengerInfoPtr = nullptr;
	    DECLARE_ZERO(VkDebugUtilsMessengerCreateInfoEXT, messengerInfo)
        if(exts_desc && exts_desc->pDebugUtilsMessenger) {
            messengerInfoPtr = exts_desc->pDebugUtilsMessenger;
        } else {
            messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			messengerInfo.pfnUserCallback = debugCallback;
			messengerInfo.messageSeverity = 
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messengerInfo.messageType = 
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			messengerInfo.flags = 0;
			messengerInfo.pUserData = NULL;
            messengerInfoPtr = &messengerInfo;
        }
		assert(vkCreateDebugUtilsMessengerEXT && "Load vkCreateDebugUtilsMessengerEXT failed!");
        VkResult res = vkCreateDebugUtilsMessengerEXT(result->pVkInstance,
            messengerInfoPtr, nullptr, &(result->pVkDebugUtilsMessenger));
        if (VK_SUCCESS != res)
        {
            assert(0 && "vkCreateDebugUtilsMessengerEXT failed - disabling Vulkan debug callbacks");
        }
    }

	return &(result->super);
}

void cgpu_free_instance_vulkan(CGpuInstanceId instance)
{
    CGpuInstance_Vulkan* to_destroy = (CGpuInstance_Vulkan*)instance;
	if(to_destroy->pVkDebugUtilsMessenger) {
		assert(vkDestroyDebugUtilsMessengerEXT && "Load vkDestroyDebugUtilsMessengerEXT failed!");
		vkDestroyDebugUtilsMessengerEXT(to_destroy->pVkInstance, to_destroy->pVkDebugUtilsMessenger, nullptr);
	}

	vkDestroyInstance(to_destroy->pVkInstance, VK_NULL_HANDLE);
	free(to_destroy->pVulkanAdapters);
	free(to_destroy);
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
	CGpuInstance_Vulkan* vkInstance = (CGpuInstance_Vulkan*)adapter->instance;
	CGpuDevice_Vulkan* vkDevice = (CGpuDevice_Vulkan*)cgpu_malloc(sizeof(CGpuDevice_Vulkan));
	CGpuAdapter_Vulkan* a = (CGpuAdapter_Vulkan*)adapter;

	*const_cast<CGpuAdapterId*>(&vkDevice->super.adapter) = adapter;
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.resize(desc->queueGroupCount);
	for(uint32_t i = 0; i < desc->queueGroupCount; i++)
	{
		VkDeviceQueueCreateInfo& info = queueCreateInfos[i];
		CGpuQueueGroupDescriptor& descriptor = desc->queueGroups[i];
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.queueCount = descriptor.queueCount;
		info.queueFamilyIndex = (uint32_t)a->mQueueFamilyIndices[descriptor.queueType];
        info.pQueuePriorities = queuePriorities;

		assert(cgpu_query_queue_count_vulkan(adapter, descriptor.queueType) >= descriptor.queueCount 
			&& "allocated too many queues!");
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
	createInfo.enabledExtensionCount = (uint32_t)deviceExtensionNames.size();

	if (vkInstance->pVkDebugUtilsMessenger) {
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &validation_layer_name;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(a->pPhysicalDevice, &createInfo, CGPU_NULLPTR, &vkDevice->pVkDevice) != VK_SUCCESS) {
		assert(0 && "failed to create logical device!");
	}

	// Single Device Only.
	volkLoadDeviceTable(&vkDevice->mVkDeviceTable, vkDevice->pVkDevice);
	
	if(desc->disable_pipeline_cache)
	{
		vkDevice->pPipelineCache = CGPU_NULLPTR;
	}
	else
	{
	    DECLARE_ZERO(VkPipelineCacheCreateInfo, info)
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		info.pNext = NULL;
		// ++TODO: Serde
		info.initialDataSize = 0;
		info.pInitialData = NULL;
		// --TODO
		info.flags = 0;
		vkDevice->mVkDeviceTable.vkCreatePipelineCache(vkDevice->pVkDevice,
			&info, GLOBAL_VkAllocationCallbacks, &vkDevice->pPipelineCache);
	}
	return &vkDevice->super;
}

void cgpu_free_device_vulkan(CGpuDeviceId device)
{
	CGpuDevice_Vulkan* D = (CGpuDevice_Vulkan*)device;
	vkDestroyDevice(D->pVkDevice, nullptr);
	cgpu_free(D);
}

#endif