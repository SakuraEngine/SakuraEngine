#include "SkrGui/framework/widget/render_native_window_widget.hpp"
#include "SkrGui/framework/element/render_native_window_element.hpp"
#include "SkrGui/framework/render_object/render_native_window.hpp"

namespace skr::gui
{
NotNull<Element*> RenderNativeWindowWidget::create_element() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<RenderNativeWindowElement>(this));
}
NotNull<RenderObject*> RenderNativeWindowWidget::create_render_object() SKR_NOEXCEPT
{
    return make_not_null(native_window_render_object);
}
void RenderNativeWindowWidget::update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT
{
}
} // namespace skr::gui