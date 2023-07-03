#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct ICanvasService;
struct ITextService;
struct INativeDevice;
struct WindowDesc;

// sandbox 是外部使用 GUI 系统的入口
// 其思想是：输入事件、Backend 等信息，输出每帧的渲染三角与命令
struct SKR_GUI_API Sandbox {
    Sandbox(INativeDevice* device, ICanvasService* canvas_service, ITextService* text_service) SKR_NOEXCEPT;

    void init();
    void shutdown();

    void set_content(NotNull<Widget*> content);
    void show(const WindowDesc& desc);

    // void update();
    // void animation();
    // void layout();
    // void paint();
    // void compose();
    // void finalize();

private:
    // backend
    INativeDevice*  _device = nullptr;
    ICanvasService* _canvas_service = nullptr;
    ITextService*   _text_service = nullptr;

    // framework
    BuildOwner*                _build_owner = nullptr;
    PipelineOwner*             _pipeline_owner = nullptr;
    RenderNativeWindow*        _root_render_object = nullptr;
    RenderNativeWindowElement* _root_element_object = nullptr;
    Widget*                    _content = nullptr;
};
} // namespace skr::gui