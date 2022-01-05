#pragma once
#ifdef __cplusplus
    #if defined(_WINDOWS) && !defined(DURANGO)
        #include "ags/amd_ags.h"
        #define AMDAGS
    #endif
extern "C" {
#endif
#include "cgpu/api.h"

ECGpuAGSReturnCode cgpu_ags_init(struct CGpuInstance* Inst);
uint32_t cgpu_ags_get_driver_version();
void cgpu_ags_exit();

#ifdef __cplusplus
}
#endif