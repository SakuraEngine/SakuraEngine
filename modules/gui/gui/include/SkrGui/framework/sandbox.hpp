#pragma once
#include "SkrGui/fwd_config.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, Widget, skr_gui_widget)

namespace skr::gui
{
struct SKR_GUI_API Sandbox {
    Sandbox(Widget* root_widget);

    void update();
    // animation
    void layout();
    // compose
    void draw();
    // finalize

private:
    Widget* _root_widget;
};
} // namespace skr::gui