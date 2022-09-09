#include <iostream>
#include <fstream>
#include "../common/common_utils.h"
#include "platform/shared_library.h"
#include "cgpu/extensions/cgpu_nsight.h"
#include "cgpu_nsight_tracker.hpp"
#include "cgpu/drivers/nsight/GFSDK_Aftermath.h"
#include "cgpu/drivers/nsight/GFSDK_Aftermath_GpuCrashDump.h"
#include "cgpu/drivers/nsight/GFSDK_Aftermath_GpuCrashDumpDecoding.h"

uint32_t CGPUNSightSingleton::rc = 0;
CGPUNSightSingleton* CGPUNSightSingleton::_this = nullptr;

inline eastl::string AftermathErrorMessage(GFSDK_Aftermath_Result result)
{
    switch (result)
    {
    case GFSDK_Aftermath_Result_FAIL_DriverVersionNotSupported:
        return "Unsupported driver version - requires an NVIDIA R495 display driver or newer.";
    default:
        return "Aftermath Error 0x" + eastl::to_string(result - GFSDK_Aftermath_Result_Fail);
    }
}

// Helper macro for checking Nsight Aftermath results and throwing exception
// in case of a failure.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
#endif

inline static void AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_Result _result)
{
    if (!GFSDK_Aftermath_SUCCEED(_result))   
    {
        SKR_LOG_ERROR("%s\n", AftermathErrorMessage(_result).c_str());
#ifdef _WIN32
        MessageBoxA(0, AftermathErrorMessage(_result).c_str(), "Aftermath Error", MB_OK); 
#endif
        SKR_BREAK();
    }
}

GFSDK_Aftermath_PFN(GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_EnableGpuCrashDumps2)(
    GFSDK_Aftermath_Version apiVersion, uint32_t watchedApis, uint32_t flags, 
    PFN_GFSDK_Aftermath_GpuCrashDumpCb gpuCrashDumpCb, PFN_GFSDK_Aftermath_ShaderDebugInfoCb shaderDebugInfoCb, 
    PFN_GFSDK_Aftermath_GpuCrashDumpDescriptionCb, PFN_GFSDK_Aftermath_ResolveMarkerCb, void* pUserData);

void CGPUNSightSingleton::register_tracker(CGPUNSightTrackerId tracker) SKR_NOEXCEPT
{
    trackers_mutex.lock();
    all_trackers.emplace_back(tracker);
    trackers_mutex.unlock();
    CGPUNSightSingleton::rc = all_trackers.size();
}

void CGPUNSightSingleton::remove_tracker(CGPUNSightTrackerId tracker) SKR_NOEXCEPT
{
    trackers_mutex.lock();
    all_trackers.erase(eastl::remove(all_trackers.begin(), all_trackers.end(), tracker));
    trackers_mutex.unlock();
    CGPUNSightSingleton::rc = all_trackers.size();
    if (!CGPUNSightSingleton::rc)
    {
        cgpu_delete(CGPUNSightSingleton::Get());
        CGPUNSightSingleton::_this = nullptr;
    }
}

struct CGPUNSightSingletonImpl : public CGPUNSightSingleton
{
    CGPUNSightSingletonImpl() SKR_NOEXCEPT;
    ~CGPUNSightSingletonImpl() SKR_NOEXCEPT;
    
    void WriteGpuCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
    {
        // Create a GPU crash dump decoder object for the GPU crash dump.
        GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};
        AFTERMATH_CHECK_ERROR(aftermath_GpuCrashDump_CreateDecoder(
            GFSDK_Aftermath_Version_API,
            pGpuCrashDump,
            gpuCrashDumpSize,
            &decoder));

