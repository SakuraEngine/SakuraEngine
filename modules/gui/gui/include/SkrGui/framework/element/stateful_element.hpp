#pragma once
#include "SkrGui/framework/element/component_element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/stateful_element.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "de275a65-9b5a-4267-bc6e-355c32ca6a22"
)
SKR_GUI_API StatefulElement : public ComponentElement {
    SKR_GENERATE_BODY()
    using Super = ComponentElement;

    StatefulElement(NotNull<StatefulWidget*> widget) SKR_NOEXCEPT;

    // build & update
    void    perform_rebuild() SKR_NOEXCEPT override;
    Widget* build() SKR_NOEXCEPT override;
    void    update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    // lifecycle
    void first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT override;
    void attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT override;
    void detach() SKR_NOEXCEPT override;
    void destroy() SKR_NOEXCEPT override;

    // getter
    inline State* state() const SKR_NOEXCEPT { return _state; }

private:
    State* _state = nullptr;
};
} // namespace skr::gui