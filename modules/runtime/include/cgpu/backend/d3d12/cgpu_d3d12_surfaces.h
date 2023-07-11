#pragma once
#include "cgpu/api.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#ifdef __cplusplus
extern "C" {
#endif

void cgpu_free_surface_d3d12(CGPUDeviceId device, CGPUSurfaceId surface);

#if defined(_WIN32) || defined(_WIN64)
CGPUSurfaceId cgpu_surface_from_hwnd_d3d12(CGPUDeviceId device, HWND window);
#endif

#ifdef __cplusplus
} // end extern "C"
#endif