#include "cgpu/backend/vulkan/cgpu_vulkan_surfaces.h"
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "cgpu/extensions/cgpu_vulkan_exts.h"

#if defined(_MACOS)
#define VK_MVK_macos_surface 1
#include "vulkan/vulkan_macos.h"
#endif

const CGPUSurfacesProcTable s_tbl_vk = {
    //
    .free_surface = cgpu_free_surface_vulkan,
#if defined(_WIN32) || defined(_WIN64)
    .from_hwnd = cgpu_surface_from_hwnd_vulkan,
#elif defined(_MACOS)
    .from_ns_view = cgpu_surface_from_ns_view_vulkan
#endif
    //
};

const CGPUSurfacesProcTable* CGPU_VulkanSurfacesProcTable() { return &s_tbl_vk; }

void cgpu_free_surface_vulkan(CGPUDeviceId device, CGPUSurfaceId surface)
{
    cgpu_assert(surface && "CGPU VULKAN ERROR: NULL surface!");

    CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)device->adapter->instance;
    VkSurfaceKHR vkSurface = (VkSurfaceKHR)surface;
    vkDestroySurfaceKHR(I->pVkInstance, vkSurface, GLOBAL_VkAllocationCallbacks);
}

#if defined(_WIN32) || defined(_WIN64)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include "windows.h"

CGPUSurfaceId cgpu_surface_from_hwnd_vulkan(CGPUDeviceId device, HWND window)
{
    cgpu_assert(window && "CGPU VULKAN ERROR: NULL HWND!");

    CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)device->adapter->instance;
    CGPUSurfaceId surface;
    VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .hinstance = GetModuleHandle(NULL),
        .hwnd = window
    };
    if (vkCreateWin32SurfaceKHR(I->pVkInstance, &create_info, GLOBAL_VkAllocationCallbacks,
        (VkSurfaceKHR*)&surface) != VK_SUCCESS)
    {
        cgpu_assert(0 && "Create VKWin32 Surface Failed!");
        return CGPU_NULLPTR;
    }
    return surface;
}

#elif defined(_MACOS)

CGPUSurfaceId cgpu_surface_from_ns_view_vulkan(CGPUDeviceId device, CGPUNSView* window)
{
    cgpu_assert(window && "CGPU VULKAN ERROR: NULL NSVIEW!");

    CGPUInstance_Vulkan* I = (CGPUInstance_Vulkan*)device->adapter->instance;
    CGPUSurfaceId surface;
    VkMacOSSurfaceCreateInfoMVK create_info = {
        .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
        .pNext = NULL,
        .flags = 0,
        .pView = window
    };
    if (vkCreateMacOSSurfaceMVK(I->pVkInstance, &create_info, GLOBAL_VkAllocationCallbacks,
        (VkSurfaceKHR*)&surface) != VK_SUCCESS)
    {
        cgpu_assert(0 && "Create VK NSView Surface Failed!");
        return CGPU_NULLPTR;
    }
    return surface;
}

#endif // create views