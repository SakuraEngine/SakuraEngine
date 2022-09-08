/*
* Copyright (c) 2019-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

/*
*   █████  █████ ██████ ████  ████   ███████   ████  ██████ ██   ██
*   ██  ██ ██      ██   ██    ██  ██ ██ ██ ██ ██  ██   ██   ██   ██
*   ██  ██ ██      ██   ██    ██  ██ ██ ██ ██ ██  ██   ██   ██   ██
*   ██████ ████    ██   ████  █████  ██ ██ ██ ██████   ██   ███████
*   ██  ██ ██      ██   ██    ██  ██ ██    ██ ██  ██   ██   ██   ██
*   ██  ██ ██      ██   █████ ██  ██ ██    ██ ██  ██   ██   ██   ██   DEBUGGER
*                                                           ██   ██
*  ████████████████████████████████████████████████████████ ██ █ ██ ████████████
*
*
*  HOW TO USE AFTERMATH GPU CRASH DUMP COLLECTION:
*  -----------------------------------------------
*
*  1)  Call 'GFSDK_Aftermath_EnableGpuCrashDumps', to enable GPU crash dump collection.
*      This must be done before any other library calls are made and before any D3D
*      device is created by the application.
*
*      With this call the application can register a callback function that is invoked
*      with the GPU crash dump data once a TDR/hang occurs. In addition, it is also
*      possible to provide optional callback functions for collecting shader debug
*      information and for providing additional descriptive data from the application to
*      include in the crash dump.
*
*      Enabling GPU crash dumps will also override any settings from an also active
*      Nsight Graphics GPU crash dump monitor for the calling process.
*
*
*  2)  On DX11/DX12, call 'GFSDK_Aftermath_DXxx_Initialize', to initialize the library and
*      to enable additional Aftermath features that will affect the data captured in the
*      GPU crash dumps, such as Aftermath event markers; automatic call stack markers for
*      all draw calls, compute dispatches, ray dispatches, or copy operations; resource
*      tracking; or shader debug information. See GFSDK_Aftermath.h for more details.
*
*      On Vulkan use the VK_NV_device_diagnostics_config extension to enable additional
*      Aftermath features, such as automatic call stack markers for all draw calls, compute
*      dispatches, ray dispatches, or copy operations; resource tracking; or shader debug
*      information. See GFSDK_Aftermath.h for more details.
*
*
*  4)  Before the application shuts down, call 'GFSDK_Aftermath_DisableGpuCrashDumps' to
*      disable GPU crash dump collection.
*
*      Disabling GPU crash dumps will also re-establish any settings from an also active
*      Nsight Graphics GPU crash dump monitor for the calling process.
*
*
*  5)  If the application detects a potential GPU crash, e.g. device removed/lost,
*      call 'GFSDK_Aftermath_GetCrashDumpStatus' to check the GPU crash dump status.
*      The application should then wait until Aftermath has finished processing the
*      crash dump before releasing the device or exiting.
*      Here is the recommended pattern:
*           a) Call 'GFSDK_Aftermath_GetCrashDumpStatus' to check the GPU crash dump status.
*           b) If the status is “Unknown”, this means the driver does not support the crash dump
*              status query feature. But it's still possible to receive the "Finished" or "Failed"
*              status. The application should continue to poll the status as step d describes.
*           c) If the status is not “Unknown” and not "NotStarted", this means Aftermath has detected
*              the GPU crash.
*           d) The application should wait for a few seconds to allow the Aftermath driver thread
*              to collect the GPU crash dump data. Start polling the status until the crash dump data
*              has been collected and the notification callback has been processed by the application
*              or a timeout of a couple of seconds has expired.
*               while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
*                      status != GFSDK_Aftermath_CrashDump_Status_Finished &&
*                      !timeout) {
*                   // Wait for a couple of milliseconds, and poll the crash dump status again.
*                   Sleep(50);
*                   GFSDK_Aftermath_GetCrashDumpStatus(&status);
*               }
*           e) Continue handling the device lost event, e.g. release the device or terminate
*              the application, if the timeout expires or status turns to “Finished” or “Failed”.
*
*
*  OPTIONAL:
*
*  o)  (Optional) Instrument the application with event markers as described in
*      GFSDK_Aftermath.h.
*  o)  (Optional, D3D12) Register D3D12 resource pointers with Aftermath as described in
*      GFSDK_Aftermath.h.
*  o)  (Optional) To disable all GPU crash dump functionality at runtime:
*          On Windows, set the registry key: 'HKEY_CURRENT_USER\Software\NVIDIA Corporation\Nsight Aftermath\ForceOff'.
*          On Linux, set environment 'NV_AFTERMATH_FORCE_OFF'.
*
*
*  PERFORMANCE TIPS:
*
*  o) Enabling shader debug information creation will introduce shader compile time
*     overhead as well as memory overhead for handling the debug information.
*
*  o) User event markers cause considerable overhead and should be used very carefully.
*     Using automatic callstack markers for draw calls, compute dispatches, ray dispatches,
*     and copy operations may be a less costly alternative to injecting an event marker for
*     every draw call. However, they also do not come for free and using them should be
*     also considered carefully.
*
*/

