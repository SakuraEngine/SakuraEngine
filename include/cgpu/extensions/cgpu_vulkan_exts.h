#pragma once
#include "cgpu/backend/vulkan/cgpu_vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CGpuVulkanInstanceDescriptor {
    CGPU_CHAINED_DESCRIPTOR_HEADER

    const char** ppInstanceLayers;
    const char** ppInstanceExtensions;
    const char** ppDeviceExtensions;
    uint32_t mInstanceLayerCount;
    uint32_t mInstanceExtensionCount;
    uint32_t mDeviceExtensionCount;
    const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessenger;
} CGpuVulkanInstanceDescriptor;

#ifdef __cplusplus
} // end extern "C"
#endif