#pragma once
#include "render-device.hpp"

// D3D11-CreateContext
class RenderContext
{
public:
    void Initialize(RenderDevice* device);
    void Destroy();
    FORCEINLINE RenderDevice* GetRenderDevice() { return device_; }

protected:
    // context states
    CGpuRootSignatureId current_root_sig_;
    // cmds
    CGpuCommandPoolId cmd_pool_;
    CGpuCommandBufferId cmd_buffer_;
    // render device
    RenderDevice* device_;
};
