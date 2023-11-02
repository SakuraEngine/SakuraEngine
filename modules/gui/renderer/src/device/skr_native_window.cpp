#include "SkrGuiRenderer/device/skr_native_window.hpp"
#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/render/skr_render_window.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"

namespace skr::gui
{
SkrNativeWindow::SkrNativeWindow(SkrNativeDevice* device)
    : _device(device)
{
}
SkrNativeWindow::~SkrNativeWindow()
{
    if (_render_window)
    {
        _device->render_device()->destroy_window(_render_window);
        _render_window = nullptr;
    }
    if (_window)
    {
        skr_free_window(_window);
        _window = nullptr;
    }
}

// init functions
void SkrNativeWindow::init_normal(const WindowDesc& desc)
{
    SWindowDescroptor create_desc;
    create_desc.width  = (int32_t)desc.size.width;
    create_desc.height = (int32_t)desc.size.height;
    create_desc.posx   = (int32_t)desc.pos.x;
    create_desc.posy   = (int32_t)desc.pos.y;
    create_desc.flags  = SKR_WINDOW_RESIZABLE;
    _window            = skr_create_window(desc.name.u8_str(), &create_desc);

    _render_window = _device->render_device()->create_window(_window);
}
void SkrNativeWindow::init_popup(const WindowDesc& desc)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrNativeWindow::init_modal(const WindowDesc& desc)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrNativeWindow::init_tooltip(const WindowDesc& desc)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// absolute/relative coordinate
Offsetf SkrNativeWindow::to_absolute(const Offsetf& relative_to_view) SKR_NOEXCEPT
{
    return relative_to_view + this->absolute_pos();
}
Rectf SkrNativeWindow::to_absolute(const Rectf& relative_to_view) SKR_NOEXCEPT
{
    return relative_to_view;
}
Sizef SkrNativeWindow::to_absolute(const Sizef& relative_to_view) SKR_NOEXCEPT
{
    return relative_to_view;
}
Offsetf SkrNativeWindow::to_relative(const Offsetf& absolute) SKR_NOEXCEPT
{
    return absolute - this->absolute_pos();
}
Rectf SkrNativeWindow::to_relative(const Rectf& absolute) SKR_NOEXCEPT
{
    return absolute;
}
Sizef SkrNativeWindow::to_relative(const Sizef& absolute) SKR_NOEXCEPT
{
    return absolute;
}

// info
INativeDevice* SkrNativeWindow::device() SKR_NOEXCEPT
{
    return _device;
}
Offsetf SkrNativeWindow::absolute_pos() SKR_NOEXCEPT
{
    int32_t pos_x, pos_y;
    skr_window_get_position(_window, &pos_x, &pos_y);
    return { (float)pos_x, (float)pos_y };
}
Sizef SkrNativeWindow::absolute_size() SKR_NOEXCEPT
{
    int32_t width, height;
    skr_window_get_extent(_window, &width, &height);
    return { (float)width, (float)height };
}
Rectf SkrNativeWindow::absolute_work_area() SKR_NOEXCEPT
{
    return Rectf::OffsetSize(absolute_pos(), absolute_size());
}
float SkrNativeWindow::pixel_ratio() SKR_NOEXCEPT
{
    return 1.0f;
}
float SkrNativeWindow::text_pixel_ratio() SKR_NOEXCEPT
{
    return 1.0f;
}
bool SkrNativeWindow::invisible() SKR_NOEXCEPT
{
    return skr_window_is_minimized(_window);
}
bool SkrNativeWindow::focused() SKR_NOEXCEPT
{
    return skr_window_is_focused(_window);
}

// operators
void SkrNativeWindow::set_absolute_pos(Offsetf absolute) SKR_NOEXCEPT
{
    skr_window_set_position(_window, (int32_t)absolute.x, (int32_t)absolute.y);
}
void SkrNativeWindow::set_absolute_size(Sizef absolute) SKR_NOEXCEPT
{
    skr_window_set_extent(_window, (int32_t)absolute.width, (int32_t)absolute.height);
}
void SkrNativeWindow::update_content(WindowLayer* root_layer) SKR_NOEXCEPT
{
    _native_layer = root_layer->type_cast_fast<NativeWindowLayer>();
}
void SkrNativeWindow::take_focus() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrNativeWindow::raise() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrNativeWindow::show() SKR_NOEXCEPT
{
    skr_show_window(_window);
}
void SkrNativeWindow::hide() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// getter
bool SkrNativeWindow::minimized() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return false;
}
bool SkrNativeWindow::maximized() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return false;
}
bool SkrNativeWindow::show_in_task_bar() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return false;
}
String SkrNativeWindow::title() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return String();
}

// setter
void SkrNativeWindow::set_minimized(bool minimized) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrNativeWindow::set_maximized(bool maximized) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrNativeWindow::set_show_in_task_bar(bool show_in_task_bar) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void SkrNativeWindow::set_title(const String& title) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
} // namespace skr::gui