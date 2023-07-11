#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/window.hpp"
#include "SkrRT/platform/window.h"
#include "SkrGui/backend/device/device.hpp"

namespace skr::gui
{
struct SkrRenderWindow;
struct SkrNativeDevice;

struct SKR_GUI_RENDERER_API SkrNativeWindow final : public INativeWindow {
    SKR_GUI_OBJECT(SkrNativeWindow, "093aa38f-f5f8-4aa9-92ed-5eafa6b797d5", INativeWindow)

    SkrNativeWindow(SkrNativeDevice* device);
    ~SkrNativeWindow();

    // init functions
    void init_normal(const WindowDesc& desc) override;
    void init_popup(const WindowDesc& desc) override;
    void init_modal(const WindowDesc& desc) override;
    void init_tooltip(const WindowDesc& desc) override;

    // absolute/relative coordinate
    Offsetf to_absolute(const Offsetf& relative_to_view) SKR_NOEXCEPT override;
    Rectf   to_absolute(const Rectf& relative_to_view) SKR_NOEXCEPT override;
    Sizef   to_absolute(const Sizef& relative_to_view) SKR_NOEXCEPT override;
    Offsetf to_relative(const Offsetf& absolute) SKR_NOEXCEPT override;
    Rectf   to_relative(const Rectf& absolute) SKR_NOEXCEPT override;
    Sizef   to_relative(const Sizef& absolute) SKR_NOEXCEPT override;

    // info
    IDevice* device() SKR_NOEXCEPT override;
    Offsetf  absolute_pos() SKR_NOEXCEPT override;
    Sizef    absolute_size() SKR_NOEXCEPT override;
    Rectf    absolute_work_area() SKR_NOEXCEPT override;
    float    pixel_ratio() SKR_NOEXCEPT override;      // frame_buffer_pixel_size / logical_pixel_size
    float    text_pixel_ratio() SKR_NOEXCEPT override; // text_texture_pixel_size / logical_pixel_size
    bool     invisible() SKR_NOEXCEPT override;        // is viewport hidden or minimized or any invisible case
    bool     focused() SKR_NOEXCEPT override;          // is viewport taken focus

    // operators
    void set_absolute_pos(Offsetf absolute) SKR_NOEXCEPT override;
    void set_absolute_size(Sizef absolute) SKR_NOEXCEPT override;
    void take_focus() SKR_NOEXCEPT override;
    void raise() SKR_NOEXCEPT override;
    void show() SKR_NOEXCEPT override;
    void hide() SKR_NOEXCEPT override;

    // rendering
    void update_content(WindowLayer* root_layer) SKR_NOEXCEPT override;

    // getter
    bool   minimized() SKR_NOEXCEPT override;
    bool   maximized() SKR_NOEXCEPT override;
    bool   show_in_task_bar() SKR_NOEXCEPT override;
    String title() SKR_NOEXCEPT override;

    // setter
    void set_minimized(bool minimized) SKR_NOEXCEPT override;
    void set_maximized(bool maximized) SKR_NOEXCEPT override;
    void set_show_in_task_bar(bool show_in_task_bar) SKR_NOEXCEPT override;
    void set_title(const String& title) SKR_NOEXCEPT override;

    inline SkrNativeDevice*   device() const SKR_NOEXCEPT { return _device; }
    inline SWindowHandle      window() const SKR_NOEXCEPT { return _window; }
    inline SkrRenderWindow*   render_window() const SKR_NOEXCEPT { return _render_window; }
    inline NativeWindowLayer* native_layer() const SKR_NOEXCEPT { return _native_layer; }

private:
    SkrNativeDevice*   _device        = nullptr;
    SWindowHandle      _window        = {};
    SkrRenderWindow*   _render_window = nullptr;
    NativeWindowLayer* _native_layer  = nullptr;
};
} // namespace skr::gui
