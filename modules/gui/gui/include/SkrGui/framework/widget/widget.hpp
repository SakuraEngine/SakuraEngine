#pragma once
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/widget_misc.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API Widget SKR_GUI_OBJECT_BASE {
    SKR_GUI_OBJECT_ROOT(Widget, "9f69910d-ba18-4ff4-bf5f-3966507c56ba");
    SKR_GUI_RAII_MIX_IN()

    // build callback
    virtual void pre_construct() SKR_NOEXCEPT {}
    virtual void post_construct() SKR_NOEXCEPT {}

    // bind element
    virtual NotNull<Element*> create_element() SKR_NOEXCEPT = 0;

    // help function
    static bool can_update(NotNull<Widget*> old_widget, NotNull<Widget*> new_widget) SKR_NOEXCEPT;

    Key key;
};
} // namespace skr::gui