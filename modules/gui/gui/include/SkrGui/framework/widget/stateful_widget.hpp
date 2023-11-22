#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/stateful_widget.generated.h"
#endif

// state
namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct("guid": "1e50e00c-6c0a-435c-817a-3970cf8b90cb")
SKR_GUI_API State : public ::skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()

    virtual NotNull<Widget*> build(NotNull<IBuildContext*> context) SKR_NOEXCEPT = 0;

    virtual void on_element_attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT;
    virtual void on_element_detach() SKR_NOEXCEPT;
    virtual void on_element_destroy() SKR_NOEXCEPT;

    void set_state(FunctionRef<void()> fn); // rebuild any way
    void set_state(FunctionRef<bool()> fn); // return true if state changed

private:
    friend struct StatefulElement;
    StatefulWidget*  _widget  = nullptr;
    StatefulElement* _element = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect

// stateful widget
namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "bb7b41aa-b827-4bb2-b025-e9803938ec2e"
)
SKR_GUI_API StatefulWidget : public Widget {
    SKR_RTTR_GENERATE_BODY()

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    virtual NotNull<State*> create_state() SKR_NOEXCEPT = 0;
};
} // namespace gui sreflect
} // namespace skr sreflect