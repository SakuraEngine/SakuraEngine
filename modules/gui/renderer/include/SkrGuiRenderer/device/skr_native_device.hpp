#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/device.hpp"
#include "SkrGui/backend/device/display.hpp"

namespace skr::gui
{
struct SkrRenderDevice;
struct SkrResourceDevice;
struct SkrNativeWindow;

struct SKR_GUI_RENDERER_API SkrNativeDevice final : public INativeDevice {
    SKR_GUI_OBJECT(SkrNativeDevice, "27cf3f49-efda-4ae8-b5e6-89fb3bb0590e", INativeDevice)

    void init();
    void shutdown();

    // view
    NotNull<IWindow*> create_window() override;
    void              destroy_window(NotNull<IWindow*> view) override;

    // sub device
    inline NotNull<SkrRenderDevice*>   render_device() const SKR_NOEXCEPT { return make_not_null(_render_device); }
    inline NotNull<SkrResourceDevice*> resource_device() const SKR_NOEXCEPT { return make_not_null(_resource_device); }

    void render_all_windows() SKR_NOEXCEPT;

    // display info
    const DisplayMetrics& display_metrics() const override;

    // resource management
    NotNull<IUpdatableImage*> create_updatable_image(const UpdatableImageDesc& desc) override;
    void                      destroy_resource(NotNull<IResource*> resource) override;

    // canvas management
    NotNull<ICanvas*> create_canvas() override;
    void              destroy_canvas(NotNull<ICanvas*> canvas) override;

    // text management
    NotNull<IParagraph*> create_paragraph() override;
    void                 destroy_paragraph(NotNull<IParagraph*> paragraph) override;

private:
    // sub devices
    SkrRenderDevice*   _render_device   = nullptr;
    SkrResourceDevice* _resource_device = nullptr;

    DisplayMetrics          _display_metrics = {};
    Array<SkrNativeWindow*> _all_windows     = {};
};
} // namespace skr::gui
