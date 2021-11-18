#pragma once
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "cgpu/extensions/cgpu_vulkan_exts.h"
#include "cgpu/backend/vulkan/vk_mem_alloc.h"

#ifdef __cplusplus
extern "C" {
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL VkUtil_DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

// Environment Setup
bool VkUtil_InitializeEnvironment(struct CGpuInstance* Inst);
void VkUtil_DeInitializeEnvironment(struct CGpuInstance* Inst);

// Instance Helpers
void VkUtil_EnableValidationLayer(CGpuInstance_Vulkan* I, const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr);
void VkUtil_QueryAllAdapters(CGpuInstance_Vulkan* I, const char* const* device_extensions, uint32_t device_extension_count);

// Device Helpers
void VkUtil_CreatePipelineCache(CGpuDevice_Vulkan* D);
void VkUtil_CreateVMAAllocator(CGpuInstance_Vulkan* I, CGpuAdapter_Vulkan* A, CGpuDevice_Vulkan* D);

// API Helpers
VkBufferUsageFlags VkUtil_DescriptorTypesToBufferUsage(CGpuDescriptorTypes descriptors, bool texel);

// Record Helpers

#ifdef __cplusplus
}
#endif