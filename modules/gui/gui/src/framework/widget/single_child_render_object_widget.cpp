#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/framework/element/single_child_render_object_element.hpp"

namespace skr::gui
{
NotNull<Element*> SingleChildRenderObjectWidget::create_element() SKR_NOEXCEPT
{
    return SkrNew<SingleChildRenderObjectElement>(this);
}
} // namespace skr::gui