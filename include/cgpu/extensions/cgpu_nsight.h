#pragma once
#include "cgpu/api.h"

typedef struct CGPUNSightTracker CGPUNSightTracker;
typedef struct CGPUNSightTracker* CGPUNSightTrackerId;

struct CGPUNSightTracker
{
    CGPUInstanceId instance;
#ifdef __cplusplus
    virtual ~CGPUNSightTracker() = default;
#endif
};

typedef struct CGPUNSightTrackerDescriptor {
    bool enable_shader_info;
} CGPUNSightTrackerDescriptor;

RUNTIME_EXTERN_C RUNTIME_API
CGPUNSightTrackerId cgpu_create_nsight_tracker(CGPUInstanceId instance, const CGPUNSightTrackerDescriptor* descriptor);

RUNTIME_EXTERN_C RUNTIME_API
void cgpu_free_nsight_tracker(CGPUNSightTrackerId tracker);