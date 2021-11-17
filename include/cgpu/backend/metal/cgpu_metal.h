#pragma once
#include "cgpu/api.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNTIME_API const CGpuProcTable* CGPU_MetalProcTable();
RUNTIME_API const CGpuSurfacesProcTable* CGPU_MetalSurfacesProcTable();

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance_metal(CGpuInstanceDescriptor const* descriptor);
RUNTIME_API void cgpu_query_instance_features_metal(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance_metal(CGpuInstanceId instance);

// Types
typedef struct CGpuInstance_Metal {
    CGpuInstance super;
} CGpuInstance_Metal;

#ifdef __cplusplus
}
#endif