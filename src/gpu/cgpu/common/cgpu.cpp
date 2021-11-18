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

// AGS
#if defined(AMDAGS)
static AGSContext* pAgsContext = NULL;
static AGSGPUInfo gAgsGpuInfo = {};
    // Actually it's always windows.
    #if defined(_WIN64)
        #pragma comment(lib, "amd_ags_x64.lib")
    #elif defined(_WIN32)
        #pragma comment(lib, "amd_ags_x86.lib")
    #endif
#endif

ECGpuAGSReturnCode cgpu_ags_init(struct CGpuInstance* Inst)
{
#if defined(AMDAGS)
    AGSConfiguration config = {};
    int apiVersion = AGS_MAKE_VERSION(6, 0, 1);
    auto Status = agsInitialize(apiVersion, &config, &pAgsContext, &gAgsGpuInfo);
    Inst->ags_status = (ECGpuAGSReturnCode)Status;
    return Inst->ags_status;
#else
    return CGPU_AGS_NONE;
#endif
}

void cgpu_ags_exit()
{
#if defined(AMDAGS)
    agsDeInitialize(pAgsContext);
#endif
}

// NVAPI
#if defined(NVAPI)
    #if defined(_WIN64)
        #pragma comment(lib, "nvapi_x64.lib")
    #elif defined(_WIN32)
        #pragma comment(lib, "nvapi_x86.lib")
    #endif
#endif
ECGpuNvAPI_Status cgpu_nvapi_init(struct CGpuInstance* Inst)
{
#if defined(NVAPI)
    auto Status = NvAPI_Initialize();
    Inst->nvapi_status = (ECGpuNvAPI_Status)Status;
    return Inst->nvapi_status;
#else
    return ECGpuNvAPI_Status::CGPU_NVAPI_NONE;
#endif
}
void cgpu_nvapi_exit()
{
#if defined(NVAPI)
    NvAPI_Unload();
#endif
}