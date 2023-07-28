#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/framework/element/single_child_render_object_element.hpp"

namespace skr::gui
{
NotNull<Element*> SingleChildRenderObjectWidget::create_element() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<SingleChildRenderObjectElement>(make_not_null(this)));
}
} // namespace skr::gui