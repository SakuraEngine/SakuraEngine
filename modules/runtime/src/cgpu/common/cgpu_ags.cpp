#include "EASTL/unordered_map.h"
#include "platform/shared_library.hpp"
#include "cgpu/api.h"
#include "cgpu/drivers/cgpu_ags.h"
#include "common_utils.h"

// AGS
#if defined(AMDAGS)
#define CGPU_AMD_AGS_SINGLETON_NAME "CGPUAMDAGSSingleton"
struct CGPUAMDAGSSingleton
{
    static CGPUAMDAGSSingleton* Get(CGPUInstanceId instance)
    {
        auto _this = (CGPUAMDAGSSingleton*)cgpu_runtime_table_try_get_custom_data(instance->runtime_table, CGPU_AMD_AGS_SINGLETON_NAME);
        if (!_this)
        {
            _this = SkrNew<CGPUAMDAGSSingleton>();
            {
                #if defined(_WIN64)
                    const char* dllname = "amd_ags_x64.dll";
                #elif defined(_WIN32)
                    const char* dllname = "amd_ags_x86.dll";
                #endif
                _this->ags_library.load(dllname);
                if (!_this->ags_library.isLoaded())
                {
                    SKR_LOG_TRACE("%s not found, amd ags is disabled", dllname);
                    _this->dll_dont_exist = true;
                }
                else
                {
                    SKR_LOG_TRACE("%s loaded", dllname);
                    // Load PFNs
                    _this->_agsInitialize = SKR_SHARED_LIB_LOAD_API(_this->ags_library, agsInitialize);
                    _this->_agsDeInitialize = SKR_SHARED_LIB_LOAD_API(_this->ags_library, agsDeInitialize);
                    // End load PFNs
                }
            }
            cgpu_runtime_table_add_custom_data(instance->runtime_table, CGPU_AMD_AGS_SINGLETON_NAME, _this);
        }
        return _this->dll_dont_exist ? nullptr : _this;
    }

    ~CGPUAMDAGSSingleton()
    {
        if (_agsDeInitialize && (agsStatus == AGS_SUCCESS)) _agsDeInitialize(pAgsContext);
        if (ags_library.isLoaded()) ags_library.unload();

        SKR_LOG_TRACE("AMD AGS unloaded");
    }

    SKR_SHARED_LIB_API_PFN(agsInitialize) _agsInitialize = nullptr;
    SKR_SHARED_LIB_API_PFN(agsDeInitialize) _agsDeInitialize = nullptr;

    AGSReturnCode agsStatus = AGS_SUCCESS;
    AGSContext* pAgsContext = NULL;
    AGSGPUInfo gAgsGpuInfo = {};
    uint32_t driverVersion = 0;
    skr::SharedLibrary ags_library;
    bool dll_dont_exist = false;
};
#endif

ECGPUAGSReturnCode cgpu_ags_init(struct CGPUInstance* Inst)
{
#if defined(AMDAGS)
    auto _this = CGPUAMDAGSSingleton::Get(Inst);
    if (!_this) return CGPU_AGS_FAILURE;
    
    AGSConfiguration config = {};
    int apiVersion = AGS_MAKE_VERSION(6, 0, 1);
    auto Status = _this->agsStatus = _this->_agsInitialize(apiVersion, &config, &_this->pAgsContext, &_this->gAgsGpuInfo);
    Inst->ags_status = (ECGPUAGSReturnCode)Status;
    if (Status == AGS_SUCCESS)
    {
        char* stopstring;
        _this->driverVersion = strtoul(_this->gAgsGpuInfo.driverVersion, &stopstring, 10);
    }
    return Inst->ags_status;
#else
    return CGPU_AGS_NONE;
#endif
}

uint32_t cgpu_ags_get_driver_version(CGPUInstanceId instance)
{
#if defined(AMDAGS)
    auto _this = CGPUAMDAGSSingleton::Get(instance);
    if (!_this) return 0;
    
    return _this->driverVersion;
#endif
    return 0;
}

void cgpu_ags_exit(CGPUInstanceId instance)
{
#if defined(AMDAGS)
    auto _this = CGPUAMDAGSSingleton::Get(instance);
    if (!_this) return;

    SkrDelete(_this);
#endif
}