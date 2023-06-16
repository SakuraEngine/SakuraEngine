#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct Widget;
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