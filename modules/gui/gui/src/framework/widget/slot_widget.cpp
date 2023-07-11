#include "SkrGui/framework/widget/slot_widget.hpp"
#include "SkrGui/framework/element/slot_element.hpp"

namespace skr::gui
{
NotNull<Element*> SlotWidget::create_element() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<SlotElement>(make_not_null(this)));
}
} // namespace skr::gui