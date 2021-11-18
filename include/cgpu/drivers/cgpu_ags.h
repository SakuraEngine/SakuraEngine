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
void cgpu_ags_exit();

#ifdef __cplusplus
}
#endif