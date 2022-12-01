#pragma once
#ifdef __cplusplus
    #if defined(_WIN32) && !defined(DURANGO)
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif
        #include "windows.h"
        #include "ags/amd_ags.h"
        #define AMDAGS
    #endif
extern "C" {
#endif
#include "cgpu/api.h"

ECGPUAGSReturnCode cgpu_ags_init(struct CGPUInstance* Inst);
uint32_t cgpu_ags_get_driver_version(CGPUInstanceId instance);
void cgpu_ags_exit(CGPUInstanceId instance);

#ifdef __cplusplus
}
#endif