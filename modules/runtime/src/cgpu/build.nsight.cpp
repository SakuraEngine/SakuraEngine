#include "cgpu/cgpu_config.h"

#ifdef ENABLE_NSIGHT_AFTERMATH
    #include "extensions/cgpu_nsight.cpp"
    #ifdef CGPU_USE_VULKAN
    #include "extensions/cgpu_nsight_vulkan.cpp"
    #endif
    #ifdef CGPU_USE_D3D12
    #include "extensions/cgpu_nsight_d3d12.cpp"
#endif
#else
    #include "cgpu/extensions/cgpu_nsight.h"
    CGPUNSightTrackerId cgpu_create_nsight_tracker(CGPUInstanceId instance, const CGPUNSightTrackerDescriptor* descriptor)
    {
        return nullptr;
    }
    void cgpu_free_nsight_tracker(CGPUNSightTrackerId tracker)
    {

    }
#endif