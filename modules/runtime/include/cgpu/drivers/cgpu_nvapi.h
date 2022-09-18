#pragma once
#ifdef __cplusplus
    #if defined(_WIN32) && !defined(DURANGO)
        #include "nvapi/nvapi.h"
        #define NVAPI
    #endif
extern "C" {
#endif
#include "cgpu/api.h"

struct ID3D12Device;

ECGPUNvAPI_Status cgpu_nvapi_init(CGPUInstanceId instance);
uint32_t cgpu_nvapi_get_driver_version(CGPUInstanceId instance);
uint64_t cgpu_nvapi_d3d12_query_cpu_visible_vram(CGPUInstanceId instance, struct ID3D12Device* Device);
void cgpu_nvapi_exit(CGPUInstanceId instance);

#ifdef __cplusplus
}
#endif