#include "cgpu/backend/d3d12/cgpu_d3d12_surfaces.h"

void cgpu_free_surface_d3d12(CGpuDeviceId device, CGpuSurfaceId surface)
{
    return;
}

CGpuSurfaceId cgpu_surface_from_hwnd_d3d12(CGpuDeviceId device, HWND window)
{
    CGpuSurfaceId res = *(CGpuSurfaceId*)&window;
    return res;
}