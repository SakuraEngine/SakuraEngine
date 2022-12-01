#include "cgpu/api.h"
#ifdef CGPU_USE_D3D12
    #include <d3d12.h>
#endif
#include "platform/shared_library.hpp"
#include "cgpu/drivers/cgpu_nvapi.h"
#include "common_utils.h"

// NVAPI
#if defined(CGPU_USE_NVAPI)
    #if defined(_WIN64)
        #pragma comment(lib, "nvapi_x64.lib")
    #elif defined(_WIN32)
        #pragma comment(lib, "nvapi_x86.lib")
    #endif
#endif

ECGPUNvAPI_Status cgpu_nvapi_init(CGPUInstanceId Inst)
{
#if defined(CGPU_USE_NVAPI)
    auto Status = NvAPI_Initialize();
    ((struct CGPUInstance*)Inst)->nvapi_status = (ECGPUNvAPI_Status)Status;
    return Inst->nvapi_status;
#else
    return ECGPUNvAPI_Status::CGPU_NVAPI_NONE;
#endif
}

uint32_t cgpu_nvapi_get_driver_version(CGPUInstanceId Inst)
{
#if defined(CGPU_USE_NVAPI)
    NvU32 v = 0;         // version
    NvAPI_ShortString b; // branch
    auto Status = NvAPI_SYS_GetDriverAndBranchVersion(&v, b);
    if (Status != NVAPI_OK)
    {
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(Status, string);
        cgpu_warn("[warn] nvapi failed to get driver version! \n message: %s", string);
        return v;
    }
    return v;
#endif
    return 0;
}

uint64_t cgpu_nvapi_d3d12_query_cpu_visible_vram(CGPUInstanceId Inst, struct ID3D12Device* Device)
{
#if defined(CGPU_USE_NVAPI)
    NvU64 total, budget;
    NvAPI_D3D12_QueryCpuVisibleVidmem(Device, &total, &budget);
    return budget;
#endif
    return 0;
}

void cgpu_nvapi_exit(CGPUInstanceId Inst)
{
#if defined(CGPU_USE_NVAPI)
    NvAPI_Unload();
#endif
}