        // Use the decoder object to read basic information, like application
        // name, PID, etc. from the GPU crash dump.
        GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
        AFTERMATH_CHECK_ERROR(aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo));

        // Use the decoder object to query the application name that was set
        // in the GPU crash dump description.
        uint32_t applicationNameLength = 0;
        AFTERMATH_CHECK_ERROR(aftermath_GpuCrashDump_GetDescriptionSize(
            decoder,
            GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
            &applicationNameLength));

        eastl::vector<char> applicationName(applicationNameLength, '\0');
        if (applicationNameLength)
        {
            AFTERMATH_CHECK_ERROR(aftermath_GpuCrashDump_GetDescription(
                decoder,
                GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
                uint32_t(applicationName.size()),
                applicationName.data()));
        }

        // Create a unique file name for writing the crash dump data to a file.
        // Note: due to an Nsight Aftermath bug (will be fixed in an upcoming
        // driver release) we may see redundant crash dumps. As a workaround,
        // attach a unique count to each generated file name.
        static int count = 0;
        const std::string baseFileName =
            applicationNameLength ? std::string(applicationName.data()) : std::string("CGPUApplication")
            + "-"
            + std::to_string(baseInfo.pid)
            + "-"
            + std::to_string(++count);

        // Write the crash dump data to a file using the .nv-gpudmp extension
        // registered with Nsight Graphics.
        const std::string crashDumpFileName = baseFileName + ".nv-gpudmp";
        std::ofstream dumpFile(crashDumpFileName, std::ios::out | std::ios::binary);
        if (dumpFile)
        {
            dumpFile.write((const char*)pGpuCrashDump, gpuCrashDumpSize);
            dumpFile.close();
        }
        // Destroy the GPU crash dump decoder object.
        AFTERMATH_CHECK_ERROR(aftermath_GpuCrashDump_DestroyDecoder(decoder));
    }

    // GPU crash dump callback.
    static void GpuCrashDumpCallback(
        const void* pGpuCrashDump,
        const uint32_t gpuCrashDumpSize,
        void* pUserData)
    {
        SKR_LOG_INFO("NSIGHT GPU Crash Dump Callback");
        for (auto tracker : _this->all_trackers)
        {
            auto tracker_impl = static_cast<CGPUNSightTrackerBase*>(tracker);
            if (auto callback = tracker_impl->descriptor.crash_dump_callback)
                callback(pGpuCrashDump, gpuCrashDumpSize, tracker_impl->descriptor.user_data);
        }
        ((CGPUNSightSingletonImpl*)_this)->WriteGpuCrashDumpToFile(pGpuCrashDump, gpuCrashDumpSize);
    }

    // Shader debug information callback.
    static void ShaderDebugInfoCallback(
        const void* pShaderDebugInfo,
        const uint32_t shaderDebugInfoSize,
        void* pUserData)
    {
        SKR_LOG_INFO("NSIGHT Shader Debug Info Callback");
        for (auto tracker : _this->all_trackers)
        {
            auto tracker_impl = static_cast<CGPUNSightTrackerBase*>(tracker);
            if (auto callback = tracker_impl->descriptor.shader_debug_info_callback)
                callback(pShaderDebugInfo, shaderDebugInfoSize, tracker_impl->descriptor.user_data);
        }
    }

    // GPU crash dump description callback.
    static void CrashDumpDescriptionCallback(
        PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription,
        void* pUserData)
    {
        SKR_LOG_INFO("NSIGHT Dump Description Callback");
        for (auto tracker : _this->all_trackers)
        {
            auto tracker_impl = static_cast<CGPUNSightTrackerBase*>(tracker);
            if (auto callback = tracker_impl->descriptor.crash_dump_description_callback)
                callback(addDescription, tracker_impl->descriptor.user_data);
        }
    }

    // App-managed marker resolve callback
    // Markers are desperated now, we use vkCmdFillBuffer & ID3D12GraphicsCommandList2::WriteBufferImmediate instead
    static void ResolveMarkerCallback(
        const void* pMarker,
        void* pUserData,
        void** resolvedMarkerData,
        uint32_t* markerSize
    )
    {
        SKR_LOG_INFO("NSIGHT Resolve Marker Callback");
    }

    skr::SharedLibrary nsight_library;
    skr::SharedLibrary llvm_library;
    PFN_GFSDK_Aftermath_EnableGpuCrashDumps2 aftermath_EnableGpuCrashDumps = nullptr;
    PFN_GFSDK_Aftermath_DisableGpuCrashDumps aftermath_DisableGpuCrashDumps = nullptr;
    PFN_GFSDK_Aftermath_GpuCrashDump_CreateDecoder aftermath_GpuCrashDump_CreateDecoder = nullptr;
    PFN_GFSDK_Aftermath_GpuCrashDump_GetBaseInfo aftermath_GpuCrashDump_GetBaseInfo = nullptr;
    PFN_GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize aftermath_GpuCrashDump_GetDescriptionSize = nullptr;
    PFN_GFSDK_Aftermath_GpuCrashDump_GetDescription aftermath_GpuCrashDump_GetDescription = nullptr;
    PFN_GFSDK_Aftermath_GpuCrashDump_DestroyDecoder aftermath_GpuCrashDump_DestroyDecoder = nullptr;
};


