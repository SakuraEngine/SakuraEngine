#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "cgpu/api.h"
#include "platform/window.h"

namespace skr::gui
{
struct SkrRenderDevice;
struct SKR_GUI_RENDERER_API SkrRenderWindow final {
    SkrRenderWindow(SkrRenderDevice* owner, SWindowHandle window);
    ~SkrRenderWindow();

    void sync_window_size();

private:
    SkrRenderDevice* _owner = nullptr;
    SWindowHandle    _window = nullptr;

    CGPUSurfaceId   _cgpu_surface = nullptr;
    CGPUSwapChainId _cgpu_swapchain = nullptr;
    CGPUFenceId     _cgpu_fence = nullptr;
};
} // namespace skr::gui