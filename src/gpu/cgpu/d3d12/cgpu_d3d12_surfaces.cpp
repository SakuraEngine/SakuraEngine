#include "cgpu/backend/d3d12/cgpu_d3d12_surfaces.h"

void cgpu_free_surface_d3d12(CGPUDeviceId device, CGPUSurfaceId surface)
{
    return;
}

CGPUSurfaceId cgpu_surface_from_hwnd_d3d12(CGPUDeviceId device, HWND window)
{
    CGPUSurfaceId res = *(CGPUSurfaceId*)&window;
    return res;
}