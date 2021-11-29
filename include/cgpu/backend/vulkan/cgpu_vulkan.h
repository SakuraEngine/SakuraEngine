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
RUNTIME_API const CGpuAdapterDetail* cgpu_query_adapter_detail_vulkan(const CGpuAdapterId adapter);
RUNTIME_API uint32_t cgpu_query_queue_count_vulkan(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device_vulkan(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device_vulkan(CGpuDeviceId device);

// API Object APIs
RUNTIME_API CGpuFenceId cgpu_create_fence_vulkan(CGpuDeviceId device);
RUNTIME_API void cgpu_free_fence_vulkan(CGpuFenceId fence);

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue_vulkan(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_submit_queue_vulkan(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc);
RUNTIME_API void cgpu_wait_queue_idle_vulkan(CGpuQueueId queue);
RUNTIME_API void cgpu_free_queue_vulkan(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool_vulkan(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc);
RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer_vulkan(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc);
RUNTIME_API void cgpu_free_command_buffer_vulkan(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_free_command_pool_vulkan(CGpuCommandPoolId pool);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library_vulkan(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_library_vulkan(CGpuShaderLibraryId shader_module);

// Buffer APIs
RUNTIME_API CGpuBufferId cgpu_create_buffer_vulkan(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc);
RUNTIME_API void cgpu_map_buffer_vulkan(CGpuBufferId buffer, const struct CGpuBufferRange* range);
RUNTIME_API void cgpu_unmap_buffer_vulkan(CGpuBufferId buffer);
RUNTIME_API void cgpu_free_buffer_vulkan(CGpuBufferId buffer);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain_vulkan(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain_vulkan(CGpuSwapChainId swapchain);

// CMDs
RUNTIME_API void cgpu_cmd_begin_vulkan(CGpuCommandBufferId cmd);
RUNTIME_API void cgpu_cmd_update_buffer_vulkan(CGpuCommandBufferId cmd, const struct CGpuBufferUpdateDescriptor* desc);
RUNTIME_API void cgpu_cmd_end_vulkan(CGpuCommandBufferId cmd);

typedef struct CGpuInstance_Vulkan {
    CGpuInstance super;
    VkInstance pVkInstance;
    VkDebugUtilsMessengerEXT pVkDebugUtilsMessenger;
    VkDebugReportCallbackEXT pVkDebugReport;
    struct CGpuAdapter_Vulkan* pVulkanAdapters;
    uint32_t mPhysicalDeviceCount;

    // Layers of Instance
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGpuVkLayersTable* pLayersTable;

    // Extension Properties of Instance
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Extensions Table
    struct CGpuVkExtensionsTable* pExtensionsTable;

    // Some Extension Queries
    uint32_t device_group_creation : 1;
    uint32_t debug_utils : 1;
    uint32_t debug_report : 1;
} CGpuInstance_Vulkan;

typedef struct CGpuAdapter_Vulkan {
    CGpuAdapter super;
    VkPhysicalDevice pPhysicalDevice;
    /// Physical Device Props & Features
    VkPhysicalDeviceProperties2 mPhysicalDeviceProps;
    VkPhysicalDeviceFeatures2 mPhysicalDeviceFeatures;
    VkPhysicalDeviceSubgroupProperties mSubgroupProperties;
    /// Queue Families
    uint32_t mQueueFamiliesCount;
    int64_t mQueueFamilyIndices[ECGpuQueueType_Count];
    struct VkQueueFamilyProperties* pQueueFamilyProperties;

    // Layers of Physical Device
    uint32_t mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char** pLayerNames;
    // Enabled Layers Table
    struct CGpuVkLayersTable* pLayersTable;

    // Extension Properties of Physical Device
    uint32_t mExtensionsCount;
    const char** pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Device Extensions Table
    struct CGpuVkExtensionsTable* pExtensionsTable;

    // Some Extension Queries
    uint32_t debug_marker : 1;
    uint32_t dedicated_allocation : 1;
    uint32_t memory_req2 : 1;
    uint32_t external_memory : 1;
    uint32_t external_memory_win32 : 1;
    uint32_t draw_indirect_count : 1;
    uint32_t amd_draw_indirect_count : 1;
    uint32_t amd_gcn_shader : 1;
    uint32_t descriptor_indexing : 1;
    uint32_t sampler_ycbcr : 1;
    uint32_t nv_diagnostic_checkpoints : 1;
    uint32_t nv_diagnostic_config : 1;

    CGpuAdapterDetail adapter_detail;
} CGpuAdapter_Vulkan;

typedef struct CGpuDevice_Vulkan {
    const CGpuDevice super;
    VkDevice pVkDevice;
    VkPipelineCache pPipelineCache;
    struct VkUtil_DescriptorPool* pDescriptorPool;
    struct VmaAllocator_T* pVmaAllocator;
    struct VolkDeviceTable mVkDeviceTable;
} CGpuDevice_Vulkan;

typedef struct CGpuFence_Vulkan {
    CGpuFence super;
    VkFence pVkFence;
    uint32_t mSubmitted : 1;
} CGpuFence_Vulkan;

typedef struct CGpuQueue_Vulkan {
    const CGpuQueue super;
    VkQueue pVkQueue;
    float mTimestampPeriod;
    uint32_t mVkQueueFamilyIndex : 5;
} CGpuQueue_Vulkan;

typedef struct CGpuCommandPool_Vulkan {
    CGpuCommandPool super;
    VkCommandPool pVkCmdPool;
} CGpuCommandPool_Vulkan;

typedef struct CGpuCommandBuffer_Vulkan {
    CGpuCommandBuffer super;
    VkCommandBuffer pVkCmdBuf;
    VkRenderPass pVkActiveRenderPass;
    VkPipelineLayout pBoundPipelineLayout;
    uint32_t mNodeIndex : 4;
    uint32_t mType : 3;
    uint32_t mPadA;
    struct CGpuCommandPool_Vulkan* pCmdPool;
    uint64_t mPadB[9];
} CGpuCommandBuffer_Vulkan;

typedef struct CGpuBuffer_Vulkan {
    CGpuBuffer super;
    VkBuffer pVkBuffer;
    VkBufferView pVkStorageTexelView;
    VkBufferView pVkUniformTexelView;
    struct VmaAllocation_T* pVkAllocation;
    uint64_t mOffset;
} CGpuBuffer_Vulkan;

typedef struct CGpuShaderLibrary_Vulkan {
    CGpuShaderLibrary super;
    VkShaderModule mShaderModule;
    struct SpvReflectShaderModule* pReflect;
} CGpuShaderLibrary_Vulkan;

typedef struct CGpuSwapChain_Vulkan {
    CGpuSwapChain super;
    VkSurfaceKHR pVkSurface;
    VkSwapchainKHR pVkSwapChain;
} CGpuSwapChain_Vulkan;

#ifdef __cplusplus
} // end extern "C"
#endif

#ifdef __cplusplus
extern "C" {
#endif

FORCEINLINE static VkFormat VkUtil_TranslatePixelFormat(const ECGpuPixelFormat format);

#include "cgpu_vulkan.inl"
#ifdef __cplusplus
} // end extern "C"
#endif