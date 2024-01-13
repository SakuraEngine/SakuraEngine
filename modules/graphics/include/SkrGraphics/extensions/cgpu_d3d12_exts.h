#pragma once
#include "SkrGraphics/api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CGPUDREDSettings {
    struct ID3D12DeviceRemovedExtendedDataSettings* pDredSettings;
} CGPUDREDSettings;
typedef struct CGPUDREDSettings* CGPUDREDSettingsId;

// Modifications to DRED settings have no effect on devices already created.
// But subsequent calls to D3D12CreateDevice use the most recent DRED settings.
CGPU_API CGPUDREDSettingsId cgpu_d3d12_enable_DRED();

// Modifications to DRED settings have no effect on devices already created.
// But subsequent calls to D3D12CreateDevice use the most recent DRED settings.
CGPU_API void cgpu_d3d12_disable_DRED(CGPUDREDSettingsId settings);

CGPU_API struct ID3D12Device* cgpu_d3d12_get_device(CGPUDeviceId device);
CGPU_API struct ID3D12GraphicsCommandList* cgpu_d3d12_get_command_list(CGPUCommandBufferId cmd);
CGPU_API struct ID3D12Resource* cgpu_d3d12_get_buffer(CGPUBufferId buffer);

#ifdef __cplusplus
} // end extern "C"
#endif