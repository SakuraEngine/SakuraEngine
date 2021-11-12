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

const CGpuProcTable* CGPU_VulkanProcTable();
const CGpuSurfacesProcTable* CGPU_VulkanSurfacesProcTable();

CGpuInstanceId cgpu_create_instance_vulkan(CGpuInstanceDescriptor const* descriptor);
void cgpu_free_instance_vulkan(CGpuInstanceId instance);
void cgpu_enum_adapters_vulkan(CGpuInstanceId instance, CGpuAdapterId* const adapters, size_t* adapters_num);
CGpuAdapterDetail cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter);
uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type);
CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
void cgpu_free_device_vulkan(CGpuDeviceId device);
CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
void cgpu_free_queue_vulkan(CGpuQueueId queue);
CGpuCommandEncoderId cgpu_create_command_encoder_vulkan(CGpuQueueId queue, const CGpuCommandEncoderDescriptor* desc);
void cgpu_free_command_encoder_vulkan(CGpuCommandEncoderId pool);
CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
void cgpu_free_swapchain_vulkan(CGpuSwapChainId swapchain);

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