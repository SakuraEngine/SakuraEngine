#pragma once
#include "cgpu/backend/vulkan/cgpu_vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNTIME_API void cgpu_free_surface_vulkan(CGPUDeviceId device, CGPUSurfaceId surface);

#if defined(_WIN32) || defined(_WIN64)
RUNTIME_API CGPUSurfaceId cgpu_surface_from_hwnd_vulkan(CGPUDeviceId device, HWND window);
#elif defined(_MACOS)
RUNTIME_API CGPUSurfaceId cgpu_surface_from_ns_view_vulkan(CGPUDeviceId device, CGPUNSView* window);
#endif

#ifdef __cplusplus
} // end extern "C"
#endif