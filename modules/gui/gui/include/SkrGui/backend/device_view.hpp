#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct ICanvas;
struct IDevice;
} // namespace skr::gui

namespace skr::gui
{
// 设备视口，某颗 UI 树（或子树）的渲染目标，可能是物理上的窗口、RenderTarget、物理窗口的某一部分、Overlay 等等
// ! 考虑一下事件路由怎么做
// FDirectPolicy: 只找根
// FToLeafmostPolicy: 只找最后的叶子
// FTunnelPolicy: 从根找到叶子
// FBubblePolicy: 从叶子找到根
//
// ! 考虑下 HitTest 怎么做
// ! 考虑下 Focus 怎么管理
struct SKR_GUI_API IDeviceView {
    virtual ~IDeviceView() = default;

    // getter
    virtual IDevice* device() = 0;
    virtual Offsetf  pos() = 0;              // in logical pixel
    virtual Sizef    size() = 0;             // in logical pixel
    virtual float    pixel_ratio() = 0;      // frame_buffer_pixel_size / logical_pixel_size
    virtual float    text_pixel_ratio() = 0; // text_texture_pixel_size / logical_pixel_size
    virtual bool     minimized() = 0;        // is minimized
    virtual bool     focused() = 0;          // is focused

    // setter
    virtual void set_pos(Offsetf pos) = 0;
    virtual void set_size(Sizef size) = 0;
    virtual void set_minimized(bool minimized) = 0;
    virtual void set_title(const String& str) = 0;
    virtual void take_focus() = 0;
    virtual void show() = 0;

    // rendering
    virtual ICanvas* create_canvas() = 0;
    virtual void     destroy_canvas(ICanvas* canvas) = 0;
    virtual void     render() = 0;
};

struct SKR_GUI_API INativeDeviceView : public IDeviceView {
};

} // namespace skr::gui