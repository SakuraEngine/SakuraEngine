#pragma once
#include "cgpu/api.h"
#if defined(_WIN32) || defined(_WIN64)
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "cgpu/backend/vulkan/volk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_VkAllocationCallbacks CGPU_NULLPTR

#ifndef VK_USE_VOLK_DEVICE_TABLE
#define VK_USE_VOLK_DEVICE_TABLE
#endif 

RUNTIME_API const CGpuProcTable* CGPU_VulkanProcTable();
RUNTIME_API const CGpuSurfacesProcTable* CGPU_VulkanSurfacesProcTable();

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance_vulkan(CGpuInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_vulkan(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_vulkan(CGpuInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters_vulkan(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API void cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail);
RUNTIME_API uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_vulkan(CGpuDeviceId device);

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_free_queue_vulkan(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool_vulkan(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc);
RUNTIME_API void cgpu_free_command_pool_vulkan(CGpuCommandPoolId pool);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library_vulkan(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_vulkan(CGpuShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGpuBufferId cgpu_create_buffer_vulkan(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
RUNTIME_API void cgpu_free_buffer_vulkan(CGpuBufferId buffer);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_vulkan(CGpuSwapChainId swapchain);

typedef struct CGpuInstance_Vulkan {
	CGpuInstance super;
	VkInstance pVkInstance;
	VkDebugUtilsMessengerEXT pVkDebugUtilsMessenger;
	struct CGpuAdapter_Vulkan* pVulkanAdapters;
	uint32_t mPhysicalDeviceCount;
} CGpuInstance_Vulkan;

typedef struct CGpuAdapter_Vulkan {
	CGpuAdapter super;
	VkPhysicalDevice pPhysicalDevice;
	VkPhysicalDeviceProperties2 mPhysicalDeviceProps;
	VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;
	VkPhysicalDeviceSubgroupProperties mSubgroupProperties;
	struct VkQueueFamilyProperties* pQueueFamilyProperties;
	uint32_t mQueueFamilyPropertiesCount;
	int64_t mQueueFamilyIndices[ECGpuQueueType_Count];
	// Some Extension Queries
	uint32_t dedicated_allocation : 1;
} CGpuAdapter_Vulkan;

typedef struct CGpuDevice_Vulkan {
	const CGpuDevice super;
	VkDevice pVkDevice;
	VkPipelineCache pPipelineCache;
	struct VmaAllocator_T* pVmaAllocator;
	struct VolkDeviceTable mVkDeviceTable;
} CGpuDevice_Vulkan;

typedef struct CGpuQueue_Vulkan {
	const CGpuQueue super;
	VkQueue pVkQueue;
	uint32_t mVkQueueFamilyIndex;
} CGpuQueue_Vulkan;

typedef struct CGpuCommandPool_Vulkan {
	CGpuCommandPool super;
	VkCommandPool pVkCmdPool;
} CGpuCommandPool_Vulkan;

typedef struct CGpuBuffer_Vulkan {
	CGpuBuffer super;
	VkBuffer   pVkBuffer;
	struct VmaAllocation_T* pVkAllocation;
} CGpuBuffer_Vulkan;

typedef struct CGpuShaderLibrary_Vulkan {
	CGpuShaderLibrary super;
	VkShaderModule mShaderModule;
} CGpuShaderLibrary_Vulkan;

typedef struct CGpuSwapChain_Vulkan {
	CGpuSwapChain  super;
	VkSurfaceKHR   pVkSurface;
	VkSwapchainKHR pVkSwapChain;
} CGpuSwapChain_Vulkan;

#ifdef __cplusplus
} // end extern "C"
#endif


#ifdef __cplusplus
extern "C" {
#endif

inline static VkFormat pf_translate_to_vulkan(const ECGpuPixelFormat format);

#include "cgpu_vulkan.inl"
#ifdef __cplusplus
} // end extern "C"
#endif