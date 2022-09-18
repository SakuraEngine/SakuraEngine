#pragma once
#include "cgpu/api.h"

typedef struct CGPUNSightTracker CGPUNSightTracker;
typedef struct CGPUNSightTracker* CGPUNSightTrackerId;

typedef void (*CGPUNSightProcAddGpuCrashDumpDescription)(uint32_t key, const char* value);
typedef void (*CGPUNSightTrackerCrashDumpCallback)(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData);
typedef void (*CGPUNSightTrackerShaderDebugInfoCallback)(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData);
typedef void (*CGPUNSightTrackerCrashDumpDescriptionCallback)(CGPUNSightProcAddGpuCrashDumpDescription addDescription, void* pUserData);

typedef struct CGPUNSightTrackerDescriptor {
    CGPUNSightTrackerCrashDumpCallback crash_dump_callback = nullptr;
    CGPUNSightTrackerShaderDebugInfoCallback shader_debug_info_callback = nullptr;
    CGPUNSightTrackerCrashDumpDescriptionCallback crash_dump_description_callback = nullptr;
    void* user_data = nullptr;
} CGPUNSightTrackerDescriptor;

struct CGPUNSightTracker
{
    CGPUInstanceId instance;
#ifdef __cplusplus
    virtual ~CGPUNSightTracker() = default;
#endif
};

RUNTIME_EXTERN_C RUNTIME_API
CGPUNSightTrackerId cgpu_create_nsight_tracker(CGPUInstanceId instance, const CGPUNSightTrackerDescriptor* descriptor);

RUNTIME_EXTERN_C RUNTIME_API
void cgpu_free_nsight_tracker(CGPUNSightTrackerId tracker);