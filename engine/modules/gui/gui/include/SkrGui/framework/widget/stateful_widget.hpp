#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/widget/stateful_widget.generated.h"

// state
namespace skr::gui
{
struct [[sattr(guid = "1e50e00c-6c0a-435c-817a-3970cf8b90cb"
)]] SKR_GUI_API State : virtual public skr::IRTTRBasic
{
    SKR_GENERATE_BODY(State)

    virtual NotNull<Widget*> build(NotNull<IBuildContext*> context) SKR_NOEXCEPT = 0;

    virtual void on_element_attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT;
    virtual void on_element_detach() SKR_NOEXCEPT;
    virtual void on_element_destroy() SKR_NOEXCEPT;

    void set_state(FunctionRef<void()> fn); // rebuild any way
    void set_state(FunctionRef<bool()> fn); // return true if state changed

private:
    friend struct StatefulElement;
    StatefulWidget* _widget = nullptr;
    StatefulElement* _element = nullptr;
};
} // namespace skr::gui

// stateful widget
namespace skr::gui
{
struct [[sattr(guid = "bb7b41aa-b827-4bb2-b025-e9803938ec2e"
)]] SKR_GUI_API StatefulWidget : public Widget
{
    SKR_GENERATE_BODY(StatefulWidget)

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    virtual NotNull<State*> create_state() SKR_NOEXCEPT = 0;
};
} // namespace skr::gui