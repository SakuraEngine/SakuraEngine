#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/device.hpp"

namespace skr::gui
{
struct SkrRenderDevice;
struct SKR_GUI_RENDERER_API SkrNativeDevice : public INativeDevice {
    SKR_GUI_OBJECT(SkrNativeDevice, "27cf3f49-efda-4ae8-b5e6-89fb3bb0590e", INativeDevice)

    // view
    virtual NotNull<IWindow*> create_window(const WindowDesc& desc) override;
    virtual void              destroy_window(NotNull<IWindow*> view) override;

    // view ops
    virtual void update_window(NotNull<IWindow*> view) override;
    virtual void draw_window(NotNull<IWindow*> view) override;

private:
    SkrRenderDevice* _render_device = nullptr;
    Array<IWindow*>  _all_views;
};
} // namespace skr::gui
