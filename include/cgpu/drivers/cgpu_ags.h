#pragma once
#ifdef __cplusplus
    #if defined(_WINDOWS) && !defined(DURANGO)
    #include "ags/amd_ags.h"
    #define AMDAGS
    #endif
extern "C" {
#endif
#include "cgpu/api.h"

typedef enum ECGpuAGSReturnCode
{
	CGPU_AGS_SUCCESS,                    ///< Successful function call
	CGPU_AGS_FAILURE,                    ///< Failed to complete call for some unspecified reason
	CGPU_AGS_INVALID_ARGS,               ///< Invalid arguments into the function
	CGPU_AGS_OUT_OF_MEMORY,              ///< Out of memory when allocating space internally
	CGPU_AGS_MISSING_D3D_DLL,            ///< Returned when a D3D dll fails to load
	CGPU_AGS_LEGACY_DRIVER,              ///< Returned if a feature is not present in the installed driver
	CGPU_AGS_NO_AMD_DRIVER_INSTALLED,    ///< Returned if the AMD GPU driver does not appear to be installed
	CGPU_AGS_EXTENSION_NOT_SUPPORTED,    ///< Returned if the driver does not support the requested driver extension
	CGPU_AGS_ADL_FAILURE,                ///< Failure in ADL (the AMD Display Library)
	CGPU_AGS_DX_FAILURE                  ///< Failure from DirectX runtime
} ECGpuAGSReturnCode;

ECGpuAGSReturnCode cgpu_ags_init();
void cgpu_ags_exit();


#ifdef __cplusplus
}
#endif