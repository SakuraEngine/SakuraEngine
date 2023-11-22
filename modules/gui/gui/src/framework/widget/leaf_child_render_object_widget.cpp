#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"
#include "SkrGui/framework/element/leaf_render_object_element.hpp"

namespace skr::gui
{
NotNull<Element*> LeafRenderObjectWidget::create_element() SKR_NOEXCEPT
{
    return SkrNew<LeafRenderObjectElement>(this);
}
} // namespace skr::gui