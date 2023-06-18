#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct Widget;

// sandbox 是外部使用 GUI 系统的入口
// 其思想是：输入事件、Backend 等信息，输出每帧的渲染三角与命令
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