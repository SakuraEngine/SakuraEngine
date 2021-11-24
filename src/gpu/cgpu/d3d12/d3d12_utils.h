#pragma once
#include "cgpu/backend/d3d12/cgpu_d3d12.h"

// Instance Helpers
void D3D12Util_QueryAllAdapters(CGpuInstance_D3D12* I, uint32_t* count, bool* foundSoftwareAdapter);
void D3D12Util_Optionalenable_debug_layer(CGpuInstance_D3D12* result, CGpuInstanceDescriptor const* descriptor);

// Device Helpers
void D3D12Util_CreateDMAAllocator(CGpuInstance_D3D12* I, CGpuAdapter_D3D12* A, CGpuDevice_D3D12* D);

// Feature Select Helpers
void D3D12Util_RecordAdapterDetail(struct CGpuAdapter_D3D12* D3D12Adapter);

// Helper Structures
