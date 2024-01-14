#include "SkrGui/framework/render_object/render_native_window.hpp"
#include "SkrGui/backend/device/window.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"
#include "SkrGui/framework/build_owner.hpp"

namespace skr::gui
{
RenderNativeWindow::RenderNativeWindow(INativeWindow* native_window)
    : RenderWindow(native_window)
{
}

NotNull<OffsetLayer*> RenderNativeWindow::update_layer(OffsetLayer* old_layer)
{
    return old_layer ? old_layer : SkrNew<NativeWindowLayer>(window()->type_cast<INativeWindow>());
}

void RenderNativeWindow::prepare_initial_frame() SKR_NOEXCEPT
{
    // schedule layout
    _relayout_boundary = this;
    _owner->schedule_layout_for(this);

    // schedule paint
    _layer = update_layer(nullptr);
    _layer->attach(_owner);
    _owner->schedule_paint_for(this);
}

bool RenderNativeWindow::hit_test(HitTestResult* result, Offsetf local_position)
{
    if (child() && child()->hit_test(result, local_position))
    {
        result->add(this);
        return true;
    }
    return false;
}

} // namespace skr::gui