#ifndef GFSDK_Aftermath_GpuCrashDump_H
#define GFSDK_Aftermath_GpuCrashDump_H

#include "GFSDK_Aftermath_Defines.h"

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif

// Flags to configure for which graphisc APIs to enable GPU crash dumps
GFSDK_AFTERMATH_DECLARE_ENUM(GpuCrashDumpWatchedApiFlags)
{
    // Default setting
    GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_None = 0x0,

    // Enable GPU crash dump tracking for the DX API
    GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX = 0x1,

    // Enable GPU crash dump tracking for the Vulkan API
    GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan = 0x2,
};

// Flags to configure GPU crash dump-specific Aftermath features
GFSDK_AFTERMATH_DECLARE_ENUM(GpuCrashDumpFeatureFlags)
{
    // Default settings
    GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default = 0x0,

    // Defer shader debug information callbacks until an actual GPU crash
    // dump is generated and also provide shader debug information
    // for the shaders related to the crash dump only.
    // Note: using this option will increase the memory footprint of the
    // application.
    GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks = 0x1,
};

// Key definitions for user-defined GPU crash dump description
GFSDK_AFTERMATH_DECLARE_ENUM(GpuCrashDumpDescriptionKey)
{
    // Predefined key for application name
    GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName = 0x00000001,

    // Predefined key for application version
    GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion = 0x00000002,

    // Base key for creating user-defined key-value pairs.
    // Any value >= GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined
    // will create a user-defined key-value pair.
    GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined = 0x00010000,
};

GFSDK_AFTERMATH_DECLARE_ENUM(CrashDump_Status)
{
    // No GPU crash has been detected by Aftermath, so far.
    GFSDK_Aftermath_CrashDump_Status_NotStarted = 0,

    // A GPU crash happened, Aftermath started to collect crash dump data.
    GFSDK_Aftermath_CrashDump_Status_CollectingData,

    // Aftermath failed to collect crash dump data. No further callback will be invoked.
    GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed,

    // Aftermath is invoking the gpuCrashDumpCb callback after collecting the crash dump data successfully.
    GFSDK_Aftermath_CrashDump_Status_InvokingCallback,

    // gpuCrashDumpCb callback returned and Aftermath finished processing the GPU crash.
    GFSDK_Aftermath_CrashDump_Status_Finished,

    // Unknown problem - likely using an older driver
    //  incompatible with this Aftermath feature.
    GFSDK_Aftermath_CrashDump_Status_Unknown,
};

// Function for adding user-defined description key-value pairs.
// Key must be one of the predefined keys of GFSDK_Aftermath_GpuCrashDumpDescriptionKey
// or a user-defined key based on GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined.
// All keys greater than the last predefined key in GFSDK_Aftermath_GpuCrashDumpDescriptionKey
// and smaller than GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined are
// considered illegal and ignored.
typedef void (GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription)(uint32_t key, const char* value);

// GPU crash dump callback definitions.
// NOTE: Except for the pUserData pointer, all pointer values passed to the
// callbacks are only valid for the duration of the call! An implementation
// must make copies of the data, if it intends to store it beyond that.
typedef void (GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_GpuCrashDumpCb)(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData);
typedef void (GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_ShaderDebugInfoCb)(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData);
typedef void (GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_GpuCrashDumpDescriptionCb)(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addValue, void* pUserData);
typedef void (GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_ResolveMarkerCb)(const void* pMarker, void* pUserData, void** resolvedMarkerData, uint32_t* markerSize);

