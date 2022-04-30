#pragma once
#include "cgpu/api.h"
#include <d3d12.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DREDSettings {
    ID3D12DeviceRemovedExtendedDataSettings* pDredSettings;
} DREDSettings;
typedef DREDSettings* DREDSettingsId;

// Modifications to DRED settings have no effect on devices already created.
// But subsequent calls to D3D12CreateDevice use the most recent DRED settings.
RUNTIME_API DREDSettingsId cgpu_d3d12_enable_DRED();

// Modifications to DRED settings have no effect on devices already created.
// But subsequent calls to D3D12CreateDevice use the most recent DRED settings.
RUNTIME_API void cgpu_d3d12_disable_DRED(DREDSettingsId settings);

#ifdef __cplusplus
} // end extern "C"
#endif