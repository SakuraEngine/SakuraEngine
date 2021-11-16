#define DLL_IMPLEMENTATION
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/cgpu_d3d12_surfaces.h"
#ifdef CGPU_USE_D3D12

const CGpuProcTable tbl_d3d12 = 
{
    .create_instance = &cgpu_create_instance_d3d12,
    .query_instance_features = &cgpu_query_instance_features_d3d12,
    .free_instance = &cgpu_free_instance_d3d12,
	.enum_adapters = &cgpu_enum_adapters_d3d12,
	.query_adapter_detail = &cgpu_query_adapter_detail_d3d12,
    .query_queue_count = &cgpu_query_queue_count_d3d12,
    .create_device = &cgpu_create_device_d3d12,
    .free_device = &cgpu_free_device_d3d12,
    .get_queue = &cgpu_get_queue_d3d12,
    .free_queue = &cgpu_free_queue_d3d12,
    .create_command_encoder = &cgpu_create_command_encoder_d3d12,
    .free_command_encoder = &cgpu_free_command_encoder_d3d12,
    .create_swapchain = &cgpu_create_swapchain_d3d12,
    .free_swapchain = &cgpu_free_swapchain_d3d12
};

const CGpuProcTable* CGPU_D3D12ProcTable()
{
    return &tbl_d3d12;
}


const CGpuSurfacesProcTable s_tbl_d3d12 = 
{
    .free_surface = cgpu_free_surface_d3d12,
#if defined(_WIN32) || defined(_WIN64)
    .from_hwnd = cgpu_surface_from_hwnd_d3d12
#endif
};

const CGpuSurfacesProcTable* CGPU_D3D12SurfacesProcTable()
{
    return &s_tbl_d3d12;
}

#endif