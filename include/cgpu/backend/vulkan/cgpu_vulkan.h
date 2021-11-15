#pragma once
#include "cgpu/api.h"
#if defined(_WIN32) || defined(_WIN64)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "cgpu/volk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_VkAllocationCallbacks CGPU_NULLPTR

#ifndef VK_USE_VOLK_DEVICE_TABLE
#define VK_USE_VOLK_DEVICE_TABLE
#endif 

RUNTIME_API const CGpuProcTable* CGPU_VulkanProcTable();
RUNTIME_API const CGpuSurfacesProcTable* CGPU_VulkanSurfacesProcTable();

RUNTIME_API CGpuInstanceId cgpu_create_instance_vulkan(CGpuInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_vulkan(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_vulkan(CGpuInstanceId instance);

RUNTIME_API void cgpu_enum_adapters_vulkan(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
RUNTIME_API void cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail);
RUNTIME_API uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type);

RUNTIME_API CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_vulkan(CGpuDeviceId device);

RUNTIME_API CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_free_queue_vulkan(CGpuQueueId queue);

RUNTIME_API CGpuCommandEncoderId cgpu_create_command_encoder_vulkan(CGpuQueueId queue, const CGpuCommandEncoderDescriptor* desc);
RUNTIME_API void cgpu_free_command_encoder_vulkan(CGpuCommandEncoderId pool);

RUNTIME_API CGpuShaderModuleId cgpu_create_shader_module_vulkan(CGpuDeviceId device, const struct CGpuShaderModuleDescriptor* desc);

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
    VkPhysicalDeviceProperties mPhysicalDeviceProps;
    VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;
    struct VkQueueFamilyProperties* pQueueFamilyProperties;
    uint32_t mQueueFamilyPropertiesCount;
    int64_t mQueueFamilyIndices[ECGpuQueueType_Count];
} CGpuAdapter_Vulkan;

typedef struct CGpuDevice_Vulkan {
    const CGpuDevice super;
    VkDevice pVkDevice;
    struct VolkDeviceTable mVkDeviceTable;
} CGpuDevice_Vulkan;

typedef struct CGpuQueue_Vulkan {
    const CGpuQueue super;
    VkQueue pVkQueue;
    uint32_t mVkQueueFamilyIndex;
} CGpuQueue_Vulkan;

typedef struct CGpuCommandEncoder_Vulkan {
    CGpuCommandEncoder super;
    VkCommandPool pVkCmdPool;
} CGpuCommandEncoder_Vulkan;

typedef struct CGpuShaderModule_Vulkan {
    CGpuShaderModule super;
    VkShaderModule mShaderModule;
} CGpuShaderModule_Vulkan;

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

VkFormat pf_translate_to_vulkan(const ECGpuPixelFormat format);



#include "cgpu_vulkan.inl"
#ifdef __cplusplus
} // end extern "C"
#endif