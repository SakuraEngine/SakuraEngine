#pragma once
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/widget_misc.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/widget.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "8cc86bf9-a351-4093-9bd4-f54789e72b10"
)
SKR_GUI_API Widget : virtual public skr::rttr::IObject {
    SKR_GENERATE_BODY()

    // build callback
    virtual void pre_construct() SKR_NOEXCEPT {}
    virtual void post_construct() SKR_NOEXCEPT {}

    // bind element
    virtual NotNull<Element*> create_element() SKR_NOEXCEPT = 0;

    // help function
    static bool can_update(NotNull<Widget*> old_widget, NotNull<Widget*> new_widget) SKR_NOEXCEPT;

    Key key = {};
};
} // namespace skr::gui