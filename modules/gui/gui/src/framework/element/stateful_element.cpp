#include "SkrGui/framework/element/stateful_element.hpp" // IWYU pragma: keep
#include "SkrGui/framework/widget/stateful_widget.hpp"

namespace skr::gui
{
StatefulElement::StatefulElement(NotNull<StatefulWidget*> widget) SKR_NOEXCEPT
    : Super(widget),
      _state(widget->create_state())
{
    _state->_element = this;
    _state->_widget  = widget;
}

// rebuild & update
void StatefulElement::perform_rebuild() SKR_NOEXCEPT
{
    // TODO. did change dependencies
    Super::perform_rebuild();
}
Widget* StatefulElement::build() SKR_NOEXCEPT
{
    return _state->build(this);
}
void StatefulElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    Super::update(new_widget);
    _state->_widget = new_widget->type_cast_fast<StatefulWidget>();
    rebuild(true);
}

// lifecycle
void StatefulElement::first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    Super::first_mount(parent, slot);
    // TODO. did change dependencies
}
void StatefulElement::attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT
{
    Super::attach(owner);
    _state->on_element_attach(owner);
    if (lifecycle() == EElementLifecycle::Unmounted)
    {
        mark_needs_build();
    }
}
void StatefulElement::detach() SKR_NOEXCEPT
{
    _state->on_element_detach();
    Super::detach();
}
void StatefulElement::destroy() SKR_NOEXCEPT
{
    Super::destroy();
    _state->on_element_destroy();
    _state->_element = nullptr;

    // TODO. maybe destroy state here?
    _state = nullptr;
}
} // namespace skr::gui