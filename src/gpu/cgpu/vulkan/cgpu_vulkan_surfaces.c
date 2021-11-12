#define DLL_IMPLEMENTATION
#include "assert.h"
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "cgpu/extensions/cgpu_vulkan_exts.h"
#include "cgpu/backend/vulkan/cgpu_vulkan_surfaces.h"
#ifdef CGPU_USE_VULKAN

const CGpuSurfacesProcTable s_tbl_vk = 
{
#if defined(_WIN32) || defined(_WIN64)
    .from_hwnd = cgpu_surface_from_hwnd_vulkan,
#endif
    .free_surface = cgpu_free_surface_vulkan
};

const CGpuSurfacesProcTable* CGPU_VulkanSurfacesProcTable()
{
	return &s_tbl_vk;
}

void cgpu_free_surface_vulkan(CGpuDeviceId device, CGpuSurfaceId surface)
{
	assert(surface && "CGPU VULKAN ERROR: NULL surface!");

	CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)device->adapter->instance;
    VkSurfaceKHR vkSurface = (VkSurfaceKHR)surface;
	vkDestroySurfaceKHR(I->pVkInstance, vkSurface, GLOBAL_VkAllocationCallbacks);
}

	#if defined(_WIN32) || defined(_WIN64)
		#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
		#endif
	#include "windows.h"

	CGpuSurfaceId cgpu_surface_from_hwnd_vulkan(CGpuDeviceId device, HWND window)
	{
		assert(window && "CGPU VULKAN ERROR: NULL HWND!");

		CGpuInstance_Vulkan* I = (CGpuInstance_Vulkan*)device->adapter->instance;
		CGpuSurfaceId surface;
		VkWin32SurfaceCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.pNext = NULL;
		create_info.flags = 0;
		create_info.hinstance = GetModuleHandle(NULL);
		create_info.hwnd = window;
		if(VK_SUCCESS != vkCreateWin32SurfaceKHR(I->pVkInstance, &create_info, 
			GLOBAL_VkAllocationCallbacks, (VkSurfaceKHR*)&surface))
		{
			assert(0 && "Create VKWin32 Surface Failed!");
			return CGPU_NULLPTR;
		}
		return surface;
	}
	#endif
#endif