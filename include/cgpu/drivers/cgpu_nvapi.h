#pragma once
#ifdef __cplusplus
    #if defined(_WINDOWS) && !defined(DURANGO)
    #include "nvapi/nvapi.h"
    #define NVAPI
    #endif
extern "C" {
#endif
#include "cgpu/api.h"

ECGpuNvAPI_Status cgpu_nvapi_init(struct CGpuInstance* Inst);
void cgpu_nvapi_exit();


#ifdef __cplusplus
}
#endif