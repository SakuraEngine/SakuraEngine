#include "SkrGui/framework/render_object/render_native_window.hpp"
#include "SkrGui/backend/device/window.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"

namespace skr::gui
{
RenderNativeWindow::RenderNativeWindow(INativeWindow* native_window)
    : RenderWindow(native_window)
{
}

NotNull<OffsetLayer*> RenderNativeWindow::update_layer(OffsetLayer* old_layer)
{
    return old_layer ? make_not_null(old_layer) : make_not_null(SkrNew<NativeWindowLayer>(SKR_GUI_CAST_FAST<INativeWindow>(window())));
}

void RenderNativeWindow::prepare_initial_frame() SKR_NOEXCEPT
{
    // TODO. schedule layout & paint & root layer
}
} // namespace skr::gui