/////////////////////////////////////////////////////////////////////////
// GFSDK_Aftermath_EnableGpuCrashDumps
// ---------------------------------
//
// apiVersion;
//      Must be set to GFSDK_Aftermath_Version_API. Used for checking against library
//      version.
//
// watchedApis;
//      Controls which graphics APIs to watch for crashes. A combination (with OR) of
//      GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags.
//
// flags;
//      Controls GPU crash dump specific behavior. A combination (with OR) of
//      GFSDK_Aftermath_GpuCrashDumpFeatureFlags.
//
// gpuCrashDumpCb;
//      Callback function to be called when new GPU crash dump data is available.
//      Callback is free-threaded, ensure the provided function is thread-safe.
//
// shaderDebugInfoCb;
//      Callback function to be called when new shader debug information data is
//      available.
//      Callback is free-threaded, ensure the provided function is thread-safe.
//      Optional, can be NULL.
//      Note: if not using GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks
//      shaderDebugInfoCb will be invoked for every shader compilation trigegred by the
//      application, even if there will be never an invocation of gpuCrashDumpCb.
//
// descriptionCb;
//      Callback function that allows the application to provide additional
//      descriptive values to be include in crash dumps. This will be called
//      before gpuCrashDumpCb.
//      Callback is free-threaded, ensure the provided function is thread-safe.
//      Optional, can be NULL.
//
// resolveMarkerCb;
//      Callback function to be called when the crash dump contains an event marker with
//      a size of zero. This means that GFSDK_Aftermath_SetEventMarker() was called with
//      markerSize=0, meaning that the marker payload itself is managed by the application
//      rather than copied by Aftermath internally. All Vulkan markers set using the 
//      NV_device_diagnostic_checkpoints extension are application-managed as well.
//      This callback allows the application to pass the marker's associated data back 
//      to the crash dump process to be included in the dump file.
//      The application should set the value of *resolvedMarkerData to the pointer of
//      the marker's data, and set the value of *markerSize to the size of the marker's 
//      data in bytes.
//      Note: ensure that the marker data memory passed back via resolvedMarkerData will 
//      remain valid for the entirety of the crash dump generation process, i.e. until
//      gpuCrashDumpCb is called.
//
// pUserData;
//      User data made available in the callbacks.
//
//// DESCRIPTION;
//      Device independent initialization call to enable Aftermath GPU crash dump
//      creation. Overrides any settings from an also active GPU crash dump monitor
//      for this process! This function must be called before any D3D device is
//      created by the application.
//
/////////////////////////////////////////////////////////////////////////
GFSDK_Aftermath_API GFSDK_Aftermath_EnableGpuCrashDumps(
    GFSDK_Aftermath_Version apiVersion,
    uint32_t watchedApis,
    uint32_t flags,
    PFN_GFSDK_Aftermath_GpuCrashDumpCb gpuCrashDumpCb,
    PFN_GFSDK_Aftermath_ShaderDebugInfoCb shaderDebugInfoCb,
    PFN_GFSDK_Aftermath_GpuCrashDumpDescriptionCb descriptionCb,
    PFN_GFSDK_Aftermath_ResolveMarkerCb resolveMarkerCb,
    void* pUserData);

/////////////////////////////////////////////////////////////////////////
// GFSDK_Aftermath_DisableGpuCrashDumps
// ---------------------------------
//
//// DESCRIPTION;
//      Device independent call to disable Aftermath GPU crash dump creation.
//      Re-enables settings from an also active GPU crash dump monitor for
//      the current process!
//
/////////////////////////////////////////////////////////////////////////
GFSDK_Aftermath_API GFSDK_Aftermath_DisableGpuCrashDumps();

/////////////////////////////////////////////////////////////////////////
// GFSDK_Aftermath_GetCrashDumpStatus
// ---------------------------------
//
// pOutStatus;
//      OUTPUT: Crash dump status.
//
//// DESCRIPTION;
//      Query the status of GPU crash detection and crash dump data collection.
//      See GFSDK_Aftermath_CrashDump_Status for details.
//
/////////////////////////////////////////////////////////////////////////
GFSDK_Aftermath_API GFSDK_Aftermath_GetCrashDumpStatus(GFSDK_Aftermath_CrashDump_Status* pOutStatus);

/////////////////////////////////////////////////////////////////////////
//
// NOTE: Function table provided - if dynamic loading is preferred.
//
/////////////////////////////////////////////////////////////////////////
GFSDK_Aftermath_PFN(GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_EnableGpuCrashDumps)(GFSDK_Aftermath_Version apiVersion, uint32_t watchedApis, uint32_t flags, PFN_GFSDK_Aftermath_GpuCrashDumpCb gpuCrashDumpCb, PFN_GFSDK_Aftermath_ShaderDebugInfoCb shaderDebugInfoCb, PFN_GFSDK_Aftermath_GpuCrashDumpDescriptionCb descriptionCb, void* pUserData);
GFSDK_Aftermath_PFN(GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_DisableGpuCrashDumps)();
GFSDK_Aftermath_PFN(GFSDK_AFTERMATH_CALL *PFN_GFSDK_Aftermath_GetCrashDumpStatus)(GFSDK_Aftermath_CrashDump_Status* pOutStatus);

#ifdef __cplusplus
} // extern "C"
#endif

#pragma pack(pop)

#endif // GFSDK_Aftermath_GpuCrashDump_H
