#pragma once
#ifdef __cplusplus
    #if defined(_WIN32) && !defined(DURANGO)
        #include "ags/amd_ags.h"
        #define AMDAGS
    #endif
extern "C" {
#endif
#include "cgpu/api.h"

ECGPUAGSReturnCode cgpu_ags_init(struct CGPUInstance* Inst);
uint32_t cgpu_ags_get_driver_version();
void cgpu_ags_exit();

#ifdef __cplusplus
}
#endif