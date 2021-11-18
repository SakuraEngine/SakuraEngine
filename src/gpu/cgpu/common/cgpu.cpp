#include "cgpu/api.h"
#ifdef CGPU_USE_VULKAN
#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#endif
#ifdef CGPU_USE_D3D12
#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#endif
#ifdef CGPU_USE_VULKAN
#include "cgpu/backend/metal/cgpu_metal.h"
#endif
#include "cgpu/drivers/cgpu_ags.h"
#include "cgpu/drivers/cgpu_nvapi.h"

#if defined(AMDAGS)
static AGSContext* pAgsContext = NULL;
static AGSGPUInfo  gAgsGpuInfo = {};
    // Actually it's always windows.
    #if defined(_WIN64)
        #pragma comment(lib, "amd_ags_x64.lib")
    #elif defined(_WIN32) 
        #pragma comment(lib, "amd_ags_x86.lib")
    #endif
#endif

ECGpuAGSReturnCode cgpu_ags_init()
{
#if defined(AMDAGS)
    AGSConfiguration config = {};
    int apiVersion = AGS_MAKE_VERSION(6, 0, 1);
    auto Result = agsInitialize(apiVersion, &config, &pAgsContext, &gAgsGpuInfo);
	return (ECGpuAGSReturnCode)Result;
#else
    return AGS_NO_AMD_DRIVER_INSTALLED;
#endif
}

void cgpu_ags_exit()
{
#if defined(AMDAGS)
	agsDeInitialize(pAgsContext);
#endif
}
