#pragma once
#ifdef __cplusplus
    #if defined(_WINDOWS) && !defined(DURANGO)
    #include "nvapi/nvapi.h"
    #define NVAPI
    #endif
extern "C" {
#endif
#include "cgpu/api.h"

typedef enum ECGpuNvAPI_Status
{
	CGPU_NVAPI_OK = 0,          //!< Success. Request is completed.
    CGPU_NVAPI_NONE = 1,
    CGPU_NVAPI_ERROR = -1,      //!< Generic error
} ECGpuNvAPI_Status;

ECGpuNvAPI_Status cgpu_nvapi_init();
void cgpu_nvapi_exit();


#ifdef __cplusplus
}
#endif