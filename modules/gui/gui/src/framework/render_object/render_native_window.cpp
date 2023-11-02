#include "SkrGui/framework/render_object/render_native_window.hpp"
#include "SkrGui/backend/device/window.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"
#include "SkrGui/framework/pipeline_owner.hpp"

namespace skr::gui
{
RenderNativeWindow::RenderNativeWindow(INativeWindow* native_window)
    : RenderWindow(native_window)
{
}

NotNull<OffsetLayer*> RenderNativeWindow::update_layer(OffsetLayer* old_layer)
{
    return old_layer ? make_not_null(old_layer) : make_not_null(SkrNew<NativeWindowLayer>(window()->type_cast<INativeWindow>()));
}

void RenderNativeWindow::prepare_initial_frame() SKR_NOEXCEPT
{
    // schedule layout
    _relayout_boundary = this;
    _owner->schedule_layout_for(make_not_null(this));

    // schedule paint
    _layer = update_layer(nullptr);
    _layer->attach(make_not_null(_owner));
    _owner->schedule_paint_for(make_not_null(this));
}

bool RenderNativeWindow::hit_test(HitTestResult* result, Offsetf local_position)
{
    if (child()->hit_test(result, local_position))
    {
        result->add(this);
        return true;
    }
    return false;
}

} // namespace skr::gui