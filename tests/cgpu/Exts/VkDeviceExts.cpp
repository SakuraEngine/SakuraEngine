#include "cgpu/api.h"
#ifdef CGPU_USE_VULKAN
    #include "cgpu/extensions/cgpu_vulkan_exts.h"
    #include "gtest/gtest.h"
    #include <vector>

class VkDeviceExtsTest : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
};

static VKAPI_ATTR VkBool32 VKAPI_CALL VkUtil_DebugUtilsCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

TEST_F(VkDeviceExtsTest, CreateVkInstance)
{
    DECLARE_ZERO(CGpuVulkanInstanceDescriptor, vkDesc)
    const char* exts[] = {
    #ifdef _WINDOWS
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif defined(__APPLE__)
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
    #endif
        VK_KHR_SURFACE_EXTENSION_NAME
    };
    vkDesc.mInstanceExtensionCount = 2;
    vkDesc.ppInstanceExtensions = exts;
    #ifdef _WINDOWS
    const char* dexts[] = {
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME
    };
    vkDesc.ppDeviceExtensions = dexts;
    vkDesc.mDeviceExtensionCount = 1;
    #endif

    // Messenger Enable.
    DECLARE_ZERO(VkDebugUtilsMessengerCreateInfoEXT, debugCreateInfo)
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = VkUtil_DebugUtilsCallback;
    vkDesc.pDebugUtilsMessenger = &debugCreateInfo;
    vkDesc.backend = CGPU_BACKEND_VULKAN;

    CGpuInstanceDescriptor desc;
    desc.backend = CGPU_BACKEND_VULKAN;
    desc.enable_gpu_based_validation = true;
    desc.enable_debug_layer = true;
    desc.chained = (const CGpuChainedDescriptor*)&vkDesc;

    auto vulkan_instance = cgpu_create_instance(&desc);
    EXPECT_TRUE(vulkan_instance != nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VkUtil_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            cgpu_trace("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            cgpu_info("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            cgpu_warn("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            cgpu_error("Vulkan validation layer: %s\n", pCallbackData->pMessage);
            break;
        default:
            return VK_TRUE;
    }
    return VK_FALSE;
}
#endif