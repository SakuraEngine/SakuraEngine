#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#ifndef __meta__
    #include "SkrGui/backend/device/device.generated.h"
#endif

namespace skr::gui
{
struct INativeWindow;
struct ICanvas;
struct DisplayMetrics;
struct IUpdatableImage;
struct IResource;
struct UpdatableImageDesc;
struct IParagraph;

// GUI 系统的对外诉求：
// 1. 资源: Canvas & TextService & Resource
// |-1.1. Material & Image & Updatable Image
// |-1.2. TextService
// |-1.3. Canvas & Triangulator
// 2. View (Window) & VirtualDesktop & Monitor
// 3. Input (Mouse & Keyboard & TouchDevice & GamePad)

// 依赖接口
// Device/DeviceView：提供 Native/Virtual 的显示设备信息，以及视口管理，输入事件从 DeviceView 注入
// Canvas/CanvasService：提供 VG 绘制服务
// TextService：文本绘制服务
// Resource/ResourceEntry/ResourceProvider：资源管理服务

// 对于一般 APP 来说 native device 是全局唯一的， 但是对于游戏来说，3D UI、离屏的 RT UI 都可以持有一个模拟的 NativeDevice
// 所以框架并不对此做出任何限制，框架通过 Owner 追溯到顶层 NativeWindow 并通过 NativeWindow 追溯到 NativeDevice
// 以这样的路径来获取各种资源，这样的设计可以让框架更加灵活，但是也会带来一些问题，即生命周期的管理被转移到了使用方
//
// 对使用方来说，无论如何，都有唯一且确定的 NativeDevice 贯穿整个 APP 的生命周期
// 使用方需要思考这些问题，并将某些 API 转发到这个全局唯一的 NativeDevice 上，而不是另外处理
sreflect_interface(
    "guid": "8ba2ea3e-8a8e-4d88-a7d6-c98552219fc8"
)
SKR_GUI_API INativeDevice : virtual public skr::rttr::IObject {
    SKR_GENERATE_BODY()

    // window
    virtual NotNull<INativeWindow*> create_window()                              = 0;
    virtual void                    destroy_window(NotNull<INativeWindow*> view) = 0;

    // display info
    virtual const DisplayMetrics& display_metrics() const = 0;

    // resource management
    virtual NotNull<IUpdatableImage*> create_updatable_image() = 0;

    // canvas management
    virtual NotNull<ICanvas*> create_canvas()                          = 0;
    virtual void              destroy_canvas(NotNull<ICanvas*> canvas) = 0;

    // text management
    virtual NotNull<IParagraph*> create_paragraph()                                = 0;
    virtual void                 destroy_paragraph(NotNull<IParagraph*> paragraph) = 0;
};
} // namespace skr::gui