#pragma once
#include "render-device.hpp"

// D3D11-CreateContext
class RenderContext
{
public:
    void Initialize(RenderDevice* device);
    void Destroy();
    FORCEINLINE RenderDevice* GetRenderDevice()
    {
        return device_;
    }

protected:
    CGpuCommandPoolId cmd_pool_;
    CGpuCommandBufferId cmd_buffer_;
    RenderDevice* device_;
};
