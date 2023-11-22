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

void State::set_state(FunctionRef<void()> fn)
{
    fn();
    _element->mark_needs_build();
}
void State::set_state(FunctionRef<bool()> fn)
{
    if (fn())
    {
        _element->mark_needs_build();
    }
}

NotNull<Element*> StatefulWidget::create_element() SKR_NOEXCEPT
{
    return SkrNew<StatefulElement>(this);
}

} // namespace skr::gui