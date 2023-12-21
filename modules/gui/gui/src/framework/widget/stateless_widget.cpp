#include "SkrGui/framework/widget/stateless_widget.hpp" // IWYU pragma: keep
#include "SkrGui/framework/element/stateless_element.hpp"

namespace skr::gui
{
NotNull<Element*> StatelessWidget::create_element() SKR_NOEXCEPT
{
    return SkrNew<StatelessElement>(this);
}
} // namespace skr::gui