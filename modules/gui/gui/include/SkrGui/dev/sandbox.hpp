#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct INativeDevice;
struct WindowDesc;

// sandbox 是外部使用 GUI 系统的入口
// 其思想是：输入事件、Backend 等信息，输出每帧的渲染三角与命令
struct SKR_GUI_API Sandbox {
    Sandbox(INativeDevice* device) SKR_NOEXCEPT;

    void init();
    void shutdown();

    void set_content(NotNull<Widget*> content);
    void show(const WindowDesc& desc);

    void update();
    void layout();
    void paint();
    void compose();

    bool hit_test(HitTestResult* result, Offsetf global_position);

    void resize_window(int32_t width, int32_t height);

private:
    // backend
    INativeDevice* _device = nullptr;

    // owner
    BuildOwner*    _build_owner    = nullptr;
    PipelineOwner* _pipeline_owner = nullptr;

    // root
    RenderNativeWindow*        _root_render_object = nullptr;
    RenderNativeWindowElement* _root_element       = nullptr;
    NativeWindowLayer*         _root_layer         = nullptr;

    // content
    Widget* _content = nullptr;
};
} // namespace skr::gui