#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "cgpu/api.h"

namespace skr::gui
{
struct SkrGUIRenderDevice;
struct SKR_GUI_RENDERER_API SkrGUIRenderDeviceView {

private:
    SkrGUIRenderDevice* _owner = nullptr;

    CGPUSurfaceId   _cgpu_surface = nullptr;
    CGPUSwapChainId _cgpu_swapchain = nullptr;
    CGPUFenceId     _cgpu_fence = nullptr;
};
} // namespace skr::gui