#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/device.hpp"
#include "SkrGui/backend/device/display.hpp"

namespace skr::gui
{
struct SkrRenderDevice;
struct SkrResourceService;
struct SkrNativeWindow;

struct SKR_GUI_RENDERER_API SkrNativeDevice final : public INativeDevice {
    SKR_GUI_OBJECT(SkrNativeDevice, "27cf3f49-efda-4ae8-b5e6-89fb3bb0590e", INativeDevice)

    void init();
    void shutdown();

    // view
    NotNull<IWindow*> create_window() override;
    void              destroy_window(NotNull<IWindow*> view) override;

    // display info
    const DisplayMetrics& display_metrics() const override;

    // sub device
    inline NotNull<SkrRenderDevice*>    render_device() const SKR_NOEXCEPT { return make_not_null(_render_device); }
    inline NotNull<SkrResourceService*> resource_device() const SKR_NOEXCEPT { return make_not_null(_resource_service); }

    void render_all_windows() SKR_NOEXCEPT;

private:
    SkrRenderDevice*        _render_device    = nullptr;
    SkrResourceService*     _resource_service = nullptr;
    DisplayMetrics          _display_metrics  = {};
    Array<SkrNativeWindow*> _all_windows      = {};
};
} // namespace skr::gui
