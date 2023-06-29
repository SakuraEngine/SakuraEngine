#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/device.hpp"
#include "SkrGui/backend/device/display.hpp"

namespace skr::gui
{
struct SkrRenderDevice;
struct SkrResourceDevice;
struct SKR_GUI_RENDERER_API SkrNativeDevice final : public INativeDevice {
    SKR_GUI_OBJECT(SkrNativeDevice, "27cf3f49-efda-4ae8-b5e6-89fb3bb0590e", INativeDevice)

    void init();
    void shutdown();

    // view
    NotNull<IWindow*> create_window() override;
    void              destroy_window(NotNull<IWindow*> view) override;

    // view ops
    void draw_window(NotNull<IWindow*> view) override;

    // display info
    const DisplayMetrics& display_metrics() const override;

    // sub device
    inline SkrRenderDevice*   render_device() const SKR_NOEXCEPT { return _render_device; }
    inline SkrResourceDevice* resource_device() const SKR_NOEXCEPT { return _resource_device; }

private:
    SkrRenderDevice*   _render_device = nullptr;
    SkrResourceDevice* _resource_device = nullptr;
    DisplayMetrics     _display_metrics;
    Array<IWindow*>    _all_views;
};
} // namespace skr::gui
