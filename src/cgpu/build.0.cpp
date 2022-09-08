#include "cgpu/cgpu_config.h"
#include "common/root_sig_table.cpp"
#include "common/root_sig_pool.cpp"
#ifdef CGPU_USE_VULKAN
    #include "vulkan/cgpu_vulkan_instance.cpp"
#endif

#ifdef CGPU_USE_D3D12
    #include "d3d12/cgpu_d3d12.cpp"
    #include "d3d12/cgpu_d3d12_rdna2.cpp"
    #include "d3d12/cgpu_d3d12_dstorage.cpp"
    #include "d3d12/d3d12_utils.cpp"
    #include "d3d12/cgpu_d3d12_surfaces.cpp"
    #include "d3d12/cgpu_d3d12_resources.cpp"
#endif

#include "common/cgpu.cpp"

#include "extensions/marker_buffer.cpp"
#ifdef CGPU_USE_VULKAN
#include "extensions/marker_buffer_vulkan.cpp"
#endif
#ifdef CGPU_USE_D3D12
#include "extensions/marker_buffer_d3d12.cpp"
#endif

#ifdef ENABLE_NSIGHT_AFTERMATH
#include "extensions/cgpu_nsight.cpp"
#ifdef CGPU_USE_VULKAN
#include "extensions/cgpu_nsight_vulkan.cpp"
#endif
#ifdef CGPU_USE_D3D12
#include "extensions/cgpu_nsight_d3d12.cpp"
#endif
#else
CGPUNSightTrackerId cgpu_create_nsight_tracker(CGPUInstanceId instance, const CGPUNSightTrackerDescriptor* descriptor)
{
    return nullptr;
}
void cgpu_free_nsight_tracker(CGPUNSightTrackerId tracker)
{
    
}
#endif