CGPUNSightSingletonImpl::CGPUNSightSingletonImpl() SKR_NOEXCEPT
{
    bool llvm = llvm_library.load("llvm_7_0_1.dll");
    bool nsight = nsight_library.load("GFSDK_Aftermath_Lib.dll") && llvm;
    if (nsight)
    {
        SKR_LOG_INFO("NSIGHT Loaded");
        if (nsight_library.hasSymbol("GFSDK_Aftermath_EnableGpuCrashDumps"))
        {
            auto addr = nsight_library.getRawAddress("GFSDK_Aftermath_EnableGpuCrashDumps");
            aftermath_EnableGpuCrashDumps = (PFN_GFSDK_Aftermath_EnableGpuCrashDumps2)addr;
        }
        if (nsight_library.hasSymbol("GFSDK_Aftermath_DisableGpuCrashDumps"))
        {
            auto addr = nsight_library.getRawAddress("GFSDK_Aftermath_DisableGpuCrashDumps");
            aftermath_DisableGpuCrashDumps = (PFN_GFSDK_Aftermath_DisableGpuCrashDumps)addr;
        }
        if (nsight_library.hasSymbol("GFSDK_Aftermath_GpuCrashDump_CreateDecoder"))
        {
            auto addr = nsight_library.getRawAddress("GFSDK_Aftermath_GpuCrashDump_CreateDecoder");
            aftermath_GpuCrashDump_CreateDecoder = (PFN_GFSDK_Aftermath_GpuCrashDump_CreateDecoder)addr;
        }
        if (nsight_library.hasSymbol("GFSDK_Aftermath_GpuCrashDump_GetBaseInfo"))
        {
            auto addr = nsight_library.getRawAddress("GFSDK_Aftermath_GpuCrashDump_GetBaseInfo");
            aftermath_GpuCrashDump_GetBaseInfo = (PFN_GFSDK_Aftermath_GpuCrashDump_GetBaseInfo)addr;
        }
        if (nsight_library.hasSymbol("GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize"))
        {
            auto addr = nsight_library.getRawAddress("GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize");
            aftermath_GpuCrashDump_GetDescriptionSize = (PFN_GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize)addr;
        }
        if (nsight_library.hasSymbol("GFSDK_Aftermath_GpuCrashDump_GetDescription"))
        {
            auto addr = nsight_library.getRawAddress("GFSDK_Aftermath_GpuCrashDump_GetDescription");
            aftermath_GpuCrashDump_GetDescription = (PFN_GFSDK_Aftermath_GpuCrashDump_GetDescription)addr;
        }
        if (nsight_library.hasSymbol("GFSDK_Aftermath_GpuCrashDump_DestroyDecoder"))
        {
            auto addr = nsight_library.getRawAddress("GFSDK_Aftermath_GpuCrashDump_DestroyDecoder");
            aftermath_GpuCrashDump_DestroyDecoder = (PFN_GFSDK_Aftermath_GpuCrashDump_DestroyDecoder)addr;
        }
    }
    else
    {
        SKR_LOG_INFO("NSIGHT dll not found");
    }
    if (aftermath_EnableGpuCrashDumps)
    {
        AFTERMATH_CHECK_ERROR(aftermath_EnableGpuCrashDumps(
            GFSDK_Aftermath_Version_API,
            GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX | GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
            GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks, // Let the Nsight Aftermath library cache shader debug information.
            &GpuCrashDumpCallback,                                             // Register callback for GPU crash dumps.
            &ShaderDebugInfoCallback,                                          // Register callback for shader debug information.
            &CrashDumpDescriptionCallback,                                     // Register callback for GPU crash dump description.
            &ResolveMarkerCallback,                                            // Register callback for resolving application-managed markers.
            this));      
    }
}

CGPUNSightSingletonImpl::~CGPUNSightSingletonImpl() SKR_NOEXCEPT
{
    if (aftermath_DisableGpuCrashDumps)
    {
        aftermath_DisableGpuCrashDumps();
        SKR_LOG_TRACE("NSIGHT Aftermath Disabled");
    }
    if (nsight_library.isLoaded()) nsight_library.unload();
    if (llvm_library.isLoaded()) llvm_library.unload();
}

CGPUNSightSingleton* CGPUNSightSingleton::Get() SKR_NOEXCEPT
{
    if (_this == nullptr)
    {
        _this = cgpu_new<CGPUNSightSingletonImpl>();
    }
    return _this;
}

CGPUNSightTrackerBase::CGPUNSightTrackerBase(const CGPUNSightTrackerDescriptor* pdesc) SKR_NOEXCEPT
    : descriptor(*pdesc)
{
    singleton = CGPUNSightSingleton::Get();
    singleton->register_tracker(this);
}

CGPUNSightTrackerBase::~CGPUNSightTrackerBase() SKR_NOEXCEPT
{
    singleton = CGPUNSightSingleton::Get();
    singleton->remove_tracker(this);
}

CGPUNSightTrackerId cgpu_create_nsight_tracker(CGPUInstanceId instance, const CGPUNSightTrackerDescriptor* descriptor)
{
    return cgpu_new<CGPUNSightTrackerBase>(descriptor);
}

void cgpu_free_nsight_tracker(CGPUNSightTrackerId tracker)
{
    cgpu_delete(tracker);
}