#pragma once
#ifdef __cplusplus
    #if defined(_WIN32) && !defined(DURANGO)
        #include "nvapi/nvapi.h"
        #define CGPU_USE_NVAPI
    #endif
#endif

#include "cgpu/api.h"

typedef struct ID3D12Device ID3D12Device;

RUNTIME_EXTERN_C ECGPUNvAPI_Status cgpu_nvapi_init(CGPUInstanceId instance);
RUNTIME_EXTERN_C uint32_t cgpu_nvapi_get_driver_version(CGPUInstanceId instance);
RUNTIME_EXTERN_C uint64_t cgpu_nvapi_d3d12_query_cpu_visible_vram(CGPUInstanceId instance, ID3D12Device* Device);
RUNTIME_EXTERN_C void cgpu_nvapi_exit(CGPUInstanceId instance);