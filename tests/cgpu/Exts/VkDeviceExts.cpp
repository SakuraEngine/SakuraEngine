#define RUNTIME_DLL

#include "cgpu/api.h"
#ifdef CGPU_USE_VULKAN
#include "cgpu/extensions/cgpu_vulkan_exts.h"
#include "gtest/gtest.h"
#include <vector>

class VkDeviceExtsTest : public testing::Test 
{
protected:
  static void SetUpTestCase() {
    
  }
  static void TearDownTestCase() {

  }

};

static VKAPI_ATTR VkBool32 VKAPI_CALL VkUtil_DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);


TEST_F(VkDeviceExtsTest, CreateVkInstance)
{
    DECLARE_ZERO(CGpuVulkanInstanceDescriptor, vkDesc)
    const char* exts[] = 
    {
#ifdef _WINDOWS
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined (__APPLE__)
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#endif
		VK_KHR_SURFACE_EXTENSION_NAME
    };
    vkDesc.mInstanceExtensionCount = 2;
    vkDesc.ppInstanceExtensions = exts;
    // Messenger Enable.
    DECLARE_ZERO(VkDebugUtilsMessengerCreateInfoEXT, debugCreateInfo)
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = VkUtil_DebugCallback;
    vkDesc.pDebugUtilsMessenger = &debugCreateInfo;
    vkDesc.backend = ECGPUBackEnd_VULKAN;

    CGpuInstanceDescriptor desc;
    desc.backend = ECGPUBackEnd_VULKAN;
    desc.enableGpuBasedValidation = true;
    desc.enableDebugLayer = true;
    desc.chained = (const CGpuChainedDescriptor*)&vkDesc;

    auto vulkan_instance = cgpu_create_instance(&desc);
    EXPECT_TRUE( vulkan_instance != nullptr );
}


static VKAPI_ATTR VkBool32 VKAPI_CALL VkUtil_DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
	switch(messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			return VK_TRUE;//printf("[verbose]");break; 
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
#endif