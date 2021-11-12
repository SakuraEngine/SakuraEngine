#pragma once
#include "cgpu/api.h"
#include <d3d12.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CGpuDREDSettings {
    ID3D12DeviceRemovedExtendedDataSettings* pDredSettings;
} CGpuDREDSettings;
typedef CGpuDREDSettings* CGpuDREDSettingsId;

// Modifications to DRED settings have no effect on devices already created. 
// But subsequent calls to D3D12CreateDevice use the most recent DRED settings.
RUNTIME_API CGpuDREDSettingsId cgpu_d3d12_enable_DRED();

// Modifications to DRED settings have no effect on devices already created. 
// But subsequent calls to D3D12CreateDevice use the most recent DRED settings.
RUNTIME_API void cgpu_d3d12_disable_DRED(CGpuDREDSettingsId settings);

#ifdef __cplusplus
} // end extern "C"
#endif