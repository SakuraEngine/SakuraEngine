#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/framework/element/multi_child_render_object_element.hpp"

namespace skr::gui
{
NotNull<Element*> MultiChildRenderObjectWidget::create_element() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<MultiChildRenderObjectElement>(make_not_null(this)));
}
} // namespace skr::gui