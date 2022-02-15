#pragma once
#include "../common/utils.h"
#include "cgpu/cgpux.hpp"

// D3D11-CreateDeviceAndSwapChain
class RenderDevice
{
public:
    void Initialize(ECGpuBackend backend);
    void Destroy();

    ECGpuBackend backend_;
    // window
    SDL_Window* sdl_window_;
    SDL_SysWMinfo wmInfo;
    // instance & adapter & device
    CGpuInstanceId instance_;
    CGpuAdapterId adapter_;
    CGpuDeviceId device_;
    CGpuQueueId gfx_queue_;
    // surface and swapchain
    CGpuSurfaceId surface_;
    CGpuSwapChainId swapchain_;
};
