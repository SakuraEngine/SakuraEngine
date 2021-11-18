#pragma once
#include "cgpu/backend/vulkan/cgpu_vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CGpuVulkanInstanceDescriptor {
    CGPU_CHAINED_DESCRIPTOR_HEADER
    // Additional Instance Layers
    const char** ppInstanceLayers;
    // Count of Additional Instance Layers
    uint32_t mInstanceLayerCount;
    // Additional Instance Extensions
    const char** ppInstanceExtensions;
    // Count of Additional Instance Extensions
    uint32_t mInstanceExtensionCount;
    // Addition Physical Device Extensions
    const char** ppDeviceExtensions;
    // Count of Addition Physical Device Extensions
    uint32_t mDeviceExtensionCount;
    const VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessenger;
} CGpuVulkanInstanceDescriptor;

#ifdef __cplusplus
} // end extern "C"
#endif