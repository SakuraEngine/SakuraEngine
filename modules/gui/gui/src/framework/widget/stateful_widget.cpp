#include "SkrGui/framework/widget/stateful_widget.hpp" // IWYU pragma: keep
#include "SkrGui/framework/element/stateful_element.hpp"

namespace skr::gui
{
void State::on_element_attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT
{
}
void State::on_element_detach() SKR_NOEXCEPT
{
}
void State::on_element_destroy() SKR_NOEXCEPT
{
}

NotNull<Element*> StatefulWidget::create_element() SKR_NOEXCEPT
{
    return SkrNew<StatefulElement>(this);
}

} // namespace skr::gui