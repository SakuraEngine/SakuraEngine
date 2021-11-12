#pragma once
#include "cgpu/backend/vulkan/cgpu_vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif


void cgpu_free_surface_vulkan(CGpuDeviceId device, CGpuSurfaceId surface);

#if defined(_WIN32) || defined(_WIN64)
CGpuSurfaceId cgpu_surface_from_hwnd_vulkan(CGpuDeviceId device, HWND window);
#endif

#ifdef __cplusplus
} // end extern "C"
#endif