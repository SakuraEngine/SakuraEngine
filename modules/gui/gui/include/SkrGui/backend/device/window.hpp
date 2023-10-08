#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/backend/device/window.generated.h"
#endif

namespace skr::gui
{
struct ICanvas;
struct IDevice;
struct Layer;
} // namespace skr::gui

namespace skr sreflect
{
namespace gui sreflect
{
enum class EWindowFlag : uint32_t
{
    None               = 0,
    NoMove             = 1 << 0,
    NoResize           = 1 << 1,
    NoTitle            = 1 << 2,
    NoBorder           = 1 << 3,
    NoMinimize         = 1 << 4,
    NoMaximize         = 1 << 5,
    NoClose            = 1 << 6,
    AutoSizedByContent = 1 << 7,
};

struct WindowDesc {
    EWindowFlag flags  = EWindowFlag::None;
    Offsetf     pos    = {};
    Alignment   anchor = Alignment::TopLeft();
    Sizef       size   = {};
    String      name   = {};
};

// 设备视口，某颗 UI 树（或子树）的渲染目标，可能是物理上的窗口、RenderTarget、物理窗口的某一部分、Overlay 等等
// ! 考虑一下事件路由怎么做
// FDirectPolicy: 只找根
// FToLeafmostPolicy: 只找最后的叶子
// FTunnelPolicy: 从根找到叶子
// FBubblePolicy: 从叶子找到根
//
// ! 考虑下 HitTest 怎么做
// ! 考虑下 Focus 怎么管理
sreflect_struct(
    "guid": "e1cb928d-482e-4795-8f49-d89f20fe171a"
)
SKR_GUI_API IWindow : virtual public skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()
    virtual ~IWindow() = default;

    // init view
    virtual void init_normal(const WindowDesc& desc)  = 0;
    virtual void init_popup(const WindowDesc& desc)   = 0;
    virtual void init_modal(const WindowDesc& desc)   = 0;
    virtual void init_tooltip(const WindowDesc& desc) = 0;

    // absolute/relative coordinate
    virtual Offsetf to_absolute(const Offsetf& relative_to_view) SKR_NOEXCEPT = 0;
    virtual Rectf   to_absolute(const Rectf& relative_to_view) SKR_NOEXCEPT   = 0;
    virtual Sizef   to_absolute(const Sizef& relative_to_view) SKR_NOEXCEPT   = 0;
    virtual Offsetf to_relative(const Offsetf& absolute) SKR_NOEXCEPT         = 0;
    virtual Rectf   to_relative(const Rectf& absolute) SKR_NOEXCEPT           = 0;
    virtual Sizef   to_relative(const Sizef& absolute) SKR_NOEXCEPT           = 0;

    // info
    virtual IDevice* device() SKR_NOEXCEPT             = 0;
    virtual Offsetf  absolute_pos() SKR_NOEXCEPT       = 0;
    virtual Sizef    absolute_size() SKR_NOEXCEPT      = 0;
    virtual Rectf    absolute_work_area() SKR_NOEXCEPT = 0;
    virtual float    pixel_ratio() SKR_NOEXCEPT        = 0; // frame_buffer_pixel_size / logical_pixel_size
    virtual float    text_pixel_ratio() SKR_NOEXCEPT   = 0; // text_texture_pixel_size / logical_pixel_size
    virtual bool     invisible() SKR_NOEXCEPT          = 0; // is viewport hidden or minimized or any invisible case
    virtual bool     focused() SKR_NOEXCEPT            = 0; // is viewport taken focus

    // operators
    virtual void set_absolute_pos(Offsetf absolute) SKR_NOEXCEPT = 0;
    virtual void set_absolute_size(Sizef absolute) SKR_NOEXCEPT  = 0;
    virtual void take_focus() SKR_NOEXCEPT                       = 0;
    virtual void raise() SKR_NOEXCEPT                            = 0;
    virtual void show() SKR_NOEXCEPT                             = 0;
    virtual void hide() SKR_NOEXCEPT                             = 0;

    // rendering
    virtual void update_content(WindowLayer* root_layer) SKR_NOEXCEPT = 0;

    // TODO. window call back
};

sreflect_struct(
    "guid": "e46f6067-1fbe-41bf-8361-14399bc7054b"
)
SKR_GUI_API INativeWindow : public IWindow {
    SKR_RTTR_GENERATE_BODY()

    // getter
    virtual bool   minimized() SKR_NOEXCEPT        = 0;
    virtual bool   maximized() SKR_NOEXCEPT        = 0;
    virtual bool   show_in_task_bar() SKR_NOEXCEPT = 0;
    virtual String title() SKR_NOEXCEPT            = 0;

    // setter
    virtual void set_minimized(bool minimized) SKR_NOEXCEPT               = 0;
    virtual void set_maximized(bool maximized) SKR_NOEXCEPT               = 0;
    virtual void set_show_in_task_bar(bool show_in_task_bar) SKR_NOEXCEPT = 0;
    virtual void set_title(const String& title) SKR_NOEXCEPT              = 0;
};

} // namespace gui sreflect
} // namespace skr sreflect