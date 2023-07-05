#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct IWindow;
struct ICanvas;
struct DisplayMetrics;

// Device/DeviceView 均通过 View/RenderView 注入到上下文中
// View 代理可以通过创建自己的 View/RenderView 来覆写 BuildOwner/PipelineOwner
// 从概念上来说，Device/DeviceView 本质上是 WidgetTree 的附属，而非宿主，仅提供必要的对 View 的操作
// DeviceView 本身不持有任何 Widget/RenderObject，与 WidgetTree 的交互行为均交给持有其实例的 Widget/RenderObject 操作
// 虽然 Device 是一个核心概念，但是资源创建并不一定适合放在 Device 中，因为资源创建本身是复杂并可拓展的

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

struct SKR_GUI_API IDevice SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IDevice, "22730f6a-c631-4982-8762-31abafc17bfe")
    virtual ~IDevice() = default;

    // window
    virtual NotNull<IWindow*> create_window()                        = 0;
    virtual void              destroy_window(NotNull<IWindow*> view) = 0;

    // TODO. resource management

    // 或许 Canvas 与 Text 的管理也完全可以放在此处

    // TODO. input
};

struct SKR_GUI_API INativeDevice : public IDevice {
    SKR_GUI_INTERFACE(INativeDevice, "209fefb2-b6dc-4035-ba71-9b5a7fc147d0", IDevice)

    // display info
    virtual const DisplayMetrics& display_metrics() const = 0;
};

} // namespace skr::gui